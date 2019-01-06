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

#ifndef FIRMWARE_UTILS_H_
#define FIRMWARE_UTILS_H_

#define utl_min(__x, __y) (((__x) > (__y)) ? (__y) : (__x))
#define utl_max(__x, __y) (((__x) > (__y)) ? (__x) : (__y))

#define BIT_SET_AT(word, bitNum)   { word |= (1 << bitNum); }
#define BIT_CLEAR_AT(word, bitNum) { word &= ~(1 << bitNum); }

#define BIT_SET_MASK(word, mask)   { word |= mask; }
#define BIT_CLEAR_MASK(word, mask) { word &= ~(mask); }

#define CONCAT_MACRO(port, letter) port ## letter
#define DECLARE_PORT(port) CONCAT_MACRO(PORT, port)
#define DECLARE_DDR(port)  CONCAT_MACRO(DDR,  port)
#define DECLARE_PIN(port)  CONCAT_MACRO(PIN,  port)

#define PIO_SET_HIGH(bank, pin) { DECLARE_PORT(bank) |= _BV(DECLARE_PIN(pin)); }
#define PIO_SET_LOW(bank, pin)  { DECLARE_PORT(bank) &= ~_BV(DECLARE_PIN(pin)); }

#define PIO_TOGGLE(bank, pin) { DECLARE_PORT(bank) ^= _BV(DECLARE_PIN(pin)); }

#define PIO_IS_HIGH(bank, pin) (bit_is_set(DECLARE_PIN(bank), DECLARE_PIN(pin)))
#define PIO_IS_LOW(bank, pin) (! bit_is_set(DECLARE_PIN(bank), DECLARE_PIN(pin)))

#define PIO_SET_OUTPUT(bank, pin) { DECLARE_DDR(bank) |= _BV(DECLARE_PIN(pin)); }
#define PIO_SET_INPUT(bank, pin)  { DECLARE_DDR(bank) &= ~(_BV(DECLARE_PIN(pin))); }

#endif /* FIRMWARE_UTILS_H_ */
