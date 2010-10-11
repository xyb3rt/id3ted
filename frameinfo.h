/* id3ted: frameinfo.h
 * Copyright (c) 2010 Bert Muennich <muennich at informatik.hu-berlin.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#ifndef __FRAMEINFO_H__
#define __FRAMEINFO_H__

#include <taglib/tbytevector.h>
#include <taglib/tstring.h>

#include "id3ted.h"

class FrameInfo {
	public:
		FrameInfo(const char*, ID3v2FrameID, const char*);

		const char* id() const { return _id; }
		ID3v2FrameID fid() const { return _fid; }
		const String& text() const { return _text; }
		const String& description() const { return _description; }
		const ByteVector& language() const { return _language; }
		const ByteVector& data() const { return _data; }

	private:
		const char *_id;
		const ID3v2FrameID _fid;
		String _text;
		String _description;
		ByteVector _language;
		ByteVector _data;

		void split2();
		void split3();
};

#endif /* __FRAMEINFO_H__ */

