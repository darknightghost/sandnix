/*
	Copyright 2015,暗夜幽灵 <darknightghost.cn@gmail.com>

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef	TAR_H_INCLUDE
#define	TAR_H_INCLUDE

#include "../../../../common/common.h"

#define	TAR_BLOCK_SIZE		512
#define	TAR_FILENAME_SIZE	100
#define	TAR_UNAME_LEN		32
#define	TAR_GNAME_LEN		32

#pragma pack(1)
typedef	struct {
	char	name[TAR_FILENAME_SIZE];
	char	mode[8];
	char	uid[8];
	char	gid[8];
	char	size[12];
	char	mtime[12];
	char	chksum[8];
	char	linkflag;
	char	linkname[TAR_FILENAME_SIZE];
	char	magic[8];
	char	uname[TAR_UNAME_LEN];
	char	gname[TAR_GNAME_LEN];
	char	devmajor[8];
	char	devminor[8];
} tar_record_header_t, *ptar_record_header_t;

typedef	union	record {
	char				charptr[TAR_BLOCK_SIZE];
	tar_record_header_t	head;
} tar_record_t, *ptar_record_t;
#pragma pack()

//The checksum field is filled with this while the checksum is computed.
#define	chkblanks	"        "		//8 blanks, no null

//The magic field is filled with this if uname and gname are valid.
#define	MAGIC		"ustar  "		//7 chars and a null

//The magic field is filled with this if this is a gnu format dump entry
#define	GNUMAGIC	"gnutar "		//7 chars and a null

//The linkflag defines the type of file
#define	TAR_LF_OLDNORMAL	'\0'	//Normal disk file, unix compatible
#define	TAR_LF_NORAML		'0'		//Normal disk file
#define	TAR_LF_LINK			'1'		//Link to previously dumped file
#define	TAR_LF_SYMLINK		'2'		//Symbolic link
#define	TAR_LF_CHR			'3'		//Character special file
#define	TAR_LF_BLK			'4'		//Block special file
#define	TAR_LF_DIR			'5'		//Directory
#define	TAR_LF_FIFO			'6'		//Fifo special file
#define	TAR_LF_CONTIG		'7'		//Contiguous file

//Bits used in the mode field - values in octal
#define	TAR_SUID	04000		//Set uid on execution
#define	TAR_SGID	02000		//Set gid on execution
#define	TAR_SVTX	01000		//Save text (sticky bit)

//File permissions
#define	TAR_UREAD	00400		//Read by owner
#define	TAR_UWRITE	00200		//Write by owner
#define	TAR_UEXEC	00100		//Execute/search by owner
#define	TAR_GREAD	00040		//Read by group
#define	TAR_GWRITE	00020		//Write by group
#define	TAR_GEXEC	00010		//Execute/search by group
#define	TAR_OREAD	00004		//Read by other
#define	TAR_OWRITE	00002		//Write by other
#define	TAR_OEXEC	00001		//Execute/search by other

#endif	//!	TAR_H_INCLUDE
