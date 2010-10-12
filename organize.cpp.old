/* id3ted: organize.cpp
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
#include <string>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <taglib/tstring.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>

#include "id3ted.h"
#include "misc.h"
#include "mp3file.h"

/* return values:
 *   0: everything went fine
 *   1: pattern wrong, -o option should be ignored
 *   2: error
 *   3: mp3 file not valid
 */
int MP3File::organize(const char *pattern, bool move, bool overwrite, struct timeval *ptimes) const {
	if (!_file.isValid())
		return 3;

	if (pattern[strlen(pattern) - 1] == '/') {
		cerr << g_progname << ": -o option ignored, because file pattern ends with a slash" << endl;
		return 1;
	}

	TagLib::Tag *tag = _id3Tag;
	if (tag == NULL) {
		tag = _file.tag();
		if (tag == NULL) {
			cerr << g_progname << ": " << _file.name() << ": No id3 tag found" << endl;
			return 2;
		}
	}

	bool wcardPresent = false;
	std::string path(pattern);
	std::ostringstream newPath;

	/* replace wildcards in pattern with corresponding tag information */
	std::string::iterator path_it = path.begin();
	while (path_it != path.end()) {

		if (*path_it == '%') {
			switch (*(path_it + 1)) {
				case '%': {
					newPath << *path_it;
					break;
				}
				case 'a': {
					TagLib::String artist = tag->artist();
					if (artist == TagLib::String::null)
						newPath << "Unknown Artist";
					else
						newPath << artist.toCString(USE_UNICODE);
					break;
				}
				case 'A': {
					TagLib::String album = tag->album();
					if (album == TagLib::String::null)
						newPath << "Unknown Album";
					else
						newPath << album.toCString(USE_UNICODE);
					break;
				}
				case 't': {
					TagLib::String title = tag->title();
					if (title == TagLib::String::null)
						newPath << "Unknown Title";
					else
						newPath << title.toCString(USE_UNICODE);
					break;
				}
				case 'g': {
					TagLib::String genre = tag->genre();
					if (!(genre == TagLib::String::null))
						newPath << genre.toCString(USE_UNICODE);
					break;
				}
				case 'y': {
					unsigned int year = tag->year();
					if (year) {
						newPath << year;
					}
					break;
				}
				case 'n': {
					unsigned int track = tag->track();
					if (track) {
						if (track < 10)
							newPath << 0;
						newPath << track;
					}
					break;
				}
				case 'd': {
					int disc = 0;
					if (_id3v2Tag != NULL) {
						ID3v2::FrameList discNumList = _id3v2Tag->frameListMap()["TPOS"];
						if (!discNumList.isEmpty())
							disc = discNumList.front()->toString().toInt();
					}
					if (disc) {
						if (disc < 10)
							newPath << 0; 
						newPath << disc;
					}
					break;
				}
				default: {
					cerr << g_progname << ": -o option ignored, because pattern contains invalid wildcard: " << *path_it << *(path_it+1) << endl;
					return 1;
				}
			}

			wcardPresent = true;
			path_it += 2;
		} else {
			newPath << *path_it++;
		}
	}

	if (!wcardPresent && !overwrite && g_numFiles > 1) {
		cerr << g_progname << ": -o option ignored, because pattern does not contain any wildcard and " << g_numFiles << " files given. Last file would overwrite all other. Use -F to make this happen" << endl;
		return 1;
	}

	path = newPath.str();
	trim_whitespace(path);

	const char *oldFile = _file.name();
	const char *newFile = path.c_str();
	if (strcmp(newFile, oldFile) == 0)
		/* source and dest are the same */
		return 0;

	std::string fntmp, dtmp;
	const char *filename = NULL;
	const char *directory = NULL;

	/* divide new file path in directory and filename */
	uint lastSlash = path.rfind("/");
	if (lastSlash != path.npos) {
		dtmp = path.substr(0, lastSlash);
		fntmp = path.substr(lastSlash + 1, path.length() - lastSlash - 1);
		directory = dtmp.c_str();
		filename = fntmp.c_str();
	} else {
		filename = newFile;
	}

	bool sameFS = false;
	struct stat oldFileStats, newFileStats;
	stat(oldFile, &oldFileStats);

	/* create target directory if necessary */
	if (directory != NULL) {
		if (access(directory, W_OK) != 0) {
			if (errno == ENOENT) {
				if (creat_dir_r(directory) != 0)
					return 2;
			} else {
				fprintf(stderr, "%s: %s: ", g_progname, directory);
				perror(NULL);
				return 2;
			}
		}

		stat(directory, &newFileStats);
		if (!(newFileStats.st_mode & S_IFDIR)) {
			cerr << g_progname << ": " << directory << ": Not a directory" << endl;
			return 2;
		}

		if (newFileStats.st_dev == oldFileStats.st_dev)
			sameFS = true;
	}

	if (stat(newFile, &newFileStats) == 0) {
		/* new file already exists */
		if (newFileStats.st_dev == oldFileStats.st_dev ) {
			sameFS = true;
			if (newFileStats.st_ino == oldFileStats.st_ino)
				/* source and dest are the same */
				return 0;
		}

		if (!(newFileStats.st_mode & S_IFREG)) {
			cerr << g_progname << ": " << newFile << ": Not a regular file" << endl;
			return 2;
		} else {
			/* overwrite? */
			if (access(newFile, W_OK) != 0) {
				cerr << g_progname << ": " <<  newFile << ": Permission denied" << endl;
				return 2;
			} else if (!overwrite) {
				/* have to ask the user if he wants to overwrite the file */
				if (!confirm_overwrite(newFile)) {
					return 0;
				}
			}
		}
	}

	if (move && sameFS) {
		/* simply rename the file */
		if (rename(oldFile, newFile) != 0) {
			cerr << g_progname << ": " << oldFile << ": Could not rename file to: " << newFile << endl;
			return 2;
		}
	} else {
		/* copy file to new position */
		FILE *outFile = fopen(newFile, "w+");
		if (outFile == NULL) {
			cerr << g_progname << ": " << newFile << ": Could not open file for writing" << endl;
			return 2;
		}

		FILE *inFile = fopen(oldFile, "r");
		if (inFile == NULL) {
			cerr << g_progname << ": " << oldFile << ": Could not open file for reading" << endl;
			return 2;
		}

		bool writeOK = true;
		char *buf = (char*) s_malloc(FILE_BUF_SIZE);
		int icnt, ocnt;

		/* copy content of inFile to outFile */
		while (!feof(inFile) && writeOK) {
			icnt = fread(buf, 1, FILE_BUF_SIZE, inFile);
			if (ferror(inFile)) {
				cerr << g_progname << ": " << oldFile << ": Error reading file" << endl;
				writeOK = false;
			} else {
				ocnt = 0;
				while (ocnt < icnt) {
					ocnt += fwrite(buf + ocnt, 1, icnt - ocnt, outFile);
					if (ferror(outFile)) {
						cerr << g_progname << ": " << newFile << ": Error writing file" << endl;
						writeOK = false;
						break;
					}
				}
			}
		}

		fclose(inFile);
		fclose(outFile);
		free(buf);
		
		if (writeOK) {
			/* delete original if user requested to move the file */
			if (move)
				unlink(oldFile);
		} else {
			unlink(newFile);
			return 2;
		}
	}

	/* apply access and modification times of original file to the new one */
	if (ptimes != NULL)
		utimes(newFile, ptimes);

	return 0;
}

