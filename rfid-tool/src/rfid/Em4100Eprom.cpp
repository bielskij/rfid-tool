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

#include "rfid/Em4100Eprom.hpp"

#include "common/DataUtils.hpp"

static uint8_t _bufferAddBit(uint32_t *buffer, uint8_t bitOffset, uint8_t bit) {
	uint8_t idx = bitOffset / 32;

	if (bit) {
		buffer[idx] |=  ((uint32_t) 1 << (bitOffset % 32));

	} else {
		buffer[idx] &= ~((uint32_t) 1 << (bitOffset % 32));
	}

	return 1;
}

static uint8_t _bufferAddBits(uint32_t *buffer, uint8_t bitOffset, uint16_t bits, uint8_t bitsCount) {
	uint8_t ret = 0;

	for (uint8_t i = 0; i < bitsCount; i++) {
		ret += _bufferAddBit(buffer, bitOffset + ret, (bits & ((uint16_t) 1 << i)) != 0);
	}

	return ret;
}


void rfid::Em4100Eprom::generate(uint32_t result[2], uint8_t customerId, uint32_t token) {
	uint8_t written = 0;
	uint8_t parity  = 0;
	uint8_t nibble  = 0;

	// Preamble
	written += _bufferAddBits(result, written, 0x1ff, 9);

	// vendor
	nibble = common::DataUtils::reverseNibble(customerId >> 4);
	written += _bufferAddBits(result, written, nibble | (common::DataUtils::parityNibble(nibble) << 4), 5);
	parity ^= nibble;

	nibble = common::DataUtils::reverseNibble(customerId & 0x0f);
	written += _bufferAddBits(result, written, nibble | (common::DataUtils::parityNibble(nibble) << 4), 5);
	parity ^= nibble;

	// code;
	for (uint8_t i = 0; i < 8; i++) {
		nibble = common::DataUtils::reverseNibble(((token >> ((7 - i) * 4)) & 0x0f));

		parity ^= nibble;

		written += _bufferAddBits(result, written, (nibble) | (common::DataUtils::parityNibble(nibble) << 4), 5);
	}

	written += _bufferAddBits(result, written, parity, 5);
}
