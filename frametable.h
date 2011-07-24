/* id3ted: frametable.h
 * Copyright (c) 2011 Bert Muennich <muennich at informatik.hu-berlin.de>
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

#ifndef __FRAMETABLE_H__
#define __FRAMETABLE_H__

#include "id3ted.h"

class FrameTable {
	private:
		typedef struct _FrameTableEntry {
			const char *id;
			ID3v2FrameID fid;
			const char *description;
		} FrameTableEntry;
		
		static FrameTableEntry _table[];
		static int _tableSize;
	
	public:
		static const char* frameDescription(const String&);
		static ID3v2FrameID frameID(const String&);
		static const char* textFrameID(ID3v2FrameID);
		static void listFrames();
		static void listGenres();
};

#endif /* __FRAMETABLE_H__ */

