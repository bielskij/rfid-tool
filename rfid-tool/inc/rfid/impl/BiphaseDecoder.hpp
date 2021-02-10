/*
 * BiphaseDecoder.h
 *
 *  Created on: 10.02.2021
 *      Author: Jaroslaw Bielski (bielski.j@gmail.com)
 */

#ifndef RFID_BIPHASEDECODER_HPP_
#define RFID_BIPHASEDECODER_HPP_

#include "rfid/CarrierDecoder.hpp"
#include "rfid/CodingDecoder.hpp"

namespace rfid {
	class BiphaseDecoder : public rfid::CodingDecoder {
		private:
			bool _lastShort;

		public:
			BiphaseDecoder(CarrierDecoder *carrierDecoder);
			virtual ~BiphaseDecoder();

			void reset();

			std::string getName() const {
				return "Biphase";
			}

		public:
			virtual void onEvent(common::Notifier &notifier, const int eventId, void *eventData);
	};
}

#endif /* RFID_BIPHASEDECODER_HPP_ */
