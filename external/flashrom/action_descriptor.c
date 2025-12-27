/* Copyright 2018 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "action_descriptor.h"
#include "chipdrivers.h"
#include "flash.h"
#include "layout.h"
#include "programmer.h"

/**
 * action_descriptor is DEPRECATED !
 * Allow for easy toggling off for the eventual removal from the tree.
 *
 * TRUE  := action_descriptor is not used.
 * FALSE := action_descriptor is used on Intel.
 */
static bool deprecate_ad = false;

/*
 * This global variable is used to communicate the type of ICH found on the
 * device. When running on non-intel platforms default value of
 * CHIPSET_ICH_UNKNOWN is used.
*/
extern enum ich_chipset ich_generation;

/*
 * Unfortunate global state.
 */
static bool dry_run = false;

/*
 * This module analyses the contents of 'before' and 'after' flash images and
 * based on the images' differences prepares a list of processing actions to
 * take.
 *
 * The goal is to prepare actions using the chip's erase capability in a most
 * efficient way: erasing smallest possible portions of the chip gives the
 * highest granularity, but if many small areas need to be erased, erasing a
 * larger area, even if re-writing it completely, is more efficient. The
 * breakdown is somewhere at 60%.
 *
 * Each flash chip description in flash.c includes a set of erase command
 * descriptors, different commands allowing to erase blocks of fixed different
 * sizes. Sometimes the erase command for a certain block size does not cover
 * the entire chip. This module preprocesses the flash chip description to
 * compile an array of erase commands with their block size indices such that
 * it is guaranteed that the command can be used to erase anywhere in the chip
 * where erase is required based on the differences between 'before' and
 * 'after' images.
 *
 * 'eraser_index' below is the index into the 'block_erasers' array of the
 * flash chip descriptor, points to the function to use to erase the block of
 * a certain size.
 *
 * The erase command could potentially operate on blocks of different sizes,
 * 'region_index' is the index into the 'block_erasers.eraseblocks' array
 * which defines what block size would be used by this erase command.
 */
struct eraser {
	int eraser_index;
	int region_index;
};

/*
 * A helper structure which holds information about blocks of a given size
 * which require writing and or erasing.
 *
 * The actual map of the blocks is pointed at by the 'block_map' field, one
 * byte per block. Block might need an erase, or just a write, depending on
 * the contents of 'before' and 'after' flash images.
 *
 * The 'limit' field holds the number of blocks of this size, which is
 * equivalent to one block of the next larger size in term of time required
 * for erasing/programming.
 */
struct range_map {
	size_t block_size;
	int limit;
	struct b_map {
		uint8_t need_change:1;
		uint8_t need_erase:1;
	} *block_map;
};

/*
 * A debug function printing out the array or processing units from an the
 * action descriptor.
 */
static void dump_descriptor(struct action_descriptor *descriptor)
{
	struct processing_unit *pu = descriptor->processing_units;

	while (pu->num_blocks) {
		msg_pdbg("%06zx..%06zx %6zx x %zd eraser %d\n", pu->offset,
			 pu->offset + pu->num_blocks * pu->block_size - 1,
			 pu->block_size, pu->num_blocks,
			 pu->block_eraser_index);
		pu++;
	}
}

/*
 * Do not allow use of unsupported erasers functions.
 *
 * On some Intel platforms the ICH SPI controller is restricting the set of
 * SPI command codes the AP can issue, in particular limiting the set of erase
 * functions to just two of them.
 *
 * This function creates a local copy of the flash chip descriptor found in
 * the main table, filtering out unsupported erase function pointers, when
 * necessary.
 *
 * flash: pointer to the master flash context, including the original chip
 *        descriptor.
 * chip: pointer to a flash chip descriptor copy, potentially with just a
 *            subset of erasers included.
 */
static void fix_erasers_if_needed(struct flashchip *chip,
				  struct flashctx *flash)
{
	int i;

	if (deprecate_ad)
		return;

	/* Need to copy no matter what. */
	*chip = *flash->chip;

#if ((defined (__i386__) || defined (__x86_64__) || defined(__amd64__))) && CONFIG_INTERNAL
	/*
	 * ich_generation is set to the chipset type when running on an x86
	 * device, even when flashrom was invoked to program the EC.
	 *
	 * But ICH type does not affect EC programming path, so no need to
	 * check if the eraser is supported in that case.
	 */
	if ((ich_generation == CHIPSET_ICH_UNKNOWN) || programming_ec()) {
		msg_pdbg("%s: kept all erasers\n",  __func__);
		return;
	}
#else
	msg_pdbg("%s: kept all erasers on non-x86\n",  __func__);
	return;
#endif /* !IS_X86 */

	/*
	 * We are dealing with an Intel controller; different chipsets allow
	 * different erase commands. Let's check the commands and allow only
	 * those which the controller accepts.
	 */
	dry_run = true;
	for (i = 0; i < NUM_ERASEFUNCTIONS; i++) {
		erasefunc_t *erase_func = lookup_erase_func_ptr(&chip->block_erasers[i]);
		/* Assume it is not allowed. */
		if (!erase_func)
			continue;

		if (!erase_func(flash, 0, flash->chip->total_size * 1024)) {
			msg_pdbg("%s: kept eraser at %d\n",  __func__, i);
			continue;
		}

		chip->block_erasers[i].block_erase = NO_BLOCK_ERASE_FUNC;
	}
	dry_run = false;
}

/*
 * Prepare a list of erasers available on this chip, sorted by the block size,
 * from lower to higher.
 *
 * @flash	    pointer to the flash context
 * @erase_size	    maximum offset which needs to be erased
 * @sorted_erasers  pointer to the array of eraser structures, large enough to
 *                  fit NUM_ERASEFUNCTIONS elements.
 *
 * Returns number of elements put into the 'sorted_erasers' array.
 */
static size_t fill_sorted_erasers(struct flashctx *flash,
				  size_t erase_size,
				  struct eraser *sorted_erasers)
{
	size_t j, k;
	size_t chip_eraser;
	size_t chip_region;
	struct flashchip chip; /* Local copy, potentially altered. */
	/*
	 * In case chip description does not include any functions covering
	 * the entire space (this could happen when the description comes from
	 * the Chrome OS TP driver for instance), use the best effort.
	 *
	 * The structure below saves information about the eraser which covers
	 * the most of the chip space, it is used if no valid functions were
	 * found, which allows programming to succeed.
	 *
	 * The issue be further investigated under b/110474116.
	 */
	struct {
		int max_total;
		int alt_function;
		int alt_region;
	} fallback = {};

	fix_erasers_if_needed(&chip, flash);

	/* Iterate over all available erase functions/block sizes. */
	for (j = k = 0; k < NUM_ERASEFUNCTIONS; k++) {
		size_t new_block_size;
		size_t m, n;

		/* Make sure there is a function in is slot */
		if (!chip.block_erasers[k].block_erase)
			continue;

		/*
		 * Make sure there is a (block size * count) combination which
		 * would erase up to required offset into the chip.
		 *
		 * If this is not the case, but the current total size exceeds
		 * the previously saved fallback total size, make the current
		 * block the best available fallback case.
		 */
		for (n = 0; n < NUM_ERASEREGIONS; n++) {
			const struct eraseblock *eb =
				chip.block_erasers[k].eraseblocks + n;
			size_t total = eb->size * eb->count;

			if (total >= erase_size)
				break;

			if (total > (size_t)fallback.max_total) {
				fallback.max_total = total;
				fallback.alt_region = n;
				fallback.alt_function = k;
			}
		}

		if (n == NUM_ERASEREGIONS) {
			 /*
			  * This function will not erase far enough into the
			  * chip.
			  */
			continue;
		}

		new_block_size = chip.block_erasers[k].eraseblocks[n].size;

		/*
		 * Place this block in the sorted position in the
		 * sorted_erasers array.
		 */
		for (m = 0; m < j; m++) {
			size_t old_block_size;

			chip_eraser = sorted_erasers[m].eraser_index;
			chip_region = sorted_erasers[m].region_index;

			old_block_size = chip.block_erasers
				[chip_eraser].eraseblocks[chip_region].size;

			if (old_block_size < new_block_size)
				continue;

			/* Do not keep duplicates in the sorted array. */
			if (old_block_size == new_block_size) {
				j--;
				break;
			}

			memmove(sorted_erasers + m + 1,
				sorted_erasers + m,
				sizeof(sorted_erasers[0]) * (j - m));
                        break;
                }
		sorted_erasers[m].eraser_index = k;
		sorted_erasers[m].region_index = n;
		j++;
        }

	if (j) {
		msg_pdbg("%s: found %zd valid erasers\n", __func__, j);
		return j;
	}

	if (!fallback.max_total) {
		msg_cerr("No erasers found for this chip (%s:%s)!\n",
			 chip.vendor, chip.name);
		exit(1);
	}

	sorted_erasers[0].eraser_index = fallback.alt_function;
	sorted_erasers[0].region_index = fallback.alt_region;
	msg_pwarn("%s: using fallback eraser: "
		  "region %d, function %d total %#x vs %#zx\n",
		  __func__, fallback.alt_region, fallback.alt_function,
		  fallback.max_total, erase_size);

	return 1;
}

/*
 * When it is determined that the larger block will have to be erased because
 * a large enough number of the blocks of the previous smaller size need to be
 * erased, all blocks of smaller sizes falling into the range of addresses of
 * this larger block will not have to be erased/written individually, so they
 * need to be unmarked for erase/change.
 *
 * This function recursively invokes itself to clean all smaller size blocks
 * which are in the range of the current larger block.
 *
 * @upper_level_map  pointer to the element of the range map array where the
 *                   current block belongs.
 * @block_index      index of the current block in the map of the blocks of
 *                   the current range map element.
 * @i		     index of this range map in the array of range maps,
 *                   guaranteed to be 1 or above, so that there is always a
 *                   smaller block size range map at i - 1.
 */
static void clear_all_nested(struct range_map *upper_level_map,
			     size_t block_index,
			     unsigned i)
{
	struct range_map *this_level_map = upper_level_map - 1;
	size_t range_start;
	size_t range_end;
	size_t j;

	range_start = upper_level_map->block_size * block_index;
	range_end = range_start + upper_level_map->block_size;

	for (j = range_start / this_level_map->block_size;
	     j < range_end / this_level_map->block_size;
	     j++) {
		this_level_map->block_map[j].need_change = 0;
		this_level_map->block_map[j].need_erase = 0;
		if (i > 1)
			clear_all_nested(this_level_map, j, i - 1);
	}
}

/*
 * Once all lowest range size blocks which need to be erased have been
 * identified, we need to see if there are so many of them that they maybe be
 * folded into larger size blocks, so that a single larger erase operation is
 * required instead of many smaller ones.
 *
 * @maps       pointer to the array of range_map structures, sorted by block
 *	       size from lower to higher, only the lower size bock map has
 *	       been filled up.
 * @num_maps   number of elements in the maps array.
 * @chip_size  size of the flash chip, in bytes.
 */
static void fold_range_maps(struct range_map *maps,
			    size_t num_maps,
			    size_t chip_size)
{
	size_t block_index;
	unsigned i;
	struct range_map *map;

	/*
	 * First go from bottom to top, marking higher size blocks which need
	 * to be erased based on the count of lower size blocks marked for
	 * erasing which fall into the range of addresses covered by the
	 * larger size block.
	 *
	 * Starting from the second element of the array, as the first element
	 * is the only one filled up so far.
	 */
	for (i = 1; i < num_maps; i++) {
		int block_mult;

		map = maps + i;

		/* How many lower size blocks fit into this block. */
		block_mult = map->block_size / map[-1].block_size;

		for (block_index = 0;
		     block_index < (chip_size/map->block_size);
		     block_index++) {
			int lower_start;
			int lower_end;
			int lower_index;
			int erase_marked_blocks;
			int change_marked_blocks;

			lower_start = block_index * block_mult;
			lower_end = lower_start + block_mult;
			erase_marked_blocks = 0;
			change_marked_blocks = 0;

			for (lower_index = lower_start;
			     lower_index < lower_end;
			     lower_index++) {

				if (map[-1].block_map[lower_index].need_erase)
					erase_marked_blocks++;

				if (map[-1].block_map[lower_index].need_change)
					change_marked_blocks++;
			}

			/*
			 * Mark larger block for erasing; if any of the
			 * smaller size blocks was marked as 'need_change',
			 * mark the larger size block as well.
			 */
			if (erase_marked_blocks > map[-1].limit) {
				map->block_map[block_index].need_erase = 1;
				map->block_map[block_index].need_change =
					change_marked_blocks ? 1 : 0;
			}
		}
	}

	/*
	 * Now let's go larger to smaller block sizes, to make sure that all
	 * nested blocks of a bigger block marked for erasing are not marked
	 * for erasing any more; erasing the encompassing block will sure
	 * erase all nested blocks of all smaller sizes.
	 */
	for (i = num_maps - 1; i > 0; i--) {
		map = maps + i;

		for (block_index = 0;
		     block_index < (chip_size/map->block_size);
		     block_index++) {
			if (!map->block_map[block_index].need_erase)
				continue;

			clear_all_nested(map, block_index, i);
		}
	}
}

/*
 * A function to fill the processing_units array of the action descriptor with
 * a set of processing units, which describe flash chip blocks which need to
 * be erased/programmed to to accomplish the action requested by user when
 * invoking flashrom.
 *
 * This set of processing units is determined based on comparing old and new
 * flashrom contents.
 *
 * First, blocks which are required to be erased and/or written are identified
 * at the finest block size granularity.
 *
 * Then the distribution of those blocks is analyzed, and if enough of smaller
 * blocks in a single larger block address range need to be erased, the larger
 * block is marked for erasing.
 *
 * This same process is applied again to increasingly larger block sizes until
 * the largest granularity blocks are marked as appropriate.
 *
 * After this the range map array is scanned from larger block sizes to
 * smaller; each time when a larger block marked for erasing is detected, all
 * smaller size blocks in the same address range are unmarked for erasing.
 *
 * In the end only blocks which need to be modified remain marked, and at the
 * finest possible granularity. The list of these blocks is added to the
 * 'processing_units' array of the descriptor and becomes the list of actions
 * to be take to program the flash chip.
 *
 * @descriptor		descriptor structure to fill, allocated by the caller.
 * @sorted_erasers      pointer to an array of eraser descriptors, sorted by
 *			block size.
 * @chip_erasers	pointer to the array of erasers from this flash
 * 			chip's descriptor.
 * @chip_size		size of this chip in bytes
 * @num_sorted_erasers  size of the sorted_erasers array
 * @erased_value	value contained in all bytes of the erased flash
 */
static void fill_action_descriptor(struct action_descriptor *descriptor,
				   struct eraser *sorted_erasers,
				   struct block_eraser* chip_erasers,
				   size_t chip_size,
				   size_t num_sorted_erasers,
				   unsigned erased_value)
{
	const uint8_t *newc;
	const uint8_t *oldc;
	int consecutive_blocks;
	size_t block_size;
	struct b_map *block_map;
	struct range_map range_maps[num_sorted_erasers];
	unsigned i;
	unsigned pu_index;

	/*
	 * This array has enough room to hold helper structures, one for each
	 * available block size.
	 */
	memset(range_maps, 0, sizeof(range_maps));

	/*
	 * Initialize range_maps array: allocate space for block_map arrays on
	 * every entry (block maps are used to keep track of blocks which need
	 * to be erased/written) and calculate the limit where smaller blocks
	 * should be replaced by the next larger size block.
	 */
	for (i = 0; i < num_sorted_erasers; i++) {
		size_t larger_block_size;
		size_t map_size;
		size_t num_blocks;
		unsigned function;
		unsigned region;

		function = sorted_erasers[i].eraser_index;
		region  = sorted_erasers[i].region_index;
		block_size =  chip_erasers[function].eraseblocks[region].size;

		range_maps[i].block_size = block_size;

		/*
		 * Allocate room for the map where blocks which require
		 * writing/erasing will be marked.
		 */
		num_blocks = chip_size/block_size;
		map_size = num_blocks * sizeof(struct b_map);
		range_maps[i].block_map = malloc(map_size);
		if (!range_maps[i].block_map) {
			msg_cerr("%s: Failed to allocate %zd bytes\n",
				 __func__, map_size);
			exit(1);
		}
		memset(range_maps[i].block_map, 0, map_size);

		/*
		 * Limit is calculated for all block sizes but the largest
		 * one, because there is no way to further consolidate the
		 * largest blocks.
		 */
		if (i < (num_sorted_erasers - 1)) {
			function = sorted_erasers[i + 1].eraser_index;
			region  = sorted_erasers[i + 1].region_index;
			larger_block_size = chip_erasers
				[function].eraseblocks[region].size;

			/*
			 * How many of the lower size blocks need to be have
			 * to be erased before it is worth moving to the
			 * larger size.
			 *
			 * The admittedly arbitrary rule of thumb here is if
			 * 70% or more of the lower size blocks need to be
			 * erased, forget the lower size blocks and move to
			 * the higher size one.
			 */
			range_maps[i].limit = ((larger_block_size /
						block_size) * 7) / 10;
		}
	}

	/* Cache pointers to 'before' and 'after' contents. */
	oldc = descriptor->oldcontents;
	newc = descriptor->newcontents;

	/* Now, let's fill up the map for the smallest bock size. */
	block_size = range_maps[0].block_size;
	block_map = range_maps[0].block_map;
	for (i = 0; i < chip_size; i++) {
		int block_index;

		if (oldc[i] == newc[i])
			continue;

		block_index = i/block_size;

		if (oldc[i] != erased_value)
			block_map[block_index].need_erase = 1;

		if (newc[i] != erased_value)
			block_map[block_index].need_change = 1;

		if (block_map[block_index].need_erase &&
		    block_map[block_index].need_change) {
			/* Can move to the next block. */
			i += range_maps[0].block_size;
			i &= ~(range_maps[0].block_size - 1);
			i--; /* adjust for increment in the for loop */
		}
	}

	/* Now let's see what can be folded into larger blocks. */
	fold_range_maps(range_maps, num_sorted_erasers, chip_size);

	/* Finally we can fill the action descriptor. */
	consecutive_blocks = 0;
	pu_index = 0;  /* Number of initialized processing units. */
	for (i = 0; i < num_sorted_erasers; i++) {
		size_t j;
		struct processing_unit *pu;
		size_t map_size = chip_size/range_maps[i].block_size;

		for (j = 0; j < map_size; j++) {

			block_map = range_maps[i].block_map + j;

			if (block_map->need_erase || block_map->need_change) {
				consecutive_blocks++;
				continue;
			}

			if (!consecutive_blocks)
				continue;

			/* Add programming/erasing uint. */
			pu = descriptor->processing_units + pu_index++;

			pu->block_size = range_maps[i].block_size;
			pu->offset = (j - consecutive_blocks) * pu->block_size;
			pu->num_blocks = consecutive_blocks;
			pu->block_eraser_index = sorted_erasers[i].eraser_index;
			pu->block_region_index = sorted_erasers[i].region_index;

			consecutive_blocks = 0;
		}

		free(range_maps[i].block_map);

		if (!consecutive_blocks)
			continue;

		/*
		 * Add last programming/erasing unit for current block
		 * size.
		 */
		pu = descriptor->processing_units + pu_index++;

		pu->block_size = range_maps[i].block_size;
		pu->offset = (j - consecutive_blocks) * pu->block_size;
		pu->num_blocks = consecutive_blocks;
		pu->block_eraser_index = sorted_erasers[i].eraser_index;
		pu->block_region_index = sorted_erasers[i].region_index;
		consecutive_blocks = 0;
	}

	descriptor->processing_units[pu_index].num_blocks = 0;
}

bool is_dry_run(void)
{
	return dry_run;
}

struct action_descriptor *prepare_action_descriptor(struct flashctx *flash,
						    void *oldcontents,
						    void *newcontents)
{
	struct eraser sorted_erasers[NUM_ERASEFUNCTIONS];
	size_t i;
	size_t num_erasers;
	int max_units;
	size_t block_size = 0;
	struct action_descriptor *descriptor;
	size_t chip_size = flash->chip->total_size * 1024;

	/*
	 * Find the maximum size of the area which might have to be erased,
	 * this is needed to ensure that the picked erase function can go all
	 * the way to the requred offset, as some of the erase functions
	 * operate only on part of the chip starting at offset zero.
	 *
	 * Not an efficient way to do it, but this is acceptable on the host.
	 *
	 * Look for the largest offset where the difference is, this is the
	 * highest offset which might need to be erased.
	 */
	for (i = 0; i < chip_size; i++)
		if (((uint8_t *)newcontents)[i] !=
		    ((uint8_t *)oldcontents)[i])
			block_size = i + 1;

	num_erasers = fill_sorted_erasers(flash, block_size, sorted_erasers);

	/*
	 * Let's allocate enough memory for the worst case action descriptor
	 * size, when we need to program half the chip using the smallest block
	 * size.
	 */
	block_size = flash->chip->block_erasers
		[sorted_erasers[0].eraser_index].eraseblocks
		[sorted_erasers[0].region_index].size;
	max_units = max( chip_size / (2 * block_size), 1 ) + 1;
	descriptor = malloc(sizeof(struct action_descriptor) +
			    sizeof(struct processing_unit) * max_units);
	if (!descriptor) {
		msg_cerr("Failed to allocate room for %d processing units!\n",
			 max_units);
		exit(1);
	}

	descriptor->newcontents = newcontents;
	descriptor->oldcontents = oldcontents;

	fill_action_descriptor(descriptor, sorted_erasers,
			       flash->chip->block_erasers, chip_size,
			       num_erasers, ERASED_VALUE(flash));

	dump_descriptor(descriptor);

	return descriptor;
}
