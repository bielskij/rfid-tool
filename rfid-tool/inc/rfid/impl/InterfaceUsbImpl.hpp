/*
 * interfaceUsbImpl.cpp
 *
 *  Created on: 05.11.2018
 *      Author: jarko
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
//				virtual void coilEnable(bool enable);
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
