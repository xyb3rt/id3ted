/* id3ted: pattern.cpp
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

#include <cassert> // DEBUG
#include <iostream>
#include <sstream>
#include <cstring>

#include "pattern.h"

bool Pattern::setPattern(const char *text, bool isRE) {
	uint i;
	bool wildcardPresent = false;
	char flag = isRE ? 'N' : 'n';
	ostringstream tmp;

	if (status > 0) {
		subExpCnt = 0;
		regfree(&regex);
		ids.clear();
		matches.clear();
		status = 0;
	}

	for (i = 0; i < strlen(text); ++i) {
		if (text[i] == '%') {
			if (i+1 >= strlen(text)) {
				cerr << command << ": -" << flag << " option ignored, "
				     << "because pattern ends with an invalid character" << endl;
				return false;
			}
			switch (text[i+1]) {
				case 'a':
				case 'A':
				case 't':
				case 'g':
					tmp << "(.+)";
					wildcardPresent = true;
					++subExpCnt;
					break;
				case 'y':
				case 'T':
				case 'd':
					tmp << "([0-9]+)";
					wildcardPresent = true;
					++subExpCnt;
					break;
				case '%':
					tmp << '%';
					break;
				default:
					cerr << command << ": -" << flag << " option ignored, because "
					     << "pattern contains invalid wildcard: `" << text[i] << text[i+1]
					     << "'" << endl;
					return false;
			}
			ids.push_back(text[++i]);
		} else {
			if (isRE) {
				if (strchr("\\*+?.^$[]{}()", text[i]))
					tmp << '\\';
			} else {
				if (text[i] == '(' && preBackslashCount(text, i) % 2 == 0)
					// although its match is never used,
					// we still have to count this subexpression
					++subExpCnt;
			}
			tmp << text[i];
		}
	}

	if (!wildcardPresent) {
		cerr << command << ": -" << flag << " option ignored, "
		     << "because pattern does not contain any wildcard" << endl;
		return false;
	}

	pattern = tmp.str();

	if (regcomp(&regex, pattern.c_str(), REG_EXTENDED) ||
			regex.re_nsub != subExpCnt) {
		cerr << command << ": error compiling regex for pattern, -" << flag
		     << " option ignored" << endl;
		return false;
	}

	status = 1;

	return true;
}

bool Pattern::needsID3v2() const {
	if (status == 0)
		return false;
	
	vector<char>::const_iterator id = ids.begin();
	for (; id != ids.end(); ++id) {
		if (*id == 'd')
			return true;
	}

	return false;
}

uint Pattern::match(const char *filename) {
	if (status == 0)
		return 0;

	string path(filename);
	regmatch_t *pmatch = new regmatch_t[subExpCnt + 1];

	if (regexec(&regex, filename, subExpCnt + 1, pmatch, 0)) {
		cout << filename << ": pattern does not match filename" << endl;
		return 0;
	}

	for (uint i = 1; i <= subExpCnt; ++i) {
		if (pmatch[i].rm_so == -1 || pmatch[i].rm_eo == -1)
			matches.push_back("");
		else
			matches.push_back(path.substr(pmatch[i].rm_so,
					pmatch[i].rm_eo = pmatch[i].rm_so));
	}

	assert(subExpCnt == ids.size()); // DEBUG
	assert(ids.size() == matches.size());

	status = 2;

	return subExpCnt;
}

MatchInfo Pattern::getMatch(uint num) const {
	MatchInfo info;

	if (status < 2 || num >= ids.size()) {
		info.id = 0;
		info.text = "";
	} else {
		info.id = ids[num];
		info.text = matches[num];
	}

	return info;
}

const char* Pattern::fill(const char *text) {
	// TODO
	return NULL;
}

int Pattern::preBackslashCount(const char *str, uint pos) const {
	uint cnt = 0;

	if (pos > strlen(str))
		return -1;
	for (; pos > 0 && str[pos-1] == '\\'; pos--)
		++cnt;

	return cnt;
}
