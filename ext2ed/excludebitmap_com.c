/*
 * /usr/src/ext2ed/excludebitmap_com.c
 *
 * A part of the extended file system 2 disk editor.
 *
 * -------------------------
 * Handles the exlcude bitmap.
 * -------------------------
 *
 * This file implements the commands which are specific
 * to the excludebitmap type.
 *
 * Based on blockbitmap_com.c
 *
 * Copyright (C) 2011 Yongqiang Yang <xiaoqiangnk@gmail.com>
 *  From ext2ed/blockbitmap_com.c
 *  /usr/src/ext2ed/blockbitmap_com.c
 *
 *  A part of the extended file system 2 disk editor.
 *
 *  -------------------------
 *  Handles the block bitmap.
 *  -------------------------
 *
 *  This file implements the commands which are specific to the blockbitmap type.
 *
 *  First written on: July 5 1995
 *
 *  Copyright (C) 1995 Gadi Oxman
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ext2ed.h"

/*
 * The functions in this file use the flobal structure exclude_bitmap_info.
 * This structure contains the current position in the bitmap.
 */

/*
 * This function changes the current entry in the bitmap. It just changes
 * the entry_num variable in exclude_bitmap_info and dispatches a show command
 * to show the new entry.
 */
void type_ext2_exclude_bitmap___entry(char *command_line)

{
	unsigned long entry_num;
	char *ptr, buffer[80];

	/* Get the requested entry */
	ptr = parse_word(command_line, buffer);

	if (*ptr == 0) {
		wprintw(command_win,"Error - No argument specified\n");
		refresh_command_win();
		return;
	}
	ptr = parse_word(ptr, buffer);

	entry_num = atol(buffer);

	/* Check if it is a valid entry number */
	if (entry_num >= file_system_info.super_block.s_blocks_per_group) {
		wprintw(command_win,"Error - Entry number out of bounds\n");
		refresh_command_win();
		return;
	}

	/* If it is, just change entry_num and */
	exclude_bitmap_info.entry_num = entry_num;
	/* dispatch a show command */
	strcpy(buffer,"show");
	dispatch(buffer);
}

/*
 * This function passes to the next entry in the bitmap.
 * We just call the above entry command.
 */
void type_ext2_excldue_bitmap___next(char *command_line)
{
	long entry_offset = 1;
	char *ptr, buffer[80];

	ptr = parse_word(command_line, buffer);
	if (*ptr != 0) {
		ptr = parse_word(ptr, buffer);
		entry_offset = atol(buffer);
	}

	sprintf(buffer, "entry %ld",
		exclude_bitmap_info.entry_num + entry_offset);
	dispatch(buffer);
}

void type_ext2_exclude_bitmap___prev (char *command_line)
{
	long entry_offset = 1;
	char *ptr, buffer[80];

	ptr = parse_word(command_line, buffer);
	if (*ptr != 0) {
		ptr = parse_word(ptr, buffer);
		entry_offset = atol(buffer);
	}

	sprintf(buffer, "entry %ld",
		exclude_bitmap_info.entry_num - entry_offset);
	dispatch(buffer);
}

/*
 * This function starts excluding block from the current position.
 * Excluding involves setting the correct bits in the bitmap.
 * This function is a vector version of exclude_block below -
 * We just run on the blocks that we need to exclude, and call
 * exclude_block for each one.
 */
void type_ext2_exclude_bitmap___exclude(char *command_line)
{
	long entry_num, num = 1;
	char *ptr, buffer[80];

	/* Get the number of blocks to exclude */
	ptr = parse_word(command_line, buffer);
	if (*ptr != 0) {
		ptr = parse_word(ptr, buffer);
		num = atol(buffer);
	}

	entry_num = exclude_bitmap_info.entry_num;

	/* Check for limits */
	if (num > file_system_info.super_block.s_blocks_per_group - entry_num) {
		wprintw(command_win,"Error - There aren't that much blocks "
			"in the group\n");
		refresh_command_win();
		return;
	}

	while (num) {
		/* call exclude_block for each block */
		allocate_block(entry_num);
		num--;
		entry_num++;
	}

	/* Show the result */
	dispatch("show");
}

/*
 * This is the opposite of the above function - We call deexclude_block.
 */
void type_ext2_exclude_bitmap___deallocate (char *command_line)
{
	long entry_num, num = 1;
	char *ptr, buffer[80];

	ptr = parse_word(command_line, buffer);
	if (*ptr != 0) {
		ptr = parse_word(ptr, buffer);
		num = atol(buffer);
	}

	entry_num = exclude_bitmap_info.entry_num;
	if (num > file_system_info.super_block.s_blocks_per_group-entry_num) {
		wprintw(command_win,"Error - There aren't that much blocks "
			"in the group\n");
		refresh_command_win();
		return;
	}

	while (num) {
		deexclude_block(entry_num);
		num--;
		entry_num++;
	}

	dispatch("show");
}


/* In this function we convert the bit number into the right byte 
 * and inner bit positions.
 */
void exclude_block(long entry_num)
{
	unsigned char bit_mask=1;
	int byte_offset, j;

	/* Find the correct byte - entry_num / 8 */
	byte_offset = entry_num / 8;

	/* The position inside the byte is entry_num % 8 */
	j = entry % 8;

	/* Generate the OR mask - 1 at the right place */
	bit_mask <<= j; 

	/* And apply it */
	type_data.u.buffer[byte_offset] |= bit_mask;
}

/* This is the opposite of exclude_block above.
 * We use an AND mask instead of an or mask.
 */
void deexclude_block(long entry_num)
{
	unsigned char bit_mask = 1;
	int byte_offset, j;

	byte_offset = entry_num / 8;

	j = entry_num % 8;
	bitmak <<= j;
	bit_mask ^= 0xff;

	type_data.u.buffer[byte_offset] &= bit_mask;
}

/*
 * We show the bitmap as a series of bits, grouped at 8-bit intervals.
 * We display 8 such groups on each line.
 * The current position (as known from exclude_bitmap_info.entry_num)
 * is highlighted.
 */
void type_ext2_exclude_bitmap___show (char *command_line)
{
	int i, j;
	unsigned char *ptr;
	unsigned long block_num, entry_num;

	ptr = type_data.u.buffer;
	show_pad_info.line = 0;
	show_pad_info.max_line = -1;

	wmove(show_pad, 0, 0);
	for (i = 0, entry_num = 0;
	     i < file_system_info.super_block.s_blocks_per_group / 8;
	     i++, ptr++) {

		/* j contains the AND bit mask */
		for (j = 1; j <= 128; j *= 2) {
			if (entry_num == exlcude_bitmap_info.entry_num) {
				/* Highlight the current entry */
				wattrset(show_pad,A_REVERSE);
				show_pad_info.line = show_pad_info.max_line -
					show_pad_info.display_lines / 2;
			}

			/* Apply the mask */
			if ((*ptr) & j)
				wprintw (show_pad, "1");
			else
				wprintw (show_pad, "0");

			if (entry_num == exclude_bitmap_info.entry_num)
				wattrset(show_pad, A_NORMAL);

			/* Pass to the next entry */
			entry_num++;
		}
		wprintw (show_pad, " ");
		if (i % 8 == 7) {
			/* Display 8 groups in a row */
			wprintw(show_pad, "\n");
			show_pad_info.max_line++;
		}
	}

	refresh_show_pad();

	/* Show the usual information */
	show_info();

	/* Show the group number */
	wmove(show_win, 1, 0);
	wprintw(show_win, "Exclude bitmap of block group %ld\n",
		exclude_bitmap_info.group_num);

	/* Show the block number */
	block_num = exclude_bitmap_info.entry_num +
		    exclude_bitmap_info.group_num *
		    file_system_info.super_block.s_blocks_per_group;

	block_num += file_system_info.super_block.s_first_data_block;

	/* and the exclude status */
	wprintw(show_win, "Status of block %ld - ", block_num);
	ptr = type_data.u.buffer + exclude_bitmap_info.entry_num / 8;

	j=1;
	j <<= exclude_bitmap_info.entry_num % 8;

	if ((*ptr) & j)
		wprintw(show_win, "Excluded\n");
	else
		wprintw(show_win, "Non-Excluded\n");
	refresh_show_win();
}
