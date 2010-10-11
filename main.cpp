/* id3ted: main.cpp
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
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <list>
#include <vector>
#include <typeinfo>
#include <sys/stat.h>

#include "id3ted.h"
#include "frameinfo.h"
#include "frametable.h"
#include "mp3file.h"
#include "options.h"

const char *command;

/* return values: (ored together)
 *   0: everything went fine
 *   1: error allocating memory
 *   2: faulty command line arguments required abort
 *   4: at least one error occured during processing of files
 */
int main(int argc, char **argv) {
	int retCode = 0;

	command = basename(argv[0]);
	if (strcmp(command, ".") == 0)
		command = PROGNAME;

	if (Options::parseCommandLine(argc, argv))
		cerr << "Try `" << argv[0] << " --help' for more information." << endl;
		exit(2);
	}

	for (uint fileIdx = 0; fileIdx < Options::fileCount; ++fileIdx) {
		const char *filename = Options::filenames[fileIdx];
		struct stat stats;

		if (stat(filename, &stats) == -1) {
			cerr << command << ": " << filename << ": Could not stat file" << endl;
			retCode |= 4;
			continue;
		}

		if (!S_ISREG(stats.st_mode)) {
			cerr << command << ": " << filename << ": Not a regular file" << endl;
			retCode |= 4;
			continue;
		}

		if (!MP3File::isReadable(file)) {
			cerr << g_progname << ": " << file << ": Could not open file for reading" << endl;
			retCode |= 4;
			continue;
		}

		if (writeFile && !MP3File::isWritable(file)) {
			cerr << g_progname << ": " << file << ": Could not open file for writing" << endl;
			retCode |= 4;
			continue;
		}

		MP3File mp3File(file);
		if (!mp3File.isValid()) {
			retCode |= 4;
			continue;
		}

		if (filenameToTag) {
			// match filename regex pattern on file,
			// because we have to extract tag information from filepath
			if (regexec(fPathRegEx, file, fPathNMatch, g_fPathPMatch, 0)) {
				cout << file << ": pattern does not match filename" << endl;
				filenameToTag = false;
			}
		}

		if (extractAPICs) {
			mp3File.extractAPICs(forceOverwrite);
		}

		// delete id3v2 frames with given frame ids
		if (framesToRemove.size() > 0 && tagsToStrip & 2) {
			cerr << g_progname << ": -r option ignored, because whole id3v2 tag gets stripped" << endl;
			framesToRemove.clear();
			retCode |= 4;
		} else {
			std::vector<char*>::const_iterator f2dIter = framesToRemove.begin();
			for (; f2dIter != framesToRemove.end(); f2dIter++) {
				mp3File.removeFrames(*f2dIter);
			}
		}

		// applying generic tag information
		std::vector<GenericInfo*>::const_iterator gmIter = genericMods.begin();
		for (; gmIter != genericMods.end(); gmIter++) {
			mp3File.apply(*gmIter);
		}

		// loop through the id3v2 frames for adding/modifying
		std::vector<FrameInfo*>::const_iterator f2mIter = framesToModify.begin();
		for (; f2mIter != framesToModify.end(); f2mIter++) {
			if (filenameToTag || (*f2mIter)->fPathIdx() == 0) {
				(*f2mIter)->applyTo(mp3File);
			}
		}

		// save the specified tags to the file
		if (writeFile) {
			if (updFlag) {
				mp3File.save(updFlag);
			} else {
				mp3File.save();
			}
		}

		// delete whole tag version
		if (tagsToStrip > 0) {
			if (!mp3File.strip(tagsToStrip)) {
				cerr << g_progname << ": " << file << ": Could not strip id3 tag" << endl;
				retCode |= 4;
			}
		}

		// print out requested information
		if (showInfo || listTags || printLameTag) {
			cout << file << ":" << endl;
			if (showInfo) mp3File.showInfo();
			if (printLameTag) mp3File.printLameTag(checkLameCRC);
			if (listTags) mp3File.listTags(listV2WithDesc);
			if (fn_idx < argc - 1) cout << endl;
		}

		// organize file in directory structure defined by pattern
		if (orgPattern != NULL) {
			int ret = mp3File.organize(orgPattern, orgMove, forceOverwrite, (preserveTimes ? ptimes : NULL));

			if (ret == 1) {
				orgPattern = NULL;
			} else if (ret > 1) {
				cerr << g_progname << ": " << file << ": Could not organize file" << endl;
				retCode |= 4;
			}
		}

		// reset access and modification times to
		// their old values present before accessing the file:
		if (preserveTimes && (orgPattern == NULL || !orgMove)) {
			utimes(file, ptimes);
		}
	}

	// cleanup - useless because at the end
	if (fPathRegEx != NULL) {
		regfree(fPathRegEx);
		free(fPathRegEx);
	}

	if (g_fPathPMatch != NULL) free(g_fPathPMatch);
	if (ptimes != NULL) free(ptimes);

	std::vector<GenericInfo*>::iterator gmIter = genericMods.begin();
	for (; gmIter != genericMods.end(); gmIter++) {
		delete *gmIter;
	}

	std::vector<FrameInfo*>::iterator f2mIter = framesToModify.begin();
	for (; f2mIter != framesToModify.end(); f2mIter++) {
		delete *f2mIter;
	}

	return retCode;
}

