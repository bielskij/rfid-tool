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

#ifndef COMMON_DATAUTILS_HPP_
#define COMMON_DATAUTILS_HPP_

#include <cstdint>

namespace common {
	class DataUtils {
		private:
			static const uint8_t REVERSED_NIBBLE[];
			static const uint16_t PARITY;

		private:
			DataUtils() = delete;
			DataUtils(const DataUtils &) = delete;

		public:
			static uint8_t parityNibble(uint8_t nibble) {
				return ((PARITY >> (nibble & 0x0f)) & 1);
			}

			static uint8_t parityByte(uint8_t byte) {
				byte ^= (byte >> 4);
				byte ^= (byte >> 2);
				byte ^= (byte >> 1);

				return (byte) & 1;
			}

			static uint8_t reverseNibble(uint8_t nibble) {
				return REVERSED_NIBBLE[nibble];
			}

			static uint8_t putBit(uint32_t *buffer, uint8_t bitOffset, uint8_t bit) {
				uint8_t idx = bitOffset / 32;

				if (bit) {
					buffer[idx] |=  ((uint32_t) 1 << (bitOffset % 32));

				} else {
					buffer[idx] &= ~((uint32_t) 1 << (bitOffset % 32));
				}

				return 1;
			}

			static uint8_t putBitsMsb(uint32_t *buffer, uint8_t bitOffset, uint16_t bits, uint8_t bitsCount) {
				for (uint8_t i = bitsCount; i > 0; i--) {
					putBit(buffer, bitOffset + bitsCount - i, (bits & ((uint16_t) 1 << (i - 1))) != 0);
				}

				return bitsCount;
			}
	};
}

#endif /* COMMON_DATAUTILS_HPP_ */
