/* id3ted: optionhandler.h
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

#ifndef __OPTIONHANDLER_H__
#define __OPTIONHANDLER_H__

#include <getopt.h>
#include <sys/time.h>
#include <vector>

#include <taglib/id3v2frame.h>
#include <taglib/tstring.h>

#include "id3ted.h"
#include "genericinfo.h"

enum LongOptOnly {
	OPT_LO_FRAME_LIST = 128,
	OPT_LO_GENRE_LIST,
	OPT_LO_ORG_MOVE
};

class OptionHandler {
	public:
		static const char *options;
		static const struct option longOptions[];
		static int optFrameID;
		
		int fileCount;
		char fieldDelimiter;
		bool error;
		
		int tagsToWrite;                   // -[123]
		int tagsToStrip;                   // -[sSD]
		bool writeFile;                    // -[123sSDaAtcgTy]
		bool extractAPICs;                 // -x
		bool showInfo;                     // -i
		bool listTags;                     // -[lL]
		bool listV2WithDesc;               // -L
		bool printLameTag;                 // -[mM]
		bool checkLameCRC;                 // -M
		bool forceOverwrite;               // -f
		struct timeval *times;             // -p
		/* TODO: Patterns */
		bool moveFiles;                    // --move
		vector<GenericInfo*> genericMods;  // -[aAtcgTy]
		vector<char*> framesToRemove;      // -r
		vector<TagLib::ID3v2::Frame*> framesToModify; // --FID

		OptionHandler(int, char**);
		~OptionHandler();

		static void printVersion();
		static void printUsage();
		
	private:
		int split2(const char*, String&, String&);
		int split3(const char*, String&, String&, String&);
};

#endif /* __OPTIONHANDLER_H__ */

