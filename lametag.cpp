/* id3ted: lametag.cpp
 * Copyright (c) 2009 Bert Muennich <muennich at informatik.hu-berlin.de>
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

/* LAME tag specification:
 *   http://gabriel.mp3-tech.org/mp3infotag.html
 *
 * other open source implementations:
 * - LameTag by phwip, http://phwip.wordpress.com/home/audio/
 * - MAD, http://www.underbit.com/products/mad/ (madplay/tag.c)
 */

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <sstream>

#include <taglib/id3v2header.h>
#include <taglib/id3v2tag.h>
#include <taglib/tfile.h>
#include <taglib/mpegfile.h>
#include <taglib/tbytevector.h>

#include "common.h"
#include "list.h"
#include "mp3file.h"
#include "misc.h"

double replayGain(const ByteVector &gainData, bool oldVersion) {
	double value = (((gainData[0] << 8) & 0x100) | (gainData[1] & 0xFF)) / 10.0;

	if (gainData[0] & 0x02)
		value *= -1;

	if (oldVersion)
		value += 6;

	return value;
}

void MP3File::printLameTag(bool checkCRC) {
	ostringstream output;
	output.setf(ios::fixed);
	output.precision(1);

	long frameOffset;
	if (_id3v2Tag && !_id3v2Tag->isEmpty())
		frameOffset = _file.firstFrameOffset();
	else
		frameOffset = _file.nextFrameOffset(0);

	int frameLength = _file.nextFrameOffset(frameOffset + 1) - frameOffset;
	if (frameLength <= 0) {
		cout << "No lame tag found" << endl;
		return;
	}

	long filePos = _file.tell();
	_file.seek(frameOffset);
	ByteVector firstFrame = _file.readBlock(frameLength);
	_file.seek(filePos);

	int xingOffset = firstFrame.find("Xing");
	if (xingOffset == -1)
		xingOffset = firstFrame.find("Info");
	if (xingOffset == -1) {
		cout << "No lame tag found" << endl;
		return;
	}

	int lameOffset = xingOffset + 0x78;
	if (lameOffset + 0x24 >= frameLength) {
		cout << "Lame tag is corrupt" << endl;
		return;
	}
	
	ByteVector lameTag = firstFrame.mid(lameOffset, 0x24);
	if (!lameTag.startsWith("LAME")) {
		cout << "Lame tag is corrupt" << endl;
		return;
	}
	
	String encoder(lameTag.mid(0,9), String::Latin1);
	cout << encoder << " tag (revision " << (lameTag[9] >> 4) << "):" << endl;

	printf("%-16s: ", "encoding method");
	int encMethod = lameTag[9] & 0x0F;
	switch (encMethod) {
		case 8:
			output << "2-pass ";
		case 1:
			output << "CBR";
			break;
		case 9:
			output << "2-pass ";
		case 2:
			output << "ABR";
			break;
		case 3:
		case 4:
		case 5:
		case 6:
			if      (encMethod == 3) output << "old/re ";
			else if (encMethod == 4) output << "new/mtrh ";
			else if (encMethod == 5) output << "new/mt ";
			output << "VBR";
			break;
		default:
			output << "unknown";
			encMethod = 0;
	}
	printf("%-15s", output.str().c_str());
	output.str("");

	printf("%-15s: ", "quality");
	int quality = firstFrame[xingOffset + 0x77];
	if (quality > 0 && quality <= 100)
		printf("V%d/q%d", (100 - quality) / 10, (100 - quality) % 10);
	else
		cout << "unknown";
	cout << endl;

	printf("%-16s: ", "stereo mode");
	switch (lameTag[24] >> 2 & 0x07) {
		case 0:
			output << "mono";
			break;
		case 1:
			output << "stereo";
			break;
		case 2:
			output << "dual";
			break;
		case 3:
			output << "joint";
			break;
		case 4:
			output << "force";
			break;
		case 5:
			output << "auto";
			break;
		case 6:
			output << "intensity";
			break;
		default:
			output << "undefined";
			break;
	}
	printf("%-15s", output.str().c_str());
	output.str("");

	printf("%-15s: ", "source rate");
	switch (lameTag[24] >> 6 & 0x03) {
		case 0:
			cout << "<= 32";
			break;
		case 1:
			cout << "44.1";
			break;
		case 2:
			cout << "48";
			break;
		default:
			cout << "> 48";
			break;
	}
	cout << " kHz" << endl;

	if (encMethod == 2 || encMethod == 9)
		output << "average ";
	else if (encMethod > 2 && encMethod < 7)
		output << "minimal ";
	output << "bitrate";
	printf("%-16s: ", output.str().c_str());
	output.str("");
	if ((unsigned char) lameTag[20] == 0xFF)
		output << ">= ";
	output << (lameTag[20] & 0xFF) << " kBit/s";
	printf("%-15s", output.str().c_str());
	output.str("");

	printf("%-15s: ", "music length");
	int musicLength = lameTag.mid(28, 4).toUInt();
	printSizeHumanReadable(musicLength);
	cout << endl;

	printf("%-16s: ", "lowpass");
	if (lameTag[10] == 0) {
		output << "unknown";
	} else {
		output << (lameTag[10] & 0xFF) << "00 Hz";
	}
	printf("%-15s", output.str().c_str());
	output.str("");

	printf("%-15s: ", "mp3gain");
	float mp3Gain = (lameTag[25] & 0x7F) * 1.5;
	if (mp3Gain != 0.0)
		printf("%s%.f dB", (lameTag[25] & 0x80 ? "-" : "+"), mp3Gain);
	else
		cout << "none";
	cout << endl;

	printf("%-16s: ", "ATH type");
	output << (lameTag[19] & 0x0F);
	printf("%-15s", output.str().c_str());
	output.str("");

	printf("%-15s: ", "encoding flags");
	bool flag = false;
	if (lameTag[19] & 0x10) {
		cout << "nspsytune ";
		flag = true;
	}
	if (lameTag[19] & 0x20) {
		cout << "nssafejoint ";
		flag = true;
	}
	if (lameTag[19] & 0xC0) {
		cout << "nogap";
		flag = true;
		if (lameTag[19] & 0x80) cout << "<";
		if (lameTag[19] & 0x40) cout << ">";
	}
	if (!flag)
		cout << "none";
	cout << endl;

	printf("%-16s: ", "encoding delay");
	output << ((lameTag[21] << 4 & 0xFF0) | (lameTag[22] >> 4 & 0x0F));
	output << " samples";
	printf("%-15s", output.str().c_str());
	output.str("");
	printf("%-15s: ", "padding");
	cout << ((lameTag[22] << 4 & 0xF00) | (lameTag[23] & 0xFF));
	cout << " samples" << endl;

	printf("%-16s: ", "noise shaping");
	output << (lameTag[24] >> 6);
	printf("%-15s", output.str().c_str());
	output.str("");
	printf("%-15s: ", "unwise settings");
	cout << (lameTag[24] & 0x20 ? "yes" : "no") << endl;

	printf("%-16s: ", "info tag CRC");
	unsigned short crc_lame = (unsigned short) lameTag.mid(34, 2).toUInt();
	printf("%04X ", crc_lame);

	if (checkCRC) {
		int blockSize = frameLength < 190 ? frameLength : 190;
		ByteVector checkBlock(firstFrame.mid(0, blockSize));
		unsigned short crc_calc = 0;

		crc16_checksum(&crc_calc, checkBlock.data(), blockSize);
		output << "(" << (crc_lame != crc_calc ? "invalid" : "correct") << ")";
	}
	printf("%-10s", output.str().c_str());
	output.str("");

	printf("%-15s: ", "music CRC");
	crc_lame = (unsigned short) lameTag.mid(32, 2).toUInt();
	printf("%04X ", crc_lame);

	if (checkCRC) {
		unsigned short crc_calc = 0;
		int musicSize = musicLength -
				(_file.nextFrameOffset(frameOffset + 1) - frameOffset);

		char *fbuf = (char*) s_malloc(FILE_BUF_SIZE);
		FILE *fptr = fopen(_file.name(), "r");
		fseek(fptr, _file.nextFrameOffset(frameOffset + 1), SEEK_SET);
	
		while (musicSize > 0 && !feof(fptr)) {
			int blockSize = musicSize < FILE_BUF_SIZE ? musicSize : FILE_BUF_SIZE;

			fread(fbuf, blockSize, 1, fptr);
			if (ferror(fptr)) {
				cerr << g_progname << ": " << _file.name()
						 << ": Error reading file" << endl;
				return;
			}

			musicSize -= blockSize;
			if (musicSize > 0) {
				crc16_block(&crc_calc, fbuf, blockSize);
			} else {
				crc16_last_block(&crc_calc, fbuf, blockSize);
			}
		}

		free(fbuf);
		cout << "(" << (crc_lame != crc_calc ? "invalid" : "correct") << ")";
	}
	cout << endl;

	printf("%-16s: ", "ReplayGain: peak");
	float peakSignal = (float) (lameTag.mid(11,4).toUInt() << 5);
	if (encoder < "LAME3.94b") peakSignal += 0.0;
	else peakSignal = (peakSignal - 0.5) / 8388608.0;
	cout << (int) peakSignal * 100;
	cout << endl;

	printf("%-16s: ", "track gain");
	bool oldVersion = false;
	if (encoder < "LAME3.95")
		oldVersion = true;
	double gain = replayGain(lameTag.mid(15, 2), oldVersion);
	output << (gain > 0.0 ? "+" : "") << gain << " dB";
	printf("%-15s", output.str().c_str());
	output.str("");

	printf("%-15s: ", "album gain");
	gain = replayGain(lameTag.mid(17, 2), oldVersion);
	printf("%s%.1lf dB", (gain > 0.0 ? "+" : ""), gain);
	cout << endl;
}

