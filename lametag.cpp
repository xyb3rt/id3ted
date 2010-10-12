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

#include <taglib/id3v2header.h>
#include <taglib/id3v2tag.h>
#include <taglib/tfile.h>
#include <taglib/mpegfile.h>

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
	long oldPos, bytesRead, xingOffset, lameOffset;
	bool oldVersion = false;

	IFile file(filename);
	if (!file.isOpen())
		return;

	oldPos = file.tell();
	file.seek(frameOffset);
	ByteVector firstFrame((uint) frameLength, 0);
	bytesRead = file.read(firstFrame.data(), frameLength);
	file.seek(oldPos);
	if (bytesRead != frameLength)
		return;

	xingOffset = firstFrame.find("Xing");
	if (xingOffset == -1)
		xingOffset = firstFrame.find("Info");
	if (xingOffset == -1)
		return;

	lameOffset = xingOffset + 0x78;
	if (lameOffset + 0x24 >= frameLength)
		return;
	
	ByteVector lameTag = firstFrame.mid(lameOffset, 0x24);
	if (!lameTag.startsWith("LAME"))
		return;
	
	encoder = lameTag.mid(0, 9);
	tagRevision = lameTag[9] >> 4;
	encodingMethod = lameTag[9] & 0x0F;
	quality = firstFrame[xingOffset + 0x77];
	stereoMode = lameTag[24] >> 2 & 0x07;
	sourceRate = lameTag[24] >> 6 & 0x03;
	bitrate = lameTag[20] & 0xFF;
	musicLength = lameTag.mid(8, 4).toUInt();
	lowpassFilter = lameTag[10] & 0xFF;
	mp3Gain = (lameTag[25] & 0x7F) * 1.5;
	if (lameTag[25] & 0x80)
		mp3Gain *= -1;
	athType = lameTag[19] & 0x0F;
	encodingFlags = lameTag[19] & 0xF0;
	encodingDelay = (lameTag[21] << 4 & 0xFF0) | (lameTag[22] >> 4 & 0x0F);
	padding = (lameTag[22] << 4 & 0xF00) | (lameTag[23] & 0xFF);
	noiseShaping = lameTag[24] >> 6;
	unwiseSettings = lameTag[24] & 0x20;
	lameTagCRC = (unsigned char) lameTag.mid(34, 2).toUInt();
	musicCRC = (unsigned char) lameTag.mid(32, 2).toUInt();
	peakSignal = (float) (lameTag.mid(11, 4).toUInt() << 5 );
	if (!(encoder < "LAME3.94b"))
		peakSignal = (peakSignal - 0.5) / 8388608.0;
	if (encoder < "LAME3.95")
		oldVersion = true;
	trackGain = replayGain(lameTag.mid(15, 2), oldVersion);
	albumGain = replayGain(lameTag.mid(17, 2), oldVersion);

	valid = true;
}

void LameTag::print(ostringstream &out, bool checkCRC) {
	/*fmtflags oldFlags = out.flags();
	streamsize oldPrecision = out.precision();*/
	char buffer[128];
	ostringstream tmp;

	tmp.setf(ios::fixed);
	tmp.precision(1);

	out << encoder << " tag (revision " << tagRevision << "):" << endl;

	sprintf(buffer, "%-16s: ", "encoding method");
	out << buffer;
	switch (encodingMethod) {
		case 8:
			tmp << "2-pass ";
		case 1:
			tmp << "CBR";
			break;
		case 9:
			tmp << "2-pass ";
		case 2:
			tmp << "ABR";
			break;
		case 3:
		case 4:
		case 5:
		case 6:
			if      (encodingMethod == 3) tmp << "old/re ";
			else if (encodingMethod == 4) tmp << "new/mtrh ";
			else if (encodingMethod == 5) tmp << "new/mt ";
			tmp << "VBR";
			break;
		default:
			tmp << "unknown";
			break;
	}
	sprintf(buffer, "%-15s", tmp.str().c_str());
	tmp.str("");
	out << buffer;

	sprintf(buffer, "%-15s: ", "quality");
	out << buffer;
	if (quality > 0 && quality <= 100) {
		sprintf(buffer, "V%d/q%d", (100 - quality) / 10, (100 - quality) % 10);
		out << buffer;
	} else {
		out << "unknown";
	}
	out << endl;

	sprintf(buffer, "%-16s: ", "stereo mode");
	out << buffer;
	switch (stereoMode) {
		case 0:
			tmp << "mono";
			break;
		case 1:
			tmp << "stereo";
			break;
		case 2:
			tmp << "dual";
			break;
		case 3:
			tmp << "joint";
			break;
		case 4:
			tmp << "force";
			break;
		case 5:
			tmp << "auto";
			break;
		case 6:
			tmp << "intensity";
			break;
		default:
			tmp << "undefined";
			break;
	}
	sprintf(buffer, "%-15s", tmp.str().c_str());
	tmp.str("");
	out << buffer;

	sprintf(buffer, "%-15s: ", "source rate");
	out << buffer;
	switch (sourceRate) {
		case 0:
			out << "<= 32";
			break;
		case 1:
			out << "44.1";
			break;
		case 2:
			out << "48";
			break;
		default:
			out << "> 48";
			break;
	}
	out << " kHz" << endl;

	if (encodingMethod == 2 || encodingMethod == 9)
		tmp << "average ";
	else if (encodingMethod > 2 && encodingMethod < 7)
		tmp << "minimal ";
	tmp << "bitrate";
	sprintf(buffer, "%-16s: ", tmp.str().c_str());
	tmp.str("");
	if (bitrate == 0xFF)
		tmp << ">= ";
	tmp << bitrate << " kBit/s";
	sprintf(buffer, "%-15s", tmp.str().c_str());
	tmp.str("");

	sprintf(buffer, "%-15s: ", "music length");
	out << buffer;
	out << FileIO::sizeHumanReadable(musicLength);
	out << endl;

	sprintf(buffer, "%-16s: ", "lowpass");
	if (lowpassFilter == 0) {
		tmp << "unknown";
	} else {
		tmp << lowpassFilter << "00 Hz";
	}
	sprintf(buffer, "%-15s", tmp.str().c_str());
	tmp.str("");
	out << buffer;

	sprintf(buffer, "%-15s: ", "mp3gain");
	out << buffer;
	if (mp3Gain != 0.0) {
		sprintf(buffer, "%+.f dB", mp3Gain);
		out << buffer;
	}
	else {
		out << "none";
	}
	out << endl;

	sprintf(buffer, "%-16s: ", "ATH type");
	out << buffer;
	tmp << athType;
	sprintf(buffer, "%-15s", tmp.str().c_str());
	tmp.str("");
	out << buffer;

	sprintf(buffer, "%-15s: ", "encoding flags");
	out << buffer;
	bool flag = false;
	if (encodingFlags & 0x10) {
		out << "nspsytune ";
		flag = true;
	}
	if (encodingFlags & 0x20) {
		out << "nssafejoint ";
		flag = true;
	}
	if (encodingFlags & 0xC0) {
		out << "nogap";
		flag = true;
		if (encodingFlags & 0x80)
			out << "<";
		if (encodingFlags & 0x40)
			out << ">";
	}
	if (!flag)
		out << "none";
	out << endl;

	sprintf(buffer, "%-16s: ", "encoding delay");
	out << buffer;
	tmp << encodingDelay << " samples";
	sprintf(buffer, "%-15s", tmp.str().c_str());
	tmp.str("");
	out << buffer;

	sprintf(buffer, "%-15s: ", "padding");
	out << buffer << padding << " samples" << endl;

	sprintf(buffer, "%-16s: ", "noise shaping");
	out << buffer;
	tmp << noiseShaping;
	sprintf(buffer, "%-15s", tmp.str().c_str());
	tmp.str("");
	out << buffer;

	sprintf(buffer, "%-15s: ", "unwise settings");
	out << buffer << (unwiseSettings ? "yes" : "no") << endl;

	/*printf("%-16s: ", "info tag CRC");
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
	cout << endl;*/

	/*out.flags(oldFlags);
	out.precision(oldPrecision);*/
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
