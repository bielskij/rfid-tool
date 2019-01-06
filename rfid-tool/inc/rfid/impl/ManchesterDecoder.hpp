#ifndef RFID_MANCHESTERDECODER_HPP_
#define RFID_MANCHESTERDECODER_HPP_

#include "rfid/CarrierDecoder.hpp"
#include "rfid/CodingDecoder.hpp"

namespace rfid {
	class ManchesterDecoder : public rfid::CodingDecoder {
		private:
			bool ffOut;
			bool timerRunning;

		public:
			ManchesterDecoder(CarrierDecoder *carrierDecoder);
			virtual ~ManchesterDecoder();

			void reset();

			std::string getName() const {
				return "Manchester";
			}

		public:
			virtual void onEvent(common::Notifier &notifier, const int eventId, void *eventData);
	};
}

#endif /* RFID_MANCHESTERDECODER_HPP_ */
