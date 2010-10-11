/* id3ted: lametag.h
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

#ifndef __LAMETAG_H__
#define __LAMETAG_H__

#include <taglib/tbytevector.h>

#include "id3ted.h"

class LameTag {
	public:
		LameTag(const char*, long);
		~LameTag();

		void print(bool);

	private:
		double replayGain(const ByteVector&, bool);

		/* calculate the crc16 checksum of a large chunk of data blockwise:
		 * call crc16Block() for every block but the last one, making sure
		 * that the following condition is true for all of them: size % 8 == 0!
		 * use crc16LastBlock() for the last block.
		 *
		 * thanks to MAD! (http://www.underbit.com/products/mad/) */
		void crc16Block(unsigned short*, const char*, size_t);
		void crc16LastBlock(unsigned short*, const char*, size_t);
		/* alias for crc16LastBlock() to calculate checksum of
		 * single-block data using a more appropriate function name */
		void crc16Checksum(unsigned short*, const char*, int);

		static unsigned short crc16Table[];
		
		String encoder;
		int encodingMethod;
		int quality;
		char stereoMode;
		char sourceRate;
		unsigned char bitrate;
		int musicLength;
		char lowpassFilter;
		float mp3Gain;
		char athType;
		char encodingFlags;
		int encodingDelay;
		int padding;
		char noiseShaping;
		bool unwiseSettings;
		unsigned char lameTagCRC;
		unsigned char musicCRC;
		float peakSignal;
		double trackGain;
		double albumGain;
};

#endif /* __LAMETAG_H__ */
