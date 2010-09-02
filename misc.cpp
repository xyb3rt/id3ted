/* id3ted: misc.cpp
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

#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <magic.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "misc.h"

static unsigned short crc16_table[256] = {
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

#ifdef NO_STR_BASENAME
// should behave like the implementation found on most *nix-systems
const char* basename(const char *string) {
  // statically allocated buffer, sometimes used for result
  static char *buf = NULL, *res;
  static int buf_size = 0;
	int len, str_len, new_len;

  if (string == NULL || *string == 0)
    return ".";
  res = strrchr(string, '/');
  if (res == NULL) {
    // string contains no slash
    res = string;
  } else {
    if (*(res + 1) == 0) {
      // right most slash is last char in string
      len = 1;
      str_len = strlen(string);
      // looping back over consecutive slashes at the end
      while (*(--res) == '/' && len < str_len) len++;
			// string only contains slashes:
      if (len == str_len) return "/";
      new_len = 1;
      // looping back to next slash or beginning of string
      while (*(--res) != '/' && len + new_len < str_len) new_len++;
      if (buf_size < new_len + 1) {
        if (buf != NULL)
					free(buf);
        buf = (char*) s_malloc(new_len + 1);
        buf_size = new_len + 1;
      }
      strncpy(buf, res+1, new_len);
      buf[new_len] = '\0';
      res = (const char*) buf;
    } else {
      res++;
    }
  }

  return res;
}
#endif

void* s_malloc(size_t cnt) {
	void *ptr = malloc(cnt);
	
	if (ptr == NULL) {
		cerr << g_progname << ": could not allocate memory" << endl;
		exit(1);
	}

	return ptr;
}

void printVersion() {
	cout << PROGNAME << " - command line id3 tag editor\n"
	     << "Version " << VERSION << ", written by Bert Muennich\n"
	     << "Uses TagLib v" << TAGLIB_MAJOR_VERSION << "." << TAGLIB_MINOR_VERSION << "." << TAGLIB_PATCH_VERSION << ", written by Scott Wheeler" << endl;
}

void printUsage() {
	cout << "Usage: " << g_progname << " [OPTIONS]... <FILES>\n\n"
	     << "OPTIONS:\n"
	     << "If a long option shows an argument as mandatory,\n"
	     << "then it is also mandatory for the equivalent short option.\n\n"
	     << "  -h, --help             display this help and exit\n"
	     << "  -v, --version          display version information and exit\n"
	     << "      --frame-list       list all possible frame types for id3v2\n"
	     << "      --genre-list       list all id3v1 genres and their corresponding numbers\n"
	     << "  -p, --preserve-times   preserve access and modification times of the files\n"
	     << "  -d, --delimiter CHAR   set the delimiter for multiple field option arguments\n"
	     << "                         to the given character (default is '" << FIELD_DELIM << "')\n\n";
	cout << "To alter the most common tag information:\n"
	     << "  -a, --artist ARTIST    set the artist information\n"
	     << "  -A, --album ALBUM      set the album title information\n"
	     << "  -t, --title SONG       set the song title information\n"
	     << "  -c, --comment COMMENT  set the comment information\n"
	     << "  -g, --genre NUM        set the genre number\n"
	     << "  -T, --track NUM[/NUM]  set the track number/optional: total # of tracks\n"
	     << "  -y, --year NUM         set the year\n\n";
	cout << "Get information from the files:\n"
	     << "  -i, --info             display general information for the files\n"
	     << "  -l, --list             list the tags on the files\n"
	     << "  -L, --list-wd          same as -l, but list id3v2 frames with description\n"
	     << "  -m, --lame-tag         print the lame tags of the files\n"
	     << "  -M, --lame-tag-crc     same as -m, but verify CRC checksums (slower)\n\n"
	     << "To remove tags & specify which tag version(s) to write:\n"
	     << "  -r, --remove FID       remove all id3v2 frames with the given frame id\n"
	     << "  -D, --delete-all       delete both id3v1 and id3v2 tag\n"
	     << "  -s, --strip-v1         strip id3v1 tag\n"
	     << "  -S, --strip-v2         strip id3v2 tag\n"
	     << "  -1                     write only id3v1 tag,\n"
	     << "                         convert v2 to v1 tag if file has no id3v1 tag\n"
	     << "  -2                     same as -1, but vice versa\n"
	     << "  -3                     write both id3v1 and id3v2 tag,\n"
	     << "                         create and convert non-existing tags\n\n";
	cout << "Filename <-> tag information:\n"
	     << "  -n, --file-pattern PATTERN\n"
	     << "                         extract tag information from the given filenames,\n"
	     << "                         using PATTERN (for supported wildcards see below)\n"
	     << "  -N, --file-regex PATTERN\n"
	     << "                         same as -n, but interpret PATTERN as an extended regex\n"
	     << "  -o, --organize PATTERN organize files into directory structure specified\n"
	     << "                         by PATTERN (for supported wildcards see below)\n"
	     << "  -x, --extract-apics    extract attached pictures into the current\n"
	     << "                         working directory as FILENAME.apic-NUM.FORMAT\n"
	     << "  -f, --force            overwrite existing files without asking (-o,-x)\n"
	     << "      --move             when using -o, move files instead of copying them\n\n";
	cout << "The following wildcards are supported for the -o,-n,-N option arguments:\n"
	     << "    %a: Artist, %A: album, %t: title, %g: genre, %y: year,\n"
	     << "    %d: disc number, %n: track number, %%: percent sign\n\n"
	     << "You can add and modify almost any id3v2 frame by using its 4-letter frame id\n"
	     << "as a long option and the value to apply as the option argument.\n"
	     << "Use --list-frames to get a list of supported frames (marked with *).\n"
	     << "The argument for --APIC has to be the path of an image file!\n\n"
	     << "There are some frames which support multiple field arguments:\n"
	     << "      --COMM COMMENT[" << FIELD_DELIM << "DESCRIPTION[" << FIELD_DELIM << "LANGUAGE]]\n"
	     << "      --TXXX TEXT" << FIELD_DELIM << "DESCRIPTION\n"
	     << "      --USLT LYRICS[" << FIELD_DELIM << "DESCRIPTION[" << FIELD_DELIM << "LANGUAGE]]\n"
	     << "      --WXXX URL[" << FIELD_DELIM << "DESCRIPTION]\n"
	     << "Fields in square brackets are optional, LANGUAGE is an ISO-639-2 3-byte code." << endl;
}

const char* getMimeType(const char *file) {
  static char *buf = NULL;
  static int buf_size = 0, mtlen;
	const char *mimetype = NULL, *mttemp;
	struct magic_set *mcookie;

	if ((mcookie = magic_open(MAGIC_MIME|MAGIC_CHECK)) != NULL) {
		if (magic_load(mcookie, NULL) == 0) {
			mimetype = magic_file(mcookie, file);
			mtlen = strlen(mimetype);
			mttemp = strchr(mimetype, ';');
			if (mttemp != NULL)
				mtlen = mttemp - mimetype;
			if (mtlen + 1 > buf_size) {
				if (buf != NULL)
					free(buf);
				buf = (char*) s_malloc(mtlen + 1);
				buf_size = mtlen + 1;
			}
			strncpy(buf, mimetype, mtlen);
			buf[mtlen] = '\0';
			mimetype = buf;
		}
		magic_close(mcookie);
	}
	
	return mimetype;
}

void trim_whitespace(std::string &str) {
	std::string::iterator str_iter = str.begin();
	while (*str_iter == ' ' && str_iter != str.end())
		str_iter++;
	if (str_iter != str.begin())
		str.erase(str.begin(), str_iter);

	std::string::reverse_iterator str_riter = str.rbegin() + 1;
	while (*str_riter == ' ' && str_riter != str.rend())
		str_riter++;
	if (str_riter != str.rbegin() + 1)
		str.erase(str_riter.base(), str.end());
}

int creat_dir_r(const char *path) {
	char *directory = (char*) s_malloc(strlen(path + 1));
	char *curr = directory;
	struct stat dir_stats;
	int ret = 0;

	strcpy(directory, path);
	while (curr != NULL && ret == 0) {
		curr = strchr(curr + 1, '/');
		if (curr != NULL)
			*curr = '\0';
		if (access(directory, F_OK) != 0 && errno == ENOENT) {
			if (mkdir(directory, 0755) != 0) {
				cerr << g_progname << ": " << directory << ": Could not create directory" << endl;
				ret = 1;
			}
		} else if (stat(directory, &dir_stats) != 0 || !(dir_stats.st_mode & S_IFDIR)) {
			cerr << g_progname << ": " << directory << ": Not a directory" << endl;
			ret = 2;
		}
		if (curr != NULL)
			*curr = '/';
	}
	if (directory != NULL)
		free(directory);

	return ret;
}

bool confirm_overwrite(const char *filename) {
	char *buf = (char*) s_malloc(10);
	char *userIn;
	bool ret = false;

	while (1) {
		cout << "overwrite " << filename << "? [yN] ";
		userIn = fgets(buf, 10, stdin);
		if (userIn == NULL)
			continue;
		if (strcmp(buf, "\n") == 0 || strcmp(buf, "n\n") == 0 || strcmp(buf, "N\n") == 0) {
			break;
		} else if (strcmp(buf, "y\n") == 0 || strcmp(buf, "Y\n") == 0) {
			ret = true;
			break;
		}
	}
	if (buf != NULL)
		free(buf);
	
	return ret;
}

/* calculate the crc16 checksum of a large chunk of data blockwise:
 * call crc16_block() for every block but the last one, making sure
 * that the following condition is true for all of them: size % 8 == 0!
 * use crc16_last_block() for the last block.
 *
 * thanks to MAD! (http://www.underbit.com/products/mad/) */
void crc16_block(unsigned short *crc, const char *data, int size) {
	for (; size >= 8; size -= 8) {
		*crc = crc16_table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16_table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16_table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16_table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16_table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16_table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16_table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16_table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	}
}

void crc16_last_block(unsigned short *crc, const char *data, int size) {
	crc16_block(crc, data, size);
	data += size - (size % 8);
	switch (size % 8) {
	  case 7: *crc = crc16_table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 6: *crc = crc16_table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 5: *crc = crc16_table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 4: *crc = crc16_table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 3: *crc = crc16_table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 2: *crc = crc16_table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 1: *crc = crc16_table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 0: break;
	}
}

/* alias for crc16_last_block() to calculate checksum of
 * single-block data using a more appropriate function name */
void crc16_checksum(unsigned short *crc, const char *data, int size) {
	crc16_last_block(crc, data, size);
}

int pre_backslash_cnt(const char* str, unsigned int i) {
	int bs_cnt = 0;

	if (i > strlen(str))
		return -1;
	for (; i > 0 && str[i-1] == '\\'; i--)
		bs_cnt++;

	return bs_cnt;
}

