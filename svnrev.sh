#! /bin/bash
#
rev_new_raw=$(svnversion -n . 2>/dev/null | tr '\n' ' ' | tr -d '\r')
[ -n "$rev_new_raw" ] || rev_new_raw=$(SubWCRev . 2>/dev/null | tr '\n' ' ' | tr -d '\r')


rev_new_raw=$(echo $rev_new_raw | sed 's/[^0-9]*\([0-9]*\)\(.*\)/\1 \2/')
rev_new=0
a=$(echo $rev_new_raw | sed 's/\([0-9]*\).*/\1/')
let "a+=0"
#find max rev
while [ "$a" ]; do
	[ "$a" -gt "$rev_new" ] && rev_new=$a
	rev_new_raw=$(echo -n $rev_new_raw | sed 's/[0-9]*[^0-9]*\([0-9]*\)\(.*\)/\1 \2/')
	a=$(echo $rev_new_raw | sed 's/\([0-9]*\).*/\1/')
done

rev_old=$(cat ./source/svnrev.c 2>/dev/null | tr -d '\n' | sed 's/[^0-9]*\([0-9]*\).*/\1/')

if [ "$rev_new" != "$rev_old" ] || [ ! -f ./source/svnrev.c ]; then

	cat <<EOF > ./source/svnrev.c
#define SVN_REV "$rev_new"

const char * SvnRev()
{
	return SVN_REV;
}
EOF

	if [ -n "$rev_new" ]; then
		echo "Changed Rev $rev_old to $rev_new" >&2
	else
		echo "svnrev.c created" >&2
	fi
	echo >&2

fi

	rev_new=`expr $rev_new + 1`
	rev_date=`date +%Y%m%d%H%M -u`

	cat <<EOF > ./HBC/META.XML
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<app version="1">
  <name>WiiXplorer</name>
  <coder>Dimok/r-win</coder>
  <version>r$rev_new</version>
  <release_date>$rev_date</release_date>
  <short_description>Wii File Browser</short_description>
  <long_description>A multi device file explorer for the Wii by dimok and r-win.
The GUI is done with LibWiiGui by Tantric and graphics by NeoRame.

Features:
 * USB2 support with cIOS202 installed
 * SMB/USB/SD/DVD recursive copy/move/delete
   of files/directories with all files in it
 * FAT and NTFS files systems support
 * Rename files/directories
 * Properties of files/directories
 * Browse through SD/USB/SMB/DVD/WiiDisk
 * Addressbar with path
 * Multilanguage with custom font support
 * Boot .dol/.elf files
 * Open TXT/XML/MP3/OGG files
 * Supported Image Formats:
   PNG/JPEG/GIF/BMP/TGA/TIFF/GD/GD2
 * Imageoperations zoom/slideshow
 * ZIP/7zip browsing and decompressing
 * Rar browsing
 * Properties of archive files
 * Textediting support
 * Auto-Update feature

Credits:
  Coding: Dimok, r-win
  Forwarder: Dimok
  Artworks: Neorame
  Languages: Dj Skual and Translaters

  Libwiigui: Tantric
  Libogc/Devkit: Shagkur and Wintermute
  FreeTypeGX: Armin Tamzarian.

Links:
  WiiXplorer Project Page:
    http://code.google.com/p/wiixplorer/
  WiiXplorer Support Site:
    http://code.google.com/p/wiixplorer/
    updates/list
  Libwiigui Website:
    http://wiibrew.org/wiki/Libwiigui/
  FreeTypeGX Project Page:
    http://code.google.com/p/freetypegx/
  Gettext Official Page:
    http://www.gnu.org/software/gettext/
    gettext.html
  Languages Page:
    http://gbatemp.net/
	index.php?showtopic=174053
  </long_description>
</app>
EOF

echo $a