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

#include <cstring>
#include <set>

#include "common/protocol.h"
#include "common/Log.hpp"
#include "InterfaceUsbImpl.hpp"


#define DEV_USB_VID  0x16c0
#define DEV_USB_PID  0x05dc
#define DEV_USB_PROD "rfid-tool"

#define DEFAULT_TIMEOUT 2000

using namespace common;


class UsbFirmwareVersion : public rfid::device::Interface::FirmwareVersion {
	public:
		UsbFirmwareVersion(uint8_t major, uint8_t minor) : FirmwareVersion(major, minor) {
		}

		bool isSupported() {
			return this->getMajor() == 0 &&
			(
				this->getMinor() == 1 ||
				this->getMinor() == 2
			);
		}
};


#define VERSION(__this)((UsbFirmwareVersion *)__this->version.get())


static int usbGetStringAscii(usb_dev_handle *dev, int index, char *buf, int buflen) {
	int ret = 0;

	do {
		char buffer[256] = { 0 };
		int rval = 0;
		int i;

		ret = usb_get_string_simple(dev, index, buf, buflen);
		if (ret >= 0) {
			Log::debug("usbGetStringAscii(): Read by usb_get_string_simple");
			break;
		}

		ret = usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + index, 0x0409, buffer, sizeof(buffer), 5000);
		if (ret < 0) {
			Log::debug("usbGetStringAscii(): usb_control_msg() returned error! %d", ret);
			break;
		}

		if (buffer[1] != USB_DT_STRING) {
			Log::debug("usbGetStringAscii(): no string data! %02x", buffer[1]);

			*buf = 0;

			ret = 0;
			break;
		}

		if ((uint8_t) buffer[0] < ret) {
			ret = (uint8_t) buffer[0];
		}

		ret /= 2;

		for (i = 1; i < rval; i++){
			if (i > buflen) {
				break;
			}

			buf[i - 1] = buffer[2 * i];

			if (buffer[2 * i + 1] != 0) {
				buf[i - 1] = '?';
			}
		}

		buf[i - 1] = 0;
	} while (0);

	return ret;
}


rfid::device::InterfaceUsbImpl::_init rfid::device::InterfaceUsbImpl::_initializer;


void rfid::device::InterfaceUsbImpl::checkConnection() {
	if (this->handle == nullptr) {
		throw NotConnectedException();
	}

	if (this->version.get()) {
		if (! VERSION(this)->isSupported()) {
			throw NotSupportedInterfaceException();
		}
	}
}


void rfid::device::InterfaceUsbImpl::checkResponse(const uint8_t *buffer, int bufferSize, int returned, bool checkRc) {
	if (bufferSize != returned) {
		Log::error("Returned %d, but expected was %d (%s)", returned, bufferSize, usb_strerror());

		throw rfid::device::Interface::InvalidResponseException(returned);
	}

	if (bufferSize > 0) {
		if (checkRc && (buffer[0] != PROTO_RC_OK)) {
			Log::error("Return code is different from OK! (%02x)", buffer[0]);

			throw rfid::device::Interface::InvalidResponseException(buffer[0]);
		}
	}
}


void rfid::device::InterfaceUsbImpl::doTransferRx(uint8_t *buffer, uint16_t bufferSize, uint16_t index, uint16_t value, uint8_t command, bool checkRc) {
	std::shared_ptr<uint8_t> transferBuffer;
	uint16_t                 transferSize = bufferSize;

	this->checkConnection();

	if (checkRc) {
		transferSize++;
	}

	if (transferSize) {
		transferBuffer.reset(new uint8_t[transferSize]);
	}

	this->checkResponse(transferBuffer.get(), transferSize,
		usb_control_msg(
			this->handle,
			USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
			command,
			value,
			index,
			(char *) transferBuffer.get(),
			transferSize,
			DEFAULT_TIMEOUT
		),
		checkRc
	);

	if (bufferSize) {
		const uint8_t *src = transferBuffer.get();

		if (checkRc) {
			src++;
		}

		memcpy(buffer, src, bufferSize);
	}
}


void rfid::device::InterfaceUsbImpl::doTransferTx(uint8_t *buffer, uint16_t bufferSize, uint16_t index, uint16_t value, uint8_t command) {
	this->checkConnection();

	this->checkResponse(buffer, bufferSize,
		usb_control_msg(
			this->handle,
			USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
			command,
			value,
			index,
			(char *) buffer,
			bufferSize,
			DEFAULT_TIMEOUT
		),
		false
	);
}


rfid::device::InterfaceUsbImpl::InterfaceUsbImpl() {
	this->handle  = nullptr;

	this->sampleBits       = 0;
	this->sampleVectorSize = 0;
	this->samplesMax       = 0;
	this->pulseBits        = 0;
	this->pulseVectorSize  = 0;
	this->pulsesMax        = 0;

	this->version.reset();

	usb_find_busses();
	usb_find_devices();

	{
		struct usb_device *dev;

		struct usb_bus *bus = usb_get_busses();

		while (bus != NULL) {
			usb_dev_handle *tmpHandle = NULL;

			dev = bus->devices;

			while (dev != NULL) {
				if (
					(dev->descriptor.idVendor  == DEV_USB_VID) &&
					(dev->descriptor.idProduct == DEV_USB_PID)
				) {
					tmpHandle = usb_open(dev);
					if (tmpHandle == nullptr) {
						continue;
					}

					if (dev->descriptor.iProduct > 0) {
						char iProductBuffer[255];

						if (usbGetStringAscii(tmpHandle, dev->descriptor.iProduct, iProductBuffer, sizeof(iProductBuffer)) >= 0) {
							if (strcmp(iProductBuffer, DEV_USB_PROD) == 0) {
								this->handle = tmpHandle;
								break;
							}
						}
					}
				}

				dev = dev->next;
			}

			if (this->handle != nullptr) {
				break;
			}

			bus = bus->next;
		}
	}

	if (this->handle) {
		uint8_t response[6];

		this->doTransferRx(response, 2, 0, 0, PROTO_CMD_GET_VERSION, true);
		Log::debug("version: %u %u", response[0], response[1]);
		this->version.reset(new UsbFirmwareVersion(response[0], response[1]));

		this->doTransferRx(response, 6, 0, 0, PROTO_CMD_GET_BUFFER_SIZE, true);

		this->pulseBits        = response[0];
		this->sampleBits       = response[1];
		this->pulseVectorSize  = (response[4] << 8) | response[5];
		this->sampleVectorSize = (response[2] << 8) | response[3];

		this->pulsesMax  = (uint32_t) (this->pulseVectorSize  * 8) / this->pulseBits;
		this->samplesMax = (uint32_t) (this->sampleVectorSize * 8) / this->sampleBits;

		Log::debug("Pulse - sample: %u, vector: %u, Sample - sample: %u, vector: %u",
			this->pulseBits, this->pulseVectorSize, this->sampleBits, this->sampleVectorSize
		);

		Log::log("Total pulses: %u, Total samples: %u", this->pulsesMax, this->samplesMax);
	}

	Log::debug("USB device handle: %p", this->handle);
}


rfid::device::InterfaceUsbImpl::~InterfaceUsbImpl() {
	if (this->handle) {
		usb_close(this->handle);

		this->handle = nullptr;
	}
}


bool rfid::device::InterfaceUsbImpl::isConnected() {
	return this->handle != nullptr;
}


void rfid::device::InterfaceUsbImpl::reset() {
	this->doTransferRx(nullptr, 0, 0, 0, PROTO_CMD_RESET, true);
}


std::shared_ptr<rfid::device::Interface::FirmwareVersion> rfid::device::InterfaceUsbImpl::getVersion() {
	this->checkConnection();

	return this->version;
}


std::shared_ptr<std::vector<rfid::device::Interface::Sample>> rfid::device::InterfaceUsbImpl::getSamples() {
	std::shared_ptr<std::vector<Sample>> ret(new std::vector<Sample>);

	const uint16_t prescaler = 8;

	{
		uint8_t transferId;

		uint8_t response[3];

		// Start sampler
		this->doTransferRx(
			response,
			1,
			PROTO_TRANSFER_FLAG_FIRST_ON_START | PROTO_TRANSFER_FLAG_START_ON_EDGE | PROTO_TRANSFER_FLAG_FALLING_EDGE | prescaler, // prescaler
			60000,
			PROTO_CMD_TRANSFER_START,
			true
		);

		transferId = response[0];

		Log::debug("Transfer id: %u", transferId);

		usleep(500 * 1000);

		// Check status
		this->doTransferRx(response, 3, transferId, 0, PROTO_CMD_TRANSFER_STATUS, true);

		Log::debug("status: %u, samplesCount: %u", response[0], (response[1] << 8) | response[2]);

		if (response[0] != PROTO_TRANSFER_STATUS_OK) {
			throw InvalidStateException();
		}
	}

	// Read samples
	{
		uint8_t samplesBuffer[this->sampleVectorSize];

		this->doTransferRx(samplesBuffer, this->sampleVectorSize, 0, 0, PROTO_CMD_PULSE_VECTOR_READ, false);

		int currentState       = -1;
		int currentStateLength = 0;

		for (int i = 0; i < this->sampleVectorSize; i++) {
			for (int j = 0; j < 8; j++) {
				int lastState = currentState;

				currentState = (samplesBuffer[i] & (1 << j)) != 0;
				if (currentState != lastState) {
					if (lastState != -1) {
						ret->push_back(Sample(currentStateLength * CARRIER_US * prescaler, ! lastState));

						currentStateLength = 1;
					}

				} else {
					currentStateLength++;
				}
			}
		}
	}

	return ret;
}


void rfid::device::InterfaceUsbImpl::putSamples(const std::vector<Sample> &samples) {
	std::set<int> pulses;

	for (auto sample : samples) {
		pulses.insert(sample.getLengthUs() / CARRIER_US);
	}

	Log::debug("Different pulses count: %zd, samples number: %zd", pulses.size(), samples.size());

	if (pulses.size() > this->pulseVectorSize) {
		throw TooManyPulsesException();
	}

	// Set pulse vector
	{
		uint8_t buffer[this->pulseVectorSize];
		uint8_t bufferWritten = 0;

		memset(buffer, 0, this->pulseVectorSize);

		for (auto pulse : pulses) {
			buffer[bufferWritten++] = pulse;
		}

		this->doTransferTx(buffer, this->pulseVectorSize, 0, 0, PROTO_CMD_SAMPLE_VECTOR_WRITE);
	}

	// Set sample vector
	{
		uint8_t  buffer[this->sampleVectorSize];
		uint16_t bufferSamplesWritten = 0;

		memset(buffer, 0xff, this->sampleVectorSize);

		for (auto sample : samples) {
			uint8_t value = (sample.isLow() ? 0 : 0x08) | std::distance(pulses.begin(), pulses.find(sample.getLengthUs() / CARRIER_US));

			if (bufferSamplesWritten & 1) {
				buffer[bufferSamplesWritten >> 1] |= (value << 4);

			} else {
				buffer[bufferSamplesWritten >> 1]  = value;
			}

			bufferSamplesWritten++;
		}

		this->doTransferTx(buffer, this->sampleVectorSize, 0, 0, PROTO_CMD_PULSE_VECTOR_WRITE);


		// Start transfer
		{
			uint8_t transferId;

			uint8_t response[3];

			// Start sampler
			this->doTransferRx(
				response,
				1,
				PROTO_TRANSFER_FLAG_TX_MODE | PROTO_TRANSFER_FLAG_START_ON_EDGE | PROTO_TRANSFER_FLAG_FALLING_EDGE | PROTO_TRANSFER_FLAG_FIRST_ON_START | 8, // prescaller
				60000 * CARRIER_US,
				PROTO_CMD_TRANSFER_START,
				true
			);

			transferId = response[0];

			Log::debug("Transfer id: %u", transferId);

			usleep(500 * 1000);

			// Check status
			this->doTransferRx(response, 3, transferId, 0, PROTO_CMD_TRANSFER_STATUS, true);

			Log::debug("status: %u, samplesCount: %u", response[0], (response[1] << 8) | response[2]);

			if (response[0] != PROTO_TRANSFER_STATUS_OK) {
				throw InvalidStateException();
			}
		}
	}
}


void rfid::device::InterfaceUsbImpl::transfer(bool enable) {
	this->doTransferRx(nullptr, 0, 0, enable ? 1 : 0, PROTO_CMD_COIL_ENABLE, true);
}
