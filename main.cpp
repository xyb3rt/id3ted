/* id3ted: main.cpp
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
#include <cstdlib>
#include <cstring>
#include <list>
#include <vector>
#include <typeinfo>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "id3ted.h"
#include "frameinfo.h"
#include "frametable.h"
#include "list.h"
#include "misc.h"
#include "mp3file.h"

#ifdef __APPLE__
#define st_atim st_atimespec
#define st_mtim st_mtimespec
#endif

enum opt_long_only {
	OPT_LO_FRAME_LIST = 128,
	OPT_LO_GENRE_LIST,
	OPT_LO_ORG_MOVE
};

static int optFrameID;
static const struct option long_options[] = {
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
//{ "AENC", required_argument, &optFrameID, FID3_AENC },
  { "APIC", required_argument, &optFrameID, FID3_APIC },
//{ "ASPI", required_argument, &optFrameID, FID3_ASPI },
  { "COMM", required_argument, &optFrameID, FID3_COMM },
//{ "COMR", required_argument, &optFrameID, FID3_COMR },
//{ "ENCR", required_argument, &optFrameID, FID3_ENCR },
//{ "EQU2", required_argument, &optFrameID, FID3_EQU2 },
//{ "EQUA", required_argument, &optFrameID, FID3_EQUA },
//{ "ETCO", required_argument, &optFrameID, FID3_ETCO },
//{ "GEOB", required_argument, &optFrameID, FID3_GEOB },
//{ "GRID", required_argument, &optFrameID, FID3_GRID },
//{ "IPLS", required_argument, &optFrameID, FID3_IPLS },
//{ "LINK", required_argument, &optFrameID, FID3_LINK },
//{ "MCDI", required_argument, &optFrameID, FID3_MCDI },
//{ "MLLT", required_argument, &optFrameID, FID3_MLLT },
//{ "OWNE", required_argument, &optFrameID, FID3_OWNE },
  { "PCNT", required_argument, &optFrameID, FID3_PRIV },
//{ "POPM", required_argument, &optFrameID, FID3_PCNT },
//{ "POSS", required_argument, &optFrameID, FID3_POPM },
//{ "PRIV", required_argument, &optFrameID, FID3_POSS },
  { "RBUF", required_argument, &optFrameID, FID3_RBUF },
//{ "RVA2", required_argument, &optFrameID, FID3_RVA2 },
//{ "RVAD", required_argument, &optFrameID, FID3_RVAD },
//{ "RVRB", required_argument, &optFrameID, FID3_RVRB },
//{ "SEEK", required_argument, &optFrameID, FID3_SEEK },
//{ "SIGN", required_argument, &optFrameID, FID3_SIGN },
//{ "SYLT", required_argument, &optFrameID, FID3_SYLT },
//{ "SYTC", required_argument, &optFrameID, FID3_SYTC },
  { "TALB", required_argument, &optFrameID, FID3_TALB },
  { "TBPM", required_argument, &optFrameID, FID3_TBPM },
  { "TCOM", required_argument, &optFrameID, FID3_TCOM },
  { "TCON", required_argument, &optFrameID, FID3_TCON },
  { "TCOP", required_argument, &optFrameID, FID3_TCOP },
//{ "TDAT", required_argument, &optFrameID, FID3_TDAT },
  { "TDEN", required_argument, &optFrameID, FID3_TDEN },
  { "TDLY", required_argument, &optFrameID, FID3_TDLY },
  { "TDOR", required_argument, &optFrameID, FID3_TDOR },
  { "TDRC", required_argument, &optFrameID, FID3_TDRC },
  { "TDRL", required_argument, &optFrameID, FID3_TDRL },
  { "TDTG", required_argument, &optFrameID, FID3_TDTG },
  { "TENC", required_argument, &optFrameID, FID3_TENC },
  { "TEXT", required_argument, &optFrameID, FID3_TEXT },
  { "TFLT", required_argument, &optFrameID, FID3_TFLT },
//{ "TIME", required_argument, &optFrameID, FID3_TIME },
//{ "TIPL", required_argument, &optFrameID, FID3_TIPL },
  { "TIT1", required_argument, &optFrameID, FID3_TIT1 },
  { "TIT2", required_argument, &optFrameID, FID3_TIT2 },
  { "TIT3", required_argument, &optFrameID, FID3_TIT3 },
  { "TKEY", required_argument, &optFrameID, FID3_TKEY },
  { "TLAN", required_argument, &optFrameID, FID3_TLAN },
  { "TLEN", required_argument, &optFrameID, FID3_TLEN },
  { "TMCL", required_argument, &optFrameID, FID3_TMCL },
  { "TMED", required_argument, &optFrameID, FID3_TMED },
  { "TMOO", required_argument, &optFrameID, FID3_TMOO },
  { "TOAL", required_argument, &optFrameID, FID3_TOAL },
  { "TOFN", required_argument, &optFrameID, FID3_TOFN },
  { "TOLY", required_argument, &optFrameID, FID3_TOLY },
  { "TOPE", required_argument, &optFrameID, FID3_TOPE },
  { "TORY", required_argument, &optFrameID, FID3_TORY },
  { "TOWN", required_argument, &optFrameID, FID3_TOWN },
  { "TPE1", required_argument, &optFrameID, FID3_TPE1 },
  { "TPE2", required_argument, &optFrameID, FID3_TPE2 },
  { "TPE3", required_argument, &optFrameID, FID3_TPE3 },
  { "TPE4", required_argument, &optFrameID, FID3_TPE4 },
  { "TPOS", required_argument, &optFrameID, FID3_TPOS },
  { "TPRO", required_argument, &optFrameID, FID3_TPRO },
  { "TPUB", required_argument, &optFrameID, FID3_TPUB },
  { "TRCK", required_argument, &optFrameID, FID3_TRCK },
//{ "TRDA", required_argument, &optFrameID, FID3_TRDA },
  { "TRSN", required_argument, &optFrameID, FID3_TRSN },
  { "TRSO", required_argument, &optFrameID, FID3_TRSO },
//{ "TSIZ", required_argument, &optFrameID, FID3_TSIZ },
  { "TSOA", required_argument, &optFrameID, FID3_TSOA },
  { "TSOP", required_argument, &optFrameID, FID3_TSOP },
  { "TSOT", required_argument, &optFrameID, FID3_TSOT },
  { "TSRC", required_argument, &optFrameID, FID3_TSRC },
  { "TSSE", required_argument, &optFrameID, FID3_TSSE },
  { "TSST", required_argument, &optFrameID, FID3_TSST },
  { "TXXX", required_argument, &optFrameID, FID3_TXXX },
//{ "TYER", required_argument, &optFrameID, FID3_TYER },
//{ "UFID", required_argument, &optFrameID, FID3_UFID },
  { "USER", required_argument, &optFrameID, FID3_USER },
  { "USLT", required_argument, &optFrameID, FID3_USLT },
  { "WCOM", required_argument, &optFrameID, FID3_WCOM },
  { "WCOP", required_argument, &optFrameID, FID3_WCOP },
  { "WOAF", required_argument, &optFrameID, FID3_WOAF },
  { "WOAR", required_argument, &optFrameID, FID3_WOAR },
  { "WOAS", required_argument, &optFrameID, FID3_WOAS },
  { "WORS", required_argument, &optFrameID, FID3_WORS },
  { "WPAY", required_argument, &optFrameID, FID3_WPAY },
  { "WPUB", required_argument, &optFrameID, FID3_WPUB },
  { "WXXX", required_argument, &optFrameID, FID3_WXXX },
  { 0, 0, 0, 0 },
};

const char *g_progname;
char g_fdelim = FIELD_DELIM;
int g_numFiles;
regmatch_t *g_fPathPMatch;


/* return values: (ored together)
 *   0: everything went fine
 *   1: error allocating memory
 *   2: faulty command line arguments required abort
 *   4: at least one given option had to be ignored
 *   8: at least one error occured during processing of files
 */
int main(int argc, char **argv) {
	int updFlag = 0;                   // -1|-2|-3
	int tagsToStrip = 0;               // -s|-S|-D
	bool writeFile = false;            // -1|-2|-3|-s|-S|-D|-a|-A|-t|-c|-g|-T|-y
	bool extractAPICs = false;         // -x
	bool listTags = false;             // -l|-L
	bool printLameTag = false;         // -e|-E
	bool checkLameCRC = false;         // -E
	bool listV2WithDesc = false;       // -L
	bool showInfo = false;             // -i
	bool forceOverwrite = false;       // -F
	bool orgMove = false;              // -m
	const char *orgPattern = NULL;     // -o; set to option argument
	struct timeval *ptimes = NULL;     // -p
	regex_t *fPathRegEx = NULL;        // -n|-N
	g_fPathPMatch = NULL;              // -n|-N
	unsigned int fPathNMatch = 1;      // -n|-N
	vector<GenericInfo*> genericMods;  // -a|-A|-t|-c|-g|-T|-y
	vector<char*> framesToRemove;      // -r; option argument added to list
	vector<FrameInfo*> framesToModify; // --FID

	int retCode = 0;
	std::list<std::string> multOptWarning;

	g_progname = basename(argv[0]);
	if (strcmp(g_progname, ".") == 0) g_progname = PROGNAME;

	/* parsing command line options */
	int opt;
	int error = 0, excVersion;

	while (!error) {
		excVersion = 0;
		optFrameID = FID3_XXXX;

		opt = getopt_long(argc, argv, "hvpd:a:A:t:c:g:T:y:ilLmMr:DsS123n:N:o:xf", long_options, NULL);

		if (opt == -1)
			break;

		switch (opt) {
			/* invalid option given */
			case '?':
				error = 2;
				break;
			/* help, general info & others */
			case 'h':
				printUsage();
				exit(0);
			case 'v':
				printVersion();
				exit(0);
			case OPT_LO_FRAME_LIST:
				FrameTable::printFrameHelp();
				exit(0);
			case OPT_LO_GENRE_LIST:
				printGenreList();
				exit(0);
			case 'p':
				ptimes = (struct timeval*) s_malloc(2 * sizeof(struct timeval));
				break;
			case 'd':
				if (strlen(optarg) == 1) {
					g_fdelim = optarg[0];
				} else {
					cerr << g_progname << ": The argument of -d/--delimiter has to be a single character" << endl;
					error = 2;
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
				GenericInfo *newGI = new GenericInfo((char) opt, optarg);

				if (newGI != NULL) {
					if (!newGI->sameIDIn(genericMods)) {
						genericMods.push_back(newGI);
						writeFile = true;
					} else {
						multOptWarning.push_back(std::string(1, opt));
						delete newGI;
					}
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
					cerr << g_progname << ": -r: invalid id3v2 frame id: " << optarg << endl;
					error = 2;
				}
				break;
			case 'D':
				tagsToStrip = 3;
				writeFile = true;
				break;
			case 's':
				tagsToStrip |= 1;
				writeFile = true;
				break;
			case 'S':
				tagsToStrip |= 2;
				writeFile = true;
				break;
			case '1':
				excVersion = 1;
			case '2':
				if (excVersion == 0) excVersion = 2;
			case '3':
				if (excVersion == 0) excVersion = 3;
				if (!updFlag) {
					updFlag = excVersion;
					writeFile = true;
				} else if (updFlag != excVersion) {
					cerr << g_progname << ": conflicting options: -" << updFlag << ", -" << excVersion << endl;
					error = 2;
				}
				break;
			/* filename <-> tag information */
			case 'N':
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
			}
			case 'o':
				if (orgPattern == NULL) {
					orgPattern = optarg;
				} else if (strcmp(orgPattern, optarg) != 0) {
					multOptWarning.push_back("o");
				}
				break;
			case 'x':
				extractAPICs = true;
				break;
			case 'f':
				forceOverwrite = true;
				break;
			case OPT_LO_ORG_MOVE:
				orgMove = true;
				break;
			/* id3v2 frame id long options */
			case 0: {
				// --FID long options
				const char *textFID = FrameTable::textFrameID((ID3v2FrameID) optFrameID);
				ID3v2FrameID fid = (ID3v2FrameID) optFrameID;
				FrameInfo *newFI = NULL;
				bool excID3v2Frame = false;

				switch(fid) {
					case FID3_XXXX: {
						break;
					}
					case FID3_APIC: {
						APICFrameInfo *apicFI = new APICFrameInfo(textFID, fid, optarg);

						if (!apicFI->readFile()) {
							delete apicFI;
						} else {
							newFI = apicFI;
						}

						break;
					}
					case FID3_TXXX:
					case FID3_WXXX: {
						TDFrameInfo *tdFI = new TDFrameInfo(textFID, fid, optarg);

						if (fid == FID3_TXXX && !tdFI->multipleFields()) {
							cerr << g_progname << ": missing description field in --TXXX option argument" << endl;
							error = 2;
							
							delete tdFI;
						} else {
							newFI = tdFI;
						}

						break;
					}
					case FID3_USLT:
					case FID3_COMM: {
						newFI = new TDLFrameInfo(textFID, fid, optarg);
						break;
					}
					default: {
						newFI = new FrameInfo(textFID, fid, optarg);
						excID3v2Frame = true;
						break;
					}
				}
			
				if (newFI != NULL) {
					if (excID3v2Frame && newFI->sameFIDIn(framesToModify)) {
						multOptWarning.push_back(textFID);
						delete newFI;
					} else {
						framesToModify.push_back(newFI);
						writeFile = true;
					}
				}

				break;
			}
		}
	}

	g_numFiles = argc - optind;

	if (!error) {
		// check if given options are in conflict
		if (tagsToStrip & updFlag) {
			cerr << g_progname << ": conflicting options: strip and write the same tag version" << endl;
			error = 2;
		}
		if (updFlag == 1 && framesToRemove.size() > 0) {
			cerr << g_progname << ": conflicting options: -1, -r" << endl;
			error = 2;
		}
		if (updFlag == 1 && framesToModify.size() > 0) {
			cerr << g_progname << ": conflicting options: -1, --" << framesToModify[0]->id() << endl;
			error = 2;
		}
		if (tagsToStrip & 2 && framesToModify.size() > 0) {
			cerr << g_progname << ": conflicting options: strip id3v2 tag, --" << framesToModify[0]->id() << endl;
			error = 2;
		}
		// check for missing mandatory arguments
		if (optind == 1) {
			cerr << g_progname << ": missing arguments" << endl;
			error = 2;
		} else if (g_numFiles == 0) {
			cerr << g_progname << ": missing <FILES>" << endl;
			error = 2;
		}
	}
	
	if (error) {
		cerr << "Try `" << argv[0] << " --help' for more information." << endl;
		exit(error);
	}

	if (multOptWarning.size() > 0) {
		multOptWarning.sort();
		multOptWarning.unique();

		std::list<std::string>::const_iterator it = multOptWarning.begin();
		for (; it != multOptWarning.end(); it++) {
			cerr << g_progname << ": only first " << (it->length() > 1 ? "" : "-") << *it << " option applied, others ignored" << endl;
		}

		multOptWarning.clear();
		retCode |= 4;
	}

	// iterating over the files
	for (int fn_idx = optind; (unsigned int) fn_idx < argc; fn_idx++) {
		const char *file = argv[fn_idx];
		bool preserveTimes = false;
		bool filenameToTag = fPathRegEx != NULL;
		struct stat stats;

		if (stat(file, &stats) == -1) {
			cerr << g_progname << ": " << file << ": Could not stat file" << endl;
			retCode |= 8;
			continue;
		}

		if (!S_ISREG(stats.st_mode)) {
			cerr << g_progname << ": " << file << ": Not a regular file" << endl;
			retCode |= 8;
			continue;
		}

		// save the access and modification times
		// of the file before the file's accessed for the first time
		if (ptimes != NULL) {
			TIMESPEC_TO_TIMEVAL(ptimes, &stats.st_atim);
			TIMESPEC_TO_TIMEVAL(ptimes+1, &stats.st_mtim);
			preserveTimes = true;
		}

		if (!MP3File::isReadable(file)) {
			cerr << g_progname << ": " << file << ": Could not open file for reading" << endl;
			retCode |= 8;
			continue;
		}

		if (writeFile && !MP3File::isWritable(file)) {
			cerr << g_progname << ": " << file << ": Could not open file for writing" << endl;
			retCode |= 8;
			continue;
		}

		MP3File mp3File(file);
		if (!mp3File.isValid()) {
			retCode |= 8;
			continue;
		}

		if (filenameToTag) {
			// match filename regex pattern on file,
			// because we have to extract tag information from filepath
			if (regexec(fPathRegEx, file, fPathNMatch, g_fPathPMatch, 0)) {
				cout << file << ": pattern does not match filename" << endl;
				filenameToTag = false;
			}
		}

		if (extractAPICs) {
			mp3File.extractAPICs(forceOverwrite);
		}

		// delete id3v2 frames with given frame ids
		if (framesToRemove.size() > 0 && tagsToStrip & 2) {
			cerr << g_progname << ": -r option ignored, because whole id3v2 tag gets stripped" << endl;
			framesToRemove.clear();
			retCode |= 4;
		} else {
			std::vector<char*>::const_iterator f2dIter = framesToRemove.begin();
			for (; f2dIter != framesToRemove.end(); f2dIter++) {
				mp3File.removeFrames(*f2dIter);
			}
		}

		// applying generic tag information
		std::vector<GenericInfo*>::const_iterator gmIter = genericMods.begin();
		for (; gmIter != genericMods.end(); gmIter++) {
			mp3File.apply(*gmIter);
		}

		// loop through the id3v2 frames for adding/modifying
		std::vector<FrameInfo*>::const_iterator f2mIter = framesToModify.begin();
		for (; f2mIter != framesToModify.end(); f2mIter++) {
			if (filenameToTag || (*f2mIter)->fPathIdx() == 0) {
				(*f2mIter)->applyTo(mp3File);
			}
		}

		// save the specified tags to the file
		if (writeFile) {
			if (updFlag) {
				mp3File.save(updFlag);
			} else {
				mp3File.save();
			}
		}

		// delete whole tag version
		if (tagsToStrip > 0) {
			if (!mp3File.strip(tagsToStrip)) {
				cerr << g_progname << ": " << file << ": Could not strip id3 tag" << endl;
				retCode |= 8;
			}
		}

		// print out requested information
		if (showInfo || listTags || printLameTag) {
			cout << file << ":" << endl;
			if (showInfo) mp3File.showInfo();
			if (printLameTag) mp3File.printLameTag(checkLameCRC);
			if (listTags) mp3File.listTags(listV2WithDesc);
			if (fn_idx < argc - 1) cout << endl;
		}

		// organize file in directory structure defined by pattern
		if (orgPattern != NULL) {
			int ret = mp3File.organize(orgPattern, orgMove, forceOverwrite, (preserveTimes ? ptimes : NULL));

			if (ret == 1) {
				orgPattern = NULL;
			} else if (ret > 1) {
				cerr << g_progname << ": " << file << ": Could not organize file" << endl;
				retCode |= 8;
			}
		}

		// reset access and modification times to
		// their old values present before accessing the file:
		if (preserveTimes && (orgPattern == NULL || !orgMove)) {
			utimes(file, ptimes);
		}
	}

	// cleanup - useless because at the end
	if (fPathRegEx != NULL) {
		regfree(fPathRegEx);
		free(fPathRegEx);
	}

	if (g_fPathPMatch != NULL) free(g_fPathPMatch);
	if (ptimes != NULL) free(ptimes);

	std::vector<GenericInfo*>::iterator gmIter = genericMods.begin();
	for (; gmIter != genericMods.end(); gmIter++) {
		delete *gmIter;
	}

	std::vector<FrameInfo*>::iterator f2mIter = framesToModify.begin();
	for (; f2mIter != framesToModify.end(); f2mIter++) {
		delete *f2mIter;
	}

	return retCode;
}

