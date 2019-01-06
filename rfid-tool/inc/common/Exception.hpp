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

#ifndef RFID_TOOL_INC_COMMON_EXCEPTION_HPP_
#define RFID_TOOL_INC_COMMON_EXCEPTION_HPP_


#include <string>

namespace common {
	class Exception {
		private:
			std::string message;
			int         code;

		private:
			Exception();

		public:
			Exception(const std::string &message) {
				this->message = message;
				this->code    = -1;
			}

			Exception(const std::string &message, int code) {
				this->message = message;
				this->code    = code;
			}

			Exception(const Exception &other) {
				this->message = other.message;
				this->code    = other.code;
			}

			virtual ~Exception() {
			}

			const std::string &getMessage() const {
				return this->message;
			}

			int getCode() const {
				return this->code;
			}
	};
}

#endif /* RFID_TOOL_INC_COMMON_EXCEPTION_HPP_ */
