/* id3ted: file.h
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

#ifndef __FILE_H__
#define __FILE_H__

#include <cstdio>
#include <taglib/tbytevector.h>

class File {
	public:
#ifdef NO_STR_BASENAME
		static const char* basename(const char*);
#endif	
		static const char* mimetype(const char*);
		static bool createDir(const char*);
		static bool confirmOverwrite(const char*);

		File(const char*, const char*);
		virtual ~File() = 0;

		bool isOpen() { return _stream != NULL; }
		int close();
		bool eof() { return _stream != NULL && feof(_stream); }
		bool error() { return _stream == NULL || ferror(_stream); }

	protected:
		FILE *_stream;
		const char *_path;
		const char *_mode;
};

class IFile : public File {
	public:
		IFile(const char *path) : File(path, "r") {}
		~IFile() { if (_stream) close(); }

		size_t read(char*, size_t);
		size_t read(ByteVector&);
};

class OFile : public File {
	public:
		OFile(const char *path) : File(path, "w+") {}
		~OFile() { if (_stream) close(); }

		size_t write(char*, size_t);
};

#endif /* __FILE_H__ */
