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

#include "rfid/T5557Encoder.hpp"

#include "common/DataUtils.hpp"
#include "common/Log.hpp"


class TransferData {
	private:
		static const int SL_FIXED_DATA_ONE;
		static const int SL_FIXED_DATA_ZERO;
		static const int SL_FIXED_WRITE_GAP;
		static const int SL_FIXED_START_GAP;

	public:
		enum Protocol {
			PROTOCOL_FIXED_BIT_LENGTH
		};

	public:
		TransferData(Protocol protocol, std::vector<rfid::device::Interface::Sample> &samples) : samples(samples) {
			this->protocol = protocol;

			switch (this->protocol) {
				case PROTOCOL_FIXED_BIT_LENGTH:
					this->addPulse(false, SL_FIXED_START_GAP);
					break;

				default:
					break;
			}
		}

		void addPulse(bool high, int length) {
			this->samples.push_back(rfid::device::Interface::Sample(length, high));
		}

		void addBit(uint8_t bit) {
			switch (this->protocol) {
				case PROTOCOL_FIXED_BIT_LENGTH:
					this->addPulse(true, bit ? SL_FIXED_DATA_ONE : SL_FIXED_DATA_ZERO);
					this->addPulse(false, SL_FIXED_WRITE_GAP);
					break;

				default:
					break;
			}
		}

		void addBitsLsb(uint32_t wordToTransfer, uint8_t bitsToTransfer) {
			for (uint8_t i = 0; i < bitsToTransfer; i++) {
				this->addBit((wordToTransfer & (((uint32_t) 1) << i)) != 0);
			}
		}

		void addBitsMsb(uint32_t wordToTransfer, uint8_t bitsToTransfer) {
			for (uint8_t i = bitsToTransfer; i > 0; i--) {
				this->addBit((wordToTransfer & (((uint32_t) 1) << (i - 1))) != 0);
			}
		}

	private:
		Protocol protocol;
		std::vector<rfid::device::Interface::Sample> &samples;
};

const int TransferData::SL_FIXED_DATA_ONE  = rfid::device::Interface::CARRIER_US * ((48 + 63) / 2);
const int TransferData::SL_FIXED_DATA_ZERO = rfid::device::Interface::CARRIER_US * ((16 + 31) / 2);
const int TransferData::SL_FIXED_WRITE_GAP = rfid::device::Interface::CARRIER_US * (( 8 + 30) / 2);
const int TransferData::SL_FIXED_START_GAP = rfid::device::Interface::CARRIER_US * ((10 + 50) / 2);


void rfid::T5557Encoder::encodeBlock(uint8_t page, uint8_t block, bool lock, bool passwordSend, uint32_t password, uint32_t data, std::vector<rfid::device::Interface::Sample> &samples) {
	TransferData tData(TransferData::PROTOCOL_FIXED_BIT_LENGTH, samples);

	// opcode
	tData.addBitsLsb(1 | ((page & 0x01) << 1), 2);

	if (passwordSend) {
		tData.addBitsMsb(password, 32);
	}

	// lock
	tData.addBit(lock);

	// word
	tData.addBitsLsb(data, 32);

	// address
	tData.addBitsMsb(block, 3);
}


void rfid::T5557Encoder::encodeParameters(const Parameters &params, bool passwordSend, uint32_t password, std::vector<rfid::device::Interface::Sample> &samples) {
	uint32_t data = 0;

	uint8_t bitOffset = 0;

	bitOffset += common::DataUtils::putBitsMsb(&data, bitOffset, params.masterKey, 4);
	bitOffset += common::DataUtils::putBitsMsb(&data, bitOffset,                0, 7);

	{
		uint8_t dataRateValue = 0;

		switch (params.dataRate) {
			case DATA_RATE_8:   dataRateValue = 0; break;
			case DATA_RATE_16:  dataRateValue = 1; break;
			case DATA_RATE_32:  dataRateValue = 2; break;
			case DATA_RATE_40:  dataRateValue = 3; break;
			case DATA_RATE_50:  dataRateValue = 4; break;
			case DATA_RATE_64:  dataRateValue = 5; break;
			case DATA_RATE_100: dataRateValue = 6; break;
			case DATA_RATE_128: dataRateValue = 7; break;
		}

		bitOffset += common::DataUtils::putBitsMsb(&data, bitOffset, dataRateValue, 3);
	}

	bitOffset += common::DataUtils::putBit(&data, bitOffset, 0);

	{
		uint8_t modulationValue = 0;

		switch (params.modulation) {
			case MODULATION_DIRECT:     modulationValue =  0; break;
			case MODULATION_PSK1:       modulationValue =  1; break;
			case MODULATION_PSK2:       modulationValue =  2; break;
			case MODULATION_PSK3:       modulationValue =  3; break;
			case MODULATION_FSK1:       modulationValue =  4; break;
			case MODULATION_FSK2:       modulationValue =  5; break;
			case MODULATION_FSK1a:      modulationValue =  6; break;
			case MODULATION_FSK2a:      modulationValue =  7; break;
			case MODULATION_MANCHESTER: modulationValue =  8; break;
			case MODULATION_BIPHASE:    modulationValue = 16; break;
		}

		bitOffset += common::DataUtils::putBitsMsb(&data, bitOffset, modulationValue, 5);
	}

	{
		uint8_t pskSubValue = 0;

		switch (params.pskSubcarrier) {
			case PSK_SUBCARRIER_RF_2: pskSubValue = 0; break;
			case PSK_SUBCARRIER_RF_4: pskSubValue = 1; break;
			case PSK_SUBCARRIER_RF_8: pskSubValue = 2; break;
		}

		bitOffset += common::DataUtils::putBitsMsb(&data, bitOffset, pskSubValue, 2);
	}

	bitOffset += common::DataUtils::putBit    (&data, bitOffset, params.answerOnRequest);
	bitOffset += common::DataUtils::putBit    (&data, bitOffset, 0);
	bitOffset += common::DataUtils::putBitsMsb(&data, bitOffset, params.maxBlock, 3);
	bitOffset += common::DataUtils::putBit    (&data, bitOffset, params.password);
	bitOffset += common::DataUtils::putBit    (&data, bitOffset, params.sequenceTerminator);
	bitOffset += common::DataUtils::putBitsMsb(&data, bitOffset, 0, 2);
	bitOffset += common::DataUtils::putBit    (&data, bitOffset, params.powerOnResetDelay);

	encodeBlock(0, 0, params.lock, passwordSend, password, data, samples);
}
