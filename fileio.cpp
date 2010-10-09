#include <cstring>
#include <cstdlib>
#include <iostream>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <magic.h>

#include <taglib/tfile.h>

#include "id3ted.h"
#include "fileio.h"

#ifdef NO_STR_BASENAME
const char* FileIO::basename(const char *path) {
	static char* buffer = NULL;
	static size_t bufferSize = 0;
	int len, pathLen, newLen;

  if (path == NULL || *path == 0)
    return ".";
  res = strrchr(path, '/');
  if (res == NULL) {
    // path contains no slash
    res = path;
  } else {
    if (*(res + 1) == 0) {
      // right most slash is last char in path
      len = 1;
      pathLen = strlen(path);
      // looping back over consecutive slashes at the end
      while (*(--res) == '/' && len < pathLen) len++;
      if (len == pathLen)
				// path only contains slashes
				return "/";
      newLen = 1;
      // looping back to next slash or beginning of path
      while (*(--res) != '/' && len + newLen < pathLen) newLen++;
      if (bufferSize < newLen + 1) {
        if (buffer != NULL)
					delete [] buffer;
        buffer = new char[newLen + 1];
        bufferSize = newLen + 1;
      }
      strncpy(buffer, res+1, newLen);
      buffer[newLen] = '\0';
      res = (const char*) buffer;
    } else {
      res++;
    }
  }
  return res;
}
#endif /* NO_STR_BASENAME */

bool FileIO::exists(const char *path) {
	return access(path, F_OK);
}

bool FileIO::isReadable(const char *path) {
	return TagLib::File::isReadable(path);
}

bool FileIO::isWritable(const char *path) {
	return TagLib::File::isWritable(path);
}

const char* FileIO::mimetype(const char *file) {
  static char *buffer = NULL;
  static size_t bufferSize = 0, len;
	const char *mimetype = NULL, *mttemp;
	struct magic_set *mcookie;

	if ((mcookie = magic_open(MAGIC_MIME|MAGIC_CHECK)) != NULL) {
		if (magic_load(mcookie, NULL) == 0) {
			mimetype = magic_file(mcookie, file);
			len = strlen(mimetype);
			mttemp = strchr(mimetype, ';');
			if (mttemp != NULL)
				len = mttemp - mimetype;
			if (len + 1 > bufferSize) {
				if (buffer != NULL)
					delete [] buffer;
				buffer = new char[len + 1];
				bufferSize = len + 1;
			}
			strncpy(buffer, mimetype, len);
			buffer[len] = '\0';
			mimetype = buffer;
		}
		magic_close(mcookie);
	}
	return mimetype;
}

bool FileIO::createDir(const char *path) {
	char *directory = new char[strlen(path + 1)];
	char *curr = directory;
	struct stat stats;
	bool error = false;

	strcpy(directory, path);
	while (curr != NULL && !error) {
		curr = strchr(curr + 1, '/');
		if (curr != NULL)
			*curr = '\0';
		if (access(directory, F_OK) != 0 && errno == ENOENT) {
			if (mkdir(directory, 0755) != 0) {
				cerr << command << ": " << directory << ": Could not create directory" << endl;
				error = true;
			}
		} else if (stat(directory, &stats) != 0 || !(stats.st_mode & S_IFDIR)) {
			cerr << command << ": " << directory << ": Not a directory" << endl;
			error = true;
		}
		if (curr != NULL)
			*curr = '/';
	}
	if (directory != NULL)
		delete [] directory;

	return error;
}

bool FileIO::confirmOverwrite(const char *filename) {
	char *buffer = new char[10];
	char *userIn;
	bool ret = false;

	while (1) {
		cout << "overwrite " << filename << "? [yN] ";
		userIn = fgets(buffer, 10, stdin);
		if (userIn == NULL)
			continue;
		if (strcmp(buffer, "\n") == 0 || strcmp(buffer, "n\n") == 0 || strcmp(buffer, "N\n") == 0) {
			break;
		} else if (strcmp(buffer, "y\n") == 0 || strcmp(buffer, "Y\n") == 0) {
			ret = true;
			break;
		}
	}
	if (buffer != NULL)
		delete [] buffer;
	
	return ret;
}

FileIO::FileIO(const char *path, const char *mode) :
		_stream(NULL), _path(path), _mode(mode) {
	if (_path == NULL || _mode == NULL)
		return;
	
	_stream = fopen(_path, _mode);
	if (_stream == NULL) {
		fprintf(stderr, "%s: %s: ", command, _path);
		perror(NULL);
	}
}

int FileIO::close() {
	int error;

	error = fclose(_stream);
	_stream = NULL;

	return error;
}

size_t IFile::read(char *buffer, size_t size) {
	size_t cnt = 0;

	if (_stream == NULL)
		return 0;

	while (cnt < size && !feof(_stream) && !ferror(_stream))
		cnt += fread(buffer, 1, size - cnt, _stream);

	return cnt;
}

size_t IFile::read(ByteVector &vector) {
	size_t cnt = 0, oldPos, size;
	char *buffer;

	if (_stream == NULL)
		return 0;

	oldPos = ftell(_stream);
	fseek(_stream, 0, SEEK_END);
	size = ftell(_stream);
	fseek(_stream, 0, SEEK_SET);

	vector.resize(size);
	buffer = vector.data();

	while (cnt < size && !feof(_stream) && !ferror(_stream))
		fread(buffer, 1, size - cnt, _stream);
	
	if (!ferror(_stream))
		fseek(_stream, oldPos, SEEK_SET);

	return cnt;
}

size_t OFile::write(const char *buffer, size_t size) {
	size_t cnt = 0;

	if (_stream == NULL)
		return 0;

	while (cnt < size && !ferror(_stream))
		cnt += fwrite(buffer, 1, size - cnt, _stream);

	return cnt;
}

size_t OFile::write(const ByteVector &vector) {
	size_t cnt = 0, size;
	const char *buffer;

	if (_stream == NULL)
		return 0;
	
	size = vector.size();
	buffer = vector.data();

	while (cnt < size && !ferror(_stream))
		cnt += fwrite(buffer, 1, size - cnt, _stream);

	return cnt;
}
