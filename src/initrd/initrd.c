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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../common/tar.h"

#define	BUF_SIZE		4096

char	block_buf[TAR_BLOCK_SIZE];
char	path_buf[PATH_MAX];
char	line_buf[BUF_SIZE];
char	name_buf[NAME_MAX];
char	src_buf[PATH_MAX];

void	usage();
int		pack(FILE* fp_list, FILE* fp_output);
void	parent_dir();
int		write_file(FILE* fp_output, FILE *p_src, char* name);
int		write_dir(FILE* fp_output);

int main(int argc, char* argv[])
{
	FILE *fp_list, *fp_output;
	char* p_list_path;
	char* p_output_path;
	int i;
	int ret;

	//Get parameters
	if(argc != 5) {
		usage();
		return EINVAL;
	}

	i = 1;

	while(i < 5) {
		if(strcmp(argv[i], "-l") == 0) {
			i++;
			p_list_path = argv[i];
			i++;

		} else if(strcmp(argv[i], "-o") == 0) {
			i++;
			p_output_path = argv[i];
			i++;

		} else {
			usage();
			return EINVAL;
		}
	}

	//Open files
	//List file
	fp_list = fopen(p_list_path, "r");

	if(fp_list == NULL) {
		printf("Failed to open list file!\n");
		return errno;
	}

	//Output file
	fp_output = fopen(p_output_path, "w");

	if(fp_output == NULL) {
		printf("Failed to open output file!\n");
		fclose(fp_list);
		return errno;
	}

	//Pack file
	ret = pack(fp_list, fp_output);
	fclose(fp_list);
	fclose(fp_output);

	return ret;
}

void usage()
{
	printf("Usage:\n");
	printf("\tinitrd -l listfile -o output\n");
	return;
}

int pack(FILE* fp_list, FILE* fp_output)
{
	int indent, i;
	char *p1, *p2;
	int ret;
	FILE *fp;
	size_t len;

	path_buf[0] = '\0';
	indent = 0;

	//Read list file
	while(fgets(line_buf, BUF_SIZE, fp_list) != NULL) {
		len = strlen(line_buf) - 1;

		if(line_buf[len] == '\n') {
			line_buf[len] = '\0';
		}

		//Get indent
		for(i = 0, p1 = line_buf;
		    *p1 == '\t';
		    i++, p1++);

		while(i < indent) {
			parent_dir();
			indent--;
		}

		//Check it is a file or directory
		p2 = p1;

		while(1) {
			p2++;

			if(*p2 == ' ' || *p2 == '\t') {
				//It is a file
				//Copy file name
				*p2 = '\0';
				p2++;
				strcpy(name_buf, p1);

				//Copy source path
				while(*p2 == ' '
				      || *p2 == '\t') {
					p2++;
				}

				if(*p2 == '\0') {
					printf("Incorrect format of list file!\n");
					return EINVAL;
				}

				strcpy(src_buf, p2);
				fp = fopen(src_buf, "r");

				if(fp == NULL) {
					ret = errno;
					printf("Failed to open file:%s\n",
					       src_buf);
					return ret;

				} else {
					printf("Added file:%s\n",
					       src_buf);
				}

				ret = write_file(fp_output, fp, name_buf);
				fclose(fp);

				if(ret != 0) {
					return ret;
				}

				break;

			} else if(*p2 == '\0') {
				//It is a directory
				if(path_buf[0] != '\0') {
					strcat(path_buf, "/");
				}

				strcat(path_buf, p1);
				ret = write_dir(fp_output);

				if(ret != 0) {
					return ret;
				}

				indent++;
				break;
			}
		}

	}

	memset(block_buf, 0, TAR_BLOCK_SIZE);
	fwrite(block_buf, TAR_BLOCK_SIZE, 1, fp_output);
	return 0;
}

void parent_dir()
{
	char* p;

	for(p = &path_buf[strlen(path_buf) - 2];
	    p >= path_buf;
	    p--) {
		if(*p == '/') {
			break;
		}
	}

	if(p < path_buf) {
		p = path_buf;
	}

	*p = '\0';

	return;
}

int write_file(FILE* fp_output, FILE *p_src, char* name)
{
	ptar_record_header_t p_head;
	unsigned int checksum;
	int i;
	unsigned int mode;
	size_t length_written;
	size_t length_read;
	struct stat file_stat;

	memset(block_buf, 0, TAR_BLOCK_SIZE);
	p_head = (ptar_record_header_t)block_buf;

	//Get file stat
	stat(src_buf, &file_stat);

	//Record head
	sprintf(p_head->header.name, "%s/%s", path_buf, name);
	mode = TAR_UREAD | TAR_GREAD | TAR_OREAD;

	if(file_stat.st_mode & S_IXUSR) {
		mode = mode | TAR_UEXEC;
	}

	if(file_stat.st_mode & S_IXGRP) {
		mode = mode | TAR_GEXEC;
	}

	if(file_stat.st_mode & S_IXOTH) {
		mode = mode | TAR_OEXEC;
	}

	sprintf(p_head->header.mode,
	        "%06o ",
	        mode);
	strcpy(p_head->header.uid,
	       "000000 ");
	strcpy(p_head->header.gid,
	       "000000 ");
	sprintf(p_head->header.size, "%011o ",
	        (unsigned int)(file_stat.st_size));
	sprintf(p_head->header.mtime, "%011o ",
	        (unsigned int)(file_stat.st_mtime));
	strcpy(p_head->header.chksum, CHK_BLANKS);
	p_head->header.linkflag = TAR_LF_NORAML;
	strcpy(p_head->header.magic, MAGIC);
	strcpy(p_head->header.uname, "root");
	strcpy(p_head->header.gname, "root");
	strcpy(p_head->header.devmajor,
	       "000000 ");
	strcpy(p_head->header.devminor,
	       "000000 ");

	//Compute checksum
	for(checksum = 0, i = 0;
	    i < sizeof(p_head->header);
	    i++) {
		checksum += block_buf[i];
	}

	sprintf(p_head->header.chksum, "0%o", checksum);
	p_head->header.linkflag = TAR_LF_NORAML;

	//Write record head
	fwrite(block_buf, TAR_BLOCK_SIZE, 1, fp_output);

	printf("Created file:%s\n", path_buf);

	//Write file data
	length_written = 0;

	while(1) {
		memset(block_buf, 0, TAR_BLOCK_SIZE);
		length_read = fread(block_buf, 1, TAR_BLOCK_SIZE, p_src);

		if(length_read == 0) {
			break;
		}

		fwrite(block_buf, TAR_BLOCK_SIZE, 1, fp_output);
		length_written += length_read;
		printf("%5u bytes written.\n",
		       (unsigned int)length_written);
	}

	return 0;
}

int write_dir(FILE* fp_output)
{
	ptar_record_header_t p_head;
	unsigned int checksum;
	int i;

	memset(block_buf, 0, TAR_BLOCK_SIZE);
	p_head = (ptar_record_header_t)block_buf;

	//Record head
	sprintf(p_head->header.name, "%s/", path_buf);
	sprintf(p_head->header.mode,
	        "%06o ",
	        TAR_UREAD | TAR_UEXEC | TAR_GREAD | TAR_GEXEC | TAR_OREAD | TAR_OEXEC);
	strcpy(p_head->header.uid,
	       "000000 ");
	strcpy(p_head->header.gid,
	       "000000 ");
	sprintf(p_head->header.size, "%011o ", 0);
	sprintf(p_head->header.mtime, "%011o ",
	        (unsigned int)time((time_t*)NULL));
	strcpy(p_head->header.chksum, CHK_BLANKS);
	p_head->header.linkflag = TAR_LF_DIR;
	strcpy(p_head->header.magic, MAGIC);
	strcpy(p_head->header.uname, "root");
	strcpy(p_head->header.gname, "root");
	strcpy(p_head->header.devmajor,
	       "000000 ");
	strcpy(p_head->header.devminor,
	       "000000 ");

	//Compute checksum
	for(checksum = 0, i = 0;
	    i < sizeof(p_head->header);
	    i++) {
		checksum += block_buf[i];
	}

	sprintf(p_head->header.chksum, "0%o", checksum);
	p_head->header.linkflag = TAR_LF_DIR;

	//Write file
	fwrite(block_buf, TAR_BLOCK_SIZE, 1, fp_output);

	printf("Created directory:%s\n", path_buf);

	return 0;
}
