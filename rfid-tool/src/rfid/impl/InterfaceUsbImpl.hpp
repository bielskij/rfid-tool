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

#include <usb.h>

#include "rfid/Interface.hpp"


namespace rfid {
	namespace device {
		class InterfaceUsbImpl : public rfid::device::Interface {
			private:
				static class _init {
					public:
						_init() {
							usb_init();
						}
				} _initializer;

			public:
				InterfaceUsbImpl();
				virtual ~InterfaceUsbImpl();

				virtual bool isConnected();
				virtual void reset();
				virtual std::shared_ptr<FirmwareVersion> getVersion();
				virtual void transfer(bool enable);
				std::shared_ptr<std::vector<Sample>> getSamples();
				virtual void putSamples(const std::vector<Sample> &samples);

			protected:
				void checkConnection();
				void checkResponse(const uint8_t *buffer, int bufferSize, int returned, bool checkRc);
				void doTransferRx(uint8_t *buffer, uint16_t bufferSize, uint16_t index, uint16_t value, uint8_t command, bool checkRc);
				void doTransferTx(uint8_t *buffer, uint16_t bufferSize, uint16_t index, uint16_t value, uint8_t command);

			private:
				struct usb_dev_handle *handle;
				std::shared_ptr<FirmwareVersion> version;

				uint16_t pulsesMax;
				uint16_t pulseVectorSize;
				uint16_t pulseBits;
				uint16_t samplesMax;
				uint16_t sampleVectorSize;
				uint16_t sampleBits;
		};
	}
}
