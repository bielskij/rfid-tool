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

#include <cstdint>

#include "common/DataUtils.hpp"

#include "rfid/Em4100Decoder.hpp"

#include "common/Log.hpp"


rfid::Em4100Decoder::Em4100Decoder(CodingDecoder *codingDecoder) : codingDecoder(codingDecoder) {
	this->reset();

	this->codingDecoder->addListener(this);
}


rfid::Em4100Decoder::~Em4100Decoder() {
	this->codingDecoder->removeListener(this);
}


void rfid::Em4100Decoder::reset() {
	this->state    = DECODER_STATE_PREAMBLE;
	this->bitCount = 0;
	this->nibble   = 0;
}


void rfid::Em4100Decoder::onEvent(common::Notifier &notifier, const int eventId, void *eventData) {
	if (eventId == rfid::CodingDecoder::EVENT_NEW_BIT) {
		CodingDecoder::EventBitData *data = reinterpret_cast<CodingDecoder::EventBitData *>(eventData);

//		common::Log::debug("BIT: %u", data->isOne);

		switch (this->state) {
			case DECODER_STATE_PREAMBLE:
				{
//					common::Log::debug("PREAMBLE");

					if (data->isOne) {
						this->bitCount++;
						if (this->bitCount == 9) {
							this->state    = DECODER_STATE_DATA;
							this->bitCount = 0;

							this->nibbleCount = 0;
							this->nibbleLen   = 0;
						}

					} else {
						this->bitCount = 0;
					}
				}
				break;

			case DECODER_STATE_DATA:
				{
//					common::Log::debug("DATA");

					if (this->nibbleLen == 4) {
						bool parityOk;

						if (this->nibbleCount == 10) {
							parityOk = ! data->isOne; // Stop bit

						} else {
							parityOk = common::DataUtils::parityNibble(this->nibble) == data->isOne;
						}

						if (parityOk) {
							uint8_t byteNo = this->nibbleCount >> 1;
							uint8_t byteHi = this->nibbleCount & 1;

							if (! byteHi) {
								this->data[byteNo] = 0;
							}

							this->data[byteNo] |= (this->nibble << (byteHi ? 0 : 4));

							this->nibbleLen = 0;
							this->nibbleCount++;

							if (this->nibbleCount == 11) {
								uint8_t colsParity = 0;

								for (uint8_t i = 0; i < 5; i++) {
									colsParity ^= this->data[i] & 0xf0;
									colsParity ^= this->data[i] << 4;
								}

								if (colsParity != this->data[5]) {
									common::Log::error("Cols parity has failed!");

								} else {
									EventNewTokenData eventData;

									eventData.versionOrCustomerId = this->data[0];

									eventData.data  = this->data[1]; eventData.data <<= 8;
									eventData.data |= this->data[2]; eventData.data <<= 8;
									eventData.data |= this->data[3]; eventData.data <<= 8;
									eventData.data |= this->data[4];

									this->notify(EVENT_NEW_TOKEN, &eventData);
								}

								this->state = DECODER_STATE_PREAMBLE;
							}

						} else {
							common::Log::error("Parity not OK!");

							this->state = DECODER_STATE_PREAMBLE;
						}

					} else {
						if (data->isOne) {
							this->nibble |= (1 << (3 - this->nibbleLen++));
						} else {
							this->nibble &= ~(1 << (3 - this->nibbleLen++));
						}
					}
				}
				break;
		}
	}
}


rfid::CodingDecoder *rfid::Em4100Decoder::getCodingDecoder() {
	return this->codingDecoder;
}
