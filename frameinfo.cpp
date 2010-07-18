/* id3ted: frameinfo.cpp
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

#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/commentsframe.h>
#include <taglib/textidentificationframe.h>
#include <taglib/unsynchronizedlyricsframe.h>
#include <taglib/urllinkframe.h>

#include "frameinfo.h"
#include "mp3file.h"

GenericInfo::GenericInfo(const char id, const char *value) :
			_id(id), _value(value) {}

bool GenericInfo::sameIDIn(const std::vector<GenericInfo*> &list) {
	std::vector<GenericInfo*>::const_iterator it = list.begin();
	for (; it != list.end(); it++) {
		if (this->_id == (*it)->_id) return true;
	}

	return false;
}

FrameInfo::FrameInfo(const char *id, ID3v2FrameID fid, const char *value, unsigned int fPathIdx) :
			_id(id), _fid(fid), _value(value, DEF_TSTR_ENC), _fPathIdx(fPathIdx) {}

bool FrameInfo::sameFIDIn(const std::vector<FrameInfo*> &list) {
	std::vector<FrameInfo*>::const_iterator it = list.begin();
	for (; it != list.end(); it++) {
		if (this->_fid == (*it)->_fid) return true;
	}

	return false;
}

bool FrameInfo::applyTo(MP3File &mp3File) {
	if (!mp3File.isValid() || mp3File.isReadOnly()) return false;

	if (!mp3File.hasID3v2Tag()) mp3File.createID3v2Tag();
	TagLib::ID3v2::Tag *id3v2Tag = mp3File._id3v2Tag;

	TagLib::ID3v2::FrameList frameList = id3v2Tag->frameListMap()[_id];
	TagLib::String input;

	if (_fPathIdx > 0) {
		if (g_fPathPMatch == NULL) return false;
		if (g_fPathPMatch[_fPathIdx].rm_so == -1 || g_fPathPMatch[_fPathIdx].rm_eo == -1) return false;

		input = TagLib::String(mp3File.filename(), DEF_TSTR_ENC).substr(g_fPathPMatch[_fPathIdx].rm_so, g_fPathPMatch[_fPathIdx].rm_eo - g_fPathPMatch[_fPathIdx].rm_so);
	} else {
		input = _value;
	}

	if (input.length() > 0) {
		if (!frameList.isEmpty()) {
			frameList.front()->setText(input);
		} else {
			TagLib::ID3v2::Frame *newFrame;
			switch (_fid) {
				case FID3_WCOM:
				case FID3_WCOP:
				case FID3_WOAF:
				case FID3_WOAR:
				case FID3_WOAS:
				case FID3_WORS:
				case FID3_WPAY:
				case FID3_WPUB: {
					newFrame = new TagLib::ID3v2::UrlLinkFrame(_id);
					break;
				}
				default: {
					newFrame = new TagLib::ID3v2::TextIdentificationFrame(_id);
					break;
				}
			}
			newFrame->setText(input);
			id3v2Tag->addFrame(newFrame);
		}
	} else if (!frameList.isEmpty()) {
		id3v2Tag->removeFrame(frameList.front());
	}

	return true;
}

APICFrameInfo::APICFrameInfo(const char *id, ID3v2FrameID fid, const char *value, unsigned int fPathIdx) :
			FrameInfo(id, fid, value, fPathIdx), _mimetype(), _picture(), _fileRead(false), _readError(false) {}

// implementation of APICFrameInfo::readFile() in apic.cpp
// implementation of APICFrameInfo::applyTo() in apic.cpp

TDFrameInfo::TDFrameInfo(const char *id, ID3v2FrameID fid, const char *value, unsigned int fPathIdx) :
			FrameInfo(id, fid, value, fPathIdx), _text(), _description(), _multipleFields(false) {
	int inputLen = _value.length();
	int textLen = inputLen, descLen;
	int descIdx = _value.find(g_fdelim, 0);
	
	if (descIdx != -1) {
		_multipleFields = true;
		textLen = descIdx++;
		descLen = inputLen - descIdx;
		_description = _value.substr(descIdx, descLen);
	}

	_text = _value.substr(0, textLen);
}

bool TDFrameInfo::applyTo(MP3File &mp3File) {
	if (!mp3File.isValid() || mp3File.isReadOnly()) return false;

	if (!mp3File.hasID3v2Tag()) mp3File.createID3v2Tag();
	TagLib::ID3v2::Tag *id3v2Tag = mp3File._id3v2Tag;

	TagLib::ID3v2::FrameList frameList = id3v2Tag->frameListMap()[_id];

	int textLength = _text.length();
	bool alreadyIn = false;
	ID3v2::UserUrlLinkFrame *uulf;
	ID3v2::UserTextIdentificationFrame *utif;

	// looking for a frame with the same type and description
	// found it -> overwrite it:
	ID3v2::FrameList::ConstIterator it = frameList.begin();
	while (it != frameList.end()) {
		if (_fid == FID3_TXXX) {
			if ((utif = dynamic_cast<ID3v2::UserTextIdentificationFrame*>(*it)) == NULL) continue;
			alreadyIn = _description == utif->description();
		} else {
			if ((uulf = dynamic_cast<ID3v2::UserUrlLinkFrame*>(*it)) == NULL) continue;
			alreadyIn = _description == uulf->description();
		}

		if (alreadyIn) {
			if (textLength > 0) {
				(*it)->setText(_text);
			} else {
				id3v2Tag->removeFrame(*it);
			}
			break;
		}
		it++;
	}
			
	// no frame with the same type, description and language
	// have to add a new one:
	if (!alreadyIn && textLength > 0) {
		if (_fid == FID3_TXXX) {
			utif = new ID3v2::UserTextIdentificationFrame(DEF_TSTR_ENC);
			utif->setDescription(_description);
			utif->setText(_text);
			id3v2Tag->addFrame(utif);
		} else {
			uulf = new ID3v2::UserUrlLinkFrame(DEF_TSTR_ENC);
			uulf->setDescription(_description);
			uulf->setUrl(_text);
			id3v2Tag->addFrame(uulf);
		}
	}

	return true;
}

TDLFrameInfo::TDLFrameInfo(const char *id, ID3v2FrameID fid, const char *value, unsigned int fPathIdx) :
			TDFrameInfo(id, fid, value, fPathIdx), _language("XXX", DEF_TSTR_ENC) {
	if (_multipleFields) {
		int descLen = _description.length();
		int langLen = descLen;
		int langIdx = _description.find(g_fdelim, 0);

		if (langIdx != -1) {
			descLen = langIdx++;
			langLen -= langIdx;
			
			if (langLen == 3) _language = _description.substr(langIdx, langLen);
			_description = _description.substr(0, descLen);
		}
	}
}

bool TDLFrameInfo::applyTo(MP3File &mp3File) {
	if (!mp3File.isValid() || mp3File.isReadOnly()) return false;

	if (!mp3File.hasID3v2Tag()) mp3File.createID3v2Tag();
	TagLib::ID3v2::Tag *id3v2Tag = mp3File._id3v2Tag;

	TagLib::ID3v2::FrameList frameList = id3v2Tag->frameListMap()[_id];

	int textLength = _text.length();
	bool alreadyIn = false;
	ID3v2::CommentsFrame *cf;
	ID3v2::UnsynchronizedLyricsFrame *ulf;

	// looking for a frame with the same type, description and language
	// found it -> overwrite it:
	ID3v2::FrameList::ConstIterator it = frameList.begin();
	while (it != frameList.end()) {
		if (_fid == FID3_COMM) {
			if ((cf = dynamic_cast<ID3v2::CommentsFrame*>(*it)) == NULL) continue;
			if (cf->language().isNull() || cf->language().isEmpty()) {
				cf->setLanguage("XXX");
			}
			alreadyIn = _description == cf->description() && _language == cf->language();
		} else {	
			if ((ulf = dynamic_cast<ID3v2::UnsynchronizedLyricsFrame*>(*it)) == NULL) continue;
			if (ulf->language().isNull() || ulf->language().isEmpty()) {
				ulf->setLanguage("XXX");
			}
			alreadyIn = _description == ulf->description() && _language == ulf->language();
		}

		if (alreadyIn) {
			if (textLength > 0) {
				(*it)->setText(_text);
			} else {
				id3v2Tag->removeFrame(*it);
			}
			break;
		}
		it++;
	}

	// no frame with the same type, description and language
	// have to add a new one:
	if (!alreadyIn && textLength > 0) {
		if (_fid == FID3_COMM) {
			cf = new ID3v2::CommentsFrame(DEF_TSTR_ENC);
			cf->setText(_text);
			cf->setDescription(_description);
			cf->setLanguage(_language.data(DEF_TSTR_ENC));
			id3v2Tag->addFrame(cf);
		} else {
			ulf = new ID3v2::UnsynchronizedLyricsFrame(DEF_TSTR_ENC);
			ulf->setText(_text);
			ulf->setDescription(_description);
			ulf->setLanguage(_language.data(DEF_TSTR_ENC));
			id3v2Tag->addFrame(ulf);
		} 
	}

	return true;
}

