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

#ifndef COMMON_LISTENER_HPP_
#define COMMON_LISTENER_HPP_

#include "common/Notifier.hpp"

namespace common {
	class Notifier;

	class Listener {
		public:
			virtual ~Listener() {};

			virtual void onEvent(common::Notifier &notifier, const int eventId, void *eventData) = 0;
	};
}

#endif /* COMMON_LISTENER_HPP_ */
