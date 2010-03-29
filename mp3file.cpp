/* id3ted: mp3file.cpp
 * Copyright (c) 2009 Bert Muennich <muennich at informatik.hu-berlin.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <iostream>

#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v1tag.h>

#include "mp3file.h"
#include "frameinfo.h"
#include "frametable.h"
#include "misc.h"

MP3File::MP3File(const char *filename) :
			_file(filename), _id3Tag(NULL), _id3v1Tag(NULL), _id3v2Tag(NULL), _tags(0) {
	if (_file.isValid()) {
		_id3Tag = _file.tag();
		_id3v1Tag = _file.ID3v1Tag();
		_id3v2Tag = _file.ID3v2Tag();

		if (_id3v1Tag != NULL && !_id3v1Tag->isEmpty()) _tags |= 1;
		if (_id3v2Tag != NULL && !_id3v2Tag->isEmpty()) _tags |= 2;
	}
}

void MP3File::createID3v2Tag() {
	if (_file.isValid() && !_file.readOnly()) {
		_id3v2Tag == _file.ID3v2Tag(true);
		_tags |= 2;
	}
}

// implementation of MP3File::listID3v1Tag() in list.cpp
// implementation of MP3File::listID3v2Tag() in list.cpp

void MP3File::apply(GenericInfo *genericInfo) {
	if (genericInfo == NULL) return;
	if (!_file.isValid() || _file.readOnly()) return;

	if (_id3Tag == NULL) {
		_id3Tag = _file.tag();
		if (_id3Tag == NULL) return;
	}

	if (_tags == 0) _tags = 2;
	TagLib::String input(genericInfo->value(), DEF_TSTR_ENC);

	switch(genericInfo->id()) {
		case 'a': {
			_id3Tag->setArtist(input);
			break;
		}
		case 'A': {
			_id3Tag->setAlbum(input);
			break;
		}
		case 't': {
			_id3Tag->setTitle(input);
			break;
		}
		case 'c': {
			_id3Tag->setComment(input);
			break;
		}
		case 'g': {
			_id3Tag->setGenre(input);
			break;
		}
		case 'T': {
			if (_tags & 1) {
				int slashIdx = input.find('/', 0);
				if (slashIdx < 0) slashIdx = input.length();
				_id3Tag->setTrack(input.substr(0, slashIdx).toInt());
			}

			if (_tags & 2) {
				FrameInfo frameInfo(FrameTable::textFrameID(FID3_TRCK), FID3_TRCK, genericInfo->value());
				frameInfo.applyTo(*this);
			}

			break;
		}
		case 'y': {
			_id3Tag->setYear(input.toInt());
			break;
		}
	}
}

void MP3File::removeFrames(const char *textFID) {
	if (textFID == NULL) return;
	if (!_file.isValid() || _file.readOnly()) return;

	if (_id3v2Tag != NULL) {
		_id3v2Tag->removeFrames(textFID);
	}
}

bool MP3File::save(int tags) {
	if (!_file.isValid() || _file.readOnly()) return false;

	// bug in TagLib 1.5.0?: deleting solely frame in id3v2 tag and
	// then saving file causes the recovery of the last deleted frame.
	// solution: strip the whole tag if it is empty before writing file!
	if ((tags & 2) && _id3v2Tag != NULL && _id3v2Tag->isEmpty()) strip(2);

	bool succ = _file.save(tags, false);

	_id3v1Tag = _file.ID3v1Tag();
	_id3v2Tag = _file.ID3v2Tag();
	if (_id3v1Tag != NULL && !_id3v1Tag->isEmpty()) _tags |= 1;
	if (_id3v2Tag != NULL && !_id3v2Tag->isEmpty()) _tags |= 2;

	return succ;
}

bool MP3File::save() {
	if (!_file.isValid() || _file.readOnly()) return false;

	if (_tags) {
		return save(_tags);
	} else {
		return strip(3);
	}
}

bool MP3File::strip(int tags) {
	if (!_file.isValid() || _file.readOnly()) return false;

	bool succ = _file.strip(tags);
	if (succ) {
		if (tags & 1) _id3v1Tag = NULL;
		if (tags & 2) _id3v2Tag = NULL;
		if (tags & 3) _id3Tag = NULL;
	} else {
		if (tags & 1) _id3v1Tag = _file.ID3v1Tag();
		if (tags & 2) _id3v2Tag = _file.ID3v2Tag();
		if (tags & 3) _id3Tag = _file.tag();
	}

	if (_id3v1Tag == NULL || _id3v1Tag->isEmpty()) _tags &= ~1;
	if (_id3v2Tag == NULL || _id3v2Tag->isEmpty()) _tags &= ~2;

	return succ;
}

// implementation of MP3File::showInfo() in list.cpp
// implementation of MP3File::listTags() in list.cpp

// implementation of MP3File::printLameTag() in lametag.cpp

// implementation of MP3File::extractAPICs() in apic.cpp

// implementation of MP3File::filenameToTag() in filename.cpp
// implementation of MP3File::organize() in filename.cpp

