/* fbhalve - halve unicode art that uses FULL BLOCK characters */

/*
 * Copyright (C) 2019  Jelle Besseling
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define STRSIZEOF(s) (sizeof(s) - 1)

#define FULL_BLOCK "\xE2\x96\x88"
#define NBSP "\xC2\xA0"

/*
 * Lookup table for the smaller characters. An index is built up by using the
 * lowest 4 bits in the following way:
 * 8		4		2		1
 * Low-right	Up-right	Low-left	Up-left
*/
static char* halveblocks[16] = {
	NBSP,
	"\xE2\x96\x98",	/* Quadrant Upper Left */
	"\xE2\x96\x96",	/* Quadrant Lower Left */
	"\xE2\x96\x8C",	/* Left Half Block */
	"\xE2\x96\x9D",	/* Quadrant Upper Right */
	"\xE2\x96\x80",	/* Upper Half Block */
	"\xE2\x96\x9E", /* Quadrant Upper Right and Lower Left */
	"\xE2\x96\x9B",	/* Quadrant Upper Left and Upper Right and Lower Left */
	"\xE2\x96\x97",	/* Quadrant Lower Right */
	"\xE2\x96\x9A", /* Quadrant Upper Left and Lower Right */
	"\xE2\x96\x84",	/* Lower Half Block */
	"\xE2\x96\x99",	/* Quadrant Upper Left and Lower Left and Lower Right */
	"\xE2\x96\x90",	/* Right Half Block */
	"\xE2\x96\x9C",	/* Quadrant Upper Left and Upper Right and Lower Right */
	"\xE2\x96\x9F",	/* Quadrant Upper Right and Lower Left and Lower Right */
	FULL_BLOCK	/* full block */
};

int
is_nbsp(char* in)
{
	return in[0] == NBSP[0] && \
	       in[1] == NBSP[1];
}

int
is_full(char *in, int *i, ssize_t length)
{
	if (*i + STRSIZEOF(FULL_BLOCK) > (unsigned long) length) {
		return 0;
	}
	if (
		in[0] == FULL_BLOCK[0] && \
		in[1] == FULL_BLOCK[1] && \
		in[2] == FULL_BLOCK[2]
	) {
		*i += STRSIZEOF(FULL_BLOCK);
		return 1;
	} else if (is_nbsp(in)) {
		*i += STRSIZEOF(NBSP);
		return 0;
	} else {
		fprintf(stderr, "%s: invalid character\n", getprogname());
		exit(EXIT_FAILURE);
	}
}

void
process_lines(char *line1, char *line2, ssize_t line1_len, ssize_t line2_len)
{
	int upper_i, lower_i, ch_index;
	upper_i = lower_i = 0;
	for(;;) {
		if (line1[upper_i] == '\n' || line1[upper_i] == '\0')
			break;

		ch_index = 0;
		ch_index = is_full(&line1[upper_i], &upper_i, line1_len) \
			| is_full(&line2[lower_i], &lower_i, line2_len) << 1;
		ch_index |= is_full(&line1[upper_i], &upper_i, line1_len) << 2 \
			| is_full(&line2[lower_i], &lower_i, line2_len) << 3;
		printf("%s", halveblocks[ch_index]);
	}
	printf("\n");
}

/*
 * fbhalve converts unicode art files that use U+2588 FULL BLOCK characters to
 * an image that is halve the size of the original, by using smaller unicode
 * block characters such as U+2580 UPPER HALF BLOCK
 */
int
main(int argc, char *argv[])
{
	FILE* input;
	ssize_t line1_len, line2_len;
	size_t linecap;
	char *line1, *line2;

	line1 = line2 = NULL;
	linecap = 0;

	setprogname(argv[0]);

	if (argc > 2) {
		fprintf(stderr, "USAGE:\t%s [inputfile]\n", getprogname());
		fprintf(
			stderr,
			"\tWithout inputfile or with '-', %s will read from standard input\n",
			getprogname()
		);
		return EXIT_FAILURE;
	}

	if (argc == 1 || strncmp(argv[1], "-", 1) == 0) {
		input = stdin;
	} else {
		input = fopen(argv[1], "r");
		if (input == NULL) {
			perror(getprogname());
			return EXIT_FAILURE;
		}
	}

	for (;;) {
		if ((line1_len = getline(&line1, &linecap, input)) < 0)
			break;
		/*
		 * Ignore errors in the second line because is_full handles
		 * them silently.
		 */
		line2_len = getline(&line2, &linecap, input);

		process_lines(line1, line2, line1_len, line2_len);
	}
	return (0);
}
