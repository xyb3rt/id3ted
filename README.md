id3ted
======

**comand line id3 tag editor**

id3ted is written in C++, uses
[TagLib](http://developer.kde.org/~wheeler/taglib/) by Scott Wheeler and should
compile on any UNIX-flavoured operating system.

The primary goal for writing id3ted was the aim to support all featues of the
id3v2 tag format. When listing id3 tags, for instance, id3ted shows every tag
information found in the files, it did not hide obscure id3v2 frame types for
user's convenience. It also regards the additional description and language
fields belonging to some frame types, when reading or writing id3v2 tags.

Features
--------

* List complete id3 tag information saved in mp3 files, in full detail
* Convert id3v1 to id3v2 tag and vice versa
* Strip tags from mp3 files
* Change the general tag information easily and chose the tag versions to write
* Remove any, add and modify almost any id3v2 frame type
* Copy or move mp3 files into directory structure given by a pattern, replacing
  wildcards by corresponding tag information
* Auto-apply information from filepaths to tags
* Extract and add attached pictures from/to id3v2 tag
* Print LAME tags and verify their checksums

News
----

**January 13, 2011**

The first beta release of version 1.0 is out. Please use this version, altough
it might have some bugs, because otherwise they might never be found and get
fixed.

**August 16, 2010**

As of today and version 0.7.3, I consider id3ted to be feature complete. I will
will not implement any new features by myself, because I do not have enough
time to do so. I will, however, fix bugs and include patches sent by other
people. The next stable release (if any) will get the version number 1.0.


Installation
------------

**Dependencies**

First of all, make sure that the following libraries are installed on your
system:

* TagLib: <http://developer.kde.org/~wheeler/taglib/>
* File/Magic: <ftp://ftp.astron.com/pub/file/>

They are both quite popular, most Linux distributions offer binary and devel
packages for them.

**Building**

id3ted uses the common Makefile procedure:

    make
    make install

The default Makefile should work on most NIX systems.
The default installation directory of id3ted is */usr/local*. You could also 
use an alternative directory, by installing id3ted with the following command
line:

    make PREFIX="/your/install/path" install

All build-time specific settings are set via preprocessor macros in the file
*config.h*. Please check and change them, so that they fit your needs.

Download & Changelog
--------------------

You can [browse](https://github.com/muennich/id3ted) the source code
repository on GitHub or get a copy using git with the following command:

    git clone https://github.com/muennich/id3ted.git

**Unstable releases**

**[v1.0b3](http://muennich.github.com/id3ted/release/id3ted-1.0b3.tar.gz)**
*(June 10, 2011)*

  * Extract APICs next to the mp3 files

[**v1.0b2**](http://muennich.github.com/id3ted/release/id3ted-1.0b2.tar.gz)
(*February 27, 2011*)

  * Fixed handling of parentheses in organize and filename-to-tag patterns

[**v1.0b1**](http://muennich.github.com/id3ted/release/id3ted-1.0b1.tar.gz)
(*January 13, 2011*)

  * The code got refactored and is much cleaner now, so there's no reason
    to feel ashamed when abandoning this project...
  * Build-time specific settings are now easily controlled at one place
    (config.h)
  * Replace special characters in filenames while organizing. List of
    characters is set at compile time (config.h)

**Stable releases**

[**v0.7.3**](http://muennich.github.com/id3ted/release/id3ted-0.7.3.tar.gz)
(*August 13, 2010*)

  * Updated documentation and helper files
  * Moved the project from sourceforge to github

[**v0.7.2**](http://muennich.github.com/id3ted/release/id3ted-0.7.2.tar.gz)
(*July 18, 2010*)

  * Changed command line options, e.g. `-e/-E` to `m/-M`, `-F` to `-f`;
    help screen got restructured, command line options arranged in categories
  * Bug fix: Segmentation fault when using -r with invalid frame id
  * Bug fix: Padding files with junk when organizing them (copying)

[**v0.7.1**](http://muennich.github.com/id3ted/release/id3ted-0.7.1.tar.gz)
(*March 10, 2010*)

  * Bug fix: Build error caused by using strstr() with wrong return type
  * Created an Arch Linux
    [PKGBUILD](http://aur.archlinux.org/packages.php?ID=35357)

[**v0.7**](http://muennich.github.com/id3ted/release/id3ted-0.7.tar.gz)
(*January 13, 2010*)

  * New options: `-e/-E`, print the LAME tags stored in the files. Using `-E` 
    additionally verifies their CRC checksums.
  * Bug fix: Handle backslashes in `-n` patterns and parenthesized
    subexpressions in `-N` patterns correctly

[**v0.6.3**](http://muennich.github.com/id3ted/release/id3ted-0.6.3.tar.gz)
(*October 31, 2009*)

  * Bug fix: Infinite loop when creating new APIC frames
  * Strip off additional information from mimetype fields of newly created
    APIC frames

[**v0.6.2**](http://muennich.github.com/id3ted/release/id3ted-0.6.2.tar.gz)
(*September 30, 2009*)

  * Bug fix: Creating extended id3v2 frames (APIC, COMM, TXXX, etc.) as simple
    ones, cutting off their additional information (picture data, description
    fields, etc.)

[**v0.6.1**](http://muennich.github.com/id3ted/release/id3ted-0.6.1.tar.gz)
(*September 24, 2009*)

  * New options: `-n/-N`, parse filepaths using given pattern with wildcards
    and auto-apply the matches for these wildcards to their corresponding tag
    frames. Both options act the same, except that the pattern of `-N` is
    interpreted as an extended regular expression.
  * Bug fix: Resetting access and modification times of files using their
    original path after moving them

[**v0.6**](http://muennich.github.com/id3ted/release/id3ted-0.6.tar.gz)
(*March 27, 2009*)

  * New option: `-p`, preserve access and modification times of the files
  * Organizing files much faster when moving them and source and destination
    reside on the same filesystem
  * Bug fix: Bus error if first percent sign in organize pattern is followed
    by an invalid wildcard
  * Improved support for building id3ted on common linux distributions

[**v0.5.1**](http://muennich.github.com/id3ted/release/id3ted-0.5.1.tar.gz)
(*August 24, 2008*)

  * Bug fix: handle PRIV frames correctly
  * Print the size of APICs in a human readable way using unit suffixes like
    Kilobyte and Megabyte
  * Regard linebreaks when printing USLT frames

[**v0.5**](http://muennich.github.com/id3ted/release/id3ted-0.5.tar.gz)
(*August 10, 2008*)

  * Support for UTF-8, using it per default
  * Ask for confirmation when extracting APIC and file already exists
  * New option: `-o`, organizing files into directory structure given by
    pattern, ability to replace wildcards in that pattern with information
    found in the tags. For instance, you can move some files to `%a/%t.mp3`
    with `%a` as a placeholder for the artist and `%t` for the title.

[**v0.4**](http://muennich.github.com/id3ted/release/id3ted-0.4.tar.gz)
(*April 15, 2008*)

  * Completely rewritten
  * Using [TagLib](http://developer.kde.org/~wheeler/taglib/) as the new base
    instead of id3lib, because it is not so buggy and it supports v2.4 tags

[**v0.3.2**](http://muennich.github.com/id3ted/release/id3ted-0.3.2.tar.gz)
(*March 24, 2008*)

  * Support for TSOA, TSOP and TSOT frames in v2.3 tag
  * Bug fix: not ignoring the version 1 tag anymore when setting the comment

[**v0.3.1**](http://muennich.github.com/id3ted/release/id3ted-0.3.1.tar.gz)
(*December 9, 2007*)

  * Check only for the minimal permissions on the files, regarding the options
    given on the command line. Do not check for write permission (and abort if
    not granted) anymore, not regarding that id3ted only needs to read a file.

[**v0.3**](http://muennich.github.com/id3ted/release/id3ted-0.3.tar.gz)
(*November 25, 2007*)

  * Dropped the `-C` option because it's effect could be achieved with the
    `-1`, `-2` and `-3` options in a slightly better way
  * I've always wanted to announce an update like Apple did it all the time:
    *improvements in speed and stability, bug fixes*

[**v0.2**](http://muennich.github.com/id3ted/release/id3ted-0.2.tar.gz)
(*October 14, 2007*)

  * New option: `-d` to set the delimiter used to distinguish the components of
    a multiple field option argument. This is helpful if for instance the
    description for the new comment contains a ':'.
  * Added support to list some more frame types

[**v0.1**](http://muennich.github.com/id3ted/release/id3ted-0.1.tar.gz)
(*September 29, 2007*)

  * Initial release
  * Based on [id3v2](http://id3v2.sourceforge.net/) by Myers Carpenter
