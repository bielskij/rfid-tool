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

#ifndef RFID_T5557ENCODER_HPP_
#define RFID_T5557ENCODER_HPP_

#include <cstdint>
#include <vector>

#include <rfid/Interface.hpp>

namespace rfid {
	class T5557Encoder {
		public:
			enum DataRate {
				DATA_RATE_8,
				DATA_RATE_16,
				DATA_RATE_32,
				DATA_RATE_40,
				DATA_RATE_50,
				DATA_RATE_64,
				DATA_RATE_100,
				DATA_RATE_128
			};

			enum Modulation {
				MODULATION_DIRECT,
				MODULATION_PSK1,
				MODULATION_PSK2,
				MODULATION_PSK3,
				MODULATION_FSK1,
				MODULATION_FSK2,
				MODULATION_FSK1a,
				MODULATION_FSK2a,
				MODULATION_MANCHESTER,
				MODULATION_BIPHASE
			};

			enum PskSubcarrier {
				PSK_SUBCARRIER_RF_2,
				PSK_SUBCARRIER_RF_4,
				PSK_SUBCARRIER_RF_8
			};

			struct Parameters {
				bool          lock;
				uint8_t       masterKey;
				DataRate      dataRate;
				Modulation    modulation;
				PskSubcarrier pskSubcarrier;
				bool          answerOnRequest;
				uint8_t       maxBlock;
				bool          password;
				bool          sequenceTerminator;
				bool          powerOnResetDelay;

				Parameters() {
					this->lock               = false;
					this->masterKey          = 0;
					this->dataRate           = DATA_RATE_16;
					this->modulation         = MODULATION_MANCHESTER;
					this->pskSubcarrier      = PSK_SUBCARRIER_RF_2;
					this->answerOnRequest    = true;
					this->maxBlock           = 0;
					this->password           = false;
					this->sequenceTerminator = false;
					this->powerOnResetDelay  = false;
				}
			};

		public:
			static void encodeParameters(const Parameters &params, bool passwordSend, uint32_t password, std::vector<rfid::device::Interface::Sample> &samples);

			static void encodeBlock(uint8_t page, uint8_t block, bool lock, bool passwordSend, uint32_t password, uint32_t data, std::vector<rfid::device::Interface::Sample> &samples);
	};
}

#endif /* RFID_T5557ENCODER_HPP_ */
