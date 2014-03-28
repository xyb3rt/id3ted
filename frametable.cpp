/* id3ted: frametable.cpp
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

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <taglib/id3v1genres.h>

#include "frametable.h"

FrameTable::FrameTableEntry FrameTable::_table[] = {
	{ "XXXX", FID3_XXXX, "Unknown frame" },
	{ "AENC", FID3_AENC, "Audio encryption" },
	{ "APIC", FID3_APIC, "Attached picture" },
	{ "ASPI", FID3_ASPI, "Audio seek point index" },
	{ "COMM", FID3_COMM, "Comments" },
	{ "COMR", FID3_COMR, "Commercial frame" },
	{ "ENCR", FID3_ENCR, "Encryption method registration" },
	{ "EQU2", FID3_EQU2, "Equalisation (2)" },
	{ "EQUA", FID3_EQUA, "Equalisation" }, /* ID3v2.3 only */
	{ "ETCO", FID3_ETCO, "Event timing codes" },
	{ "GEOB", FID3_GEOB, "General encapsulated object" },
	{ "GRID", FID3_GRID, "Group identification registration" },
	{ "IPLS", FID3_IPLS, "Involved people list" }, /* ID3v2.3 only */
	{ "LINK", FID3_LINK, "Linked information" },
	{ "MCDI", FID3_MCDI, "Music CD identifier" },
	{ "MLLT", FID3_MLLT, "MPEG location lookup table" },
	{ "OWNE", FID3_OWNE, "Ownership frame" },
	{ "PCNT", FID3_PCNT, "Play counter" },
	{ "POPM", FID3_POPM, "Popularimeter" },
	{ "POSS", FID3_POSS, "Position synchronisation frame" },
	{ "PRIV", FID3_PRIV, "Private frame" },
	{ "RBUF", FID3_RBUF, "Recommended buffer size" },
	{ "RVA2", FID3_RVA2, "Relative volume adjustment (2)" },
	{ "RVAD", FID3_RVAD, "Relative volume adjustment" }, /* ID3v2.3 only */
	{ "RVRB", FID3_RVRB, "Reverb" },
	{ "SEEK", FID3_SEEK, "Seek frame" },
	{ "SIGN", FID3_SIGN, "Signature frame" },
	{ "SYLT", FID3_SYLT, "Synchronised lyric/text" },
	{ "SYTC", FID3_SYTC, "Synchronised tempo codes" },
	{ "TALB", FID3_TALB, "Album/Movie/Show title" },
	{ "TBPM", FID3_TBPM, "BPM (beats per minute)" },
	{ "TCOM", FID3_TCOM, "Composer" },
	{ "TCON", FID3_TCON, "Content type" },
	{ "TCOP", FID3_TCOP, "Copyright message" },
	{ "TDAT", FID3_TDAT, "Date" }, /* ID3v2.3 only */
	{ "TDEN", FID3_TDEN, "Encoding time" },
	{ "TDLY", FID3_TDLY, "Playlist delay" },
	{ "TDOR", FID3_TDOR, "Original release time" },
	{ "TDRC", FID3_TDRC, "Recording time" },
	{ "TDRL", FID3_TDRL, "Release time" },
	{ "TDTG", FID3_TDTG, "Tagging time" },
	{ "TENC", FID3_TENC, "Encoded by" },
	{ "TEXT", FID3_TEXT, "Lyricist/Text writer" },
	{ "TFLT", FID3_TFLT, "File type" },
	{ "TIME", FID3_TIME, "Time" }, /* ID3v2.3 only */
	{ "TIPL", FID3_TIPL, "Involved people list (2)" },
	{ "TIT1", FID3_TIT1, "Content group description" },
	{ "TIT2", FID3_TIT2, "Title/songname/content description" },
	{ "TIT3", FID3_TIT3, "Subtitle/Description refinement" },
	{ "TKEY", FID3_TKEY, "Initial key" },
	{ "TLAN", FID3_TLAN, "Language(s)" },
	{ "TLEN", FID3_TLEN, "Length" },
	{ "TMCL", FID3_TMCL, "Musician credits list" },
	{ "TMED", FID3_TMED, "Media type" },
	{ "TMOO", FID3_TMOO, "Mood" },
	{ "TOAL", FID3_TOAL, "Original album/movie/show title" },
	{ "TOFN", FID3_TOFN, "Original filename" },
	{ "TOLY", FID3_TOLY, "Original lyricist(s)/text writer(s)" },
	{ "TOPE", FID3_TOPE, "Original artist(s)/performer(s)" },
	{ "TORY", FID3_TORY, "Original release year" }, /* ID3v2.3 only */
	{ "TOWN", FID3_TOWN, "File owner/licensee" },
	{ "TPE1", FID3_TPE1, "Lead performer(s)/Soloist(s)" },
	{ "TPE2", FID3_TPE2, "Band/orchestra/accompaniment" },
	{ "TPE3", FID3_TPE3, "Conductor/performer refinement" },
	{ "TPE4", FID3_TPE4, "Interpreted, remixed, or otherwise modified by" },
	{ "TPOS", FID3_TPOS, "Part of a set" },
	{ "TPRO", FID3_TPRO, "Produced notice" },
	{ "TPUB", FID3_TPUB, "Publisher" },
	{ "TRCK", FID3_TRCK, "Track number/Position in set" },
	{ "TRDA", FID3_TRDA, "Recording dates" }, /* ID3v2.3 only */
	{ "TRSN", FID3_TRSN, "Internet radio station name" },
	{ "TRSO", FID3_TRSO, "Internet radio station owner" },
	{ "TSIZ", FID3_TSIZ, "Size" }, /* ID3v2.3 only */
	{ "TSOA", FID3_TSOA, "Album sort order" },
	{ "TSOP", FID3_TSOP, "Performer sort order" },
	{ "TSOT", FID3_TSOT, "Title sort order" },
	{ "TSRC", FID3_TSRC, "ISRC (international standard recording code)" },
	{ "TSSE", FID3_TSSE, "Software/Hardware and settings used for encoding" },
	{ "TSST", FID3_TSST, "Set subtitle" },
	{ "TXXX", FID3_TXXX, "User defined text information frame" },
	{ "TYER", FID3_TYER, "Year" }, /* ID3v2.3 only */
	{ "UFID", FID3_UFID, "Unique file identifier" },
	{ "USER", FID3_USER, "Terms of use" },
	{ "USLT", FID3_USLT, "Unsynchronised lyric/text transcription" },
	{ "WCOM", FID3_WCOM, "Commercial information" },
	{ "WCOP", FID3_WCOP, "Copyright/Legal information" },
	{ "WOAF", FID3_WOAF, "Official audio file webpage" },
	{ "WOAR", FID3_WOAR, "Official artist/performer webpage" },
	{ "WOAS", FID3_WOAS, "Official audio source webpage" },
	{ "WORS", FID3_WORS, "Official Internet radio station homepage" },
	{ "WPAY", FID3_WPAY, "Payment" },
	{ "WPUB", FID3_WPUB, "Publishers official webpage" },
	{ "WXXX", FID3_WXXX, "User defined URL link frame" },
};
int FrameTable::_tableSize = sizeof(_table) / sizeof(FrameTableEntry);

const char* FrameTable::frameDescription(const String &textFID) {
	for (uint i = 1; i < _tableSize; i++) {
		if (textFID == _table[i].id)
			return _table[i].description;
	}
	cerr << "   no table entry found for field " << textFID 
		<< ".  returning " << _table[0].description << endl;
	return _table[0].description;
}

ID3v2FrameID FrameTable::frameID(const String &textFID) {
	for (uint i = 1; i < _tableSize; i++) {
		if (textFID == _table[i].id)
			return _table[i].fid;
	}
	return _table[0].fid;
}

const char* FrameTable::textFrameID(ID3v2FrameID frameID) {
	for (uint i = 1; i < _tableSize; i++) {
		if (frameID == _table[i].fid)
			return _table[i].id;
	}
	return _table[0].id;
}

void FrameTable::listFrames() {
	for (int i = 1; i < _tableSize; i++) {
		switch (_table[i].fid) {
			case FID3_AENC:
			case FID3_ASPI:
			case FID3_COMR:
			case FID3_ENCR:
			case FID3_EQU2:
			case FID3_EQUA:
			case FID3_ETCO:
			case FID3_GEOB:
			case FID3_GRID:
			case FID3_IPLS:
			case FID3_LINK:
			case FID3_MCDI:
			case FID3_MLLT:
			case FID3_OWNE:
			case FID3_POPM:
			case FID3_POSS:
			case FID3_PRIV:
			case FID3_RVA2:
			case FID3_RVAD:
			case FID3_RVRB:
			case FID3_SEEK:
			case FID3_SIGN:
			case FID3_SYLT:
			case FID3_SYTC:
			case FID3_TDAT:
			case FID3_TIME:
			case FID3_TIPL:
			case FID3_TRDA:
			case FID3_TSIZ:
			case FID3_TYER:
			case FID3_UFID:
				printf("    ");
				break;
			default:
				printf("  * ");
				break;
		}
		printf("%s  %s\n", _table[i].id, _table[i].description);
	}

	printf("\n"
	       "(*): Frame with write support: You can use the 4-letter frame ID as a\n"
	       "     long option to set the content of the frame to the option argument.\n");
}

void FrameTable::listGenres() {
	for (uint i = 0; i < ID3v1::genreList().size(); i++)
		printf("  %5d: %s\n", i, ID3v1::genre(i).toCString());
}

