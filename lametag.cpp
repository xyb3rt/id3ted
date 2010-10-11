/* id3ted: lametag.cpp
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

/* LAME tag specification:
 *   http://gabriel.mp3-tech.org/mp3infotag.html
 *
 * other open source implementations:
 *   - LameTag by phwip, http://phwip.wordpress.com/home/audio/
 *   - MAD, http://www.underbit.com/products/mad/ (madplay/tag.c)
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

#include "lametag.h"
#include "fileio.h"

unsigned short LameTag::crc16Table[] = {
	0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
	0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
	0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
	0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
	0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
	0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
	0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
	0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
	0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
	0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
	0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
	0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
	0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
	0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
	0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
	0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
	0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
	0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
	0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
	0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
	0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
	0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
	0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
	0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
	0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
	0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
	0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
	0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
	0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
	0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
	0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
	0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};

LameTag::LameTag(const char *filename, long frameOffset, long frameLength) :
		valid(false) {
	IFile file(filename);
	if (!file.isOpen())
		return;

	long oldPos = file.tell();
	file.seek(frameOffset);
	ByteVector firstFrame((uint) frameLength, 0);
	file.read(firstFrame.data(), frameLength);
	file.seek(oldPos);

	int xingOffset = firstFrame.find("Xing");
	if (xingOffset == -1)
		xingOffset = firstFrame.find("Info");
	if (xingOffset == -1)
		return;

	int lameOffset = xingOffset + 0x78;
	if (lameOffset + 0x24 >= frameLength)
		return;
	
	ByteVector lameTag = firstFrame.mid(lameOffset, 0x24);
	if (!lameTag.startsWith("LAME"))
		return;
	
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
				cerr << command << ": " << _file.name()
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
	printf("%s%.1f dB", (gain > 0.0 ? "+" : ""), gain);
	cout << endl;
}

void LameTag::print(bool checkCRC) {
	ostringstream output;
	output.setf(ios::fixed);
	output.precision(1);

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
				cerr << command << ": " << _file.name()
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
	printf("%s%.1f dB", (gain > 0.0 ? "+" : ""), gain);
	cout << endl;
}

double LameTag::replayGain(const ByteVector &gainData, bool oldVersion) {
	double value = (((gainData[0] << 8) & 0x100) | (gainData[1] & 0xFF)) / 10.0;

	if (gainData[0] & 0x02)
		value *= -1;
	if (oldVersion)
		value += 6;

	return value;
}

void LameTag::crc16Block(unsigned short *crc, const char *data, size_t size) {
	for (; size >= 8; size -= 8) {
		*crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	}
}

void LameTag::crc16LastBlock(unsigned short *crc, const char *data, size_t size) {
	crc16Block(crc, data, size);
	data += size - (size % 8);
	switch (size % 8) {
	  case 7: *crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 6: *crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 5: *crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 4: *crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 3: *crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 2: *crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 1: *crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 0: break;
	}
}

void LameTag::crc16Checksum(unsigned short *crc, const char *data, int size) {
	crc16LastBlock(crc, data, size);
}
