/* id3ted: pattern.h
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

#ifndef PATTERN_H
#define PATTERN_H

#include <string>
#include <vector>
#include <regex.h>

#include "id3ted.h"

typedef struct {
	char id;
	string text;
} MatchInfo;

class Pattern {
	public:
		Pattern() : status(0) {}

		bool needsID3v2() const;
		uint count() const;
		virtual MatchInfo getMatch(uint) const = 0;

	protected:
		uint status;
		vector<char> ids;
};

class IPattern : public Pattern {
	public:
		bool setPattern(const char*, bool);
		uint match(const char*);
		MatchInfo getMatch(uint) const;

	private:
		string pattern;
		regex_t regex;
		vector<char> subs;
		vector<string> matches;

		int preBackslashCount(const char*, uint) const;
};

class OPattern : public Pattern {
	public:
		bool setPattern(const char*);
		MatchInfo getMatch(uint) const;
		void setMatch(uint, const MatchInfo&);
		void replaceSpecialChars(char);
		const string& getText() const { return text; }

	private:
		string text;
		vector<uint> pos;
		vector<uint> len;
};

#endif /* PATTERN_H */
