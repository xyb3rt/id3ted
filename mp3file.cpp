/* id3ted: mp3file.cpp
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

#include <iostream>
#include <sstream>
#include <cctype>
#include <cstdio>
#include <cstring>

#include <taglib/id3v1tag.h>
#include <taglib/id3v1genres.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/commentsframe.h>
#include <taglib/textidentificationframe.h>
#include <taglib/unsynchronizedlyricsframe.h>
#include <taglib/urllinkframe.h>

#include "mp3file.h"
#include "fileio.h"
#include "frametable.h"

MP3File::MP3File(const char *filename, int _tags, bool lame) :
		file(filename), id3Tag(NULL), id3v1Tag(NULL), id3v2Tag(NULL),
		lameTag(NULL), tags(_tags) {
	if (file.isValid()) {
		id3v1Tag = file.ID3v1Tag(tags & 1);
		id3v2Tag = file.ID3v2Tag(tags & 2);
		id3Tag = file.tag();

		if (tags == 0) {
			// tag version to write not given on command line
			// -> write only the tags already in the file
			if (id3v1Tag != NULL && !id3v1Tag->isEmpty())
				tags |= 1;
			if (id3v2Tag != NULL && !id3v2Tag->isEmpty())
				tags |= 2;
			if (tags == 0) {
				// no tags found -> use version 2 as default
				tags = 2;
				id3v2Tag = file.ID3v2Tag(true);
			}
		}

		if (lame) {
			long frameOffset, frameLength;
			if (id3v2Tag != NULL && !id3v2Tag->isEmpty())
				frameOffset = file.firstFrameOffset();
			else
				frameOffset = file.nextFrameOffset(0);
			frameLength = file.nextFrameOffset(frameOffset + 1) - frameOffset;
			lameTag = new LameTag(filename, frameOffset, frameLength);
		}
	}
}

MP3File::~MP3File() {
	if (lameTag != NULL)
		delete lameTag;
}

bool MP3File::hasLameTag() const {
	return lameTag != NULL && lameTag->isValid();
}

bool MP3File::hasID3v1Tag() const {
	return id3v1Tag != NULL && !id3v1Tag->isEmpty();
}

bool MP3File::hasID3v2Tag() const {
	return id3v1Tag != NULL && !id3v2Tag->isEmpty();
}

void MP3File::apply(GenericInfo *info) {
	if (info == NULL)
		return;
	if (!file.isValid() || file.readOnly())
		return;
	if (id3Tag == NULL) {
		id3Tag = file.tag();
		if (id3Tag == NULL)
			return;
	}

	switch(info->id()) {
		case 'a': {
			id3Tag->setArtist(info->value());
			break;
		}
		case 'A': {
			id3Tag->setAlbum(info->value());
			break;
		}
		case 't': {
			id3Tag->setTitle(info->value());
			break;
		}
		case 'c': {
			id3Tag->setComment(info->value());
			break;
		}
		case 'g': {
			id3Tag->setGenre(info->value());
			break;
		}
		case 'T': {
			if (tags & 1) {
				int slash = info->value().find('/', 0);
				if (slash < 0)
					slash = info->value().length();
				id3Tag->setTrack(info->value().substr(0, slash).toInt());
			}
			if (tags & 2) {
				FrameInfo trackInfo(FrameTable::textFrameID(FID3_TRCK),
						FID3_TRCK, info->value().toCString(USE_UTF8));
				apply(&trackInfo);
			}
			break;
		}
		case 'y': {
			id3Tag->setYear(info->value().toInt());
			break;
		}
	}
}

void MP3File::apply(FrameInfo *info) {
	if (!file.isValid() || file.readOnly())
		return;
	if (id3v2Tag == NULL || info == NULL)
		return;
	
	vector<ID3v2::Frame*> frameList = find(info);
	vector<ID3v2::Frame*>::iterator eachFrame = frameList.begin();

	if (info->text().isEmpty() && info->fid() != FID3_APIC) {
		if (!frameList.empty()) {
			for (; eachFrame != frameList.end(); ++eachFrame)
				id3v2Tag->removeFrame(*eachFrame);
		}
	} else {
		if (frameList.empty() || info->fid() == FID3_APIC) {
			switch (info->fid()) {
				case FID3_APIC: {
					ID3v2::AttachedPictureFrame *apic;
					for (; eachFrame != frameList.end(); ++eachFrame) {
						apic = dynamic_cast<ID3v2::AttachedPictureFrame*>(*eachFrame);
						if (apic != NULL && apic->picture() == info->data())
							return;
					}
					apic = new ID3v2::AttachedPictureFrame();
					apic->setMimeType(info->description());
					apic->setType(ID3v2::AttachedPictureFrame::FrontCover);
					apic->setPicture(info->data());
					id3v2Tag->addFrame(apic);
					break;
				}
				case FID3_COMM: {
					if (info->text().isEmpty())
						return;
					ID3v2::CommentsFrame *comment = new ID3v2::CommentsFrame(DEF_TSTR_ENC);
					comment->setText(info->text());
					comment->setDescription(info->description());
					comment->setLanguage(info->language());
					id3v2Tag->addFrame(comment);
					break;
				}
				case FID3_TXXX: {
					ID3v2::UserTextIdentificationFrame *userText =
							new ID3v2::UserTextIdentificationFrame(DEF_TSTR_ENC);
					userText->setText(info->text());
					userText->setDescription(info->description());
					id3v2Tag->addFrame(userText);
					break;
				}
				case FID3_USLT: {
					ID3v2::UnsynchronizedLyricsFrame *lyrics =
							new ID3v2::UnsynchronizedLyricsFrame(DEF_TSTR_ENC);
					lyrics->setText(info->text());
					lyrics->setDescription(info->description());
					lyrics->setLanguage(info->language());
					id3v2Tag->addFrame(lyrics);
					break;
				}
				case FID3_WCOM:
				case FID3_WCOP:
				case FID3_WOAF:
				case FID3_WOAR:
				case FID3_WOAS:
				case FID3_WORS:
				case FID3_WPAY:
				case FID3_WPUB: {
					ID3v2::UrlLinkFrame *urlLink = new ID3v2::UrlLinkFrame(info->id());
					urlLink->setUrl(info->text());
					id3v2Tag->addFrame(urlLink);
					break;
				}
				case FID3_WXXX: {
					ID3v2::UserUrlLinkFrame *userUrl =
							new ID3v2::UserUrlLinkFrame(DEF_TSTR_ENC);
					userUrl->setUrl(info->text());
					userUrl->setDescription(info->description());
					id3v2Tag->addFrame(userUrl);
					break;
				}
				default: {
					ID3v2::TextIdentificationFrame *textFrame =
							new ID3v2::TextIdentificationFrame(info->id(), DEF_TSTR_ENC);
					textFrame->setText(info->text());
					id3v2Tag->addFrame(textFrame);
					break;
				}
			}
		} else {
			frameList.front()->setText(info->text());
		}
	}
}

void MP3File::apply(const MatchInfo &info) {
	if (!file.isValid() || file.readOnly())
		return;
	if (info.id == 0 || info.text.length() == 0)
		return;

	switch (info.id) {
		case 'a':
		case 'A':
		case 't':
		case 'c':
		case 'g':
		case 'T':
		case 'y': {
			GenericInfo genInfo(info.id, info.text.c_str());
			apply(&genInfo);
			break;
		}
		case 'd': {
			if (id3v2Tag != NULL) {
				FrameInfo frameInfo("TPOS", FID3_TPOS, info.text.c_str());
				apply(&frameInfo);
			}
			break;
		}
	}
}

void MP3File::fill(MatchInfo &info) {
	string &text = info.text;
	ostringstream tmp;

	tmp.fill('0');

	if (!file.isValid())
		return;
	if (info.id == 0)
		return;

	switch (info.id) {
		case 'a':
			text = id3Tag->artist().toCString(USE_UTF8);
			if (text.empty())
				text = "Unknown Artist";
			break;
		case 'A':
			text = id3Tag->album().toCString(USE_UTF8);
			if (text.empty())
				text = "Unknown Album";
			break;
		case 't':
			text = id3Tag->title().toCString(USE_UTF8);
			if (text.empty())
				text = "Unknown Title";
			break;
		case 'g':
			text = id3Tag->genre().toCString(USE_UTF8);
			break;
		case 'y': {
			uint year = id3Tag->year();
			if (year) {
				tmp << year;
				text = tmp.str();
			}
			break;
		}
		case 'T': {
			uint track = id3Tag->track();
			if (track) {
				tmp.width(2);
				tmp << track;
				text = tmp.str();
			}
			break;
		}
		case 'd': {
			if (id3v2Tag != NULL) {
				ID3v2::FrameList list = id3v2Tag->frameListMap()["TPOS"];
				if (!list.isEmpty()) {
					uint disc = list.front()->toString().toInt();
					if (disc) {
						tmp << disc;
						text = tmp.str();
					}
				}
			}
			break;
		}
	}
}

void MP3File::removeFrames(const char *textFID) {
	if (textFID == NULL)
		return;
	if (!file.isValid() || file.readOnly())
		return;

	if (id3v2Tag != NULL)
		id3v2Tag->removeFrames(textFID);
}

bool MP3File::save() {
	if (!file.isValid() || file.readOnly())
		return false;

	// bug in TagLib 1.5.0?: deleting solely frame in id3v2 tag and
	// then saving file causes the recovery of the last deleted frame.
	// solution: strip the whole tag if it is empty before writing file!
	if (tags & 2 && id3v2Tag != NULL && id3v2Tag->isEmpty())
		strip(2);

	return file.save(tags, false);
}

bool MP3File::strip(int tags) {
	if (!file.isValid() || file.readOnly())
		return false;

	return file.strip(tags);
}

void MP3File::showInfo() const {
	MPEG::Properties *properties;
	const char *version;
	const char *channelMode;

	if (!file.isValid())
		return;

	if ((properties = file.audioProperties()) == NULL)
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
	printf("MPEG %s Layer %d %s\n", version, properties->layer(), channelMode);
	printf("bitrate: %d kBit/s, sample rate: %d Hz, length: %02d:%02d:%02d\n",
			properties->bitrate(), properties->sampleRate(),
			length / 3600, length / 60, length % 60);
}

void MP3File::printLameTag(bool checkCRC) const {
	if (!file.isValid())
		return;

	if (lameTag != NULL)
		lameTag->print(checkCRC);
}

void MP3File::listID3v1Tag() const {
	if (!file.isValid())
		return;
	if (id3v1Tag == NULL || id3v1Tag->isEmpty())
		return;

	int year = id3v1Tag->year();
	TagLib::String genreStr = id3v1Tag->genre();
	int genre = ID3v1::genreIndex(genreStr);
	
	printf("ID3v1:\n");
	printf("Title  : %-30s  Track: %d\n",
			id3v1Tag->title().toCString(USE_UTF8), id3v1Tag->track());
	printf("Artist : %-30s  Year : %-4s\n",
			id3v1Tag->artist().toCString(USE_UTF8),
			(year != 0 ? TagLib::String::number(year).toCString() : ""));
	printf("Album  : %-30s  Genre: %s (%d)\n",
			id3v1Tag->album().toCString(USE_UTF8),
			(genre == 255 ? "Unknown" : genreStr.toCString()), genre);
	printf("Comment: %s\n", id3v1Tag->comment().toCString(USE_UTF8));
}

void MP3File::listID3v2Tag(bool withDesc) const {
	if (!file.isValid())
		return;
	if (id3v2Tag == NULL || id3v2Tag->isEmpty())
		return;

	int frameCount = id3v2Tag->frameList().size(); 
	cout << "ID3v2." << id3v2Tag->header()->majorVersion() << " - "
	     << frameCount << (frameCount != 1 ? " frames:" : " frame:") << endl;
	
	ID3v2::FrameList::ConstIterator frame = id3v2Tag->frameList().begin();
	for (; frame != id3v2Tag->frameList().end(); ++frame) {
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
					cout << apic->mimeType() << ", " << FileIO::sizeHumanReadable(size);
				}
				break;
			}
			case FID3_COMM: {
				ID3v2::CommentsFrame *comment =
						dynamic_cast<ID3v2::CommentsFrame*>(*frame);
				if (comment != NULL) {
					TagLib::ByteVector lang = comment->language();
					bool showLanguage = lang.size() == 3 && isalpha(lang[0]) && 
					                    isalpha(lang[1]) && isalpha(lang[2]);

					cout << "[" << comment->description().toCString(USE_UTF8) << "]";
					if (showLanguage)
						cout << "(" << lang[0] << lang[1] << lang[2];
					else
						cout << "(XXX";
					cout << "): " << comment->toString().toCString(USE_UTF8);
				}
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
					const char *text = lyrics->text().toCString(USE_UTF8);
					const char *indent = "    ";
					TagLib::ByteVector lang = lyrics->language();
					bool showLanguage = lang.size() == 3 && isalpha(lang[0]) && 
					                    isalpha(lang[1]) && isalpha(lang[2]);

					cout << "[" << lyrics->description().toCString(USE_UTF8) << "]";
					if (showLanguage)
						cout << "(" << lang[0] << lang[1] << lang[2];
					else
						cout << "(XXX";
					cout << "):\n" << indent;
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
					cout << "[" << userText->description().toCString(USE_UTF8)
					     << "]: ";
					if (textList.size() > 1)
						cout << textList[1].toCString(USE_UTF8);
				}
				break;
			}
			case FID3_WXXX: {
				ID3v2::UserUrlLinkFrame *userUrl =
						dynamic_cast<ID3v2::UserUrlLinkFrame*>(*frame);
				if (userUrl != NULL)
					cout << "[" << userUrl->description().toCString(USE_UTF8)
					     << "]: " << userUrl->url().toCString(USE_UTF8);
				break;
			}
			case FID3_XXXX: {
				break;
			}
			default:
				cout << (*frame)->toString().toCString(USE_UTF8);
				break;
		}
		cout << endl;
	}
}

void MP3File::extractAPICs(bool overwrite) const {
	if (!file.isValid() || id3v2Tag == NULL)
		return;

	int num = 0;
	const char *mimetype, *filetype;
	ostringstream filename;

	ID3v2::FrameList apicList = id3v2Tag->frameListMap()["APIC"];
	ID3v2::FrameList::ConstIterator each = apicList.begin();

	for (; each != apicList.end(); ++each) {
		ID3v2::AttachedPictureFrame *apic =
				dynamic_cast<ID3v2::AttachedPictureFrame*>(*each);
		if (apic == NULL)
			continue;

		mimetype = apic->mimeType().toCString();
		if (mimetype != NULL && strlen(mimetype) > 0) {
			filetype = strrchr(mimetype, '/');
			if (filetype != NULL && strlen(filetype+1) > 0)
				++filetype;
			else
				filetype = mimetype;
		} else {
			filetype = "bin";
		}

		filename.str("");
		filename << file.name() << ".apic-" << (++num < 10 ? "0" : "");
		filename << num << "." << filetype;

		if (FileIO::exists(filename.str().c_str())) {
			if (!overwrite && !FileIO::confirmOverwrite(filename.str().c_str()))
				continue;
		}

		OFile outFile(filename.str().c_str());
		if (!outFile.isOpen())
			continue;

		outFile.write(apic->picture());
		if (outFile.error())
			warn("%s: Could not write file", filename.str().c_str());

		outFile.close();
	}
}

vector<ID3v2::Frame*> MP3File::find(FrameInfo *info) {
	vector<ID3v2::Frame*> list;

	if (id3v2Tag == NULL || info == NULL)
		return list;
	
	ID3v2::FrameList frameList = id3v2Tag->frameListMap()[info->id()];
	ID3v2::FrameList::ConstIterator each = frameList.begin();

	for (; each != frameList.end(); ++each) {
		switch (FrameTable::frameID((*each)->frameID())) {
			case FID3_APIC: {
				ID3v2::AttachedPictureFrame *apic =
						dynamic_cast<ID3v2::AttachedPictureFrame*>(*each);
				if (apic == NULL)
					continue;
				if (info->data() == apic->picture() &&
						info->description() == apic->mimeType())
					list.push_back(*each);
				break;
			}
			case FID3_COMM: {
				ID3v2::CommentsFrame *comment =
						dynamic_cast<ID3v2::CommentsFrame*>(*each);
				if (comment == NULL)
					continue;
				if (comment->language().isEmpty())
					comment->setLanguage("XXX");
				if (info->description() == comment->description() &&
						info->language() == comment->language())
					list.push_back(*each);
				break;
			}
			case FID3_TXXX: {
				ID3v2::UserTextIdentificationFrame *userText =
						dynamic_cast<ID3v2::UserTextIdentificationFrame*>(*each);
				if (userText == NULL)
					continue;
				if (info->description() == userText->description())
					list.push_back(*each);
				break;
			}
			case FID3_USLT: {
				ID3v2::UnsynchronizedLyricsFrame *lyrics =
						dynamic_cast<ID3v2::UnsynchronizedLyricsFrame*>(*each);
				if (lyrics == NULL)
					continue;
				if (lyrics->language().isEmpty())
					lyrics->setLanguage("XXX");
				if (info->description() == lyrics->description() &&
						info->language() == lyrics->language())
					list.push_back(*each);
				break;
			}
			case FID3_WXXX: {
				ID3v2::UserUrlLinkFrame *userUrl =
						dynamic_cast<ID3v2::UserUrlLinkFrame*>(*each);
				if (userUrl == NULL)
					continue;
				if (info->description() == userUrl->description())
					list.push_back(*each);
				break;
			}
			default:
				list.push_back(*each);
				break;
		}
	}
	return list;
}
