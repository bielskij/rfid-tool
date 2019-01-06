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

#ifndef RFID_EM4100DECODER_HPP_
#define RFID_EM4100DECODER_HPP_

#include "common/Notifier.hpp"
#include "rfid/CodingDecoder.hpp"

namespace rfid {
	class Em4100Decoder : public common::Listener, public common::Notifier {
		public:
			enum Event {
				EVENT_NEW_TOKEN
			};

			struct EventNewTokenData {
				uint32_t data;
				uint8_t  versionOrCustomerId;
			};

		private:
			enum DecoderState {
				DECODER_STATE_PREAMBLE,
				DECODER_STATE_DATA
			};

			DecoderState state;

			uint8_t bitCount;

			uint8_t nibble;
			uint8_t nibbleLen;
			uint8_t nibbleCount;

			uint8_t data[6];

			CodingDecoder *codingDecoder;

		public:
			Em4100Decoder(CodingDecoder *codingDecoder);
			virtual ~Em4100Decoder();

			void reset();

			void onEvent(common::Notifier &notifier, const int eventId, void *eventData);

			CodingDecoder *getCodingDecoder();
	};
}

#endif /* RFID_EM4100DECODER_HPP_ */
