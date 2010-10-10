/* id3ted: mp3file.cpp
 * Copyright (c) 2010 Bert Muennich <muennich at informatik.hu-berlin.de>
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
#include <taglib/textidentificationframe.h>

#include "mp3file.h"
#include "frametable.h"

MP3File::MP3File(const char *filename, int tags, bool lameTag) : 
		_file(filename), _id3Tag(NULL), _id3v1Tag(NULL), _id3v2Tag(NULL),
		_lameTag(NULL), _tags(tags) {
	if (_file.isValid()) {
		_id3v1Tag = _file.ID3v1Tag(_tags & 1);
		_id3v2Tag = _file.ID3v2Tag(_tags & 2);
		_id3Tag = _file.tag();

		if (lameTag) {
			// TODO: calc offset
			long offset = 0L;
			_lameTag = new LameTag(filename, offset);
		}

		if (_tags == 0) {
			// tag version to write not given on command line
			// -> write only the tags already in the file
			if (_id3v1Tag != NULL && !_id3v1Tag->isEmpty())
				_tags |= 1;
			if (_id3v2Tag != NULL && !_id3v2Tag->isEmpty())
				_tags |= 2;
			if (_tags == 0)
				// no tags found -> use version 2 as default
				_tags = 2;
		}
	}
}

MP3File::~MP3File() {
	if (_lameTag != NULL)
		delete _lameTag;
}

void MP3File::apply(GenericInfo *genericInfo) {
	if (genericInfo == NULL)
		return;
	if (!_file.isValid() || _file.readOnly())
		return;
	if (_id3Tag == NULL) {
		_id3Tag = _file.tag();
		if (_id3Tag == NULL)
			return;
	}

	switch(genericInfo->id()) {
		case 'a': {
			_id3Tag->setArtist(genericInfo->value());
			break;
		}
		case 'A': {
			_id3Tag->setAlbum(genericInfo->value());
			break;
		}
		case 't': {
			_id3Tag->setTitle(genericInfo->value());
			break;
		}
		case 'c': {
			_id3Tag->setComment(genericInfo->value());
			break;
		}
		case 'g': {
			_id3Tag->setGenre(genericInfo->value());
			break;
		}
		case 'T': {
			if (_tags & 1) {
				int slash = genericInfo->value().find('/', 0);
				if (slash < 0)
					slash = genericInfo->value().length();
				_id3Tag->setTrack(genericInfo->value().substr(0, slash).toInt());
			}
			if (_tags & 2) {
				ID3v2::TextIdentificationFrame track(textFrameID(FID3_TRCK),
						DEF_TSTR_ENC);
				track.setText(genericInfo->value());
				apply(&track);
			}
			break;
		}
		case 'y': {
			_id3Tag->setYear(genericInfo->value().toInt());
			break;
		}
	}
}

void MP3File::removeFrames(const char *textFID) {
	if (textFID == NULL)
		return;
	if (!_file.isValid() || _file.readOnly())
		return;

	if (_id3v2Tag != NULL)
		_id3v2Tag->removeFrames(textFID);
}

bool MP3File::save() {
	if (!_file.isValid() || _file.readOnly())
		return false;

	// bug in TagLib 1.5.0?: deleting solely frame in id3v2 tag and
	// then saving file causes the recovery of the last deleted frame.
	// solution: strip the whole tag if it is empty before writing file!
	if (_tags & 2 && _id3v2Tag != NULL && _id3v2Tag->isEmpty())
		strip(2);

	return _file.save(_tags, false);
}

bool MP3File::strip(int tags) {
	if (!_file.isValid() || _file.readOnly())
		return false;

	return _file.strip(tags);
}

void MP3File::showInfo() const {
	MPEG::Properties *properties;
	const char *version;
	const char *channelMode;
	
	if (!_file.isValid())
		return;

	if ((properties = _file.audioProperties()) == NULL)
		return;

	switch (properties->version()) {
		case 1:
			version = "2";
			break;
		case 2:
			version = "2.5";
			break;
		default:
			version = "1";
			break;
	}

	switch (properties->channelMode()) {
		case 0:
			channelMode = "Stereo";
			break;
		case 1:
			channelMode = "JointStereo";
			break;
		case 2:
			channelMode = "DualChannel";
			break;
		default:
			channelMode = "SingleChannel";
			break;
	}

	int length = properties->length();
	cout << "MPEG " << version << " Layer " << properties->layer()
	     << " " << channelMode << endl;
	printf("bitrate: %d kBit/s, sample rate: %d Hz, length: %02d:%02d:%02d\n",
			properties->bitrate(), properties->sampleRate(),
			length / 3600, length / 60, length % 60);
}

void MP3File::listID3v1Tag() const {
	if (!_file.isValid())
		return;
	if (_id3v1Tag == NULL || _id3v1Tag->isEmpty())
		return;

	int year = _id3v1Tag->year();
	TagLib::String genreStr = _id3v1Tag->genre();
	int genre = ID3v1::genreIndex(genreStr);
	
	cout << "ID3v1" << ":\n";
	printf("Title  : %-30s  Track: %d\n",
			_id3v1Tag->title().toCString(USE_UNICODE), _id3v1Tag->track());
	printf("Artist : %-30s  Year : %-4s\n",
			_id3v1Tag->artist().toCString(USE_UNICODE),
			(year != 0 ? TagLib::String::number(year).toCString() : ""));
	printf("Album  : %-30s  Genre: %s (%d)\n",
			_id3v1Tag->album().toCString(USE_UNICODE),
			(genreIdx == 255 ? "Unknown" : genreStr.toCString()), genre);
	printf("Comment: %s\n", _id3v1Tag->comment().toCString(USE_UNICODE));
}

bool MP3File::listID3v2Tag(bool withDesc) const {
	if (!_file.isValid())
		return;
	if (_id3v2Tag == NULL || _id3v2Tag->isEmpty())
		return false;

	int frameCount = _id3v2Tag->frameList().size(); 
	cout << "ID3v2." << _id3v2Tag->header()->majorVersion() << " - "
	     << frameCount << (frameCount != 1 ? " frames:" : " frame:") << endl;
	
	ID3v2::FrameList::ConstIterator frame = _id3v2Tag->frameList().begin();
	for (; frame != _id3v2Tag->frameList().end(); ++frame) {
		String textFID((*frame)->frameID(), DEF_TSTR_ENC);

		cout << textFID;
		if (withDesc)
			cout << " (" << FrameTable::frameDescription(textFID) << ")";
		cout << ": ";
		
		switch (FrameTable::frameID(textFID)) {
			case FID3_APIC: {
				ID3v2::AttachedPictureFrame *apic =
						dynamic_cast<ID3v2::AttachedPictureFrame*>(*frame);
				if (apic != NULL) {
					int size = apic->picture().size();
					cout << apic->mimeType() << ", ";
					printSizeHumanReadable(size);
				}
				break;
			}
			case FID3_COMM: {
				ID3v2::CommentsFrame *comment =
						dynamic_cast<ID3v2::CommentsFrame*>(*frame);
				if (comment != NULL)
					cout << "[" << comment->description().toCString(USE_UNICODE)
					     << "](" << comment->language() << "): "
							 << comment->toString().toCString(USE_UNICODE);
				break;
			}
			case FID3_TCON: {
				String genreStr = (*frame)->toString();
				int genre = 255;

				sscanf(genreStr.toCString(), "(%d)", &genre);
				if (genre == 255)
					sscanf(genreStr.toCString(), "%d", &genre);
				if (genre != 255)
					genreStr = ID3v1::genre(genre);
				cout << genreStr;
				break;
			}
			case FID3_USLT: {
				ID3v2::UnsynchronizedLyricsFrame *lyrics =
						dynamic_cast<ID3v2::UnsynchronizedLyricsFrame*>(*frame);
				if (lyrics != NULL) {
					const char *text = ulf->text().toCString(USE_UNICODE);
					const char *indent = "    ";

					cout << "[" << lyrics->description().toCString(USE_UNICODE)
					     << "](" << lyrics->language().toCString(USE_UNICODE)
					     << "):\n" << indent;
					while (*text != '\0') {
						if (*text == (char) 10 || *text == (char) 13)
							cout << "\n" << indent;
						else
							putchar(*text);
						++text;
					}
				}
				break;
			}
			case FID3_TXXX: {
				ID3v2::UserTextIdentificationFrame *userText =
						dynamic_cast<ID3v2::UserTextIdentificationFrame*>(*frame);
				if (userText != NULL) {
					StringList textList = userText->fieldList();
					cout << "[" << userText->description().toCString(USE_UNICODE)
					     << "]: ";
					if (textList.size() > 1)
						cout << textList[1].toCString(USE_UNICODE);
				}
				break;
			}
			case FID3_WXXX: {
				ID3v2::UserUrlLinkFrame *userUrl = dynamic_cast<ID3v2::UserUrlLinkFrame*>(*frame);
				if (userUrl != NULL)
					cout << "[" << userUrl->description().toCString(USE_UNICODE)
					     << "]: " << userUrl->url().toCString(USE_UNICODE);
				break;
			}
			case FID3_XXXX: {
				break;
			}
			default:
				cout << (*frame)->toString().toCString(USE_UNICODE);
				break;
		}
		cout << endl;
	}
}

void MP3File::printLameTag(bool checkCRC) {
	if (!_file.isValid())
		return;
	
	if (_lameTag != NULL)
		lameTag->print(checkCRC);
}

