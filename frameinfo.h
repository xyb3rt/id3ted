/* id3ted: frameinfo.h
 * Copyright (c) 2009 Bert Muennich <muennich at informatik.hu-berlin.de>
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

#include "common.h"

class MP3File;

class GenericInfo {
	private:
		const char _id;
		const char *_value;

	public:
		GenericInfo(const char, const char*);
		const char id() const { return _id; }
		const char *value() const { return _value; }
		bool sameIDIn(const std::vector<GenericInfo*>&);
};

class FrameInfo {
	protected:
		const char *_id;
		const ID3v2FrameID _fid;
		const TagLib::String _value;
		const unsigned int _fPathIdx;
		
	public:
		FrameInfo(const char*, ID3v2FrameID, const char*, unsigned int = 0);
		virtual ~FrameInfo() {}

		const char* id() const { return _id; }
		ID3v2FrameID fid() const { return _fid; }
		const TagLib::String &value() const { return _value; }
		unsigned int fPathIdx() const { return _fPathIdx; }

		bool sameFIDIn(const std::vector<FrameInfo*>&);
		virtual bool applyTo(MP3File&);
};

class APICFrameInfo : public FrameInfo {
	protected:
		TagLib::String _mimetype;
		TagLib::ByteVector _picture;
		bool _fileRead;
		bool _readError;
	
	public:
		APICFrameInfo(const char*, ID3v2FrameID, const char*, unsigned int = 0);

		const TagLib::String& mimetype() const { return _mimetype; }
		const TagLib::ByteVector& picture() const { return _picture; }
		bool fileRead() { return _fileRead; }
		bool readError() { return _readError; }

		bool readFile();
		bool applyTo(MP3File&);
};

class TDFrameInfo : public FrameInfo {
	protected:
		TagLib::String _text;
		TagLib::String _description;
		bool _multipleFields;

	public:
		TDFrameInfo(const char*, ID3v2FrameID, const char*, unsigned int = 0);
		virtual ~TDFrameInfo() {}

		const TagLib::String& text() const { return _text; }
		const TagLib::String& description() const { return _description; }
		bool multipleFields() { return _multipleFields; }

		virtual bool applyTo(MP3File&);
};

class TDLFrameInfo : public TDFrameInfo {
	protected:
		TagLib::String _language;
	
	public:
		TDLFrameInfo(const char*, ID3v2FrameID, const char*, unsigned int = 0);

		const TagLib::String& language() const { return _language; }

		bool applyTo(MP3File&);
};

#endif /* __FRAMEINFO_H__ */

