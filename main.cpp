/* id3ted: main.cpp
 * Copyright (c) 2011 Bert Muennich <muennich at informatik.hu-berlin.de>
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

/* return values: (ored together)
 *   0: everything went fine
 *   1: error allocating memory
 *   2: faulty command line arguments required abort
 *   4: error processing files
 */
int main(int argc, char **argv) {
	int retCode = 0;
	bool firstOutput = true;

	if (Options::parseCommandLine(argc, argv)) {
		cerr << "Try `" << argv[0] << " --help' for more information." << endl;
		exit(2);
	}

	for (uint fileIdx = 0; fileIdx < Options::fileCount; ++fileIdx) {
		const char *filename = Options::filenames[fileIdx];
		FileTimes ptimes;
		bool preserveTimes = Options::preserveTimes &&
				FileIO::saveTimes(filename, ptimes) == FileIO::Success;

		if (!FileIO::isRegular(filename)) {
			warn("%s: Not a regular file", filename);
			retCode |= 4;
			continue;
		}

		if (!FileIO::isReadable(filename)) {
			warn("%s: Could not open file for reading", filename);
			retCode |= 4;
			continue;
		}

		if (Options::writeFile && !FileIO::isWritable(filename)) {
			warn("%s: Could not open file for writing", filename);
			retCode |= 4;
			continue;
		}

		MP3File file(filename, Options::tagsToWrite, Options::printLameTag);
		if (!file.isValid()) {
			retCode |= 4;
			continue;
		}

		if (Options::filenameToTag) {
			uint matches = Options::inPattern.match(filename);
			for (uint i = 0; i < matches; ++i)
				file.apply(Options::inPattern.getMatch(i));
		}

		if (Options::extractAPICs)
			file.extractAPICs(Options::forceOverwrite);

		if (Options::framesToRemove.size() > 0 && Options::tagsToStrip & 2) {
			warn("-r option ignored, because whole id3v2 tag gets stripped");
			Options::framesToRemove.clear();
			retCode |= 4;
		} else {
			std::vector<char*>::const_iterator frameID =
					Options::framesToRemove.begin();
			for (; frameID != Options::framesToRemove.end(); ++frameID) {
				file.removeFrames(*frameID);
			}
		}

		std::vector<GenericInfo*>::const_iterator genInfo =
				Options::genericMods.begin();
		for (; genInfo != Options::genericMods.end(); ++genInfo)
			file.apply(*genInfo);

		std::vector<FrameInfo*>::const_iterator frameInfo = 
				Options::framesToModify.begin();
		for (; frameInfo != Options::framesToModify.end(); ++frameInfo)
			file.apply(*frameInfo);

		if (Options::writeFile)
			file.save();

		if (Options::tagsToStrip != 0) {
			if (!file.strip(Options::tagsToStrip)) {
				warn("%s: Could not strip id3 tag", filename);
				retCode |= 4;
			}
		}

		if (Options::showInfo || Options::listTags || Options::printLameTag) {
			if (Options::fileCount > 1 && (Options::showInfo || 
					(Options::listTags && (file.hasID3v1Tag() || file.hasID3v2Tag())) ||
					(Options::printLameTag && file.hasLameTag()))) {
				if (!firstOutput)
					cout << endl;
				else
					firstOutput = false;
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

		if (Options::organize) {
			for (uint i = 0; i < Options::outPattern.count(); ++i) {
				MatchInfo minfo = Options::outPattern.getMatch(i);
				file.fill(minfo);
				Options::outPattern.setMatch(i, minfo);
			}
			Options::outPattern.replaceSpecialChars(REPLACE_CHAR);
			string newPath = Options::outPattern.getText();
			if (!newPath.empty()) {
				FileIO::Status ret = FileIO::copy(filename, newPath.c_str());
				if (ret == FileIO::Error) {
					warn("%s: Could not organize file", filename);
					retCode |= 4;
				} else if (ret == FileIO::Success && preserveTimes) {
					FileIO::resetTimes(newPath.c_str(), ptimes);
				}
			}
		}

		if (preserveTimes && (!Options::organize || !Options::moveFiles))
			FileIO::resetTimes(filename, ptimes);
	}

	return retCode;
}

void warn(const char* fmt, ...) {
	va_list args;

	if (!fmt)
		return;

	va_start(args, fmt);
	fprintf(stderr, "%s: ", PROGNAME);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}
