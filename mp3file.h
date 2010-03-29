/* id3ted: mp3file.h
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

#ifndef __MP3FILE_H__
#define __MP3FILE_H__

#include <taglib/mpegfile.h>

#include "common.h"

class GenericInfo;
class FrameInfo;
class APICFrameInfo;
class TDFrameInfo;
class TDLFrameInfo;

class MP3File {
	friend class FrameInfo;
	friend class APICFrameInfo;
	friend class TDFrameInfo;
	friend class TDLFrameInfo;

	private:
		TagLib::MPEG::File _file;
		TagLib::Tag *_id3Tag;
		TagLib::ID3v1::Tag *_id3v1Tag;
		TagLib::ID3v2::Tag *_id3v2Tag;

		int _tags;

		bool hasID3v2Tag() const { return _tags & 2; }
		void createID3v2Tag();
		bool listID3v1Tag() const;
		bool listID3v2Tag(bool) const;

	public:
		explicit MP3File(const char*);

		bool isValid() const { return _file.isValid(); }
		bool isReadOnly() const { return _file.readOnly(); }
		static bool isReadable(const char *filename) { return TagLib::MPEG::File::isReadable(filename); }
		static bool isWritable(const char *filename) { return TagLib::MPEG::File::isWritable(filename); }
		const char* filename() const { return _file.name(); }

		void apply(GenericInfo*);
		void removeFrames(const char*);
		bool save(int);
		bool save();
		bool strip(int);

		void showInfo() const;
		void listTags(bool) const;
		void printLameTag(bool);
		void extractAPICs(bool) const;

		int filenameToTag(const char*);
		int organize(const char*, bool = false, bool = false, struct timeval* = NULL) const;
};

#endif /* __MP3FILE_H__ */

