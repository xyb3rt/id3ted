/* id3ted: misc.h
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

#ifndef __MISC_H__
#define __MISC_H__

#include "id3ted.h"

#ifdef NO_STR_BASENAME
const char* basename(const char*);
#endif
void* s_malloc(size_t);
void printVersion();
void printUsage();
const char* getMimeType(const char*);
void trim_whitespace(std::string&);
int creat_dir_r(const char*);
bool confirm_overwrite(const char*);
void crc16_block(unsigned short*, const char*, int);
void crc16_last_block(unsigned short*, const char*, int);
void crc16_checksum(unsigned short*, const char*, int);
int pre_backslash_cnt(const char*, unsigned int);

#endif /* __MISC_H__ */

