/*
 * Copyright (C) 2018  Jaroslaw Bielski (bielski.j@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define FIRMWARE_VERSION_MAJOR 0
#define FIRMWARE_VERSION_MINOR 1

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "utils.h"
#include "usbdrv/usbdrv.c"
#include "common/protocol.h"

/*
 * Timer configuration
 * - Timer0 - transmitter/receiver prescaler
 * - Timer1 - coil timebase generator (125kHz)
 */

#define PIO_COIL_BANK B
#define PIO_COIL_PIN  0

#define PIO_GEN_BANK  B
#define PIO_GEN_PIN   1

#define PIO_DEMOD_BANK B
#define PIO_DEMOD_PIN  5

#define PIO_CLK_BANK   B
#define PIO_CLK_PIN    2

#define PULSE_VECTOR_SIZE      7
#define SAMPLE_BUFFER_SIZE   192

#define PRESCALER_MINIMAL_VALUE 4

#define ADC_REF_MV ((uint16_t)4750)
#define ADC_HI_MV  ((uint16_t) 100)
#define ADC_LO_MV  ((uint16_t) 350)
#define ADC_MAX    ((uint16_t) 255)

#define VOLTAGE_HIGH_MV (uint32_t)((ADC_REF_MV) - ADC_HI_MV)
#define VOLTAGE_LOW_MV  (uint32_t)((ADC_REF_MV) - ADC_LO_MV)

#define ADC_HI (uint8_t)((VOLTAGE_HIGH_MV * ADC_MAX) / ADC_REF_MV)
#define ADC_LO (uint8_t)((VOLTAGE_LOW_MV  * ADC_MAX) / ADC_REF_MV)

#define TIMER_PRESCALER_T0 (_BV(CS02) | _BV(CS01))

typedef enum _State {
	STATE_IDLE,
	STATE_STARTING,
	STATE_WAIT_CONDITION,
	STATE_RX,
	STATE_TX,
	STATE_FINISHED
} State;

typedef struct _CommonContext {
	State    state;
	uint16_t timeout;
	uint8_t  timedOut             : 1;
	uint16_t fallingEdge          : 1;
	uint8_t  prescalerCompOnStart : 1;
	uint8_t  edgeStart            : 1;
	uint8_t  tx                   : 1;
	uint8_t  prescalerValue;
	uint8_t  id;
} CommonContext;


// Pending command
static volatile uint8_t command = PROTO_CMD_NOP;

// USB response buffer
static volatile uint8_t response[7];

// Vector of pulse width
static volatile uint8_t sampleVector[PULSE_VECTOR_SIZE];

// IO Data buffer
static volatile uint8_t ioBuffer[SAMPLE_BUFFER_SIZE];
static volatile uint8_t ioBufferOffset = 0;

// Operation (RX/TX) samples length
static volatile uint16_t opLength;
// Current offset in sample buffer regarding RX/TX operation
static volatile uint16_t opOffset;

// Common context
static volatile CommonContext _commonCtx = {
	.state                = STATE_IDLE,
	.timeout              = 0,
	.timedOut             = 0,
	.fallingEdge          = 0,
	.prescalerCompOnStart = 0,
	.edgeStart            = 0,
	.tx                   = 0,
	.prescalerValue       = PRESCALER_MINIMAL_VALUE,
	.id                   = 0
};

// ADC context
static volatile int8_t   adcLastSignal;
static volatile uint8_t  adcLowVal = ADC_LO;
static volatile uint8_t  adcHiVal  = ADC_HI;


static void _prescallerStart(uint8_t interrupt, uint8_t prescalerValue, uint8_t compareMatchOnStart);
static void _prescallerStop();
static void _adcStop();

static void _stopTx() {
	_prescallerStop();

	PIO_SET_INPUT(PIO_COIL_BANK, PIO_COIL_PIN);

	_commonCtx.state = STATE_FINISHED;
}

// This interrupt is used by transmitter
ISR(TIMER0_COMPA_vect) {
	if (opOffset == opLength) {
		_stopTx();
		return;
	}

	{
		uint8_t pulseIdx = ioBuffer[opOffset >> 1];

		if (opOffset & 1) {
			pulseIdx >>= 4;

		} else {
			pulseIdx &= 0x0f;
		}

		if ((pulseIdx & 0x07) == 0x07) {
			_stopTx();

		} else {
			OCR0A = sampleVector[pulseIdx & 0x07] - 1;

			if (pulseIdx & 0x08) {
				PIO_SET_INPUT(PIO_COIL_BANK, PIO_COIL_PIN);

			} else {
				PIO_SET_OUTPUT(PIO_COIL_BANK, PIO_COIL_PIN);
			}

			opOffset++;
		}
	}
}

// This interrupt is used by sampler
ISR(ADC_vect) {
	uint8_t oldSignal = adcLastSignal;

	// Clear interrupt flag on prescaler timer
	TIFR |= _BV(OCF0A);

//	PIO_SET_HIGH(PIO_DEBUG_BANK, PIO_DEBUG_PIN);

	if (ADCH >= adcHiVal) {
		adcLastSignal = 0;

	} else if (ADCH <= adcLowVal) {
		adcLastSignal = 1;
	}

	do {
		// return if last sample was unknown
		if (oldSignal == -1) {
			break;
		}

		// Check synchronization edge of signal
		if (_commonCtx.state == STATE_WAIT_CONDITION) {
			if (_commonCtx.fallingEdge) {
				if (oldSignal && ! adcLastSignal) {
					_commonCtx.state = _commonCtx.tx ? STATE_TX : STATE_RX;
				}

			} else {
				if (! oldSignal && adcLastSignal) {
					_commonCtx.state = _commonCtx.tx ? STATE_TX : STATE_RX;
				}
			}

			if (_commonCtx.state == STATE_WAIT_CONDITION) {
				if (_commonCtx.timeout) {
					_commonCtx.timeout--;

				} else {
					_commonCtx.state    = STATE_FINISHED;
					_commonCtx.timedOut = 1;

					_prescallerStop();
				}

			} else {
				// set desired value of the prescaler and tx/rx mode
				_prescallerStop();

				if (_commonCtx.state == STATE_TX) {
					_adcStop();
					_prescallerStart(1, _commonCtx.prescalerValue, _commonCtx.prescalerCompOnStart);

				} else {
					_prescallerStart(0, _commonCtx.prescalerValue, _commonCtx.prescalerCompOnStart);
				}
			}

			break;
		}

		if (adcLastSignal) {
			ioBuffer[opOffset / 8] |= (1 << (opOffset % 8));

		} else {
			ioBuffer[opOffset / 8] &= ~(1 << (opOffset % 8));
		}

		if (++opOffset == opLength) {
			_prescallerStop();

			_commonCtx.state = STATE_FINISHED;
		}
	} while (0);

//	PIO_SET_LOW(PIO_DEBUG_BANK, PIO_DEBUG_PIN);
}


static void _adcInit() {
	// Initialize ADC
	ADMUX  |= _BV(ADLAR); // Left adjusted, VCC reference voltage, ADC0 input

	ADCSRA |= (_BV(ADPS2) | _BV(ADEN)); // Prescaller 16, enable ADC

	// Trigger from timer0 compare match A
	ADCSRB |= (_BV(ADTS0) | _BV(ADTS1));

	// Start first conversion
	ADCSRA |= _BV(ADSC);

	{
		uint8_t adcVal;

		while(! (ADCSRA & _BV(ADIF)));
		adcVal = ADCH;
		ADCSRA |= _BV(ADIF);
	}
}


static void _adcStart() {
	ADCSRA |= (_BV(ADATE) | _BV(ADIE) | _BV(ADSC));
}


static void _adcStop() {
	ADCSRA &= ~(_BV(ADATE) | _BV(ADIE) | _BV(ADSC));
}


static void _prescallerInit() {
	// Initialize prescaler timer
	TCCR0A |= _BV(WGM01);
	TIFR   |= _BV(OCF0A);
}


static void _prescallerStart(uint8_t interrupt, uint8_t prescalerValue, uint8_t compareMatchOnStart) {
	GTCCR |= (_BV(TSM) | _BV(PSR0));

	OCR0A = prescalerValue - 1;

	if (compareMatchOnStart) {
		if (prescalerValue > 2) {
			TCNT0 = prescalerValue - 2;

		} else {
			TCNT0 = 0;
		}

	} else {
		TCNT0 = 0;
	}

	if (interrupt) {
		TIFR  |= _BV(OCF0A);
		TIMSK |= _BV(OCIE0A);
	}

	TCCR0B |= TIMER_PRESCALER_T0;

	GTCCR &= ~_BV(TSM);
}


static void _prescallerStop() {
	TCCR0B &= ~TIMER_PRESCALER_T0;
	TIMSK  &= ~_BV(OCIE0A);
}


static void _coilInit() {
	PIO_SET_OUTPUT(PIO_GEN_BANK, PIO_GEN_PIN);

	OCR1C  = 65;
	TCCR1 |= (_BV(CTC1) | _BV(CS10));
}


static void _coilStart() {
	TCCR1 |= _BV(COM1A0);
}


static void _coilStop() {
	TCCR1 &= ~_BV(COM1A0);
}


static uchar usbFunctionRead(uchar *data, uchar len) {
	uchar ret = 0;

	volatile uint16_t srcSize = 0;
	volatile uint8_t *src     = NULL;

	switch (command) {
		case PROTO_CMD_SAMPLE_VECTOR_READ:
			src     = sampleVector;
			srcSize = PULSE_VECTOR_SIZE;
			break;

		case PROTO_CMD_PULSE_VECTOR_READ:
			src     = ioBuffer;
			srcSize = SAMPLE_BUFFER_SIZE;
			break;
	}

	if (src != NULL) {
		while (ret < len) {
			if (ioBufferOffset == srcSize) {
				break;
			}

			data[ret++] = src[ioBufferOffset++];
		}
	}

	return ret;
}


static uchar usbFunctionWrite(uchar *data, uchar len) {
	uchar ret = 0;

	volatile uint16_t dstSize = 0;
	volatile uint8_t *dst     = NULL;

	switch (command) {
		case PROTO_CMD_SAMPLE_VECTOR_WRITE:
			dst     = sampleVector;
			dstSize = PULSE_VECTOR_SIZE;
			break;

		case PROTO_CMD_PULSE_VECTOR_WRITE:
			dst     = ioBuffer;
			dstSize = SAMPLE_BUFFER_SIZE;
			break;
	}

	while (ret < len) {
		if (ioBufferOffset == dstSize) {
			break;
		}

		dst[ioBufferOffset++] = data[ret++];
	}

	return ret;
}


static usbMsgLen_t usbFunctionSetup(uint8_t data[8]) {
	usbMsgLen_t ret = 0;

	usbRequest_t *rq = (void *) data;

	switch (rq->bRequest) {
		case PROTO_CMD_COIL_ENABLE:
			if (rq->wValue.word) {
				_coilStart();

			} else {
				_coilStop();
			}
			break;

		case PROTO_CMD_GET_VERSION:
			{
				response[ret++] = PROTO_RC_OK;
				response[ret++] = FIRMWARE_VERSION_MAJOR;
				response[ret++] = FIRMWARE_VERSION_MINOR;
			}
			break;

		case PROTO_CMD_GET_BUFFER_SIZE:
			{
				response[ret++] = PROTO_RC_OK;
				response[ret++] = 4;
				response[ret++] = 1;
				response[ret++] = SAMPLE_BUFFER_SIZE >> 8;
				response[ret++] = SAMPLE_BUFFER_SIZE & 0xff;
				response[ret++] = PULSE_VECTOR_SIZE >> 8;
				response[ret++] = PULSE_VECTOR_SIZE & 0xff;
			}
			break;

		case PROTO_CMD_RESET:
			{
				command = rq->bRequest;

				response[ret++] = PROTO_RC_OK;
			}
			break;

		case PROTO_CMD_TRANSFER_START:
			{
				_commonCtx.prescalerValue       = (rq->wIndex.word & 0xff);
				_commonCtx.prescalerCompOnStart = (rq->wIndex.word & PROTO_TRANSFER_FLAG_FIRST_ON_START) != 0;
				_commonCtx.fallingEdge          = (rq->wIndex.word & PROTO_TRANSFER_FLAG_FALLING_EDGE)   != 0;
				_commonCtx.tx                   = (rq->wIndex.word & PROTO_TRANSFER_FLAG_TX_MODE)        != 0;
				_commonCtx.edgeStart            = (rq->wIndex.word & PROTO_TRANSFER_FLAG_START_ON_EDGE)  != 0;

				_commonCtx.state    = STATE_STARTING;
				_commonCtx.timeout  = rq->wValue.word;
				_commonCtx.timedOut = 0;

				response[ret++] = PROTO_RC_OK;
				response[ret++] = ++_commonCtx.id;
			}
			break;

		case PROTO_CMD_TRANSFER_STATUS:
			{
				if (rq->wIndex.word != _commonCtx.id) {
					response[ret++] = PROTO_RC_INVALID_VAL;
					response[ret++] = PROTO_TRANSFER_STATUS_UNKNOWN;
					response[ret++] = 0;
					response[ret++] = 0;

				} else {
					response[ret++] = PROTO_RC_OK;

					if (_commonCtx.state == STATE_IDLE) {
						if (_commonCtx.timedOut) {
							response[ret++] = PROTO_TRANSFER_STATUS_TIMEOUT;

						} else {
							response[ret++] = PROTO_TRANSFER_STATUS_OK;
						}

					} else {
						response[ret++] = PROTO_TRANSFER_STATUS_IN_PROGRESS;
					}

					response[ret++] = opOffset >> 8;
					response[ret++] = opOffset & 0xff;
				}
			}
			break;

		case PROTO_CMD_PULSE_VECTOR_READ:
		case PROTO_CMD_PULSE_VECTOR_WRITE:
		case PROTO_CMD_SAMPLE_VECTOR_READ:
		case PROTO_CMD_SAMPLE_VECTOR_WRITE:
			{
				ioBufferOffset = 0;
				command        = rq->bRequest;
				ret            = 0xff;
			}
			break;

		default:
			response[ret++] = PROTO_RC_INVALID_CMD;
			break;
	}

	if (ret) {
		usbMsgPtr = (usbMsgPtr_t) response;
	}

	return ret;
}


static void _init(void) {
	// TCCR1 in synchronous mode
	PLLCSR &= ~_BV(PCKE);

	usbDeviceDisconnect();  // do this while interrupts are disabled
	_delay_ms(300);
	usbDeviceConnect();

	usbInit();    // Initialize INT settings after reconnect

	PIO_SET_INPUT(PIO_COIL_BANK, PIO_COIL_PIN);
	PIO_SET_LOW(PIO_COIL_BANK, PIO_COIL_PIN);

	_coilInit();
	_prescallerInit();
	_adcInit();
}

static void _calibrateOscillator(void) {
	uchar step = 128;
	uchar trialValue = 0;
	uchar optimumValue;

	int x;
	int optimumDev;
	int targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);

	/* do a binary search: */
	do{
		OSCCAL = trialValue + step;
		x = usbMeasureFrameLength();    // proportional to current real frequency
		if (x < targetValue)            // frequency still too low
			trialValue += step;
		step >>= 1;
	} while (step > 0);

	/* We have a precision of +/- 1 for optimum OSCCAL here */
	/* now do a neighborhood search for optimum value */
	optimumValue = trialValue;
	optimumDev   = x; // this is certainly far away from optimum

	for (OSCCAL = trialValue - 1; OSCCAL <= trialValue + 1; OSCCAL++) {
		x = usbMeasureFrameLength() - targetValue;
		if (x < 0) {
			x = -x;
		}

		if (x < optimumDev){
			optimumDev   = x;
			optimumValue = OSCCAL;
		}
	}

	OSCCAL = optimumValue;
}

void USB_INTR_VECTOR(void);

__attribute__((OS_main)) main(void) {
	cli();

	_init();

	_coilStart();
	do {
		// 15 clockcycles per loop.
		// adjust fastctr for 5ms timeout

		uint16_t fastctr  = (uint16_t)(F_CPU / (1000.0f * 15.0f / 5.0f));
		uint8_t  resetctr = 100;

		do {
			if ((USBIN & USBMASK) != 0) {
				resetctr = 100;
			}

			if (! --resetctr) { // reset encountered
				usbNewDeviceAddr = 0;   // bits from the reset handling of usbpoll()
				usbDeviceAddr    = 0;

				_calibrateOscillator();
			}

			if (USB_INTR_PENDING & (1<<USB_INTR_PENDING_BIT)) {
				USB_INTR_VECTOR();

				USB_INTR_PENDING = 1<<USB_INTR_PENDING_BIT;  // Clear int pending, in case timeout occured during SYNC
				break;
			}
		} while (--fastctr);

		wdt_reset();

		if (! fastctr) {
			// commands are only evaluated after next USB transmission or after 5 ms passed
			switch (command) {
				case PROTO_CMD_RESET:
					{
						CLKPR = 0x80;
						CLKPR = 0;
						void (*ptrToFunction)(); // allocate a function pointer
						ptrToFunction = 0x0000; // set function pointer to bootloader reset vector
						(*ptrToFunction)(); // jump to reset, which bounces in to bootloader
					}
					break;

				case PROTO_CMD_NOP:
					break;

				default:
					break;
			}

			// Handle transfer state change
			switch (_commonCtx.state) {
				case STATE_STARTING:
					{
						// Transfer buffer
						opOffset = 0;
						opLength = (uint16_t) SAMPLE_BUFFER_SIZE;

						if (_commonCtx.tx) {
							opLength *= 2;

						} else {
							opLength *= 8;
						}

						// ADC synchronization flag
						adcLastSignal = -1;

						// Change state
						if (_commonCtx.edgeStart) {
							_commonCtx.state = STATE_WAIT_CONDITION;

						} else {
							if (_commonCtx.tx) {
								_commonCtx.state = STATE_TX;

							} else {
								_commonCtx.state = STATE_RX;
							}
						}

						sei();

						_adcStart();
						if (_commonCtx.state == STATE_WAIT_CONDITION) {
							_prescallerStart(0, PRESCALER_MINIMAL_VALUE, 0);
						} else {
							_prescallerStart(_commonCtx.tx, _commonCtx.prescalerValue, 1);
						}
					}
					break;

				case STATE_FINISHED:
					{
						cli();

						_prescallerStop();
						_adcStop();

						_commonCtx.state = STATE_IDLE;
					}
					break;

				default:
					break;
			}
		}

		{
			// This is usbpoll() minus reset logic and double buffering
			int8_t  len = usbRxLen - 3;

			if (len >= 0){
				usbProcessRx(usbRxBuf + 1, len); // only single buffer due to in-order processing
				usbRxLen = 0;                    // mark rx buffer as available
			}

			if(usbTxLen & 0x10) {             // transmit system idle
				if(usbMsgLen != USB_NO_MSG) { // transmit data pending?
					usbBuildTxBlock();
				}
			}
		}

		// Usbpoll() collided with data packet
		if (USB_INTR_PENDING & (1<<USB_INTR_PENDING_BIT)) {
			uint8_t ctr;

			// loop takes 5 cycles
			__asm__ volatile(
			"         ldi  %0,%1 \n\t"
			"loop%=:  sbis %2,%3  \n\t"
			"         ldi  %0,%1  \n\t"
			"         subi %0,1   \n\t"
			"         brne loop%= \n\t"
			: "=&d" (ctr)
			:  "M" ((uint8_t)(8.8f * (F_CPU / 1.0e6f) / 5.0f + 0.5)), "I" (_SFR_IO_ADDR(USBIN)), "M" (USB_CFG_DMINUS_BIT)
			);

			USB_INTR_PENDING = 1<<USB_INTR_PENDING_BIT;
		}
	} while(1);
	_coilStop();

	USB_INTR_ENABLE = 0;
	USB_INTR_CFG    = 0; // also reset config bits
}
