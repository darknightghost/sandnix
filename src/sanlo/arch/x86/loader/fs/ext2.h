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

#ifndef	EXT2_H_INCLUDE
#define	EXT2_H_INCLUDE

#include "fs.h"

#define	EXT2_SUPER_BLOCK_SIZE		1024
#define	EXT2_NDIR_BLOCKS			12
#define	EXT2_IND_BLOCK				EXT2_NDIR_BLOCKS
#define	EXT2_DIND_BLOCK				(EXT2_IND_BLOCK + 1)
#define	EXT2_TIND_BLOCK				(EXT2_DIND_BLOCK + 1)
#define	EXT2_N_BLOCKS				(EXT2_TIND_BLOCK + 1)

#pragma	pack(1)

/*
 * Structure of the super block
 */
typedef	struct _ext2_super_block {
	le32	s_inodes_count;		/* Inodes count */
	le32	s_blocks_count;		/* Blocks count */
	le32	s_r_blocks_count;	/* Reserved blocks count */
	le32	s_free_blocks_count;	/* Free blocks count */
	le32	s_free_inodes_count;	/* Free inodes count */
	le32	s_first_data_block;	/* First Data Block */
	le32	s_log_block_size;	/* Block size */
	le32	s_log_frag_size;	/* Fragment size */
	le32	s_blocks_per_group;	/* # Blocks per group */
	le32	s_frags_per_group;	/* # Fragments per group */
	le32	s_inodes_per_group;	/* # Inodes per group */
	le32	s_mtime;		/* Mount time */
	le32	s_wtime;		/* Write time */
	le16	s_mnt_count;		/* Mount count */
	le16	s_max_mnt_count;	/* Maximal mount count */
	le16	s_magic;		/* Magic signature */
	le16	s_state;		/* File system state */
	le16	s_errors;		/* Behaviour when detecting errors */
	le16	s_minor_rev_level; 	/* minor revision level */
	le32	s_lastcheck;		/* time of last check */
	le32	s_checkinterval;	/* max. time between checks */
	le32	s_creator_os;		/* OS */
	le32	s_rev_level;		/* Revision level */
	le16	s_def_resuid;		/* Default uid for reserved blocks */
	le16	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 *
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	le32	s_first_ino; 		/* First non-reserved inode */
	le16	s_inode_size; 		/* size of inode structure */
	le16	s_block_group_nr; 	/* block group # of this superblock */
	le32	s_feature_compat; 	/* compatible feature set */
	le32	s_feature_incompat; 	/* incompatible feature set */
	le32	s_feature_ro_compat; 	/* readonly-compatible feature set */
	u8		s_uuid[16];		/* 128-bit uuid for volume */
	char	s_volume_name[16]; 	/* volume name */
	char	s_last_mounted[64]; 	/* directory where last mounted */
	le32	s_algorithm_usage_bitmap; /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT2_COMPAT_PREALLOC flag is on.
	 */
	u8	s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	u8	s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	u16	s_padding1;
	/*
	 * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
	u8	s_journal_uuid[16];	/* uuid of journal superblock */
	u32	s_journal_inum;		/* inode number of journal file */
	u32	s_journal_dev;		/* device number of journal file */
	u32	s_last_orphan;		/* start of list of inodes to delete */
	u32	s_hash_seed[4];		/* HTREE hash seed */
	u8	s_def_hash_version;	/* Default hash version to use */
	u8	s_reserved_char_pad;
	u16	s_reserved_word_pad;
	le32	s_default_mount_opts;
	le32	s_first_meta_bg; 	/* First metablock block group */
	u32	s_reserved[190];	/* Padding to the end of the block */
} ext2_super_block, *pext2_super_block;

/*
 * Structure of a blocks group descriptor
 */
typedef	struct _ext2_group_desc {
	le32	bg_block_bitmap;		/* Blocks bitmap block */
	le32	bg_inode_bitmap;		/* Inodes bitmap block */
	le32	bg_inode_table;		/* Inodes table block */
	le16	bg_free_blocks_count;	/* Free blocks count */
	le16	bg_free_inodes_count;	/* Free inodes count */
	le16	bg_used_dirs_count;	/* Directories count */
	le16	bg_pad;
	le32	bg_reserved[3];
} ext2_group_desc, *pext2_group_desc;

/*
 * Structure of an inode on the disk
 */
typedef	struct _ext2_inode {
	le16	i_mode;		/* File mode */
	le16	i_uid;		/* Low 16 bits of Owner Uid */
	le32	i_size;		/* Size in bytes */
	le32	i_atime;	/* Access time */
	le32	i_ctime;	/* Creation time */
	le32	i_mtime;	/* Modification time */
	le32	i_dtime;	/* Deletion Time */
	le16	i_gid;		/* Low 16 bits of Group Id */
	le16	i_links_count;	/* Links count */
	le32	i_blocks;	/* Blocks count */
	le32	i_flags;	/* File flags */
	union {
		struct {
			le32  l_i_reserved1;
		} linux1;
		struct {
			le32  h_i_translator;
		} hurd1;
		struct {
			le32  m_i_reserved1;
		} masix1;
	} osd1;				/* OS dependent 1 */
	le32	i_block[EXT2_N_BLOCKS];/* Pointers to blocks */
	le32	i_generation;	/* File version (for NFS) */
	le32	i_file_acl;	/* File ACL */
	le32	i_dir_acl;	/* Directory ACL */
	le32	i_faddr;	/* Fragment address */
	union {
		struct {
			u8	l_i_frag;	/* Fragment number */
			u8	l_i_fsize;	/* Fragment size */
			u16	i_pad1;
			le16	l_i_uid_high;	/* these 2 fields    */
			le16	l_i_gid_high;	/* were reserved2[0] */
			u32	l_i_reserved2;
		} linux2;
		struct {
			u8	h_i_frag;	/* Fragment number */
			u8	h_i_fsize;	/* Fragment size */
			le16	h_i_mode_high;
			le16	h_i_uid_high;
			le16	h_i_gid_high;
			le32	h_i_author;
		} hurd2;
		struct {
			u8	m_i_frag;	/* Fragment number */
			u8	m_i_fsize;	/* Fragment size */
			u16	m_pad1;
			u32	m_i_reserved2[2];
		} masix2;
	} osd2;				/* OS dependent 2 */
} ext2_inode, *pext2_inode;

/*
 * The new version of the directory entry.  Since EXT2 structures are
 * stored in intel byte order, and the name_len field could never be
 * bigger than 255 chars, it's safe to reclaim the extra byte for the
 * file_type field.
 */
typedef	struct _ext2_dir_entry_2 {
	le32	inode;			/* Inode number */
	le16	rec_len;		/* Directory entry length */
	u8		name_len;		/* Name length */
	u8		file_type;
	char	name[256];			/* File name, up to EXT2_NAME_LEN */
} ext2_dir_entry_2, *pext2_dir_entry_2;
/*
 * Ext2 directory file types.  Only the low 3 bits are used.  The
 * other bits are reserved for now.
 */
enum {
	EXT2_FT_UNKNOWN		= 0,
	EXT2_FT_REG_FILE	= 1,
	EXT2_FT_DIR			= 2,
	EXT2_FT_CHRDEV		= 3,
	EXT2_FT_BLKDEV		= 4,
	EXT2_FT_FIFO		= 5,
	EXT2_FT_SOCK		= 6,
	EXT2_FT_SYMLINK		= 7,
	EXT2_FT_MAX
};
#pragma	pack()

typedef	struct _ext2_file_info {
	u32					block_size;
	ext2_inode			inode;
	pext2_group_desc	p_group_desc;
	u32					current_block;
	char*				block_buf;
} ext2_file_info, *pext2_file_info;

#define	EXT2_DIR_ENTRY_BASIC_SIZE	(sizeof(ext2_dir_entry_2)-256)
bool		ext2_open(pfile fp, char* path);
u32			ext2_read(pfile fp, u8* buf, size_t len);
void		ext2_close(pfile fp);

#endif	//!	EXT2_H_INCLUDE
