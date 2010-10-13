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
#include <cstdlib>
#include <cstring>
#include <list>
#include <vector>
#include <typeinfo>
#include <sys/stat.h>

#include "id3ted.h"
#include "fileio.h"
#include "frameinfo.h"
#include "frametable.h"
#include "mp3file.h"
#include "options.h"
#include "pattern.h"

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

	if (Options::parseCommandLine(argc, argv)) {
		cerr << "Try `" << argv[0] << " --help' for more information." << endl;
		exit(2);
	}

	for (uint fileIdx = 0; fileIdx < Options::fileCount; ++fileIdx) {
		const char *filename = Options::filenames[fileIdx];
		FileTimes ptimes;
		bool preserveTimes = Options::preserveTimes &&
				FileIO::saveTimes(filename, ptimes);

		if (!FileIO::isRegular(filename)) {
			cerr << command << ": " << filename << ": Not a regular file" << endl;
			retCode |= 4;
			continue;
		}

		if (!FileIO::isReadable(filename)) {
			cerr << command << ": " << filename
			     << ": Could not open file for reading" << endl;
			retCode |= 4;
			continue;
		}

		if (Options::writeFile && !FileIO::isWritable(filename)) {
			cerr << command << ": " << filename
			     << ": Could not open file for writing" << endl;
			retCode |= 4;
			continue;
		}

		MP3File file(filename, Options::tagsToWrite, Options::printLameTag);
		if (!file.isValid()) {
			retCode |= 4;
			continue;
		}

		if (Options::filenameToTag) {
			uint matches = Options::filePattern.match(filename);
			for (uint i = 0; i < matches; ++i)
				file.apply(Options::filePattern.getMatch(i));
		}

		if (Options::extractAPICs)
			file.extractAPICs(Options::forceOverwrite);

		// delete id3v2 frames with given frame ids
		if (Options::framesToRemove.size() > 0 && Options::tagsToStrip & 2) {
			cerr << command << ": -r option ignored, because whole id3v2 tag "
			     << "gets stripped" << endl;
			Options::framesToRemove.clear();
			retCode |= 4;
		} else {
			std::vector<char*>::const_iterator frameID =
					Options::framesToRemove.begin();
			for (; frameID != Options::framesToRemove.end(); ++frameID) {
				file.removeFrames(*frameID);
			}
		}

		// applying generic tag information
		std::vector<GenericInfo*>::const_iterator genInfo =
				Options::genericMods.begin();
		for (; genInfo != Options::genericMods.end(); ++genInfo)
			file.apply(*genInfo);

		// loop through the id3v2 frames for adding/modifying
		std::vector<FrameInfo*>::const_iterator frameInfo = 
				Options::framesToModify.begin();
		for (; frameInfo != Options::framesToModify.end(); ++frameInfo)
			file.apply(*frameInfo);

		// save the specified tags to the file
		if (Options::writeFile)
			file.save();

		// delete whole tag version
		if (Options::tagsToStrip != 0) {
			if (!file.strip(Options::tagsToStrip)) {
				cerr << command << ": " << filename
				     << ": Could not strip id3 tag" << endl;
				retCode |= 4;
			}
		}

		// print out requested information
		if (Options::showInfo || Options::listTags || Options::printLameTag) {
			if (Options::fileCount > 1 && (Options::showInfo || 
					(Options::listTags && (file.hasID3v1Tag() || file.hasID3v2Tag())) ||
					(Options::printLameTag && file.hasLameTag()))) {
				if (fileIdx > 0)
					cout << endl;
				cout << filename << ":" << endl;
			}
			if (Options::showInfo)
				file.showInfo();
			if (Options::printLameTag)
				file.printLameTag(Options::checkLameCRC);
			if (Options::listTags) {
				file.listID3v1Tag();
				file.listID3v2Tag(Options::listV2WithDesc);
			}
		}

		// organize file in directory structure defined by pattern
		/*if (orgPattern != NULL) {
			int ret = mp3File.organize(orgPattern, orgMove, forceOverwrite, (preserveTimes ? ptimes : NULL));

			if (ret == 1) {
				orgPattern = NULL;
			} else if (ret > 1) {
				cerr << g_progname << ": " << file << ": Could not organize file" << endl;
				retCode |= 4;
			}
		}*/

		// reset access and modification times to
		// their old values present before accessing the file:
		if (preserveTimes && !Options::moveFiles)
			FileIO::resetTimes(filename, ptimes);
	}

	return retCode;
}

