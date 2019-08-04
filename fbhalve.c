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
#include <libgen.h>
#include <string.h>
#include <assert.h>

#define STRSIZEOF(s) (sizeof(s) - 1)

#define FULL_BLOCK "\xE2\x96\x88"
#define NBSP "\xC2\xA0"
#define ERR_UNALIGNED -1
#define ERR_INVALCHAR -2

char* PROGNAME;

/*
 * Lookup table for the smaller characters. An index is built up by using the
 * lowest 4 bits in the following way:
 * 8            4           2           1
 * Low-right    Up-right    Low-left    Up-left
*/
static const char *halveblocks[16] = {
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

static int
is_nbsp(char *in, char *end)
{
	if (in + STRSIZEOF(NBSP) > end)
		return -1;
	return in[0] == NBSP[0] && \
	       in[1] == NBSP[1];
}

static int
is_full(char *in, char *end)
{
	if (in + STRSIZEOF(FULL_BLOCK) > end)
		return -1;
	return in[0] == FULL_BLOCK[0] && \
	       in[1] == FULL_BLOCK[1] && \
	       in[2] == FULL_BLOCK[2];
}

static void
set_bit(size_t *ch_index, int bit)
{
	*ch_index |= 1 << bit;
}

static size_t
check_char(char *line, char *end, size_t *ch_index, int bit, int *err) {
	if (*err != 0)
		return 0;
	int ret;
	ret = is_full(line, end);
	if (ret < 0) {
		*err = ERR_UNALIGNED;
		return 0;
	}
	if (ret) {
		set_bit(ch_index, bit);
		return STRSIZEOF(FULL_BLOCK);
	}
	ret = is_nbsp(line, end);
	if (ret < 0) {
		*err = ERR_UNALIGNED;
		return 0;
	}
	if (ret)
		return STRSIZEOF(NBSP);
	*err = ERR_INVALCHAR;
	return 0;
}

static int
process_lines(char *upper, char *lower, size_t upper_len, size_t lower_len)
{
	size_t ch_index;
	char *upper_end, *lower_end;
	int err;

	upper_end = upper + upper_len;
	lower_end = lower + lower_len;

	while(!(upper[0] == '\n' || upper[0] == '\0')) {
		ch_index = err = 0;
		upper += check_char(upper, upper_end, &ch_index, 0, &err);
		upper += check_char(upper, upper_end, &ch_index, 2, &err);
		if (!(lower[0] == '\n' || lower[0] == '\0')) {
			lower += check_char(lower, lower_end, &ch_index, 1, &err);
			lower += check_char(lower, lower_end, &ch_index, 3, &err);
		}
		if (err != 0)
			return err;
		printf("%s", halveblocks[ch_index]);
	}
	printf("\n");
	return 0;
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
	int ret;
	char *line1, *line2;

	line1 = line2 = NULL;
	linecap = 0;

	PROGNAME = basename(argv[0]);

	if (argc > 2) {
		fprintf(stderr, "USAGE:\t%s [inputfile]\n", PROGNAME);
		fprintf(
			stderr,
			"\tWithout inputfile or with '-', %s will read from standard input\n",
			PROGNAME
		);
		return EXIT_FAILURE;
	}

	if (argc == 1 || strncmp(argv[1], "-", 2) == 0) {
		input = stdin;
	} else {
		input = fopen(argv[1], "r");
		if (input == NULL) {
			perror(PROGNAME);
			return EXIT_FAILURE;
		}
	}

	for (;;) {
		if ((line1_len = getline(&line1, &linecap, input)) == EOF)
			break;
		if (line1_len == -1) {
			perror(PROGNAME);
			return EXIT_FAILURE;
		}

		if ((line2_len = getline(&line2, &linecap, input)) == EOF)
			line1_len = 0;
		if (line2_len == -1) {
			perror(PROGNAME);
			return EXIT_FAILURE;
		}

		ret = process_lines(line1, line2,
		                    (size_t) line1_len, (size_t) line2_len);
		switch (ret) {
		case ERR_INVALCHAR:
			fprintf(
				stderr,
				"%s: Invalid character in input file\n",
				PROGNAME
			);
			return EXIT_FAILURE;
		case (ERR_UNALIGNED):
			fprintf(
				stderr,
				"%s: Unaligned input file\n",
				PROGNAME
			);
			return EXIT_FAILURE;
		case 0:
			/* Success */
			break;
		default:
			/* Unreachable */
			assert(0);
		}
	}
	return EXIT_SUCCESS;
}
