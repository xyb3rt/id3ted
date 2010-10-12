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

LameTag::LameTag(const char *_filename, long _frameOffset, long _frameLength) :
		valid(false), filename(_filename),
		frameOffset(_frameOffset), frameLength(_frameLength) {
	long oldPos, bytesRead, xingOffset, lameOffset;
	bool oldVersion = false;

	if (filename == NULL)
		return;
	IFile file(filename);
	if (!file.isOpen())
		return;

	oldPos = file.tell();
	file.seek(frameOffset);
	frame.resize(frameLength);
	bytesRead = file.read(frame.data(), frameLength);
	file.seek(oldPos);
	if (bytesRead != frameLength)
		return;

	xingOffset = frame.find("Xing");
	if (xingOffset == -1)
		xingOffset = frame.find("Info");
	if (xingOffset == -1)
		return;

	lameOffset = xingOffset + 0x78;
	if (lameOffset + 0x24 >= frameLength)
		return;
	
	ByteVector lameTag = frame.mid(lameOffset, 0x24);
	if (!lameTag.startsWith("LAME"))
		return;
	
	encoder = lameTag.mid(0, 9);
	tagRevision = lameTag[9] >> 4;
	encodingMethod = lameTag[9] & 0x0F;
	quality = frame[xingOffset + 0x77];
	stereoMode = lameTag[24] >> 2 & 0x07;
	sourceRate = lameTag[24] >> 6 & 0x03;
	bitrate = lameTag[20] & 0xFF;
	musicLength = lameTag.mid(28, 4).toUInt();
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
	tagCRC = (unsigned short) lameTag.mid(34, 2).toUInt();
	musicCRC = (unsigned short) lameTag.mid(32, 2).toUInt();
	peakSignal = (float) (lameTag.mid(11, 4).toUInt() << 5);
	if (!(encoder < "LAME3.94b"))
		peakSignal = (peakSignal - 0.5) / 8388608.0;
	if (encoder < "LAME3.95")
		oldVersion = true;
	trackGain = replayGain(lameTag.mid(15, 2), oldVersion);
	albumGain = replayGain(lameTag.mid(17, 2), oldVersion);

	valid = true;
}

void LameTag::print(ostringstream &out, bool checkCRC) {
	const uint bufSize = 32;
	char buf[bufSize];
	ostringstream tmp;

	ios::fmtflags oldFlags = out.flags();
	streamsize oldWidth = out.width();
	char oldFillChar = out.fill();

	out.setf(ios::left);
	out.fill(' ');
	tmp.setf(ios::fixed);
	tmp.precision(1);

	out << encoder << " tag (revision " << (tagRevision) << "):" << endl;

	leftKey(out, "encoding method");
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
	leftValue(out, tmp.str());
	tmp.str("");

	rightKey(out, "quality");
	if (quality > 0 && quality <= 100) {
		snprintf(buf, bufSize, "V%d/q%d", (100 - quality) / 10,
				(100 - quality) % 10);
		rightValue(out, buf);
	} else {
		rightValue(out, "unknown");
	}

	leftKey(out, "stereo mode");
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
	leftValue(out, tmp.str());
	tmp.str("");

	rightKey(out, "source rate");
	switch (sourceRate) {
		case 0:
			tmp << "<= 32";
			break;
		case 1:
			tmp << "44.1";
			break;
		case 2:
			tmp << "48";
			break;
		default:
			tmp << "> 48";
			break;
	}
	tmp << " kHz";
	rightValue(out, tmp.str());
	tmp.str("");

	if (encodingMethod == 2 || encodingMethod == 9)
		tmp << "average ";
	else if (encodingMethod > 2 && encodingMethod < 7)
		tmp << "minimal ";
	tmp << "bitrate";
	leftKey(out, tmp.str());
	tmp.str("");
	if (bitrate == 0xFF)
		tmp << ">= ";
	tmp << bitrate << " kBit/s";
	leftValue(out, tmp.str());
	tmp.str("");

	rightKey(out, "music length");
	rightValue(out, FileIO::sizeHumanReadable(musicLength));

	leftKey(out, "lowpass");
	if (lowpassFilter == 0) {
		tmp << "unknown";
	} else {
		tmp << lowpassFilter << "00 Hz";
	}
	leftValue(out, tmp.str());
	tmp.str("");

	rightKey(out, "mp3gain");
	if (mp3Gain != 0.0) {
		snprintf(buf, bufSize, "%+.f dB", mp3Gain);
		rightValue(out, buf);
	}
	else {
		rightValue(out, "none");
	}

	leftKey(out, "ATH type");
	tmp << athType;
	leftValue(out, tmp.str());
	tmp.str("");

	rightKey(out, "encoding flags");
	bool flag = false;
	if (encodingFlags & 0x10) {
		tmp << "nspsytune ";
		flag = true;
	}
	if (encodingFlags & 0x20) {
		tmp << "nssafejoint ";
		flag = true;
	}
	if (encodingFlags & 0xC0) {
		tmp << "nogap";
		flag = true;
		if (encodingFlags & 0x80)
			tmp << "<";
		if (encodingFlags & 0x40)
			tmp << ">";
	}
	if (!flag)
		tmp << "none";
	rightValue(out, tmp.str());
	tmp.str("");

	leftKey(out, "encoding delay");
	tmp << encodingDelay << " samples";
	leftValue(out, tmp.str());
	tmp.str("");

	rightKey(out, "padding");
	tmp << padding << " samples";
	rightValue(out, tmp.str());
	tmp.str("");

	leftKey(out, "noise shaping");
	tmp << noiseShaping;
	leftValue(out, tmp.str());
	tmp.str("");

	rightKey(out, "unwise settings");
	tmp << (unwiseSettings ? "yes" : "no");
	rightValue(out, tmp.str());
	tmp.str("");

	leftKey(out, "info tag CRC");
	snprintf(buf, bufSize, "%04X ", tagCRC);
	tmp << buf;

	if (checkCRC) {
		int size = frameLength < 190 ? frameLength : 190;
		unsigned short crc = 0;

		crc16Checksum(&crc, frame.data(), size);
		tmp << "(" << (crc != tagCRC ? "invalid" : "correct") << ")";
	}
	leftValue(out, tmp.str());
	tmp.str("");

	rightKey(out, "music CRC");
	snprintf(buf, bufSize, "%04X ", musicCRC);
	tmp << buf;

	if (checkCRC) {
		unsigned short crc = 0;
		size_t size = musicLength - frameLength;
		char *buffer = new char[FILE_BUF_SIZE];
		IFile file(filename);

		file.seek(frameOffset + frameLength);
		while (size > 0 && !file.eof() && !file.error()) {
			size_t blockSize = size < FILE_BUF_SIZE ? size : FILE_BUF_SIZE;
			file.read(buffer, blockSize);
			if (file.error()) {
				cerr << command << ": " << filename
						 << ": Error reading file" << endl;
				return;
			}
			size -= blockSize;
			if (size > 0) {
				crc16Block(&crc, buffer, blockSize);
			} else {
				crc16LastBlock(&crc, buffer, blockSize);
			}
		}

		delete [] buffer;
		tmp << "(" << (crc != musicCRC ? "invalid" : "correct") << ")";
	}
	rightValue(out, tmp.str());
	tmp.str("");

	leftKey(out, "ReplayGain: peak");
	tmp << (int) peakSignal * 100;
	rightValue(out, tmp.str());
	tmp.str("");

	leftKey(out, "track gain");
	if (trackGain > 0.0)
		tmp << "+";
	tmp << trackGain << " dB";
	leftValue(out, tmp.str());
	tmp.str("");

	rightKey(out, "album gain");
	if (albumGain > 0.0)
		tmp << "+";
	tmp << albumGain << " dB";
	rightValue(out, tmp.str());
	tmp.str("");

	out.flags(oldFlags);
	out.width(oldWidth);
	out.fill(oldFillChar);
}

void LameTag::leftKey(ostringstream &out, const string &key) {
	out.width(16);
	out << key << ": ";
}

void LameTag::leftValue(ostringstream &out, const string &value) {
	out.width(15);
	out << value;
}

void LameTag::rightKey(ostringstream &out, const string &key) {
	out.width(15);
	out << key << ": ";
}

void LameTag::rightValue(ostringstream &out, const string &value) {
	out.width(1);
	out << value << endl;
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
