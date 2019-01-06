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

#ifndef RFID_CODINGDECODER_HPP_
#define RFID_CODINGDECODER_HPP_

#include "common/Notifier.hpp"
#include "rfid/CarrierDecoder.hpp"

namespace rfid {
	class CodingDecoder : public common::Listener, public common::Notifier {
		public:
			enum Event {
				EVENT_NEW_BIT,
				EVENT_RESET
			};

			struct EventBitData {
				bool isOne;
			};

		public:
			CodingDecoder(CarrierDecoder *carrierDecoder) : carrierDecoder(carrierDecoder) {
				this->carrierDecoder->addListener(this);
			}

			virtual ~CodingDecoder() {
				this->carrierDecoder->removeListener(this);
			}

			virtual void reset() = 0;
			virtual std::string getName() const = 0;

			CarrierDecoder *getCarrierDecoder() const {
				return this->carrierDecoder;
			}

			virtual void onEvent(common::Notifier &notifier, const int eventId, void *eventData) = 0;

		private:
			CarrierDecoder *carrierDecoder;
	};
}

#endif /* RFID_CODINGDECODER_HPP_ */
