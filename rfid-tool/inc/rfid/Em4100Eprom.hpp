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

#ifndef RFID_EM4100EPROM_HPP_
#define RFID_EM4100EPROM_HPP_

#include <cstdint>

namespace rfid {
	class Em4100Eprom {
		public:
			virtual ~Em4100Eprom() {
			}

		private:
			Em4100Eprom() = delete;
			Em4100Eprom(const Em4100Eprom &other) = delete;

		public:
			static void generate(uint32_t result[2], uint8_t customerId, uint32_t token);

	};
}

#endif /* RFID_EM4100EPROM_HPP_ */
