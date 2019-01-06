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

#ifndef RFID_TOOL_INC_INTERFACE_HPP_
#define RFID_TOOL_INC_INTERFACE_HPP_

#include <memory>
#include <vector>

#include "common/Exception.hpp"

namespace rfid {
	namespace device {
		class Interface {
			public:
				static const int CARRIER_US;

			public:
				class NotConnectedException : public common::Exception {
					public:
						NotConnectedException() : common::Exception("Interface was not detected!") {
						}
				};

				class NotSupportedInterfaceException : public common::Exception {
					public:
						NotSupportedInterfaceException() : common::Exception("Not supported interface!") {
						}
				};

				class InvalidResponseException : public common::Exception {
					public:
						InvalidResponseException(int code) : common::Exception("Invalid response!", code) {
						}
				};

				class InvalidStateException : public common::Exception {
					public:
						InvalidStateException() : common::Exception("Invalid state!") {
						}
				};

				class TimeoutException : public common::Exception {
					public:
						TimeoutException() : common::Exception("Timeout exception!") {
						}
				};

				class TooManyPulsesException : public common::Exception {
					public:
						TooManyPulsesException() : common::Exception("Too many pulses requested!") {
						}
				};

				class TooManySamplesException : public common::Exception {
					public:
						TooManySamplesException() : common::Exception("Too many samples requested!") {

						}
				};

				class FirmwareVersion {
					private:
						uint8_t major;
						uint8_t minor;

					private:
						FirmwareVersion(const FirmwareVersion &other) = delete;

					protected:
						FirmwareVersion(uint8_t major, uint8_t minor) {
							this->major = major;
							this->minor = minor;
						}

						FirmwareVersion() {
							this->major = 0;
							this->minor = 0;
						}

					public:
						virtual ~FirmwareVersion() {
						}

						uint8_t getMajor() const {
							return this->major;
						}

						uint8_t getMinor() const {
							return this->minor;
						}
				};

				class Sample {
					public:
						virtual ~Sample() {}

						Sample(int sampleUs, bool isHigh) {
							this->sampleUs = sampleUs;
							this->isHigh   = isHigh;
						}

						int getLengthUs() const {
							return this->sampleUs;
						}

						bool isLow() const {
							return ! this->isHigh;
						}

					private:
						int  sampleUs;
						bool isHigh;
				};

			public:
				virtual ~Interface() {
				}

				virtual bool isConnected() = 0;

				virtual void reset() = 0;

				virtual std::shared_ptr<FirmwareVersion> getVersion() = 0;

				virtual std::shared_ptr<std::vector<Sample>> getSamples() = 0;

				virtual void putSamples(const std::vector<Sample> &samples) = 0;
		};
	}
}

#endif /* RFID_TOOL_INC_INTERFACE_HPP_ */
