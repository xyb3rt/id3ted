/* id3ted: list.cpp
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

#include <cstdlib>
#include <cstdio>
#include <iostream>

#include <taglib/mpegfile.h>
#include <taglib/id3v1tag.h>
#include <taglib/id3v1genres.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/commentsframe.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/textidentificationframe.h>
#include <taglib/urllinkframe.h>
#include <taglib/unsynchronizedlyricsframe.h>

#include "common.h"
#include "list.h"
#include "frametable.h"
#include "mp3file.h"

void printGenreList() {
	int i;

	for (i = 0; i < ID3v1::genreList().size(); i++) {
		printf("  %5d: ", i);
		cout << ID3v1::genre(i) << endl;
	}
}

bool MP3File::listID3v1Tag() const {
	if (_id3v1Tag == NULL || _id3v1Tag->isEmpty()) {
		return false;
	}

	int year = _id3v1Tag->year();
	TagLib::String genreStr = _id3v1Tag->genre();
	int genreIdx = ID3v1::genreIndex(genreStr);
	
	cout << "ID3v1" << ":\n";

	printf("Title  : %-30s  Track: %d\n",
			_id3v1Tag->title().toCString(USE_UNICODE), _id3v1Tag->track());
	
	printf("Artist : %-30s  Year : %-4s\n",
			_id3v1Tag->artist().toCString(USE_UNICODE),
			(year != 0 ? TagLib::String::number(year).toCString() : ""));

	printf("Album  : %-30s  Genre: %s (%d)\n",
			_id3v1Tag->album().toCString(USE_UNICODE),
			(genreIdx == 255 ? "Unknown" : genreStr.toCString()), genreIdx);

	printf("Comment: %s\n", _id3v1Tag->comment().toCString(USE_UNICODE));

	return true;
}

bool MP3File::listID3v2Tag(bool withDesc) const {
	if (_id3v2Tag == NULL || _id3v2Tag->isEmpty()) {
		return false;
	}

	int frameCount = _id3v2Tag->frameList().size(); 

	cout << "ID3v2." << _id3v2Tag->header()->majorVersion() << " - " << frameCount << (frameCount != 1 ? " frames:" : " frame:") << "\n";
	
	ID3v2::FrameList::ConstIterator it = _id3v2Tag->frameList().begin();
	for (; it != _id3v2Tag->frameList().end(); it++) {
		String textFID((*it)->frameID(), DEF_TSTR_ENC);
		const char *ctextFID = textFID.toCString(USE_UNICODE);

		cout << textFID;
		if (withDesc) cout << " (" << FrameTable::frameDescription(ctextFID) << ")";
		cout << ": ";
		
		switch (FrameTable::frameID(ctextFID)) {
			case FID3_APIC: {
				ID3v2::AttachedPictureFrame *apf = dynamic_cast<ID3v2::AttachedPictureFrame*>(*it);
				if (apf != NULL) {
					int picSize = apf->picture().size();
					cout << apf->mimeType() << ", ";
					printSizeHumanReadable(picSize);
				}

				break;
			}
			case FID3_COMM: {
				ID3v2::CommentsFrame *cf = dynamic_cast<ID3v2::CommentsFrame*>(*it);
				if (cf != NULL) cout << "[" << cf->description().toCString(USE_UNICODE) << "](" << cf->language() << "): " << cf->toString().toCString(USE_UNICODE);

				break;
			}
			case FID3_TCON: {
				String genreStr = (*it)->toString();
				int genreIdx = 255;
				sscanf(genreStr.toCString(), "(%d)", &genreIdx);

				if (genreIdx == 255) {
					sscanf(genreStr.toCString(), "%d", &genreIdx);
				}
				if (genreIdx != 255) {
					genreStr = ID3v1::genre(genreIdx);
				}

				cout << genreStr;
				break;
			}
			case FID3_USLT: {
				ID3v2::UnsynchronizedLyricsFrame *ulf = dynamic_cast<ID3v2::UnsynchronizedLyricsFrame*>(*it);
				if (ulf != NULL) {
					const char *lyrics = ulf->text().toCString(USE_UNICODE);
					const char *indent = "    ";

					cout << "[" << ulf->description() << "](" << ulf->language() << "):\n" << indent;
					while (*lyrics != '\0') {
						if (*lyrics == (char) 10 || *lyrics == (char) 13) {
							cout << "\n" << indent;
						} else {
							putchar(*lyrics);
						}
						lyrics++;
					}
				}

				break;
			}
			case FID3_TXXX: {
				ID3v2::UserTextIdentificationFrame *utif = dynamic_cast<ID3v2::UserTextIdentificationFrame*>(*it);
				if (utif != NULL) {
					StringList textList = utif->fieldList();
					cout << "[" << utif->description() << "]: ";
					if (textList.size() > 1) cout << textList[1].toCString(USE_UNICODE);
				}

				break;
			}
			case FID3_WXXX: {
				ID3v2::UserUrlLinkFrame *uulf = dynamic_cast<ID3v2::UserUrlLinkFrame*>(*it);
				if (uulf != NULL) cout << "[" << uulf->description() << "]: " << uulf->url().toCString(USE_UNICODE);

				break;
			}
			case FID3_XXXX: {
				break;
			}
			default: {
				cout << (*it)->toString().toCString(USE_UNICODE);
				break;
			}
		}

		cout << endl;
	}
	
	return true;
}

void MP3File::listTags(bool v2WithDesc) const {
	if (!_file.isValid()) return;

	int tagsListed = 0;

	if (listID3v1Tag()) {
		tagsListed |= 1;
	}

	if (listID3v2Tag(v2WithDesc)) {
		tagsListed |= 2;
	}

	if (tagsListed == 0) {
		cout << "No id3 tag found" << endl;
	}
}

void MP3File::showInfo() const {
	if (!_file.isValid()) return;

	MPEG::Properties *properties = _file.audioProperties();
	const char *version;
	const char *channelMode;
	
	if (properties == NULL) return;

	switch (properties->version()) {
		case 1: {
			version = "2";
			break;
		}
		case 2: {
			version = "2.5";
			break;
		}
		default: {
			version = "1";
			break;
		}
	}

	switch (properties->channelMode()) {
		case 0: {
			channelMode = "Stereo";
			break;
		}
		case 1: {
			channelMode = "JointStereo";
			break;
		}
		case 2: {
			channelMode = "DualChannel";
			break;
		}
		default: {
			channelMode = "SingleChannel";
			break;
		}
	}

	cout << "MPEG " << version << " Layer " << properties->layer() << " " << channelMode << endl;

	int length = properties->length();
	printf("bitrate: %d kBit/s, sample rate: %d Hz, length: %02d:%02d:%02d\n",
			properties->bitrate(), properties->sampleRate(),
			length / 3600, length / 60, length % 60);
}

void printSizeHumanReadable(unsigned int size) {
	float size_hr = size;
	const char *unit;

	if (size_hr >= 1073741824) {
		size_hr /= 1073741824;
		unit = "GB";
	} else if (size_hr >= 1048576) {
		size_hr /= 1048576;
		unit = "MB";
	} else if (size_hr >= 1024) {
		size_hr /= 1024;
		unit = "KB";
	}

	if (size == size_hr) {
		cout << size << " bytes";
	} else {
		printf("%.2f %s (%d bytes)", size_hr, unit, size);
	}
}

