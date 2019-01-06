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

#include "rfid/CarrierDecoder.hpp"
#include "rfid/impl/ManchesterDecoder.hpp"


rfid::ManchesterDecoder::ManchesterDecoder(CarrierDecoder *carrierDecoder) : CodingDecoder(carrierDecoder) {
	this->reset();
}


rfid::ManchesterDecoder::~ManchesterDecoder() {

}


void rfid::ManchesterDecoder::reset() {
	this->ffOut        = false;
	this->timerRunning = false;

//	this->notify(rfid::CodingDecoder::EVENT_RESET, nullptr);
}


void rfid::ManchesterDecoder::onEvent(common::Notifier &notifier, const int eventId, void *eventData) {
	if (eventId == rfid::CarrierDecoder::EVENT_NEW_PULSE) {
		rfid::CarrierDecoder::EventPulseData *data = reinterpret_cast<rfid::CarrierDecoder::EventPulseData*>(eventData);

		if (this->timerRunning) {
			this->ffOut        = data->isHigh;
			this->timerRunning = data->isLong;

			{
				EventBitData eventData;

				eventData.isOne = this->ffOut;

				this->notify(EVENT_NEW_BIT, &eventData);
			}

			if (this->timerRunning) {
				this->timerRunning = this->ffOut ^ (data->isHigh);
			}

		} else {
			bool timerTrigger = data->isHigh ^ this->ffOut;

			if (timerTrigger) {
				if (data->isLong) {
					this->timerRunning = false;
					this->ffOut        = data->isHigh;

					{
						EventBitData eventData;

						eventData.isOne = this->ffOut;

						this->notify(EVENT_NEW_BIT, &eventData);
					}

				} else {
					this->timerRunning = true;
				}
			}
		}

	} else if (eventId == rfid::CarrierDecoder::EVENT_SYNC_CHANGED) {
		this->reset();
	}
}
