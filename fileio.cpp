#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <magic.h>

#include <taglib/tfile.h>

#include "fileio.h"

#ifdef __APPLE__
#define st_atim st_atimespec
#define st_mtim st_mtimespec
#endif

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
	return !access(path, F_OK);
}

bool FileIO::isRegular(const char *path) {
	struct stat stats;

	if (stat(path, &stats) == -1) {
		cerr << command << ": " << path << ": Could not stat file" << endl;
		return false;
	}

	return S_ISREG(stats.st_mode);
}


bool FileIO::isReadable(const char *path) {
	return TagLib::File::isReadable(path);
}

bool FileIO::isWritable(const char *path) {
	return TagLib::File::isWritable(path);
}

string FileIO::sizeHumanReadable(unsigned long size) {
	float size_hr = size;
	const char *unit = NULL;
	ostringstream ret;

	ret.setf(ios::fixed);
	ret.precision(2);

	if (size_hr >= 1024) {
		size_hr /= 1024;
		unit = "KB";
	}
	if (size_hr >= 1024) {
		size_hr /= 1024;
		unit = "MB";
	}
	if (size_hr >= 1024) {
		size_hr /= 1024;
		unit = "GB";
	}

	if (size != size_hr)
		ret << size_hr << " " << unit << " (" << size << " bytes)";
	else
		ret << size << " bytes";

	return ret.str();
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

bool FileIO::saveTimes(const char *filename, FileTimes &times) {
	struct stat stats;

	if (filename == NULL)
		return false;
	if (stat(filename, &stats) == -1) {
		cerr << command << ": " << filename << ": Could not stat file" << endl;
		return false;
	}
	TIMESPEC_TO_TIMEVAL(&times.access, &stats.st_atim);
	TIMESPEC_TO_TIMEVAL(&times.modification, &stats.st_mtim);
	
	return true;
}

bool FileIO::resetTimes(const char *filename, const FileTimes &times) {
	struct timeval ptimes[2];

	if (filename == NULL)
		return false;
	ptimes[0] = times.access;
	ptimes[1] = times.modification;

	return utimes(filename, ptimes) == 0;
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
		if (strcmp(buffer, "\n") == 0 || strcmp(buffer, "n\n") == 0 ||
				strcmp(buffer, "N\n") == 0) {
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

FileIO::FileIO(const char *_path, const char *_mode) :
		stream(NULL), path(_path), mode(_mode) {
	if (path == NULL || mode == NULL)
		return;
	
	stream = fopen(path, mode);
	if (stream == NULL) {
		fprintf(stderr, "%s: %s: ", command, path);
		perror(NULL);
	}
}

FileIO::~FileIO() {}

int FileIO::close() {
	int error;

	error = fclose(stream);
	stream = NULL;

	return error;
}

long FileIO::tell() {
	if (stream == NULL)
		return -1;
	else
		return ftell(stream);
}

bool FileIO::seek(long offset) {
	if (stream == NULL)
		return false;
	else
		return fseek(stream, offset, SEEK_SET) == 0;
}

size_t IFile::read(char *buffer, size_t size) {
	size_t cnt = 0;

	if (stream == NULL)
		return 0;

	while (cnt < size && !feof(stream) && !ferror(stream))
		cnt += fread(buffer, 1, size - cnt, stream);

	return cnt;
}

size_t IFile::read(ByteVector &vector) {
	size_t cnt = 0, oldPos, size;
	char *buffer;

	if (stream == NULL)
		return 0;

	oldPos = ftell(stream);
	fseek(stream, 0, SEEK_END);
	size = ftell(stream);
	fseek(stream, 0, SEEK_SET);

	vector.resize(size);
	buffer = vector.data();

	while (cnt < size && !feof(stream) && !ferror(stream))
		fread(buffer, 1, size - cnt, stream);
	
	if (!ferror(stream))
		fseek(stream, oldPos, SEEK_SET);

	return cnt;
}

size_t OFile::write(const char *buffer, size_t size) {
	size_t cnt = 0;

	if (stream == NULL)
		return 0;

	while (cnt < size && !ferror(stream))
		cnt += fwrite(buffer, 1, size - cnt, stream);

	return cnt;
}

size_t OFile::write(const ByteVector &vector) {
	size_t cnt = 0, size;
	const char *buffer;

	if (stream == NULL)
		return 0;
	
	size = vector.size();
	buffer = vector.data();

	while (cnt < size && !ferror(stream))
		cnt += fwrite(buffer, 1, size - cnt, stream);

	return cnt;
}
