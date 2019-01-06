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
#include <getopt.h>
#include <string.h>

#include <iostream>
#include <unistd.h>
#include <common/Exception.hpp>
#include <rfid/InterfaceFactory.hpp>
#include <rfid/CarrierDecoder.hpp>
#include <rfid/CodingDecoder.hpp>
#include <rfid/Em4100Decoder.hpp>
#include <rfid/Em4100Eprom.hpp>
#include <rfid/T5557Encoder.hpp>

#include <rfid/impl/ManchesterDecoder.hpp>

#include "common/Log.hpp"


using namespace common;

enum Modulation {
	MODULATION_UNKNOWN,

	MODULATION_MANCHESTER,
};

enum Bitrate {
	BITRATE_UNKNOWN,

	BITRATE_16,
	BITRATE_32,
	BITRATE_64,
};

class Em4100DecoderListener : public common::Listener {
	public:
		void onEvent(common::Notifier &notifier, const int eventId, void *eventData) {
			rfid::Em4100Decoder::EventNewTokenData *data = reinterpret_cast<rfid::Em4100Decoder::EventNewTokenData *>(eventData);

			rfid::Em4100Decoder &em4100Decoder = static_cast<rfid::Em4100Decoder &>(notifier);

			common::Log::reportStdOut("New token, carrier divider: %u, modulation: %s, customer ID: %u (%#02x), token: %u (%#x)\n",
				em4100Decoder.getCodingDecoder()->getCarrierDecoder()->getCarrierDivider(), em4100Decoder.getCodingDecoder()->getName().c_str(), data->versionOrCustomerId, data->versionOrCustomerId, data->data, data->data
			);
		}
};

struct ExecutionOptions {
	bool showHelp;
	bool resetIface;
	bool read;
	bool write;

	uint8_t    customerId;
	uint32_t   token;
	Modulation modulation;
	Bitrate    bitrate;

	ExecutionOptions() {
		this->showHelp   = false;
		this->resetIface = false;
		this->write      = false;
		this->read       = false;

		this->customerId = 0;
		this->token      = 0;
		this->modulation = MODULATION_UNKNOWN;
		this->bitrate    = BITRATE_UNKNOWN;
	}
};

static struct option longOpts[] = {
	{ "verbose",    no_argument,       0, 'v' },
	{ "help",       no_argument,       0, 'h' },
	{ "reset",      no_argument,       0, 'R' },
	{ "read",       no_argument,       0, 'r' },
	{ "program",    no_argument,       0, 'p' },
	{ "bitrate",    required_argument, 0, 'b' },
	{ "modulation", required_argument, 0, 'm' },
	{ "customerid", required_argument, 0, 'c' },
	{ "token",      required_argument, 0, 't' },
	{ 0, 0, 0, 0 }
};

static const char *shortOpts = "vhRrpb:m:c:t:";

static ExecutionOptions options;


static void _showHelp(const char *progName, const char *errorMessage) {
	if (errorMessage != nullptr) {
		Log::reportStdOut("%s\n\n", errorMessage);
	}

	Log::reportStdOut("Usage: %s\n", progName);

	for (auto &i : longOpts) {
		if (i.name) {
			Log::reportStdOut("  -%c --%s\n", i.val, i.name);
		}
	}
}

int main(int argc, char *argv[]) {
	int ret = 0;

	const char *progName = basename(argv[0]);

	rfid::device::Interface *iface = nullptr;

	do {
		int option;
		int longIndex;

		while ((option = getopt_long(argc, argv, shortOpts, longOpts, &longIndex)) != -1) {
			switch (option) {
				case 'h':
					options.showHelp = true;
					break;

				case 'r':
					options.read = true;
					break;

				case 'p':
					options.write = true;
					break;

				case 'R':
					options.resetIface = true;
					break;

				case 'b':
					if (strcmp(optarg, "16") == 0) {
						options.bitrate = BITRATE_16;

					} else if (strcmp(optarg, "32") == 0) {
						options.bitrate = BITRATE_32;

					} else if (strcmp(optarg, "64") == 0) {
						options.bitrate = BITRATE_64;
					}
					break;

				case 'm':
					if (strcmp(optarg, "manchester") == 0) {
						options.modulation = MODULATION_MANCHESTER;
					}
					break;

				case 'c':
					options.customerId = strtol(optarg, nullptr, 0);
					break;

				case 't':
					options.token = strtol(optarg, nullptr, 0);
					break;

				case 'v':
					common::Log::setLevel(common::Log::DEBUG);
					break;

				default:
					options.showHelp = true;
			}
		}

		if (options.showHelp) {
			_showHelp(progName, nullptr);

			ret = -1;
			break;
		}

		if (options.write) {
			if (options.modulation == MODULATION_UNKNOWN) {
				_showHelp(progName, "Unknown modulation!");
				ret = -1;
				break;
			}

			if (options.bitrate == BITRATE_UNKNOWN) {
				_showHelp(progName, "Unknown bitrate");
				ret = -1;
				break;
			}
		}

		try {
			iface = rfid::device::InterfaceFactory::newInstance(
				rfid::device::InterfaceFactory::TYPE_USB
			);

			// report version
			{
				std::shared_ptr<rfid::device::Interface::FirmwareVersion> version = iface->getVersion();

				Log::log("Detected device with firmware version: %u.%u", version->getMajor(), version->getMinor());
			}

			if (options.resetIface) {
				iface->reset();
				break;
			}

			if (! options.read && ! options.write) {
				_showHelp(progName, nullptr);
				break;
			}

			if (options.read) {
				std::shared_ptr<std::vector<rfid::device::Interface::Sample>> samples = iface->getSamples();

				rfid::CarrierDecoder    carrierDecoder;
				rfid::ManchesterDecoder manchesterDecoder(&carrierDecoder);
				rfid::Em4100Decoder     em4100Decoder(&manchesterDecoder);

				em4100Decoder.addListener(new Em4100DecoderListener());
				
				carrierDecoder.checkPulses(*samples.get());

			} else {
				std::vector<rfid::device::Interface::Sample> samples;

				uint32_t em4100Eprom[2];

				{
					rfid::T5557Encoder::Parameters params;

					switch (options.bitrate) {
						case BITRATE_16: params.dataRate = rfid::T5557Encoder::DATA_RATE_16; break;
						case BITRATE_32: params.dataRate = rfid::T5557Encoder::DATA_RATE_32; break;
						case BITRATE_64: params.dataRate = rfid::T5557Encoder::DATA_RATE_64; break;

						default:
							break;
					}

					switch (options.modulation) {
						case MODULATION_MANCHESTER: params.modulation = rfid::T5557Encoder::MODULATION_MANCHESTER; break;

						default:
							break;
					}

					// Block 0 is a configuration block, 1 and 2 are data blocks.
					params.maxBlock = sizeof(em4100Eprom) / sizeof(*em4100Eprom);

					samples.clear();
					rfid::T5557Encoder::encodeParameters(params, false, 0, samples);
					iface->putSamples(samples);
				}

				{
					rfid::Em4100Eprom::generate(em4100Eprom, options.customerId, options.token);

					samples.clear();
					rfid::T5557Encoder::encodeBlock(0, 1, false, false, 0, em4100Eprom[0], samples);
					iface->putSamples(samples);

					samples.clear();
					rfid::T5557Encoder::encodeBlock(0, 2, false, false, 0, em4100Eprom[1], samples);
					iface->putSamples(samples);
				}
			}

		} catch (const common::Exception &ex) {
			Log::reportStdErr(ex.getMessage() + "\n");
			ret = -1;

		} catch (...) {
			Log::reportStdErr("Not supported exception!");
			ret = -1;
		}
	} while (0);

	if (iface) {
		delete iface;
	}

	return ret;
}
