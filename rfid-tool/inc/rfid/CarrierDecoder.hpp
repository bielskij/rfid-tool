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

#ifndef RFID_CARRIERDECODER_HPP_
#define RFID_CARRIERDECODER_HPP_

#include <cstdint>

#include "common/Notifier.hpp"
#include "rfid/Interface.hpp"

namespace rfid {
	class CarrierDecoder : public common::Notifier {
		public:
			enum Event {
				EVENT_SYNC_CHANGED,
				EVENT_NEW_PULSE
			};

			struct EventSyncData {
				bool    hasSync;
				uint8_t carrierDivider;
			};

			struct EventPulseData {
				bool isHigh;
				bool isLong;
			};

		private:
			// Pulse length range
			uint16_t longMin;
			uint16_t longMax;
			uint16_t shortMin;
			uint16_t shortMax;

			// Pulses number (0 - 50)
			uint8_t pulseCount;
			uint8_t loCount;
			uint8_t hiCount;
			// Pulse errors count (0 - 50)
			// Current value is checked on pulseCount overflow
			uint8_t errorCount;

			uint8_t carrierDivider;

			bool    hasSync;

		public:
			CarrierDecoder();
			virtual ~CarrierDecoder();

			void reset();
			void checkPulses(const std::vector<rfid::device::Interface::Sample> &samples);

			uint8_t getCarrierDivider() {
				return this->carrierDivider;
			}

		private:
			void updatePulseRange();
	};
}

#endif /* RFID_CARRIERDECODER_HPP_ */
