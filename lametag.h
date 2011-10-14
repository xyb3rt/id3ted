/* id3ted: lametag.h
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

#ifndef __LAMETAG_H__
#define __LAMETAG_H__

#include <taglib/tbytevector.h>

#include "id3ted.h"

class LameTag {
	public:
		LameTag(const char*, long, long);

		bool isValid() const { return valid; }
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
		
		bool valid;
		const char *filename;
		long frameOffset;
		long frameLength;
		ByteVector frame;

		String encoder;
		short tagRevision;
		int encodingMethod;
		int quality;
		short stereoMode;
		short sourceRate;
		unsigned short bitrate;
		unsigned int musicLength;
		short lowpassFilter;
		float mp3Gain;
		short athType;
		short encodingFlags;
		int encodingDelay;
		int padding;
		short noiseShaping;
		bool unwiseSettings;
		unsigned short tagCRC;
		unsigned short musicCRC;
		float peakSignal;
		double trackGain;
		double albumGain;
};

#endif /* __LAMETAG_H__ */
