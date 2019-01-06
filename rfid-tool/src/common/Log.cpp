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

#include <cstdint>
#include <cctype>

#include "common/Log.hpp"

using namespace common;


Log::Level Log::_level = Log::NONE;


void Log::setLevel(Level level) {
	_level = level;
}


void Log::report(Level level, const std::string &format, ...) {
	va_list args;

	va_start(args, format);

	report(level, format, args);

	va_end(args);
}


void Log::report(Level level, const std::string &format, va_list &list) {
	if (_level >= level) {
		switch (level) {
			case DEBUG: printf("[DBG]: "); break;
			case LOG:   printf("[LOG]: "); break;
			case WARN:  printf("[WRN]: "); break;
			case ERROR: printf("[ERR]: "); break;
			default:
				break;
		}

		vprintf(format.c_str(), list);
		printf("\n");
	}
}


void Log::debug(const std::string &format, ...) {
	va_list args;

	va_start(args, format);

	report(Level::DEBUG, format, args);

	va_end(args);
}


void Log::log(const std::string &format, ...) {
	va_list args;

	va_start(args, format);

	report(Level::LOG, format, args);

	va_end(args);
}


void Log::warn(const std::string &format, ...) {
	va_list args;

	va_start(args, format);

	report(Level::WARN, format, args);

	va_end(args);
}


void Log::error(const std::string &format, ...) {
	va_list args;

	va_start(args, format);

	report(Level::ERROR, format, args);

	va_end(args);
}


void Log::reportStdOut(const std::string &format, ...) {
	va_list args;

	va_start(args, format);

	vfprintf(stdout, format.c_str(), args);

	va_end(args);
}


void Log::reportStdErr(const std::string &format, ...) {
	va_list args;

	va_start(args, format);

	vfprintf(stderr, format.c_str(), args);

	va_end(args);
}


void Log::dump(void *buffer, size_t bufferSize, size_t lineLength) {
	size_t i;

	char asciiBuffer[lineLength + 1];

	for (i = 0; i < bufferSize; i++) {
		if (i % lineLength == 0) {
			if (i != 0) {
				Log::reportStdOut("  %s\n", asciiBuffer);
			}

			Log::reportStdOut("%04x:  ", i);
		}

		Log::reportStdOut(" %02x", reinterpret_cast<uint8_t *>(buffer)[i]);

		if (! std::isprint(reinterpret_cast<char *>(buffer)[i])) {
			asciiBuffer[i % lineLength] = '.';

		} else {
			asciiBuffer[i % lineLength] = reinterpret_cast<char *>(buffer)[i];
		}

		asciiBuffer[(i % lineLength) + 1] = '\0';
	}

	while ((i % 16) != 0) {
		Log::reportStdOut("   ");
		i++;
	}

	Log::reportStdOut("  %s\n", asciiBuffer);
}
