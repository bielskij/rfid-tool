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

#ifndef COMMON_NOTIFIER_H_
#define COMMON_NOTIFIER_H_

#include <vector>
#include <algorithm>

#include "common/Listener.hpp"


namespace common {
	class Notifier {
		private:
			std::vector<common::Listener *> listeners;

		public:
			virtual ~Notifier() {}

			void addListener(common::Listener *listener) {
				this->listeners.push_back(listener);
			}

			void removeListener(common::Listener *listener) {
				auto it = std::find(this->listeners.begin(), this->listeners.end(), listener);
				if (it != this->listeners.end()) {
					this->listeners.erase(it);
				}
			}

		protected:
			void notify(const int eventId, void *eventData) {
				for (auto i : this->listeners) {
					i->onEvent(*this, eventId, eventData);
				}
			}
	};
}

#endif /* COMMON_NOTIFIER_H_ */
