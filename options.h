/* id3ted: options.h
 * Copyright (c) 2010 Bert Muennich <muennich at informatik.hu-berlin.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <getopt.h>
#include <vector>

#include <taglib/id3v2frame.h>
#include <taglib/tstring.h>

#include "id3ted.h"
#include "frameinfo.h"
#include "genericinfo.h"

enum LongOptOnly {
	OPT_LO_FRAME_LIST = 128,
	OPT_LO_GENRE_LIST,
	OPT_LO_ORG_MOVE
};

class Options {
	public:
		static int tagsToWrite;                   // -[123]
		static int tagsToStrip;                   // -[sSD]
		static bool writeFile;                    // -[123sSDaAtcgTy]
		static bool extractAPICs;                 // -x
		static bool showInfo;                     // -i
		static bool listTags;                     // -[lL]
		static bool listV2WithDesc;               // -L
		static bool printLameTag;                 // -[mM]
		static bool checkLameCRC;                 // -M
		static bool forceOverwrite;               // -f
		static char fieldDelimiter;               // -d
		static bool preserveTimes;                // -p
		/* TODO: Patterns */
		static bool moveFiles;                    // --move
		static vector<GenericInfo*> genericMods;  // -[aAtcgTy]
		static vector<char*> framesToRemove;      // -r
		static vector<FrameInfo*> framesToModify; // --FID

		static uint fileCount;
		static char **filenames;
		
		static bool parseCommandLine(int, char**);
		static void printVersion();
		static void printUsage();
		
	private:
		static const char *options;
		static const struct option longOptions[];
		static int optFrameID;
};

#endif /* __OPTIONS_H__ */

