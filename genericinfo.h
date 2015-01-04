/* id3ted: genericinfo.h
 * Copyright (c) 2011 Bert Muennich <be.muennich at googlemail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef GENERICINFO_H
#define GENERICINFO_H

#include "id3ted.h"

class GenericInfo {
	public:
		GenericInfo(const char id, const char *value) :
				_id(id), _value(value, DEF_TSTR_ENC) {}
		const char id() const { return _id; }
		const String& value() const { return _value; }

	private:
		const char _id;
		const String _value;
};

#endif /* GENERICINFO_H */
