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
#include "common/Log.hpp"

#define CARRIER_PULSE_LENGTH 8 // us

#define PULSE_MAX      50
#define ERROR_TRESHOLD 10 // 20%

#define PULSE_SHORT(__dec, __pulse) ((__pulse >= __dec->shortMin) && (__pulse <= __dec->shortMax))
#define PULSE_LONG(__dec, __pulse) ((__pulse >= __dec->longMin) && (__pulse <= __dec->longMax))


void rfid::CarrierDecoder::updatePulseRange() {
	uint16_t pulseLength = (this->carrierDivider * CARRIER_PULSE_LENGTH) / 2;
	uint16_t delta       = pulseLength >> 2; // 25%

	this->shortMin = pulseLength - delta;
	this->shortMax = pulseLength + delta;

	pulseLength <<= 1;

	this->longMin = pulseLength - delta;
	this->longMax = pulseLength + delta;
}

rfid::CarrierDecoder::CarrierDecoder() {
	this->reset();
}


rfid::CarrierDecoder::~CarrierDecoder() {

}


void rfid::CarrierDecoder::reset() {
	this->carrierDivider = 16;
	this->errorCount     = 0;
	this->pulseCount     = 0;
	this->hasSync        = false;

	this->loCount = 0;
	this->hiCount = 0;

	this->updatePulseRange();
}


void rfid::CarrierDecoder::checkPulses(const std::vector<rfid::device::Interface::Sample> &samples) {
	for (const auto &sample : samples) {
		bool nextCarrier = false;

		this->pulseCount++;

		// Handle error
		if (this->errorCount >= ERROR_TRESHOLD) {
			common::Log::debug("next carrier! error count: %u, loCount: %u, hiCount: %u, pulseCount: %u, divider: %u",
				this->errorCount, this->loCount, this->hiCount, this->pulseCount, this->carrierDivider
			);

			nextCarrier = true;

		} else if (this->pulseCount == PULSE_MAX) {
			// Next loop
			this->pulseCount = 0;
			this->errorCount = 0;
		}

		if (nextCarrier) {
			this->hasSync    = false;
			this->loCount    = 0;
			this->hiCount    = 0;
			this->errorCount = 0;
			this->pulseCount = 0;

			{
				EventSyncData data;

				data.hasSync        = this->hasSync;
				data.carrierDivider = 0;

				this->notify(EVENT_SYNC_CHANGED, &data);
			}

			switch (this->carrierDivider) {
				case 16: this->carrierDivider = 32; break;
				case 32: this->carrierDivider = 64; break;
				case 64: this->carrierDivider = 16; break;
			}

			this->updatePulseRange();

		} else {
			// NO_SYNC -> SYNC
			if (! this->hasSync) {
				if (PULSE_SHORT(this, sample.getLengthUs())) {
					this->loCount++;

				} else if (PULSE_LONG(this, sample.getLengthUs())) {
					this->hiCount++;

				} else {
					//common::Log::debug("Out of sync, bad pulse: %uus, (short %uus - %uus, long %uus - %uus)",
					//	sample.getLengthUs(), this->shortMin, this->shortMax, this->longMin, this->longMax
					//);

					this->errorCount++;
				}

				if (this->hiCount > 4 && this->loCount > 4 && this->errorCount < 4) {
					this->hasSync = true;

					{
						EventSyncData data;

						data.hasSync        = this->hasSync;
						data.carrierDivider = this->carrierDivider;

						this->notify(EVENT_SYNC_CHANGED, &data);
					}
				}

			} else {
				EventPulseData data;

				data.isHigh = ! sample.isLow();

				if (PULSE_LONG(this, sample.getLengthUs())) {
					data.isLong = true;

					this->notify(EVENT_NEW_PULSE, &data);

				} else if (PULSE_SHORT(this, sample.getLengthUs())) {
					data.isLong = false;

					this->notify(EVENT_NEW_PULSE, &data);

				} else {
					//common::Log::debug("In sync, bad pulse: %uus, (short %uus - %uus, long %uus - %uus)",
					//	sample.getLengthUs(), this->shortMin, this->shortMax, this->longMin, this->longMax
					//);

					this->errorCount++;
				}
			}
		}

//		common::Log::debug("sync: %u, divider: %u, short %u - %u, long: %u - %u, errors: %u, pulses: %u, hi: %u, lo: %u",
//			this->hasSync, this->carrierDivider, this->shortMin, this->shortMax,
//			this->longMin, this->longMax, this->errorCount, this->pulseCount,
//			this->hiCount, this->loCount
//		);
	}
}
