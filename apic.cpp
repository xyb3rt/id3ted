/* id3ted: apic.cpp
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
#include <cstdio>
#include <cstring>

#include <taglib/tbytevector.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/attachedpictureframe.h>

#include "id3ted.h"
#include "frameinfo.h"
#include "misc.h"
#include "mp3file.h"

bool APICFrameInfo::readFile() {
	if (_fileRead) return true;
	if (_readError) return false;

	const char *filename = _value.toCString(USE_UNICODE);
	FILE *fptr = fopen(filename, "r");
	if (fptr == NULL) {
		fprintf(stderr, "%s: %s: ", g_progname, filename);
		perror(NULL);
		_readError = true;
		return false;
	}

	const char *mimetype = getMimeType(filename);
	const char *mttemp = strstr(mimetype, "image");
	if (mttemp == NULL) {
		cerr << g_progname << ": " << filename << ": Wrong mime-type: " << mimetype << "! Not an image, not attached." << endl;
		fclose(fptr);
		_readError = true;
		return false;
	}

	fseek(fptr, 0, SEEK_END);
	size_t fileSize = ftell(fptr);
	fseek(fptr, 0, SEEK_SET);

	char *picFile = (char*) s_malloc(fileSize);
	fread(picFile, 1, fileSize, fptr);

	_mimetype = mimetype;
	_picture.setData(picFile, fileSize);

	fclose(fptr);
	free(picFile);

	_fileRead = true;

	return true;
}

bool APICFrameInfo::applyTo(MP3File &mp3File) {
	if (!mp3File.isValid() || mp3File.isReadOnly()) return false;

	if (!_fileRead) readFile();
	if (_readError) return false;

	if (!mp3File.hasID3v2Tag()) mp3File.createID3v2Tag();
	TagLib::ID3v2::Tag *id3v2Tag = mp3File._id3v2Tag;

	TagLib::ID3v2::FrameList frameList = id3v2Tag->frameListMap()[_id];

	TagLib::ID3v2::AttachedPictureFrame *apf = NULL;
	bool alreadyIn = false;

	ID3v2::FrameList::ConstIterator fl_it = frameList.begin();
	for (; fl_it != frameList.end() && !alreadyIn; fl_it++) {
		apf = dynamic_cast<ID3v2::AttachedPictureFrame*>(*fl_it);
		if (apf == NULL) continue;
		alreadyIn = _picture == apf->picture();
	}
				
	if (!alreadyIn) {
		apf = new ID3v2::AttachedPictureFrame();
		apf->setMimeType(_mimetype);
		apf->setType(ID3v2::AttachedPictureFrame::FrontCover);
		apf->setPicture(_picture);
		id3v2Tag->addFrame(apf);
	}

	return true;
}

void MP3File::extractAPICs(bool overwrite) const {
	if (!_file.isValid()) return;
	if (_id3v2Tag == NULL || _id3v2Tag->isEmpty()) return;

	const char *infname = basename(_file.name());
	int picNum = 0, infnlen = strlen(infname);
	char *outfile = NULL;
	const char *outftype, *mimetype;
	FILE *fptr;
	
	ID3v2::FrameList apicList = _id3v2Tag->frameListMap()["APIC"];
	ID3v2::FrameList::ConstIterator fl_it = apicList.begin();
	for (; fl_it != apicList.end() && picNum <= 20; fl_it++) {
		ID3v2::AttachedPictureFrame *apf = dynamic_cast<ID3v2::AttachedPictureFrame*>(*fl_it);
		if (apf == NULL) continue;
		picNum++;

		mimetype = apf->mimeType().toCString();
		if (mimetype != NULL && strlen(mimetype) > 0) {
			outftype = strrchr(mimetype, '/');
			if (outftype != NULL && strlen(outftype+1) > 0) {
				outftype++;
			} else {
				outftype = mimetype;
			}
		} else {
			outftype = "bin";
		}

		if (picNum < 2) {
			outfile = (char*) s_malloc(infnlen + 14);
			strcpy(outfile, infname);
			strcat(outfile, ".apic-");
			sprintf(outfile + infnlen + 6, "%02d", picNum);
			strcat(outfile, ".");
			strcat(outfile, outftype);
		} else {
			sprintf(outfile + infnlen + 6, "%02d", picNum);
			outfile[infnlen + 8] = '.';
			strcpy(outfile + infnlen + 9, outftype);
		}

		if (access(outfile, F_OK) == 0) {
			if (!overwrite && !confirm_overwrite(outfile)) {
				continue;
			}
		}
				
		if ((fptr = fopen(outfile, "w+")) == NULL) {
			cerr << g_progname << ": " << outfile << ": Could not open file for writing" << endl;
			continue;
		}

		ByteVector picData = apf->picture();
		unsigned int dataLen = picData.size();
		fwrite(picData.data(), dataLen, 1, fptr);
		fclose(fptr);
	}

	if (outfile != NULL) free(outfile);
}

