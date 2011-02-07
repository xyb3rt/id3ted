/* id3ted: id3ted.h
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

#ifndef __ID3TED_H__
#define __ID3TED_H__

#include <taglib/taglib.h>

#include "config.h"

#define PROGNAME "id3ted"
#define VERSION  "git-20110207"

using namespace std;
using namespace TagLib;
using TagLib::uint;

extern const char *command;

typedef enum _ID3v2FrameID {
	FID3_XXXX,  /* Unknown frame */
  FID3_AENC,  /* Audio encryption */
  FID3_APIC,  /* Attached picture */
  FID3_ASPI,  /* Audio seek point index */
  FID3_COMM,  /* Comments */
  FID3_COMR,  /* Commercial frame */
  FID3_ENCR,  /* Encryption method registration */
  FID3_EQU2,  /* Equalisation (2) */
	FID3_EQUA,  /* Equalisation -- ID3v2.3 only */
  FID3_ETCO,  /* Event timing codes */
  FID3_GEOB,  /* General encapsulated object */
  FID3_GRID,  /* Group identification registration */
	FID3_IPLS,  /* Involved people list -- ID3v2.3 only */
  FID3_LINK,  /* Linked information */
  FID3_MCDI,  /* Music CD identifier */
  FID3_MLLT,  /* MPEG location lookup table */
  FID3_OWNE,  /* Ownership frame */
  FID3_PCNT,  /* Play counter */
  FID3_POPM,  /* Popularimeter */
  FID3_POSS,  /* Position synchronisation frame */
  FID3_PRIV,  /* Private frame */
  FID3_RBUF,  /* Recommended buffer size */
  FID3_RVA2,  /* Relative volume adjustment (2) */
	FID3_RVAD,  /* Relative volume adjustment -- ID3v2.3 only */
  FID3_RVRB,  /* Reverb */
  FID3_SEEK,  /* Seek frame */
  FID3_SIGN,  /* Signature frame */
  FID3_SYLT,  /* Synchronised lyric/text */
  FID3_SYTC,  /* Synchronised tempo codes */
  FID3_TALB,  /* Album/Movie/Show title */
  FID3_TBPM,  /* BPM (beats per minute) */
  FID3_TCOM,  /* Composer */
	FID3_TCON,  /* Content type */
  FID3_TCOP,  /* Copyright message */
	FID3_TDAT,  /* Date -- ID3v2.3 only */
  FID3_TDEN,  /* Encoding time */
  FID3_TDLY,  /* Playlist delay */
  FID3_TDOR,  /* Original release time */
  FID3_TDRC,  /* Recording time */
  FID3_TDRL,  /* Release time */
  FID3_TDTG,  /* Tagging time */
  FID3_TENC,  /* Encoded by */
  FID3_TEXT,  /* Lyricist/Text writer */
  FID3_TFLT,  /* File type */
	FID3_TIME,  /* Time -- ID3v2.3 only */
  FID3_TIPL,  /* Involved people list (2) */
  FID3_TIT1,  /* Content group description */
  FID3_TIT2,  /* Title/songname/content description */
  FID3_TIT3,  /* Subtitle/Description refinement */
  FID3_TKEY,  /* Initial key */
  FID3_TLAN,  /* Language(s) */
  FID3_TLEN,  /* Length */
  FID3_TMCL,  /* Musician credits list */
  FID3_TMED,  /* Media type */
  FID3_TMOO,  /* Mood */
  FID3_TOAL,  /* Original album/movie/show title */
  FID3_TOFN,  /* Original filename */
  FID3_TOLY,  /* Original lyricist(s)/text writer(s) */
  FID3_TOPE,  /* Original artist(s)/performer(s) */
	FID3_TORY,  /* Original release year -- ID3v2.3 only */
  FID3_TOWN,  /* File owner/licensee */
  FID3_TPE1,  /* Lead performer(s)/Soloist(s) */
  FID3_TPE2,  /* Band/orchestra/accompaniment */
  FID3_TPE3,  /* Conductor/performer refinement */
  FID3_TPE4,  /* Interpreted, remixed, or otherwise modified by */
  FID3_TPOS,  /* Part of a set */
  FID3_TPRO,  /* Produced notice */
  FID3_TPUB,  /* Publisher */
  FID3_TRCK,  /* Track number/Position in set */
	FID3_TRDA,  /* Recording dates -- ID3v2.3 only */
  FID3_TRSN,  /* Internet radio station name */
  FID3_TRSO,  /* Internet radio station owner */
	FID3_TSIZ,  /* Size -- ID3v2.3 only */
  FID3_TSOA,  /* Album sort order */
  FID3_TSOP,  /* Performer sort order */
  FID3_TSOT,  /* Title sort order */
  FID3_TSRC,  /* ISRC (international standard recording code) */
  FID3_TSSE,  /* Software/Hardware and settings used for encoding */
  FID3_TSST,  /* Set subtitle */
  FID3_TXXX,  /* User defined text information frame */
  FID3_TYER,  /* Year -- ID3v2.3 only */
  FID3_UFID,  /* Unique file identifier */
  FID3_USER,  /* Terms of use */
  FID3_USLT,  /* Unsynchronised lyric/text transcription */
  FID3_WCOM,  /* Commercial information */
  FID3_WCOP,  /* Copyright/Legal information */
  FID3_WOAF,  /* Official audio file webpage */
  FID3_WOAR,  /* Official artist/performer webpage */
  FID3_WOAS,  /* Official audio source webpage */
  FID3_WORS,  /* Official Internet radio station homepage */
  FID3_WPAY,  /* Payment */
  FID3_WPUB,  /* Publishers official webpage */
  FID3_WXXX   /* User defined URL link frame */
} ID3v2FrameID;

#endif /* __ID3TED_H__ */

