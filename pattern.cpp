/* id3ted: pattern.cpp
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

#include <iostream>
#include <sstream>
#include <cstring>

#include "pattern.h"

uint Pattern::count() const {
	if (status == 0)
		return 0;
	else
		return ids.size();
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

bool IPattern::setPattern(const char *text, bool isRE) {
	uint i;
	bool wildcardPresent = false;
	char flag = isRE ? 'N' : 'n';
	ostringstream tmp;

	if (status > 0)
		regfree(&regex);
	ids.clear();
	subs.clear();
	matches.clear();
	status = 0;

	for (i = 0; i < strlen(text); ++i) {
		if (text[i] == '%') {
			if (i+1 >= strlen(text)) {
				warn("-%c option ignored, because patterns ends with an invalid character",
				     flag);
				return false;
			}
			switch (text[i+1]) {
				case 'a':
				case 'A':
				case 't':
				case 'g':
					tmp << "(.+)";
					subs.push_back(text[++i]);
					wildcardPresent = true;
					break;
				case 'y':
				case 'T':
				case 'd':
					tmp << "([0-9]+)";
					subs.push_back(text[++i]);
					wildcardPresent = true;
					break;
				case '%':
					tmp << '%';
					break;
				default:
					warn("-%c option ignored, because pattern contains invalid wildcard: %c%c",
					     text[i], text[i+1]);
					return false;
			}
		} else {
			if (!isRE) {
				if (strchr("\\*+?.^$[]{}()", text[i]))
					tmp << '\\';
			} else {
				if (text[i] == '(' && preBackslashCount(text, i) % 2 == 0)
					// although its match is never used,
					// we still have to count this subexpression
					subs.push_back(0);
			}
			tmp << text[i];
		}
	}

	if (!wildcardPresent) {
		warn("-%c option ignored, because pattern does not contain any wildcard",
		     flag);
		return false;
	}

	pattern = tmp.str();

	if (regcomp(&regex, pattern.c_str(), REG_EXTENDED) ||
			regex.re_nsub != subs.size()) {
		warn("error compiling regex for pattern, -%c option ignored", flag);
		return false;
	}

	status = 1;

	return true;
}

uint IPattern::match(const char *filename) {
	if (status == 0)
		return 0;

	string path(filename);
	regmatch_t *pmatch = new regmatch_t[subs.size() + 1];

	status = 1;
	ids.clear();
	matches.clear();

	if (regexec(&regex, filename, subs.size() + 1, pmatch, 0)) {
		warn("%s: pattern does not match filename", filename);
		return 0;
	}

	for (uint i = 1; i <= subs.size(); ++i) {
		if (subs[i-1]) {
			if (pmatch[i].rm_so == -1 || pmatch[i].rm_eo == -1)
				matches.push_back("");
			else
				matches.push_back(path.substr(pmatch[i].rm_so,
						pmatch[i].rm_eo - pmatch[i].rm_so));
			ids.push_back(subs[i-1]);
		}
	}

	status = 2;
	delete [] pmatch;

	return matches.size();
}

MatchInfo IPattern::getMatch(uint num) const {
	MatchInfo info;

	if (status < 2 || num >= ids.size() || num >= matches.size()) {
		info.id = 0;
		info.text = "";
	} else {
		info.id = ids[num];
		info.text = matches[num];
	}

	return info;
}

int IPattern::preBackslashCount(const char *str, uint pos) const {
	uint cnt = 0;

	if (pos > strlen(str))
		return -1;
	for (; pos > 0 && str[pos-1] == '\\'; pos--)
		++cnt;

	return cnt;
}

bool OPattern::setPattern(const char *pattern) {
	uint i;
	bool wildcardPresent = false;

	ids.clear();
	pos.clear();
	len.clear();
	status = 0;

	if (strlen(pattern) == 0)
		return false;
	
	if (pattern[strlen(pattern) - 1] == '/') {
		warn("-o option ignored, because pattern ends with a slash");
		return false;
	}

	for (i = 0; i < strlen(pattern); ++i) {
		if (pattern[i] == '%') {
			if (i+1 >= strlen(pattern)) {
				warn("-o option ignored, because pattern ends with an invalid character");
				return false;
			}
			switch (pattern[i+1]) {
				case 'a':
				case 'A':
				case 't':
				case 'c':
				case 'g':
				case 'T':
				case 'y':
				case 'd':
					ids.push_back(pattern[i+1]);
					pos.push_back(i++);
					len.push_back(2);
					wildcardPresent = true;
					break;
				default:
					warn("-o option ignored, because pattern contains invalid wildcard: %c%c",
					     pattern[i], pattern[i+1]);
					return false;
			}
		}
	}

	if (!wildcardPresent) {
		warn("-o option ignored, because pattern does not contain any wildcard");
		return false;
	}

	text = pattern;
	status = 1;

	return true;
}

MatchInfo OPattern::getMatch(uint num) const {
	MatchInfo info;

	if (status < 1 || num >= ids.size())
		info.id = 0;
	else
		info.id = ids[num];
	info.text = "";

	return info;
}

void OPattern::setMatch(uint num, const MatchInfo &info) {
	int diff;
	uint newlen, oldlen;

	if (status < 1 || num >= ids.size())
		return;

	oldlen = len[num];
	newlen = info.text.length();
	diff = newlen - oldlen;

	text.replace(pos[num], oldlen, info.text);
	len[num] = newlen;

	vector<uint>::iterator each = pos.begin() + num + 1;
	for (; each != pos.end(); ++each)
		*each += diff;
}

void OPattern::replaceSpecialChars(char replaceChar) {
	size_t pos;

	while ((pos = text.find_first_of("*~")) != string::npos)
		text[pos] = replaceChar;
}
