/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "vocal.h"

char* getline (FILE *fp)  {
	char ch = 0;
	char *line = NULL;
	unsigned int size=0;

	while (ch!='\n' && ch!='\r' && !feof(fp))  {
		ch = fgetc(fp);
		line = (char*) realloc(line, ++size);
		line[size-1]=ch;
	}

	if (size)
		line[size]=0;

	if (line[size-1] == '\n' || line[size-1] == '\r')
		line[size-1]=0;

	return line;
}

char** preg_match (char *regex, char* s, int *size)  {
	char **match = NULL;
	regex_t re;
	regmatch_t off;
	*size=0;

	if (regcomp(&re,regex,REG_EXTENDED)!=0)
		return match;
				 
	for (; !regexec(&re, s, 1, &off, 0); s += off.rm_eo)  {
		match = (char**) realloc(match, (++(*size))*sizeof(char*));
		match[*size-1] = (char*) malloc(BUFSIZ);
		memset (match[*size-1], 0x0, BUFSIZ);
		memcpy (match[*size-1], s + off.rm_so, off.rm_eo - off.rm_so);
	}

	regfree(&re);
	return match;
}

double ABS (double x)  { return (x >= 0) ? x : -x; }

