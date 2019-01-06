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
#include "common/Exception.hpp"

#include "rfid/InterfaceFactory.hpp"

#include "impl/InterfaceUsbImpl.hpp"

rfid::device::Interface *rfid::device::InterfaceFactory::newInstance(Type type) {
	rfid::device::Interface *ret = nullptr;

	switch (type) {
		case InterfaceFactory::TYPE_USB:
			ret = new rfid::device::InterfaceUsbImpl();
			break;

		default:
			break;
	}

	if (ret == nullptr) {
		throw common::Exception("Not supported type!", 1);
	}

	return ret;
}
