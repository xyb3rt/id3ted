/* id3ted: pattern.h
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

#ifndef __PATTERN_H__
#define __PATTERN_H__

#include <string>
#include <vector>
#include <regex.h>

#include "id3ted.h"

typedef struct _MatchInfo {
	char id;
	string text;
} MatchInfo;

class Pattern {
	public:
		Pattern() : status(0) {}

		bool setPattern(const char*, bool);
		bool needsID3v2() const;
		uint match(const char*);
		MatchInfo getMatch(uint) const;

		static const char* fill(const char*);

	private:
		uint status;
		string pattern;
		uint subExpCnt;
		regex_t regex;
		vector<char> ids;
		vector<string> matches;

		int preBackslashCount(const char*, uint) const;
};

#endif /* __PATTERN_H__ */
