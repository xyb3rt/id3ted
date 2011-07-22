/* size of buffer used for file copy (in bytes):      */
enum { FILE_BUF_SIZE = 4096 };

/* default delimiter for multiple field id3v2 frames: */
const char FIELD_DELIM = ':';

/* use unicode in string <-> bytevector conversions?  */
const bool USE_UNICODE = true;

/* default encoding used for TagLib::String objects:  */
/* (see http://developer.kde.org/~wheeler/taglib/api/classTagLib_1_1String.html) */
#define DEF_TSTR_ENC TagLib::String::UTF8

/* replace special characters in filenames with:      */
const char REPLACE_CHAR = '_';
