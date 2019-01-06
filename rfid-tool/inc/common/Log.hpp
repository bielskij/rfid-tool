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

#ifndef RFID_TOOL_INC_COMMON_LOG_HPP_
#define RFID_TOOL_INC_COMMON_LOG_HPP_

#include <string>
#include <stdarg.h>

namespace common {
	class Log {
		public:
			enum Level {
				NONE, ERROR, WARN, LOG, DEBUG
			};

		public:
			static void debug(const std::string &format, ...);
			static void log(const std::string &format, ...);
			static void warn(const std::string &format, ...);
			static void error(const std::string &format, ...);

			static void reportStdOut(const std::string &format, ...);
			static void reportStdErr(const std::string &format, ...);

			static void dump(void *buffer, size_t bufferSize, size_t lineLength = 32);

			static void setLevel(Level level);

		protected:
			static void report(Level level, const std::string &format, ...);
			static void report(Level level, const std::string &format, va_list &list);

		private:
			static Level _level;
	};
}

#endif /* RFID_TOOL_INC_COMMON_LOG_HPP_ */
