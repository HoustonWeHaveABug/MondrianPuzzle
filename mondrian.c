#include <stdio.h>
#include <stdlib.h>
#include "mp_utils.h"

#define PAINT_WIDTH_MIN 3
#define SIZE_T_MAX (size_t)-1
#define OPTIONS_LO_MIN 2

typedef struct {
	int height;
	int width;
	int area;
	int delta;
	int slots_n;
	int y_priority;
	int x_priority;
	int area_left;
}
tile_t;

typedef struct option_s option_t;

struct option_s {
	int height;
	int width;
	int area;
	int delta;
	int slots_n;
	int y_priority;
	int x_priority;
	int rotate_flag;
	int slot_height;
	int y_slot_lo;
	int y_slot_hi;
	int slot_width;
	int x_slot_lo;
	int x_slot_hi;
	option_t *y_last;
	option_t *y_next;
	option_t *d_last;
	option_t *d_next;
	option_t *out_last;
	option_t *out_next;
};

typedef struct bar_s bar_t;

struct bar_s {
	int y_slot;
	int height;
	int y_space;
	int x_space;
	bar_t *last;
	bar_t *next;
};

typedef struct choice_s choice_t;

struct choice_s {
	int y_slot;
	int x_slot;
	choice_t *last;
	choice_t *next;
};

static int search_defect(void);
static void add_tiles(int);
static void add_tile(int, int, int);
static int check_tile1(int, int, int);
static int check_tile2(int, int, int, int, int);
static int check_big_tile1(int, int, int, int);
static int check_big_tile2(int, int, int);
static int is_valid_area(int);
static int check_defect(int, int, int);
static int is_valid_tile(int);
static int add_mondrian_tile(int, int, int);
static int is_mondrian(void);
static int is_dominated(const option_t *, const option_t *);
static int search_y_slot(int, bar_t *, option_t *);
static int check_slot_height(bar_t *, int);
static int choose_y_slot(int, bar_t *, option_t *, int, int);
static void rollback_y_slot(bar_t *, bar_t *, bar_t *, int, int);
static int search_x_slot(choice_t *, choice_t *);
static int is_valid_choice(int, int);
static int is_valid_option(const option_t *, int, int);
static int no_edges_overlap(int, int, int, int);
static void set_tile(tile_t *, int, int);
static int compare_tiles(const void *, const void *);
static void copy_tile(option_t *, const tile_t *);
static void set_option(option_t *);
static int compare_options(const void *, const void *);
static int compare_priorities(const option_t *, const option_t *);
static void restore_option_y(option_t *);
static void link_options_y(option_t *, option_t *);
static void insert_option_d(option_t *, option_t *, option_t *);
static void link_options_d(option_t *, option_t *);
static void link_options_out(option_t *, option_t *);
static void print_option(const option_t *);
static void set_bar(bar_t *, int, int, int);
static void insert_bar(bar_t *, bar_t *, bar_t *);
static void link_bars(bar_t *, bar_t *);
static void set_choice(choice_t *, int, int);
static int compare_choices(const choice_t *, const choice_t *);
static void insert_choice(choice_t *, choice_t *, choice_t *);
static void link_choices(choice_t *, choice_t *);

static int paint_height, paint_width, rotate_flag, defect_a, defect_b, options_lo, verbose_flag, paint_area, *counts, tiles_max, mondrian_tiles_max, success_tiles_n, defect_cur, mondrian_tiles_cur, tiles_n, mondrian_tiles_n, tile_stop, tiles_area, mondrian_defect, option_sym_flag, y_slot_sym_max, y_slot_sym_max_rotated, bars_n, options_in_n, x_slot_sym_max;
static mp_t y_cost, x_cost;
static tile_t *tiles, *success_tiles, **mondrian_tiles;
static option_t *options, **options_in, *options_header, *dominances_header, *option_sym;
static bar_t *bars, *bars_header;
static choice_t *choices, *choices_header;

int main(void) {
	int choices_n, r;
	if (scanf("%d%d%d%d%d%d%d", &paint_height, &paint_width, &rotate_flag, &defect_a, &defect_b, &options_lo, &verbose_flag) != 7 || paint_height < 1 || paint_width < PAINT_WIDTH_MIN || paint_height > paint_width || (unsigned)paint_height > SIZE_T_MAX/(unsigned)paint_width || defect_a < 0 || defect_b < 0 || options_lo < OPTIONS_LO_MIN) {
		fprintf(stderr, "Expected parameters: paint_height (>= 1), paint_width (>= %d and >= paint_height), rotate_flag, defect_a (>= 0), defect_b (>= 0), options_lo (>= %d), verbose_flag.\n", PAINT_WIDTH_MIN, OPTIONS_LO_MIN);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	paint_area = paint_height*paint_width;
	if (SIZE_T_MAX/(unsigned)paint_area < sizeof(int)) {
		fputs("Will not be able to allocate memory for counts\n", stderr);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	counts = malloc(sizeof(int)*(size_t)paint_area);
	if (!counts) {
		fputs("Could not allocate memory for counts\n", stderr);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	tiles = malloc(sizeof(tile_t)*(size_t)(1+options_lo));
	if (!tiles) {
		fputs("Could not allocate memory for tiles\n", stderr);
		fflush(stderr);
		free(counts);
		return EXIT_FAILURE;
	}
	tiles_max = 1;
	success_tiles = tiles+tiles_max;
	mondrian_tiles = malloc(sizeof(tile_t *)*(size_t)options_lo);
	if (!mondrian_tiles) {
		fputs("Could not allocate memory for mondrian_tiles\n", stderr);
		fflush(stderr);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	mondrian_tiles_max = options_lo;
	options = malloc(sizeof(option_t)*(size_t)(options_lo*2+2));
	if (!options) {
		fputs("Could not allocate memory for options\n", stderr);
		fflush(stderr);
		free(mondrian_tiles);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	bars = malloc(sizeof(bar_t)*(size_t)(paint_height+1));
	if (!bars) {
		fputs("Could not allocate memory for bars\n", stderr);
		fflush(stderr);
		free(options);
		free(mondrian_tiles);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	set_bar(bars, 0, paint_height, paint_width);
	bars_header = bars+paint_height;
	set_bar(bars_header, paint_height, 0, 0);
	insert_bar(bars, bars_header, bars_header);
	options_in = malloc(sizeof(option_t *)*(size_t)options_lo);
	if (!options_in) {
		fputs("Could not allocate memory for options_in\n", stderr);
		fflush(stderr);
		free(bars);
		free(options);
		free(mondrian_tiles);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	choices_n = options_lo*2;
	choices = malloc(sizeof(choice_t)*(size_t)(choices_n+1));
	if (!choices) {
		fputs("Could not allocate memory for choices\n", stderr);
		fflush(stderr);
		free(options_in);
		free(bars);
		free(options);
		free(mondrian_tiles);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	choices_header = choices+choices_n;
	set_choice(choices, 0, 0);
	success_tiles_n = 0;
	defect_cur = defect_a;
	if (defect_a <= defect_b) {
		do {
			mondrian_tiles_cur = options_lo;
			r = search_defect();
			if (!r) {
				++defect_cur;
			}
		}
		while (defect_cur <= defect_b && !r);
	}
	else {
		mondrian_tiles_cur = options_lo;
		do {
			r = search_defect();
		}
		while (defect_cur >= defect_b && r == 1);
	}
	free(choices);
	free(options_in);
	free(bars);
	free(options);
	free(mondrian_tiles);
	free(tiles);
	free(counts);
	if (r < 0) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

static int search_defect(void) {
	int area, width, height;
	for (area = paint_area; area--; ) {
		counts[area] = 0;
	}
	for (width = 1; width < paint_height; ++width) {
		area = 0;
		for (height = 1; height < width; ++height) {
			area += width;
			if (is_valid_area(area)) {
				if (rotate_flag) {
					if ((check_tile1(width, height, area) || check_tile1(height, width, area))) {
						++counts[area-1];
					}
				}
				else {
					if (check_tile1(width, height, area)) {
						++counts[area-1];
					}
					if (check_tile1(height, width, area)) {
						++counts[area-1];
					}
				}
			}
		}
		area += width;
		if (check_tile1(width, width, area) && is_valid_area(area)) {
			++counts[area-1];
		}
	}
	area = 0;
	if (paint_height < paint_width) {
		for (height = 1; height < paint_height; ++height) {
			area += width;
			if (is_valid_area(area)) {
				if (rotate_flag) {
					if ((check_tile1(height, width, area) || check_big_tile1(paint_width, height, width, area))) {
						++counts[area-1];
					}
				}
				else {
					if (check_tile1(height, width, area)) {
						++counts[area-1];
					}
					if (check_big_tile1(paint_width, height, width, area)) {
						++counts[area-1];
					}
				}
			}
		}
		area += width;
		if (check_big_tile1(paint_width, width, width, area) && is_valid_area(area)) {
			++counts[area-1];
		}
		for (++width; width < paint_width; ++width) {
			area = 0;
			for (height = 1; height < paint_height; ++height) {
				area += width;
				if (check_tile1(height, width, area) && is_valid_area(area)) {
					++counts[area-1];
				}
			}
			area += width;
			if (check_big_tile1(paint_width, width, height, area) && is_valid_area(area)) {
				++counts[area-1];
			}
		}
		area = 0;
		for (height = 1; height < paint_height; ++height) {
			area += width;
			if (check_big_tile1(paint_height, height, width, area) && is_valid_area(area)) {
				++counts[area-1];
			}
		}
	}
	else {
		for (height = 1; height < paint_height; ++height) {
			area += width;
			if (check_big_tile1(paint_width, height, width, area) && is_valid_area(area)) {
				++counts[area-1];
				if (!rotate_flag) {
					++counts[area-1];
				}
			}
		}
	}
	for (area = paint_area; area--; ) {
		if (counts[area]) {
			int offset;
			for (offset = area < defect_cur ? area:defect_cur; offset >= 0; --offset) {
				int counts_sum = 0, counts_area = 0, count_lo = area-offset, count_hi = defect_cur+count_lo < paint_area ? defect_cur+count_lo:paint_area-1, count_idx;
				for (count_idx = count_lo; count_idx <= count_hi; ++count_idx) {
					counts_sum += counts[count_idx];
					counts_area += (count_idx+1)*counts[count_idx];
					if (counts_sum >= options_lo && counts_area >= paint_area) {
						break;
					}
				}
				if (count_idx <= count_hi) {
					break;
				}
			}
			if (offset < 0) {
				counts[area] = 0;
			}
		}
	}
	add_tiles(0);
	printf("Current %d Tiles %d\n", defect_cur, tiles_n);
	fflush(stdout);
	if (tiles_n >= options_lo) {
		int tile_idx;
		if (tiles_n > tiles_max) {
			tile_t *tiles_tmp = realloc(tiles, sizeof(tile_t)*(size_t)(tiles_n+mondrian_tiles_max));
			if (!tiles_tmp) {
				fputs("Could not reallocate memory for tiles\n", stderr);
				fflush(stderr);
				return -1;
			}
			tiles = tiles_tmp;
			tiles_max = tiles_n;
			success_tiles = tiles+tiles_max;
		}
		add_tiles(1);
		qsort(tiles, (size_t)tiles_n, sizeof(tile_t), compare_tiles);
		mondrian_tiles_n = 0;
		area = 0;
		for (tile_idx = tiles_n; tile_idx && area < paint_area; --tile_idx) {
			++mondrian_tiles_n;
			area += tiles[tile_idx-1].area;
		}
		if (mondrian_tiles_n > mondrian_tiles_max) {
			int choices_n;
			tile_t *tiles_tmp = realloc(tiles, sizeof(tile_t)*(size_t)(tiles_max+mondrian_tiles_n)), **mondrian_tiles_tmp;
			option_t *options_tmp, **options_in_tmp;
			choice_t *choices_tmp;
			if (!tiles_tmp) {
				fputs("Could not reallocate memory for tiles\n", stderr);
				fflush(stderr);
				return -1;
			}
			tiles = tiles_tmp;
			success_tiles = tiles+tiles_max;
			mondrian_tiles_tmp = realloc(mondrian_tiles, sizeof(tile_t *)*(size_t)mondrian_tiles_n);
			if (!mondrian_tiles_tmp) {
				fputs("Could not reallocate memory for mondrian_tiles\n", stderr);
				fflush(stderr);
				return -1;
			}
			mondrian_tiles = mondrian_tiles_tmp;
			mondrian_tiles_max = mondrian_tiles_n;
			options_tmp = realloc(options, sizeof(option_t)*(size_t)(mondrian_tiles_n*2+2));
			if (!options_tmp) {
				fputs("Could not reallocate memory for options\n", stderr);
				fflush(stderr);
				return -1;
			}
			options = options_tmp;
			options_in_tmp = realloc(options_in, sizeof(option_t *)*(size_t)mondrian_tiles_n);
			if (!options_in_tmp) {
				fputs("Could not reallocate memory for options_in\n", stderr);
				fflush(stderr);
				return -1;
			}
			options_in = options_in_tmp;
			choices_n = mondrian_tiles_n*2;
			choices_tmp = realloc(choices, sizeof(choice_t)*(size_t)(choices_n+1));
			if (!choices_tmp) {
				fputs("Could not reallocate memory for choices\n", stderr);
				fflush(stderr);
				return -1;
			}
			choices = choices_tmp;
			choices_header = choices+choices_n;
		}
		mondrian_tiles_n = 0;
		tile_stop = 0;
		tiles_area = 0;
		return add_mondrian_tile(0, 1, paint_height == paint_width && !rotate_flag);
	}
	return 0;
}

static void add_tiles(int set_flag) {
	int width, area, height;
	tiles_n = 0;
	for (width = 1; width < paint_height; ++width) {
		area = 0;
		for (height = 1; height < width; ++height) {
			area += width;
			if (counts[area-1] && is_valid_area(area)) {
				if (rotate_flag) {
					if ((check_tile1(width, height, area) || check_tile1(height, width, area))) {
						add_tile(set_flag, height, width);
					}
				}
				else {
					if (check_tile1(width, height, area)) {
						add_tile(set_flag, width, height);
					}
					if (check_tile1(height, width, area)) {
						add_tile(set_flag, height, width);
					}
				}
			}
		}
		area += width;
		if (counts[area-1] && check_tile1(width, width, area) && is_valid_area(area)) {
			add_tile(set_flag, width, width);
		}
	}
	area = 0;
	if (paint_height < paint_width) {
		for (height = 1; height < paint_height; ++height) {
			area += width;
			if (counts[area-1] && is_valid_area(area)) {
				if (rotate_flag) {
					if ((check_tile1(height, width, area) || check_big_tile1(paint_width, height, width, area))) {
						add_tile(set_flag, height, width);
					}
				}
				else {
					if (check_tile1(height, width, area)) {
						add_tile(set_flag, height, width);
					}
					if (check_big_tile1(paint_width, height, width, area)) {
						add_tile(set_flag, width, height);
					}
				}
			}
		}
		area += width;
		if (counts[area-1] && check_big_tile1(paint_width, width, width, area) && is_valid_area(area)) {
			add_tile(set_flag, width, width);
		}
		for (++width; width < paint_width; ++width) {
			area = 0;
			for (height = 1; height < paint_height; ++height) {
				area += width;
				if (counts[area-1] && check_tile1(height, width, area) && is_valid_area(area)) {
					add_tile(set_flag, height, width);
				}
			}
			area += width;
			if (counts[area-1] && check_big_tile1(paint_width, width, height, area) && is_valid_area(area)) {
				add_tile(set_flag, height, width);
			}
		}
		area = 0;
		for (height = 1; height < paint_height; ++height) {
			area += width;
			if (counts[area-1] && check_big_tile1(paint_height, height, width, area) && is_valid_area(area)) {
				add_tile(set_flag, height, width);
			}
		}
	}
	else {
		for (height = 1; height < paint_height; ++height) {
			area += width;
			if (counts[area-1] && check_big_tile1(paint_width, height, width, area) && is_valid_area(area)) {
				add_tile(set_flag, height, width);
				if (!rotate_flag) {
					add_tile(set_flag, width, height);
				}
			}
		}
	}
}

static void add_tile(int set_flag, int height, int width) {
	if (set_flag) {
		set_tile(tiles+tiles_n, height, width);
	}
	++tiles_n;
}

static int check_tile1(int height, int width, int area) {
	return check_tile2(area, paint_height-height, paint_width, height, paint_width-width) || check_tile2(area, paint_height-height, width, paint_height, paint_width-width);
}

static int check_tile2(int area, int height_delta, int width, int height, int width_delta) {
	return area-height_delta*width <= defect_cur && area-height*width_delta <= defect_cur;
}

static int check_big_tile1(int paint_len, int len, int big_len, int area) {
	int delta = paint_len-len;
	if (delta != len) {
		return area-delta*big_len <= defect_cur;
	}
	if (delta < big_len) {
		return check_big_tile2(big_len, area, delta);
	}
	return check_big_tile2(delta, area, big_len);
}

static int check_big_tile2(int big_len, int area, int delta) {
	if (big_len%2 == 0) {
		return area-delta*(big_len/2-1) <= defect_cur;
	}
	return area-delta*(big_len/2) <= defect_cur;
}

static int is_valid_area(int area) {
	int options_n = paint_area/area, area_mod, others_n;
	if (options_n < options_lo) {
		return check_defect(area, area+(area-paint_area)/(options_lo-1), -1);
	}
	area_mod = paint_area%area;
	others_n = options_n-1;
	if (check_defect(area, area_mod%others_n ? area_mod/others_n+1:area_mod/others_n, 1)) {
		return 1;
	}
	return check_defect(area, area+(area-paint_area)/options_n, -1);
}

static int check_defect(int area, int defect, int sign) {
	while (defect <= defect_cur && !is_valid_tile(area+defect*sign)) {
		++defect;
	}
	return defect <= defect_cur;
}

static int is_valid_tile(int area) {
	int height;
	for (height = 1; height <= paint_height; ++height) {
		int width = area/height;
		if (height > width) {
			break;
		}
		if (width <= paint_width && height*width == area) {
			return 1;
		}
	}
	return 0;
}

static int add_mondrian_tile(int tiles_start, int same_flag, int sym_flag) {
	int r = 0, tile_idx1;
	for (tile_idx1 = tiles_start; tile_idx1 < tiles_n && !r; ++tile_idx1) {
		if (same_flag) {
			if (mondrian_tiles_n < success_tiles_n) {
				int compare_flag = compare_tiles(tiles+tile_idx1, success_tiles+mondrian_tiles_n);
				if (compare_flag < 0) {
					continue;
				}
				same_flag = !compare_flag;
			}
			else {
				same_flag = 0;
			}
		}
		if (mondrian_tiles_n) {
			if (sym_flag) {
				if (tiles[tile_idx1].delta >= 0) {
					if (mondrian_tiles[mondrian_tiles_n-1]->delta > 0) {
						sym_flag = 0;
					}
				}
				else {
					if (mondrian_tiles[mondrian_tiles_n-1]->delta <= 0) {
						continue;
					}
					if (tiles[tile_idx1].height != mondrian_tiles[mondrian_tiles_n-1]->width || tiles[tile_idx1].width != mondrian_tiles[mondrian_tiles_n-1]->height) {
						sym_flag = 0;
					}
				}
			}
			if (mondrian_tiles_n < mondrian_tiles_cur) {
				if (tile_idx1+mondrian_tiles_cur-mondrian_tiles_n > tile_stop) {
					break;
				}
			}
			else {
				if (tile_idx1 >= tile_stop) {
					break;
				}
			}
			if (tiles_area+tiles[tile_idx1].area_left < paint_area) {
				break;
			}
		}
		else {
			int area_left, tile_idx2;
			if (sym_flag && tiles[tile_idx1].delta < 0) {
				continue;
			}
			if (tile_stop < tiles_n && tiles[tile_idx1].area-tiles[tile_stop].area <= defect_cur) {
				for (++tile_stop; tile_stop < tiles_n && tiles[tile_idx1].area-tiles[tile_stop].area <= defect_cur; ++tile_stop);
			}
			else {
				for (; tile_stop > tile_idx1 && tiles[tile_idx1].area-tiles[tile_stop-1].area > defect_cur; --tile_stop);
			}
			area_left = 0;
			for (tile_idx2 = tile_stop-1; tile_idx2 >= tile_idx1; --tile_idx2) {
				if (area_left <= paint_area) {
					area_left += tiles[tile_idx2].area;
				}
				tiles[tile_idx2].area_left = area_left;
			}
		}
		if (mondrian_tiles_n < mondrian_tiles_cur && tiles_area+tiles[tile_stop-mondrian_tiles_cur+mondrian_tiles_n].area_left > paint_area) {
			continue;
		}
		mondrian_tiles[mondrian_tiles_n++] = tiles+tile_idx1;
		tiles_area += tiles[tile_idx1].area;
		if (mondrian_tiles_n > mondrian_tiles_cur && tiles_area <= paint_area) {
			mondrian_tiles_cur = mondrian_tiles_n;
			printf("Mondrian tiles %d\n", mondrian_tiles_cur);
			fflush(stdout);
		}
		if (tiles_area < paint_area) {
			r = add_mondrian_tile(tile_idx1+1, same_flag, sym_flag);
		}
		else if (tiles_area == paint_area) {
			mondrian_defect = mondrian_tiles[0]->area-tiles[tile_idx1].area;
			if (defect_a <= defect_b) {
				if (mondrian_defect == defect_cur) {
					r = is_mondrian();
				}
			}
			else {
				if (mondrian_defect >= defect_b) {
					r = is_mondrian();
					if (r == 1) {
						int tile_idx2;
						if (mondrian_tiles_n > options_lo) {
							options_lo = mondrian_tiles_n;
						}
						for (tile_idx2 = mondrian_tiles_n; tile_idx2--; ) {
							success_tiles[tile_idx2] = *(mondrian_tiles[tile_idx2]);
						}
						if (mondrian_tiles_n > success_tiles_n) {
							success_tiles_n = mondrian_tiles_n;
						}
						defect_cur = mondrian_defect-1;
					}
				}
			}
		}
		tiles_area -= tiles[tile_idx1].area;
		--mondrian_tiles_n;
	}
	return r;
}

static int is_mondrian(void) {
	int option_idx, dominances_n, r;
	option_t *dominances, *option, *option_sym_d_last, *option_sym_d_next;
	if (verbose_flag) {
		printf("is_mondrian options %d defect %d\n", mondrian_tiles_n, mondrian_defect);
		fflush(stdout);
	}
	for (option_idx = mondrian_tiles_n; option_idx--; ) {
		copy_tile(options+option_idx, mondrian_tiles[option_idx]);
	}
	qsort(options, (size_t)mondrian_tiles_n, sizeof(option_t), compare_options);
	options_header = options+mondrian_tiles_n;
	for (option_idx = mondrian_tiles_n; option_idx--; ) {
		link_options_y(options+option_idx, options+option_idx+1);
	}
	link_options_y(options_header, options);
	dominances = options_header+1;
	dominances_n = 0;
	for (option = options; option != options_header; option = option->y_next) {
		for (option_idx = 0; option_idx < dominances_n && !is_dominated(option, dominances[option_idx].d_last); ++option_idx);
		if (option_idx == dominances_n) {
			link_options_d(dominances+dominances_n, dominances+dominances_n);
			++dominances_n;
		}
		insert_option_d(option, dominances[option_idx].d_last, dominances+option_idx);
	}
	dominances_header = dominances+dominances_n;
	for (option_idx = dominances_n; option_idx--; ) {
		link_options_y(dominances+option_idx, dominances+option_idx+1);
	}
	link_options_y(dominances_header, dominances);
	option_sym = options;
	for (option = options->y_next; option != options_header; option = option->y_next) {
		if (compare_priorities(option, option_sym) < 0) {
			option_sym = option;
		}
	}
	option_sym_flag = 1;
	y_slot_sym_max = (paint_height-option_sym->height)/2;
	y_slot_sym_max_rotated = (paint_height-option_sym->width)/2;
	option_sym_d_last = option_sym->d_last;
	option_sym_d_next = option_sym->d_next;
	link_options_d(option_sym_d_last, option_sym_d_next);
	if (option_sym_d_last == option_sym_d_next) {
		link_options_y(option_sym_d_last->y_last, option_sym_d_last->y_next);
	}
	for (option = options; option != options_header; option = option->y_next) {
		option->rotate_flag = rotate_flag && option->delta && (paint_height < paint_width || option != option_sym);
	}
	if (verbose_flag && !mp_new(&y_cost)) {
		return -1;
	}
	bars_n = 1;
	r = search_y_slot(mondrian_tiles_n, bars, options);
	if (verbose_flag) {
		fputs("search_y_slot ", stdout);
		mp_print("cost", &y_cost);
		fflush(stdout);
		mp_free(&y_cost);
	}
	return r;
}

static int is_dominated(const option_t *option_a, const option_t *option_b) {
	return option_a->height <= option_b->height && option_a->width <= option_b->width;
}

static int search_y_slot(int bars_hi, bar_t *bar_start_bak, option_t *options_start) {
	int r, option_idx;
	bar_t *bar_start;
	if (verbose_flag && !mp_inc(&y_cost)) {
		return -1;
	}
	for (bar_start = bar_start_bak; bar_start != bars_header && !bar_start->x_space; bar_start = bar_start->next) {
		++bars_hi;
	}
	if (bars_hi < bars_n) {
		return 0;
	}
	if (bar_start != bars_header) {
		int slot_height, slot_width, x_space, x_delta, x_min, y_min;
		option_t *option, *last_chance, *dominance;
		bar_t *bar_cur, *bar_cur_next, *bar_start_next, *bar;
		if ((option_sym_flag && bar_start->y_slot > y_slot_sym_max) || bar_start->y_space*bar_start->x_space < mondrian_tiles[mondrian_tiles_n-1]->area) {
			return 0;
		}
		if (bar_start != bar_start_bak) {
			options_start = options_header->y_next;
		}
		slot_height = bar_start->y_space;
		slot_width = bar_start->x_space;
		if (bars_hi == bars_n) {
			for (option = options_start; option != options_header; option = option->y_next) {
				if (option->width == slot_width && check_slot_height(bar_start, option->height)) {
					return choose_y_slot(bars_hi, bar_start, option, option->height, option->width);
				}
				if (option->rotate_flag && (option != option_sym || bar_start->y_slot <= y_slot_sym_max_rotated) && option->height == slot_width && check_slot_height(bar_start, option->width)) {
					return choose_y_slot(bars_hi, bar_start, option, option->width, option->height);
				}
			}
			return 0;
		}
		x_space = 0;
		x_delta = 0;
		x_min = paint_width;
		y_min = paint_height;
		for (option = options_start; option != options_header; option = option->y_next) {
			if (option->height <= slot_height && option->width <= slot_width) {
				x_space += option->width;
				if (option->rotate_flag && option->width <= slot_height && option->height <= slot_width) {
					x_delta += option->delta;
					if (option->height < x_min) {
						x_min = option->height;
					}
				}
				else {
					if (option->width < x_min) {
						x_min = option->width;
					}
				}
				if (option->height < y_min) {
					y_min = option->height;
				}
			}
			else if (option->rotate_flag && (option != option_sym || bar_start->y_slot <= y_slot_sym_max_rotated) && option->width <= slot_height && option->height <= slot_width) {
				x_space += option->height;
				if (option->height < x_min) {
					x_min = option->height;
				}
				if (option->width < y_min) {
					y_min = option->width;
				}
			}
		}
		if (x_space < slot_width || (x_space > slot_width+x_delta && x_space < slot_width+x_min)) {
			return 0;
		}
		for (bar_cur = bar_start; bar_cur != bars_header && bar_cur->height <= y_min; bar_cur = bar_cur->next) {
			y_min -= bar_cur->height;
			bar_cur->x_space -= slot_width;
		}
		bar_cur_next = bar_cur->next;
		if (y_min) {
			set_bar(bars+bars_n, bar_cur->y_slot+y_min, bar_cur->height-y_min, bar_cur->x_space);
			bar_cur->x_space -= slot_width;
			insert_bar(bars+bars_n, bar_cur, bar_cur_next);
		}
		bar_start_next = bar_start->next;
		last_chance = options_header;
		if (option_sym_flag) {
			if (option_sym->rotate_flag) {
				for (bar = bar_start_next; bar != bars_header && bar->x_space < option_sym->height; bar = bar->next);
				if (bar == bars_header || bar->y_slot > y_slot_sym_max_rotated || bar->y_space < option_sym->width) {
					for (; bar != bars_header && bar->x_space < option_sym->width; bar = bar->next);
					if (bar == bars_header || bar->y_slot > y_slot_sym_max || bar->y_space < option_sym->height) {
						if (option_sym < options_start || ((bar_start->y_slot > y_slot_sym_max_rotated || option_sym->width > slot_height || option_sym->height > slot_width) && (option_sym->height > slot_height || option_sym->width > slot_width))) {
							rollback_y_slot(bar_start, bar_cur, bar_cur_next, y_min, slot_width);
							return 0;
						}
						last_chance = option_sym;
					}
				}
			}
			else {
				for (bar = bar_start_next; bar != bars_header && bar->x_space < option_sym->width; bar = bar->next);
				if (bar == bars_header || bar->y_slot > y_slot_sym_max || bar->y_space < option_sym->height) {
					if (option_sym < options_start || option_sym->height > slot_height || option_sym->width > slot_width) {
						rollback_y_slot(bar_start, bar_cur, bar_cur_next, y_min, slot_width);
						return 0;
					}
					last_chance = option_sym;
				}
			}
		}
		for (dominance = dominances_header->y_next; dominance != dominances_header; dominance = dominance->y_next) {
			option = dominance->d_next;
			if (option->rotate_flag) {
				for (bar = bar_start_next; bar != bars_header && bar->x_space < option->height; bar = bar->next);
				if (bar == bars_header || bar->y_space < option->width) {
					for (; bar != bars_header && bar->x_space < option->width; bar = bar->next);
					if (bar == bars_header || bar->y_space < option->height) {
						if (option < options_start || ((option->width > slot_height || option->height > slot_width) && (option->height > slot_height || option->width > slot_width))) {
							rollback_y_slot(bar_start, bar_cur, bar_cur_next, y_min, slot_width);
							return 0;
						}
						if (option < last_chance) {
							last_chance = option;
						}
					}
				}
			}
			else {
				for (bar = bar_start_next; bar != bars_header && bar->x_space < option->width; bar = bar->next);
				if (bar == bars_header || bar->y_space < option->height) {
					if (option < options_start || option->height > slot_height || option->width > slot_width) {
						rollback_y_slot(bar_start, bar_cur, bar_cur_next, y_min, slot_width);
						return 0;
					}
					if (option < last_chance) {
						last_chance = option;
					}
				}
			}
		}
		rollback_y_slot(bar_start, bar_cur, bar_cur_next, y_min, slot_width);
		for (bar = bar_start->next; bar != bars_header; bar = bar->next) {
			if (option_sym_flag && ((bar->y_slot <= y_slot_sym_max && bar->y_space >= option_sym->height && bar->x_space >= option_sym->width) || (option_sym->rotate_flag && bar->y_slot <= y_slot_sym_max_rotated && bar->y_space >= option_sym->width && bar->x_space >= option_sym->height))) {
				continue;
			}
			for (dominance = dominances_header->y_next; dominance != dominances_header; dominance = dominance->y_next) {
				option = dominance->d_last;
				if ((option->height <= bar->y_space && option->width <= bar->x_space) || (option->rotate_flag && option->width <= bar->y_space && option->height <= bar->x_space)) {
					break;
				}
			}
			if (dominance == dominances_header) {
				return 0;
			}
		}
		r = 0;
		for (option = options_start; option != options_header; option = option->y_next) {
			if (option->height <= slot_height && option->width <= slot_width) {
				r = choose_y_slot(bars_hi, bar_start, option, option->height, option->width);
				if (r) {
					break;
				}
				x_space -= option->width;
			}
			if (option->rotate_flag && (option != option_sym || bar_start->y_slot <= y_slot_sym_max_rotated) && option->width <= slot_height && option->height <= slot_width) {
				if (option->height <= slot_height && option->width <= slot_width) {
					if (x_space+option->height >= slot_width) {
						r = choose_y_slot(bars_hi, bar_start, option, option->width, option->height);
						if (r) {
							break;
						}
					}
				}
				else {
					r = choose_y_slot(bars_hi, bar_start, option, option->width, option->height);
					if (r) {
						break;
					}
					x_space -= option->height;
				}
			}
			if (x_space < slot_width || option == last_chance) {
				break;
			}
		}
		return r;
	}
	for (option_idx = mondrian_tiles_n; option_idx--; ) {
		set_option(options+option_idx);
		link_options_out(options+option_idx, options+option_idx+1);
	}
	link_options_out(options_header, options);
	options_in_n = 0;
	x_slot_sym_max = (paint_width-option_sym->slot_width)/2;
	if (verbose_flag && !mp_new(&x_cost)) {
		return -1;
	}
	insert_choice(choices, choices_header, choices_header);
	r = search_x_slot(choices, choices);
	link_choices(choices_header, choices_header);
	if (verbose_flag) {
		fputs("search_x_slot ", stdout);
		mp_print("cost", &x_cost);
		fflush(stdout);
		mp_free(&x_cost);
	}
	return r;
}

static int check_slot_height(bar_t *bar_start, int slot_height) {
	bar_t *bar;
	for (bar = bar_start; bar != bars_header && bar->height <= slot_height; bar = bar->next) {
		slot_height -= bar->height;
	}
	return !slot_height;
}

static int choose_y_slot(int bars_hi, bar_t *bar_start, option_t *option, int slot_height, int slot_width) {
	int r;
	option_t *option_d_last = option->d_last, *option_d_next = option->d_next;
	bar_t *bar_cur, *bar;
	option->slot_height = slot_height;
	option->y_slot_lo = bar_start->y_slot;
	link_options_y(option->y_last, option->y_next);
	if (option != option_sym) {
		link_options_d(option_d_last, option_d_next);
		if (option_d_last == option_d_next) {
			link_options_y(option_d_last->y_last, option_d_last->y_next);
		}
	}
	else {
		option_sym_flag = 0;
	}
	for (bar_cur = bar_start; bar_cur != bars_header && bar_cur->height <= slot_height; bar_cur = bar_cur->next) {
		slot_height -= bar_cur->height;
		bar_cur->x_space -= slot_width;
	}
	bar = bar_cur->next;
	if (slot_height) {
		set_bar(bars+bars_n, bar_cur->y_slot+slot_height, bar_cur->height-slot_height, bar_cur->x_space);
		bar_cur->height = slot_height;
		bar_cur->x_space -= slot_width;
		insert_bar(bars+bars_n, bar_cur, bar);
		++bars_n;
	}
	r = search_y_slot(bars_hi-1, bar_start, option->y_next);
	if (slot_height) {
		--bars_n;
		link_bars(bar_cur, bar);
		bar_cur->x_space += slot_width;
		bar_cur->height += bars[bars_n].height;
	}
	for (bar = bar_start; bar != bar_cur; bar = bar->next) {
		bar->x_space += slot_width;
	}
	if (option != option_sym) {
		if (option_d_last == option_d_next) {
			restore_option_y(option_d_last);
		}
		option->d_next->d_last = option;
		option->d_last->d_next = option;
	}
	else {
		option_sym_flag = 1;
	}
	restore_option_y(option);
	return r;
}

static void rollback_y_slot(bar_t *bar_start, bar_t *bar_cur, bar_t *bar_cur_next, int y_min, int slot_width) {
	bar_t *bar;
	if (y_min) {
		link_bars(bar_cur, bar_cur_next);
		bar_cur->x_space += slot_width;
	}
	for (bar = bar_start; bar != bar_cur; bar = bar->next) {
		bar->x_space += slot_width;
	}
}

static int search_x_slot(choice_t *choices_lo, choice_t *choices_hi) {
	int option_idx;
	if (verbose_flag && !mp_inc(&x_cost)) {
		return -1;
	}
	if (options_header->out_next != options_header) {
		int r;
		option_t *option_out;
		for (; choices_lo != choices_header && !is_valid_choice(choices_lo->y_slot, choices_lo->x_slot); choices_lo = choices_lo->next);
		if (choices_lo == choices_header) {
			return 0;
		}
		r = 0;
		for (option_out = options_header->out_next; option_out != options_header; option_out = option_out->out_next) {
			if (option_out->y_slot_lo == choices_lo->y_slot) {
				if (option_out == option_sym && choices_lo->x_slot > x_slot_sym_max) {
					break;
				}
				for (option_idx = 0; option_idx < options_in_n && (no_edges_overlap(option_out->y_slot_lo, option_out->y_slot_hi, options_in[option_idx]->y_slot_lo, options_in[option_idx]->y_slot_hi) || no_edges_overlap(choices_lo->x_slot, choices_lo->x_slot+option_out->slot_width, options_in[option_idx]->x_slot_lo, options_in[option_idx]->x_slot_hi)); ++option_idx);
				if (option_idx == options_in_n) {
					option_out->x_slot_lo = choices_lo->x_slot;
					option_out->x_slot_hi = choices_lo->x_slot+option_out->slot_width;
					link_options_out(option_out->out_last, option_out->out_next);
					options_in[options_in_n++] = option_out;
					if (option_out->y_slot_hi < paint_height) {
						choice_t *choice_last;
						++choices_hi;
						set_choice(choices_hi, option_out->y_slot_hi, option_out->x_slot_lo);
						for (choice_last = choices_header->last; choice_last != choices_header && compare_choices(choice_last, choices_hi) > 0; choice_last = choice_last->last);
						insert_choice(choices_hi, choice_last, choice_last->next);
					}
					if (option_out->x_slot_hi < paint_width) {
						choice_t *choice_next;
						++choices_hi;
						set_choice(choices_hi, option_out->y_slot_lo, option_out->x_slot_hi);
						for (choice_next = choices_header->next; choice_next != choices_header && compare_choices(choice_next, choices_hi) < 0; choice_next = choice_next->next);
						insert_choice(choices_hi, choice_next->last, choice_next);
					}
					r = search_x_slot(choices_lo->next, choices_hi);
					if (option_out->x_slot_hi < paint_width) {
						link_choices(choices_hi->last, choices_hi->next);
						--choices_hi;
					}
					if (option_out->y_slot_hi < paint_height) {
						link_choices(choices_hi->last, choices_hi->next);
						--choices_hi;
					}
					--options_in_n;
					option_out->out_next->out_last = option_out;
					option_out->out_last->out_next = option_out;
					if (r) {
						break;
					}
				}
			}
		}
		return r;
	}
	printf("0 %d %d %d\n", paint_height, paint_width, mondrian_tiles_n);
	for (option_idx = 0; option_idx < options_in_n; ++option_idx) {
		print_option(options_in[option_idx]);
	}
	printf("Defect %d\n", mondrian_defect);
	fflush(stdout);
	return 1;
}

static int is_valid_choice(int y_slot, int x_slot) {
	int option_idx;
	for (option_idx = 0; option_idx < options_in_n && is_valid_option(options_in[option_idx], y_slot, x_slot); ++option_idx);
	return option_idx == options_in_n;
}

static int is_valid_option(const option_t *option, int y_slot, int x_slot) {
	return option->y_slot_lo > y_slot || option->y_slot_hi <= y_slot || option->x_slot_lo > x_slot || option->x_slot_hi <= x_slot;
}

static int no_edges_overlap(int a1, int a2, int b1, int b2) {
	return a1 >= b2 || a2 <= b1;
}

static void set_tile(tile_t *tile, int height, int width) {
	int y_slots_n = paint_height-height+1, x_slots_n = paint_width-width+1;
	tile->height = height;
	tile->width = width;
	tile->area = height*width;
	tile->delta = width-height;
	tile->slots_n = y_slots_n*x_slots_n;
	tile->y_priority = y_slots_n;
	tile->x_priority = x_slots_n;
	if (rotate_flag && width <= paint_height && height <= paint_width) {
		tile->slots_n += (paint_height-width+1)*(paint_width-height+1);
		if (tile->delta) {
			tile->y_priority += paint_height;
		}
	}
}

static int compare_tiles(const void *a, const void *b) {
	const tile_t *tile_a = (const tile_t *)a, *tile_b = (const tile_t *)b;
	if (tile_a->area != tile_b->area) {
		return tile_b->area-tile_a->area;
	}
	if (tile_a->slots_n != tile_b->slots_n) {
		return tile_b->slots_n-tile_a->slots_n;
	}
	return tile_b->width-tile_a->width;
}

static void copy_tile(option_t *option, const tile_t *tile) {
	option->height = tile->height;
	option->width = tile->width;
	option->area = tile->area;
	option->delta = tile->delta;
	option->slots_n = tile->slots_n;
	option->y_priority = tile->y_priority;
	option->x_priority = tile->x_priority;
}

static void set_option(option_t *option) {
	option->y_slot_hi = option->y_slot_lo+option->slot_height;
	option->slot_width = option->slot_height == option->height ? option->width:option->height;
}

static int compare_options(const void *a, const void *b) {
	const option_t *option_a = (const option_t *)a, *option_b = (const option_t *)b;
	if (option_a->slots_n != option_b->slots_n) {
		return option_a->slots_n-option_b->slots_n;
	}
	if (option_a->area != option_b->area) {
		return option_a->area-option_b->area;
	}
	return option_b->width-option_a->width;
}

static int compare_priorities(const option_t *option_a, const option_t *option_b) {
	if (option_a->y_priority != option_b->y_priority) {
		return option_b->y_priority-option_a->y_priority;
	}
	return option_b->x_priority-option_a->x_priority;
}

static void restore_option_y(option_t *option) {
	option->y_next->y_last = option;
	option->y_last->y_next = option;
}

static void link_options_y(option_t *last, option_t *next) {
	last->y_next = next;
	next->y_last = last;
}

static void insert_option_d(option_t *option, option_t *last, option_t *next) {
	link_options_d(last, option);
	link_options_d(option, next);
}

static void link_options_d(option_t *last, option_t *next) {
	last->d_next = next;
	next->d_last = last;
}

static void link_options_out(option_t *last, option_t *next) {
	last->out_next = next;
	next->out_last = last;
}

static void print_option(const option_t *option) {
	printf("%dx%d;%dx%d\n", option->y_slot_lo, option->x_slot_lo, option->slot_height, option->slot_width);
}

static void set_bar(bar_t *bar, int y_slot, int height, int x_space) {
	bar->y_slot = y_slot;
	bar->height = height;
	bar->y_space = paint_height-y_slot;
	bar->x_space = x_space;
}

static void insert_bar(bar_t *bar, bar_t *last, bar_t *next) {
	link_bars(last, bar);
	link_bars(bar, next);
}

static void link_bars(bar_t *last, bar_t *next) {
	last->next = next;
	next->last = last;
}

static void set_choice(choice_t *choice, int y_slot, int x_slot) {
	choice->y_slot = y_slot;
	choice->x_slot = x_slot;
}

static int compare_choices(const choice_t *choice_a, const choice_t *choice_b) {
	if (choice_a->y_slot != choice_b->y_slot) {
		return choice_a->y_slot-choice_b->y_slot;
	}
	return choice_a->x_slot-choice_b->x_slot;
}

static void insert_choice(choice_t *choice, choice_t *last, choice_t *next) {
	link_choices(last, choice);
	link_choices(choice, next);
}

static void link_choices(choice_t *last, choice_t *next) {
	last->next = next;
	next->last = last;
}
