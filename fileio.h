/* id3ted: fileio.h
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

#ifndef __FILEIO_H__
#define __FILEIO_H__

#include <cstdio>
#include <sys/time.h>

#include <taglib/tbytevector.h>

#include "id3ted.h"

typedef struct _FileTimes {
	struct timeval access;
	struct timeval modification;
} FileTimes;

class FileIO {
	public:
#ifdef NO_STR_BASENAME
		static const char* basename(const char*);
#endif	
		static bool exists(const char*);
		static bool isRegular(const char*);
		static bool isReadable(const char*);
		static bool isWritable(const char*);
		static string sizeHumanReadable(unsigned long);
		static const char* mimetype(const char*);
		static bool saveTimes(const char*, FileTimes&);
		static bool resetTimes(const char*, const FileTimes&);
		static bool createDir(const char*);
		static bool confirmOverwrite(const char*);

		FileIO(const char*, const char*);
		virtual ~FileIO() = 0;

		bool isOpen() { return stream != NULL; }
		int close();
		bool eof() { return stream != NULL && feof(stream); }
		bool error() { return stream == NULL || ferror(stream); }
		long tell();
		bool seek(long);

	protected:
		FILE *stream;
		const char *path;
		const char *mode;
};

class IFile : public FileIO {
	public:
		IFile(const char *_path) : FileIO(_path, "r") {}
		~IFile() { if (stream) close(); }

		size_t read(char*, size_t);
		size_t read(ByteVector&);
};

class OFile : public FileIO {
	public:
		OFile(const char *_path) : FileIO(_path, "w+") {}
		~OFile() { if (stream) close(); }

		size_t write(const char*, size_t);
		size_t write(const ByteVector&);
};

#endif /* __FILEIO_H__ */
