#include <cstring>
#include <cstdlib>

#include <taglib/tbytevector.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/commentsframe.h>
#include <taglib/textidentificationframe.h>
#include <taglib/unsynchronizedlyricsframe.h>
#include <taglib/urllinkframe.h>

#include "frametable.h"
#include "options.h"

int Options::tagsToWrite = 0;
int Options::tagsToStrip = 0;
bool Options::writeFile = false;
bool Options::extractAPICs = false;
bool Options::showInfo = false;
bool Options::listTags = false;
bool Options::listV2WithDesc = false;
bool Options::printLameTag = false;
bool Options::checkLameCRC = false;
bool Options::forceOverwrite = false;
char Options::fieldDelimiter = FIELD_DELIM;
bool Options::preserveTimes = false;
bool Options::moveFiles = false;

bool Options::parseCommandLine(int argc, char **argv) {
	int opt;
	bool error = false;

	while (!error) {
		optFrameID = FID3_XXXX;

		opt = getopt_long(argc, argv, options, longOptions, NULL);

		if (opt == -1)
			break;

		switch (opt) {
			/* invalid option given */
			case '?':
				error = true;
				break;
			/* help, general info & others */
			case 'h':
				printUsage();
				exit(0);
			case 'v':
				printVersion();
				exit(0);
			case OPT_LO_FRAME_LIST:
				FrameTable::listFrames();
				exit(0);
			case OPT_LO_GENRE_LIST:
				FrameTable::listGenres();
				exit(0);
			case 'p':
				preserveTimes = true;
				break;
			case 'd':
				if (strlen(optarg) == 1) {
					fieldDelimiter = optarg[0];
				} else {
					cerr << command << ": The argument of -d/--delimiter "
					     << "has to be a single character" << endl;
					error = true;
				}
				break;
			/* generic tag infomation */
			case 'a':
			case 'A':
			case 't':
			case 'c':
			case 'g':
			case 'T':
			case 'y': {
				GenericInfo *info = new GenericInfo((char) opt, optarg);
				if (info != NULL) {
					genericMods.push_back(info);
					writeFile = true;
				}
				break;
			}
			/* information from the files */
			case 'i':
				showInfo = true;
				break;
			case 'L':
				listV2WithDesc = true;
			case 'l':
				listTags = true;
				break;
			case 'M':
				checkLameCRC = true;
			case 'm':
				printLameTag = true;
				break;
			/* tag removal & version to write */
			case 'r':
				if (FrameTable::frameID(optarg) != FID3_XXXX) {
					framesToRemove.push_back(optarg);
					writeFile = true;
				} else {
					cerr << command << ": -r: invalid id3v2 frame id: " << optarg << endl;
					error = true;
				}
				break;
			case 's':
				tagsToStrip |= 1;
				writeFile = true;
				break;
			case 'S':
				tagsToStrip |= 2;
				writeFile = true;
				break;
			case 'D':
				tagsToStrip = 3;
				writeFile = true;
				break;
			case '1':
				tagsToWrite |= 1;
				writeFile = true;
				break;
			case '2':
				tagsToWrite |= 2;
				writeFile = true;
				break;
			case '3':
				tagsToWrite = 3;
				writeFile = true;
				break;
			/* filename <-> tag information */
			/*case 'N':
			case 'n': {
				if (fPathRegEx != NULL) {
					multOptWarning.push_back("n");
					break;
				}

				fPathRegEx = (regex_t*) s_malloc(sizeof(regex_t));
				ostringstream regExPattern;
				bool wcardPresent = false;

				for (uint i = 0; i < strlen(optarg) && fPathRegEx != NULL; i++) {
					if (optarg[i] == '%') {
						if (i + 1 >= strlen(optarg)) {
							cerr << g_progname << ": -" << (char) opt << " option ignored, because pattern ends with an invalid character" << endl;
							free(fPathRegEx);
							fPathRegEx = NULL;
							break;
						}

						const char *textFID;
						FrameInfo *newFI;

						switch (optarg[i+1]) {
							case 'a': {
								textFID = FrameTable::textFrameID(FID3_TPE1);
								newFI = new FrameInfo(textFID, FID3_TPE1, "", fPathNMatch++);
								regExPattern << "(.+)";
								wcardPresent = true;
								break;
							}
							case 'A': {
								textFID = FrameTable::textFrameID(FID3_TALB);
								newFI = new FrameInfo(textFID, FID3_TALB, "", fPathNMatch++);
								regExPattern << "(.+)";
								wcardPresent = true;
								break;
							}
							case 't': {
								textFID = FrameTable::textFrameID(FID3_TIT2);
								newFI = new FrameInfo(textFID, FID3_TIT2, "", fPathNMatch++);
								regExPattern << "(.+)";
								wcardPresent = true;
								break;
							}
							case 'g': {
								textFID = FrameTable::textFrameID(FID3_TCON);
								newFI = new FrameInfo(textFID, FID3_TCON, "", fPathNMatch++);
								regExPattern << "(.+)";
								wcardPresent = true;
								break;
							}
							case 'y': {
								textFID = FrameTable::textFrameID(FID3_TDRC);
								newFI = new FrameInfo(textFID, FID3_TDRC, "", fPathNMatch++);
								regExPattern << "([0-9]+)";
								wcardPresent = true;
								break;
							}
							case 'n': {
								textFID = FrameTable::textFrameID(FID3_TRCK);
								newFI = new FrameInfo(textFID, FID3_TRCK, "", fPathNMatch++);
								regExPattern << "([0-9]+)";
								wcardPresent = true;
								break;
							}
							case 'd': {
								textFID = FrameTable::textFrameID(FID3_TPOS);
								newFI = new FrameInfo(textFID, FID3_TPOS, "", fPathNMatch++);
								regExPattern << "([0-9]+)";
								wcardPresent = true;
								break;
							}
							case '%': {
								regExPattern << "%";
								i++;
								continue;
							}
							default: {
								cerr << g_progname << ": -" << (char) opt << " option ignored, because pattern contains invalid wildcard: `" << optarg[i] << optarg[i+1] << "'" << endl;
								free(fPathRegEx);
								fPathRegEx = NULL;
								continue;
							}
						}

						if (fPathRegEx != NULL && optarg[i+1] != '%' && pre_backslash_cnt(optarg, i) % 2 == 1) {
							cerr << g_progname << ": -" << (char) opt << " option ignored, because pattern contains escaped wildcard: `" << optarg[i-1] << optarg[i] << optarg[i+1] << "'" << endl;
							free(fPathRegEx);
							fPathRegEx = NULL;
							break;
						}

						if (newFI->sameFIDIn(framesToModify)) {
							multOptWarning.push_back(textFID);
							delete newFI;
						} else {
							framesToModify.push_back(newFI);
							writeFile = true;
						}

						i++;
					} else {
						if (opt == 'n') {
							if (strchr("\\*+?.^$[]{}()", optarg[i])) {
								regExPattern << '\\';
							}
						} else {
							if (optarg[i] == '(' && pre_backslash_cnt(optarg, i) % 2 == 0) {
								// although its match is never used, we have to count this subexpression
								fPathNMatch++;
							}
						}

						regExPattern << optarg[i];
					}
				}

				if (fPathRegEx == NULL) {
					break;
				}

				if (!wcardPresent) {
					cerr << g_progname << ": -" << (char) opt << " option ignored, because pattern does not contain any wildcard" << endl;
					free(fPathRegEx);
					fPathRegEx = NULL;
					break;
				}

				if (regcomp(fPathRegEx, regExPattern.str().c_str(), REG_EXTENDED) || fPathRegEx->re_nsub != fPathNMatch - 1) {
					cerr << g_progname << ": error compiling regex for pattern, -" << (char) opt << " option ignored" << endl;
					free(fPathRegEx);
					fPathRegEx = NULL;
					break;
				}

				g_fPathPMatch = (regmatch_t*) s_malloc(sizeof(regmatch_t) * fPathNMatch);

				break;
			}*/
			/*case 'o':
				if (orgPattern == NULL) {
					orgPattern = optarg;
				} else if (strcmp(orgPattern, optarg) != 0) {
					multOptWarning.push_back("o");
				}
				break;*/
			case 'x':
				extractAPICs = true;
				break;
			case 'f':
				forceOverwrite = true;
				break;
			case OPT_LO_ORG_MOVE:
				moveFiles = true;
				break;
			/* id3v2 frame id long options */
			case 0: {
				ID3v2FrameID fid = (ID3v2FrameID) optFrameID;
				const char *textFID = FrameTable::textFrameID(fid);
				FrameInfo *info = new FrameInfo(textFID, fid, optarg);

				if (fid == FID3_TXXX && info->description().isEmpty()) {
					cerr << command << ": missing description field in --TXXX "
					     << "option argument" << endl;
					delete info;
					error = true;
					break;
				}
				framesToModify.push_back(info);
				tagsToWrite |= 2;
				writeFile = true;
				break;
			}
		}
	}

	fileCount = argc - optind;
	filenames = argv + optind;

	if (!error) {
		// check if given options are in conflict
		if (tagsToWrite == 1 && framesToRemove.size() > 0) {
			cerr << command << ": conflicting options: -1, -r" << endl;
			error = true;
		}
		if (tagsToWrite == 1 && framesToModify.size() > 0) {
			cerr << command << ": conflicting options: -1, --"
			     << framesToModify[0]->fid() << endl;
			error = true;
		}
		if (tagsToStrip & 2 && framesToModify.size() > 0) {
			cerr << command << ": conflicting options: strip id3v2 tag, --"
			     << framesToModify[0]->fid() << endl;
			error = true;
		}
		if (tagsToStrip & tagsToWrite) {
			cerr << command << ": conflicting options: strip and write "
			     << "the same tag version" << endl;
			error = true;
		}
		// check for missing mandatory arguments
		if (optind == 1) {
			cerr << command << ": missing arguments" << endl;
			error = true;
		} else if (fileCount == 0) {
			cerr << command << ": missing <FILES>" << endl;
			error = true;
		}
	}

	return error;
}

void Options::printVersion() {
	cout << PROGNAME << " - command line id3 tag editor\n"
	     << "Version " << VERSION << ", written by Bert Muennich\n"
	     << "Uses TagLib v" << TAGLIB_MAJOR_VERSION << "."
	     << TAGLIB_MINOR_VERSION << "." << TAGLIB_PATCH_VERSION
	     << ", written by Scott Wheeler" << endl;
}

void Options::printUsage() {
	cout << "Usage: " << command << " [OPTIONS]... <FILES>\n\n"
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

const char* Options::options = "hvpd:a:A:t:c:g:T:y:ilLmMr:DsS123n:N:o:xf";
const struct option Options::longOptions[] = {
  /* help, general info & others */
  { "help",           no_argument,       NULL, 'h' },
  { "version",        no_argument,       NULL, 'v' },
  { "frame-list",     no_argument,       NULL, OPT_LO_FRAME_LIST },
  { "genre-list",     no_argument,       NULL, OPT_LO_GENRE_LIST },
  { "preserve-times", no_argument,       NULL, 'p' },
  { "delimiter",      required_argument, NULL, 'd' },
  /* alter generic tag infomation */
  { "artist",         required_argument, NULL, 'a' },
  { "album",          required_argument, NULL, 'A' },
  { "title",          required_argument, NULL, 't' },
  { "comment",        required_argument, NULL, 'c' },
  { "genre",          required_argument, NULL, 'g' },
  { "track",          required_argument, NULL, 'T' },
  { "year",           required_argument, NULL, 'y' },
  /* get information from the files */
  { "info",           no_argument,       NULL, 'i' },
  { "list",           no_argument,       NULL, 'l' },
  { "list-wd",        no_argument,       NULL, 'L' },
  { "lame-tag",       no_argument,       NULL, 'm' },
  { "lame-tag-crc",   no_argument,       NULL, 'M' },
  /* Remove tags & specify which versions to write */
  { "remove",         required_argument, NULL, 'r' },
  { "delete-all",     no_argument,       NULL, 'D' },
  { "strip-v1",       no_argument,       NULL, 's' },
  { "strip-v2",       no_argument,       NULL, 'S' },
  { "",               no_argument,       NULL, '1' },
  { "",               no_argument,       NULL, '2' },
  { "",               no_argument,       NULL, '3' },
	/* Filename <-> tag information */
  { "file-pattern",   required_argument, NULL, 'n' },
  { "file-regex",     required_argument, NULL, 'N' },
  { "organize",       required_argument, NULL, 'o' },
  { "extract-apics",  no_argument,       NULL, 'x' },
  { "force",          no_argument,       NULL, 'f' },
  { "move",           no_argument,       NULL, OPT_LO_ORG_MOVE },
  /* id3v2 frame ids for direct tagging */
//{ "AENC", required_argument, &Options::optFrameID, FID3_AENC },
  { "APIC", required_argument, &Options::optFrameID, FID3_APIC },
//{ "ASPI", required_argument, &Options::optFrameID, FID3_ASPI },
  { "COMM", required_argument, &Options::optFrameID, FID3_COMM },
//{ "COMR", required_argument, &Options::optFrameID, FID3_COMR },
//{ "ENCR", required_argument, &Options::optFrameID, FID3_ENCR },
//{ "EQU2", required_argument, &Options::optFrameID, FID3_EQU2 },
//{ "EQUA", required_argument, &Options::optFrameID, FID3_EQUA },
//{ "ETCO", required_argument, &Options::optFrameID, FID3_ETCO },
//{ "GEOB", required_argument, &Options::optFrameID, FID3_GEOB },
//{ "GRID", required_argument, &Options::optFrameID, FID3_GRID },
//{ "IPLS", required_argument, &Options::optFrameID, FID3_IPLS },
//{ "LINK", required_argument, &Options::optFrameID, FID3_LINK },
//{ "MCDI", required_argument, &Options::optFrameID, FID3_MCDI },
//{ "MLLT", required_argument, &Options::optFrameID, FID3_MLLT },
//{ "OWNE", required_argument, &Options::optFrameID, FID3_OWNE },
  { "PCNT", required_argument, &Options::optFrameID, FID3_PRIV },
//{ "POPM", required_argument, &Options::optFrameID, FID3_PCNT },
//{ "POSS", required_argument, &Options::optFrameID, FID3_POPM },
//{ "PRIV", required_argument, &Options::optFrameID, FID3_POSS },
  { "RBUF", required_argument, &Options::optFrameID, FID3_RBUF },
//{ "RVA2", required_argument, &Options::optFrameID, FID3_RVA2 },
//{ "RVAD", required_argument, &Options::optFrameID, FID3_RVAD },
//{ "RVRB", required_argument, &Options::optFrameID, FID3_RVRB },
//{ "SEEK", required_argument, &Options::optFrameID, FID3_SEEK },
//{ "SIGN", required_argument, &Options::optFrameID, FID3_SIGN },
//{ "SYLT", required_argument, &Options::optFrameID, FID3_SYLT },
//{ "SYTC", required_argument, &Options::optFrameID, FID3_SYTC },
  { "TALB", required_argument, &Options::optFrameID, FID3_TALB },
  { "TBPM", required_argument, &Options::optFrameID, FID3_TBPM },
  { "TCOM", required_argument, &Options::optFrameID, FID3_TCOM },
  { "TCON", required_argument, &Options::optFrameID, FID3_TCON },
  { "TCOP", required_argument, &Options::optFrameID, FID3_TCOP },
//{ "TDAT", required_argument, &Options::optFrameID, FID3_TDAT },
  { "TDEN", required_argument, &Options::optFrameID, FID3_TDEN },
  { "TDLY", required_argument, &Options::optFrameID, FID3_TDLY },
  { "TDOR", required_argument, &Options::optFrameID, FID3_TDOR },
  { "TDRC", required_argument, &Options::optFrameID, FID3_TDRC },
  { "TDRL", required_argument, &Options::optFrameID, FID3_TDRL },
  { "TDTG", required_argument, &Options::optFrameID, FID3_TDTG },
  { "TENC", required_argument, &Options::optFrameID, FID3_TENC },
  { "TEXT", required_argument, &Options::optFrameID, FID3_TEXT },
  { "TFLT", required_argument, &Options::optFrameID, FID3_TFLT },
//{ "TIME", required_argument, &Options::optFrameID, FID3_TIME },
//{ "TIPL", required_argument, &Options::optFrameID, FID3_TIPL },
  { "TIT1", required_argument, &Options::optFrameID, FID3_TIT1 },
  { "TIT2", required_argument, &Options::optFrameID, FID3_TIT2 },
  { "TIT3", required_argument, &Options::optFrameID, FID3_TIT3 },
  { "TKEY", required_argument, &Options::optFrameID, FID3_TKEY },
  { "TLAN", required_argument, &Options::optFrameID, FID3_TLAN },
  { "TLEN", required_argument, &Options::optFrameID, FID3_TLEN },
  { "TMCL", required_argument, &Options::optFrameID, FID3_TMCL },
  { "TMED", required_argument, &Options::optFrameID, FID3_TMED },
  { "TMOO", required_argument, &Options::optFrameID, FID3_TMOO },
  { "TOAL", required_argument, &Options::optFrameID, FID3_TOAL },
  { "TOFN", required_argument, &Options::optFrameID, FID3_TOFN },
  { "TOLY", required_argument, &Options::optFrameID, FID3_TOLY },
  { "TOPE", required_argument, &Options::optFrameID, FID3_TOPE },
  { "TORY", required_argument, &Options::optFrameID, FID3_TORY },
  { "TOWN", required_argument, &Options::optFrameID, FID3_TOWN },
  { "TPE1", required_argument, &Options::optFrameID, FID3_TPE1 },
  { "TPE2", required_argument, &Options::optFrameID, FID3_TPE2 },
  { "TPE3", required_argument, &Options::optFrameID, FID3_TPE3 },
  { "TPE4", required_argument, &Options::optFrameID, FID3_TPE4 },
  { "TPOS", required_argument, &Options::optFrameID, FID3_TPOS },
  { "TPRO", required_argument, &Options::optFrameID, FID3_TPRO },
  { "TPUB", required_argument, &Options::optFrameID, FID3_TPUB },
  { "TRCK", required_argument, &Options::optFrameID, FID3_TRCK },
//{ "TRDA", required_argument, &Options::optFrameID, FID3_TRDA },
  { "TRSN", required_argument, &Options::optFrameID, FID3_TRSN },
  { "TRSO", required_argument, &Options::optFrameID, FID3_TRSO },
//{ "TSIZ", required_argument, &Options::optFrameID, FID3_TSIZ },
  { "TSOA", required_argument, &Options::optFrameID, FID3_TSOA },
  { "TSOP", required_argument, &Options::optFrameID, FID3_TSOP },
  { "TSOT", required_argument, &Options::optFrameID, FID3_TSOT },
  { "TSRC", required_argument, &Options::optFrameID, FID3_TSRC },
  { "TSSE", required_argument, &Options::optFrameID, FID3_TSSE },
  { "TSST", required_argument, &Options::optFrameID, FID3_TSST },
  { "TXXX", required_argument, &Options::optFrameID, FID3_TXXX },
//{ "TYER", required_argument, &Options::optFrameID, FID3_TYER },
//{ "UFID", required_argument, &Options::optFrameID, FID3_UFID },
  { "USER", required_argument, &Options::optFrameID, FID3_USER },
  { "USLT", required_argument, &Options::optFrameID, FID3_USLT },
  { "WCOM", required_argument, &Options::optFrameID, FID3_WCOM },
  { "WCOP", required_argument, &Options::optFrameID, FID3_WCOP },
  { "WOAF", required_argument, &Options::optFrameID, FID3_WOAF },
  { "WOAR", required_argument, &Options::optFrameID, FID3_WOAR },
  { "WOAS", required_argument, &Options::optFrameID, FID3_WOAS },
  { "WORS", required_argument, &Options::optFrameID, FID3_WORS },
  { "WPAY", required_argument, &Options::optFrameID, FID3_WPAY },
  { "WPUB", required_argument, &Options::optFrameID, FID3_WPUB },
  { "WXXX", required_argument, &Options::optFrameID, FID3_WXXX },
  { 0, 0, 0, 0 },
};

