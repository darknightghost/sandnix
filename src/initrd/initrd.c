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
int		write_file(FILE *p_src, char* name);
int		write_dir();

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
	bool dir_flag;
	int ret;
	FILE *fp;

	path_buf[0] = '\0';
	indent = 0;

	//Read list file
	while(fgets(buf, BUF_SIZE, fp_list) != NULL) {
		//Get indent
		for(i = 0, p1 = buf;
		    *p1 != '\t';
		    i++, p1++);

		while(i < indent) {
			parent_dir();
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

				ret = write_file(fp, name_buf);
				fclose(fp);

				if(ret != 0) {
					return ret;
				}

				break;

			} else if(*p2 == '\0') {
				//It is a directory
				strcat(path_buf, p1);
				ret = write_dir();

				if(ret != 0) {
					return ret;
				}

				indent++;
				break;
			}
		}

	}
}

void parent_dir()
{
	char* p;

	for(p = &path_buf[strlen(path_buf) - 2];
	    p >= path_buf;
	    p--;) {
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
