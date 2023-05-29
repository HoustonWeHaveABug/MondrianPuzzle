#include <stdio.h>
#include <stdlib.h>
#include "mp_utils.h"

#define SQUARE_ORDER_MIN 3
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
	tile_t *tile;
	int rotate_flag;
	int y_slot_lo;
	int slot_height;
	int y_slot_hi;
	int slot_width;
	int x_slot_lo;
	int x_slot_hi;
	option_t *y_last;
	option_t *y_next;
	option_t *d_last;
	option_t *d_next;
	option_t *x_out_last;
	option_t *x_out_next;
	option_t *x_in_last;
	option_t *x_in_next;
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
static int check_tile(int, int, int);
static int check_big_tile(int, int);
static int best_defect(int);
static int add_mondrian_tile(int, int, int);
static int is_mondrian(void);
static int is_dominated(const tile_t *, const tile_t *);
static int search_y_slot(int, bar_t *, option_t *);
static int check_slot_height(bar_t *, int);
static int choose_y_slot(int, bar_t *, option_t *, int, int);
static int search_x_slot(choice_t *, choice_t *);
static int is_valid_choice(int, int);
static int edges_no_overlap(int, int, int, int);
static void set_tile(tile_t *, int, int);
static int compare_tiles(const void *, const void *);
static int compare_priorities(const tile_t *, const tile_t *);
static void set_option(option_t *);
static int compare_options(const void *, const void *);
static void restore_option_y(option_t *);
static void link_options_y(option_t *, option_t *);
static void insert_option_d(option_t *, option_t *, option_t *);
static void link_options_d(option_t *, option_t *);
static void link_options_x_out(option_t *, option_t *);
static void link_options_x_in(option_t *, option_t *);
static void set_bar(bar_t *, int, int, int);
static void insert_bar(bar_t *, bar_t *, bar_t *);
static void link_bars(bar_t *, bar_t *);
static void set_choice(choice_t *, int, int);
static int compare_choices(const choice_t *, const choice_t *);
static void insert_choice(choice_t *, choice_t *, choice_t *);
static void link_choices(choice_t *, choice_t *);

static int square_order, rotate_flag, defect_a, defect_b, options_lo, verbose_flag, square_area, *counts, tiles_max, mondrian_tiles_max, options_max, choices_max, success_tiles_n, defect_cur, mondrian_tiles_cur, tiles_n, mondrian_tiles_n, tile_stop, tiles_area, mondrian_defect, option_sym_flag, y_slot_sym_max, x_slot_sym_max, bars_n;
static mp_t y_cost, x_cost;
static tile_t *tiles, **mondrian_tiles, *success_tiles, *tile_sym;
static option_t *options, *options_header, *dominances_header, *option_sym;
static bar_t *bars, *bars_header;
static choice_t *choices, *choices_header;

int main(void) {
	int r;
	if (scanf("%d%d%d%d%d%d", &square_order, &rotate_flag, &defect_a, &defect_b, &options_lo, &verbose_flag) != 6 || square_order < SQUARE_ORDER_MIN || (unsigned)square_order > SIZE_T_MAX/(unsigned)square_order || defect_a < 0 || defect_b < 0 || options_lo < OPTIONS_LO_MIN) {
		fprintf(stderr, "Expected parameters: square_order (>= %d), rotate_flag, defect_a (>= 0), defect_b (>= 0), options_lo (>= %d), verbose_flag.\n", SQUARE_ORDER_MIN, OPTIONS_LO_MIN);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	square_area = square_order*square_order;
	if (SIZE_T_MAX/(unsigned)square_area < sizeof(int)) {
		fputs("Will not be able to allocate memory for counts\n", stderr);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	counts = malloc(sizeof(int)*(size_t)square_area);
	if (!counts) {
		fputs("Could not allocate memory for counts\n", stderr);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	tiles = malloc(sizeof(tile_t));
	if (!tiles) {
		fputs("Could not allocate memory for tiles\n", stderr);
		fflush(stderr);
		free(counts);
		return EXIT_FAILURE;
	}
	tiles_max = 1;
	mondrian_tiles = malloc(sizeof(tile_t *)*(size_t)(options_lo+1));
	if (!mondrian_tiles) {
		fputs("Could not allocate memory for mondrian_tiles\n", stderr);
		fflush(stderr);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	mondrian_tiles_max = options_lo+1;
	options = malloc(sizeof(option_t)*(size_t)(options_lo*2+2));
	if (!options) {
		fputs("Could not allocate memory for options\n", stderr);
		fflush(stderr);
		free(mondrian_tiles);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	options_max = options_lo;
	bars = malloc(sizeof(bar_t)*(size_t)(square_order+1));
	if (!bars) {
		fputs("Could not allocate memory for bars\n", stderr);
		fflush(stderr);
		free(options);
		free(mondrian_tiles);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	set_bar(bars, 0, square_order, square_order);
	bars_header = bars+square_order;
	set_bar(bars_header, square_order, 0, 0);
	insert_bar(bars, bars_header, bars_header);
	choices = malloc(sizeof(choice_t)+sizeof(choice_t)*(size_t)(1+options_lo*2+1));
	if (!choices) {
		fputs("Could not allocate memory for choices\n", stderr);
		fflush(stderr);
		free(bars);
		free(options);
		free(mondrian_tiles);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	choices_max = 1+options_lo*2;
	set_choice(choices, 0, 0);
	success_tiles = malloc(sizeof(tile_t));
	if (!success_tiles) {
		fputs("Could not allocate memory for success_tiles\n", stderr);
		fflush(stderr);
		free(choices);
		free(bars);
		free(options);
		free(mondrian_tiles);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
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
	free(success_tiles);
	free(choices);
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
	int count_idx1, width, area, height;
	for (count_idx1 = square_area; count_idx1--; ) {
		counts[count_idx1] = 0;
	}
	for (width = 1; width < square_order; ++width) {
		area = 0;
		for (height = 1; height < width; ++height) {
			area += width;
			if (check_tile(width, height, area)) {
				++counts[area-1];
				if (!rotate_flag) {
					++counts[area-1];
				}
			}
		}
		area += width;
		if (check_tile(width, width, area)) {
			++counts[area-1];
		}
	}
	area = 0;
	for (height = 1; height < square_order; ++height) {
		area += square_order;
		if (check_big_tile(height, area)) {
			++counts[area-1];
			if (!rotate_flag) {
				++counts[area-1];
			}
		}
	}
	for (count_idx1 = square_area; count_idx1--; ) {
		if (counts[count_idx1]) {
			int offset;
			for (offset = count_idx1 < defect_cur ? count_idx1:defect_cur; offset >= 0; --offset) {
				int counts_sum = 0, counts_area = 0, count_lo = count_idx1-offset, count_hi = defect_cur+count_lo < square_area ? defect_cur+count_lo:square_area-1, count_idx2;
				for (count_idx2 = count_lo; count_idx2 <= count_hi; ++count_idx2) {
					counts_sum += counts[count_idx2];
					counts_area += (count_idx2+1)*counts[count_idx2];
					if (counts_sum >= options_lo && counts_area >= square_area) {
						break;
					}
				}
				if (count_idx2 <= count_hi) {
					break;
				}
			}
			if (offset < 0) {
				counts[count_idx1] = 0;
			}
		}
	}
	add_tiles(0);
	if (tiles_n > tiles_max) {
		tile_t *tiles_tmp = realloc(tiles, sizeof(tile_t)*(size_t)tiles_n);
		if (!tiles_tmp) {
			fputs("Could not reallocate memory for tiles\n", stderr);
			fflush(stderr);
			return -1;
		}
		tiles = tiles_tmp;
		tiles_max = tiles_n;
	}
	add_tiles(1);
	qsort(tiles, (size_t)tiles_n, sizeof(tile_t), compare_tiles);
	printf("Current %d Tiles %d\n", defect_cur, tiles_n);
	fflush(stdout);
	if (tiles_n >= options_lo) {
		mondrian_tiles_n = 0;
		tile_stop = 0;
		tiles_area = 0;
		return add_mondrian_tile(0, 1, !rotate_flag);
	}
	return 0;
}

static void add_tiles(int set_flag) {
	int width, area, height;
	tiles_n = 0;
	for (width = 1; width < square_order; ++width) {
		area = 0;
		for (height = 1; height < width; ++height) {
			area += width;
			if (counts[area-1] && check_tile(width, height, area)) {
				add_tile(set_flag, height, width);
				if (!rotate_flag) {
					add_tile(set_flag, width, height);
				}
			}
		}
		area += width;
		if (counts[area-1] && check_tile(width, width, area)) {
			add_tile(set_flag, width, width);
		}
	}
	area = 0;
	for (height = 1; height < square_order; ++height) {
		area += square_order;
		if (counts[area-1] && check_big_tile(height, area)) {
			add_tile(set_flag, height, square_order);
			if (!rotate_flag) {
				add_tile(set_flag, square_order, height);
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

static int check_tile(int len_max, int len_min, int area) {
	return area-(square_order-len_max)*square_order <= defect_cur && area-len_max*(square_order-len_min) <= defect_cur && best_defect(area) <= defect_cur;
}

static int check_big_tile(int len, int area) {
	int delta = square_order-len;
	if (delta < len) {
		return area-delta*square_order <= defect_cur && best_defect(area) <= defect_cur;
	}
	if (delta == len) {
		return area-delta*(square_order/2-1) <= defect_cur && best_defect(area) <= defect_cur;
	}
	if (delta*square_order-area <= defect_cur && best_defect(area) <= defect_cur) {
		return 1;
	}
	if (square_order%2 == 0) {
		return area-delta*(square_order/2-1) <= defect_cur && best_defect(area) <= defect_cur;
	}
	return area-delta*(square_order/2) <= defect_cur && best_defect(area) <= defect_cur;
}

static int best_defect(int area) {
	int options_n = square_area/area, area_mod, best, others_n, defect;
	if (options_n < options_lo) {
		return area+(area-square_area)/(options_lo-1);
	}
	area_mod = square_area%area;
	if (!area_mod) {
		return 0;
	}
	best = area-area_mod;
	others_n = options_n-1;
	defect = area_mod%others_n ? area_mod/others_n+1:area_mod/others_n;
	if (defect < best) {
		best = defect;
	}
	defect = area+(area-square_area)/(others_n+1);
	if (defect < best) {
		best = defect;
	}
	return best;
}

static int add_mondrian_tile(int tiles_start, int same_flag, int sym_flag) {
	int r, tile_idx1;
	if (mondrian_tiles_n == mondrian_tiles_max) {
		tile_t **mondrian_tiles_tmp = realloc(mondrian_tiles, sizeof(tile_t *)*(size_t)(mondrian_tiles_n+1));
		if (!mondrian_tiles_tmp) {
			fputs("Could not reallocate memory for mondrian_tiles\n", stderr);
			fflush(stderr);
			return -1;
		}
		mondrian_tiles = mondrian_tiles_tmp;
		mondrian_tiles_max = mondrian_tiles_n+1;
	}
	r = 0;
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
		mondrian_tiles[mondrian_tiles_n] = tiles+tile_idx1;
		if (mondrian_tiles_n) {
			if (sym_flag) {
				if (tiles[tile_idx1].height <= tiles[tile_idx1].width) {
					if (mondrian_tiles[mondrian_tiles_n-1]->height < mondrian_tiles[mondrian_tiles_n-1]->width) {
						sym_flag = 0;
					}
				}
				else {
					if (mondrian_tiles[mondrian_tiles_n-1]->height >= mondrian_tiles[mondrian_tiles_n-1]->width) {
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
			if (tiles_area+tiles[tile_idx1].area_left < square_area) {
				break;
			}
			if (mondrian_tiles_n < mondrian_tiles_cur && tiles_area+tiles[tile_stop-mondrian_tiles_cur+mondrian_tiles_n].area_left > square_area) {
				continue;
			}
		}
		else {
			int tile_stop_bak;
			if (sym_flag && tiles[tile_idx1].height > tiles[tile_idx1].width) {
				continue;
			}
			tile_stop_bak = tile_stop;
			for (; tile_stop < tiles_n && tiles[tile_idx1].area-tiles[tile_stop].area <= defect_cur; ++tile_stop);
			if (tile_stop > tile_stop_bak) {
				int area_left = 0, tile_idx2;
				for (tile_idx2 = tile_stop-1; tile_idx2 >= tile_idx1; tile_idx2--) {
					if (area_left <= square_area) {
						area_left += tiles[tile_idx2].area;
					}
					tiles[tile_idx2].area_left = area_left;
				}
			}
		}
		++mondrian_tiles_n;
		tiles_area += tiles[tile_idx1].area;
		if (mondrian_tiles_n > mondrian_tiles_cur && tiles_area <= square_area) {
			mondrian_tiles_cur = mondrian_tiles_n;
			printf("Mondrian tiles %d\n", mondrian_tiles_cur);
			fflush(stdout);
		}
		if (tiles_area < square_area) {
			r = add_mondrian_tile(tile_idx1+1, same_flag, sym_flag);
		}
		else if (tiles_area == square_area) {
			mondrian_defect = mondrian_tiles[0]->area-tiles[tile_idx1].area;
			if (defect_a <= defect_b) {
				if (mondrian_defect == defect_cur) {
					r = is_mondrian();
				}
			}
			else {
				if (mondrian_defect >= defect_b) {
					r = is_mondrian();
					if (mondrian_tiles_n > success_tiles_n && r == 1) {
						tile_t *success_tiles_tmp = realloc(success_tiles, sizeof(tile_t)*(size_t)mondrian_tiles_n);
						if (success_tiles_tmp) {
							success_tiles = success_tiles_tmp;
							success_tiles_n = mondrian_tiles_n;
						}
						else {
							fputs("Could not reallocate memory for success_tiles\n", stderr);
							fflush(stderr);
							r = -1;
						}
					}
					if (r == 1) {
						int tile_idx2;
						if (mondrian_tiles_n > options_lo) {
							options_lo = mondrian_tiles_n;
						}
						for (tile_idx2 = mondrian_tiles_n; tile_idx2--; ) {
							success_tiles[tile_idx2] = *(mondrian_tiles[tile_idx2]);
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
	if (mondrian_tiles_n > options_max) {
		option_t *options_tmp = realloc(options, sizeof(option_t)*(size_t)(mondrian_tiles_n*2+2));
		if (!options_tmp) {
			fputs("Could not reallocate memory for options\n", stderr);
			fflush(stderr);
			return -1;
		}
		options = options_tmp;
		options_max = mondrian_tiles_n;
	}
	if (verbose_flag) {
		printf("is_mondrian options %d defect %d\n", mondrian_tiles_n, mondrian_defect);
		fflush(stdout);
	}
	for (option_idx = mondrian_tiles_n; option_idx--; ) {
		options[option_idx].tile = mondrian_tiles[option_idx];
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
		for (option_idx = 0; option_idx < dominances_n && !is_dominated(option->tile, dominances[option_idx].d_last->tile); ++option_idx);
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
		if (compare_priorities(option->tile, option_sym->tile) < 0) {
			option_sym = option;
		}
	}
	option_sym_flag = 1;
	tile_sym = option_sym->tile;
	y_slot_sym_max = (square_order-tile_sym->height)/2;
	x_slot_sym_max = (square_order-tile_sym->width)/2;
	option_sym_d_last = option_sym->d_last;
	option_sym_d_next = option_sym->d_next;
	link_options_d(option_sym_d_last, option_sym_d_next);
	if (option_sym_d_last == option_sym_d_next) {
		link_options_y(option_sym_d_last->y_last, option_sym_d_last->y_next);
	}
	for (option = options; option != options_header; option = option->y_next) {
		tile_t *tile = option->tile;
		option->rotate_flag = rotate_flag && tile->height != tile->width && option != option_sym;
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

static int is_dominated(const tile_t *tile_a, const tile_t *tile_b) {
	return tile_a->height <= tile_b->height && tile_a->width <= tile_b->width;
}

static int search_y_slot(int bars_hi, bar_t *bar_start_bak, option_t *options_start) {
	int r, option_idx, choices_n;
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
		int x_space, x_delta, x_min, slot_height, slot_width;
		option_t *option, *last_chance, *dominance;
		bar_t *bar_cur, *bar_cur_next, *bar_start_next, *bar;
		if (bar_start != bar_start_bak) {
			options_start = options_header->y_next;
		}
		if (bars_hi == bars_n) {
			for (option = options_start; option != options_header; option = option->y_next) {
				tile_t *tile = option->tile;
				if (tile->width == bar_start->x_space && check_slot_height(bar_start, tile->height)) {
					return choose_y_slot(bars_hi, bar_start, option, tile->height, tile->width);
				}
				if (option->rotate_flag && tile->height == bar_start->x_space && check_slot_height(bar_start, tile->width)) {
					return choose_y_slot(bars_hi, bar_start, option, tile->width, tile->height);
				}
			}
			return 0;
		}
		x_space = 0;
		x_delta = 0;
		x_min = square_order;
		slot_height = square_order;
		for (option = options_start; option != options_header; option = option->y_next) {
			tile_t *tile = option->tile;
			if (tile->height <= bar_start->y_space && tile->width <= bar_start->x_space) {
				x_space += tile->width;
				if (option->rotate_flag && tile->width <= bar_start->y_space && tile->height <= bar_start->x_space) {
					x_delta += tile->delta;
					if (tile->height < x_min) {
						x_min = tile->height;
					}
				}
				else {
					if (tile->width < x_min) {
						x_min = tile->width;
					}
				}
				if (tile->height < slot_height) {
					slot_height = tile->height;
				}
			}
			else if (option->rotate_flag && tile->width <= bar_start->y_space && tile->height <= bar_start->x_space) {
				x_space += tile->height;
				if (tile->height < x_min) {
					x_min = tile->height;
				}
				if (tile->width < slot_height) {
					slot_height = tile->width;
				}
			}
		}
		if (x_space < bar_start->x_space || (x_space > bar_start->x_space+x_delta && x_space < bar_start->x_space+x_min)) {
			return 0;
		}
		slot_width = bar_start->x_space;
		for (bar_cur = bar_start; bar_cur != bars_header && bar_cur->height <= slot_height; bar_cur = bar_cur->next) {
			slot_height -= bar_cur->height;
			bar_cur->x_space -= slot_width;
		}
		bar_cur_next = bar_cur->next;
		if (slot_height) {
			set_bar(bars+bars_n, bar_cur->y_slot+slot_height, bar_cur->height-slot_height, bar_cur->x_space);
			bar_cur->x_space -= slot_width;
			insert_bar(bars+bars_n, bar_cur, bar_cur_next);
		}
		bar_start_next = bar_start->next;
		last_chance = options_header;
		if (option_sym_flag) {
			for (bar = bar_start_next; bar != bars_header && bar->x_space < tile_sym->width; bar = bar->next);
			if (bar == bars_header || bar->y_slot > y_slot_sym_max || tile_sym->height > bar->y_space) {
				if (option_sym < options_start || bar_start->y_slot > y_slot_sym_max || tile_sym->height > bar_start->y_space || tile_sym->width > slot_width) {
					if (slot_height) {
						link_bars(bar_cur, bar_cur_next);
						bar_cur->x_space += slot_width;
					}
					for (bar = bar_start; bar != bar_cur; bar = bar->next) {
						bar->x_space += slot_width;
					}
					return 0;
				}
				last_chance = option_sym;
			}
		}
		for (dominance = dominances_header->y_next; dominance != dominances_header; dominance = dominance->y_next) {
			tile_t *tile;
			option = dominance->d_next;
			tile = option->tile;
			if (option->rotate_flag) {
				for (bar = bar_start_next; bar != bars_header && bar->x_space < tile->height; bar = bar->next);
				if (bar == bars_header || bar->y_space < tile->width) {
					for (; bar != bars_header && bar->x_space < tile->width; bar = bar->next);
					if (bar == bars_header || bar->y_space < tile->height) {
						if (option < options_start || ((tile->width > bar_start->y_space || tile->height > slot_width) && (tile->height > bar_start->y_space || tile->width > slot_width))) {
							if (slot_height) {
								link_bars(bar_cur, bar_cur_next);
								bar_cur->x_space += slot_width;
							}
							for (bar = bar_start; bar != bar_cur; bar = bar->next) {
								bar->x_space += slot_width;
							}
							return 0;
						}
						if (option < last_chance) {
							last_chance = option;
						}
					}
				}
			}
			else {
				for (bar = bar_start_next; bar != bars_header && bar->x_space < tile->width; bar = bar->next);
				if (bar == bars_header || bar->y_space < tile->height) {
					if (option < options_start || tile->height > bar_start->y_space || tile->width > slot_width) {
						if (slot_height) {
							link_bars(bar_cur, bar_cur_next);
							bar_cur->x_space += slot_width;
						}
						for (bar = bar_start; bar != bar_cur; bar = bar->next) {
							bar->x_space += slot_width;
						}
						return 0;
					}
					if (option < last_chance) {
						last_chance = option;
					}
				}
			}
		}
		if (slot_height) {
			link_bars(bar_cur, bar_cur_next);
			bar_cur->x_space += slot_width;
		}
		for (bar = bar_start; bar != bar_cur; bar = bar->next) {
			bar->x_space += slot_width;
		}
		for (bar = bar_start->next; bar != bars_header; bar = bar->next) {
			if (option_sym_flag && bar->y_slot <= y_slot_sym_max && bar->y_space >= tile_sym->height && bar->x_space >= tile_sym->width) {
				continue;
			}
			for (dominance = dominances_header->y_next; dominance != dominances_header; dominance = dominance->y_next) {
				tile_t *tile;
				option = dominance->d_last;
				tile = option->tile;
				if ((tile->height <= bar->y_space && tile->width <= bar->x_space) || (option->rotate_flag && tile->width <= bar->y_space && tile->height <= bar->x_space)) {
					break;
				}
			}
			if (dominance == dominances_header) {
				return 0;
			}
		}
		r = 0;
		for (option = options_start; option != options_header; option = option->y_next) {
			tile_t *tile = option->tile;
			if (tile->height <= bar_start->y_space && tile->width <= bar_start->x_space) {
				x_space -= tile->width;
				r = choose_y_slot(bars_hi, bar_start, option, tile->height, tile->width);
			}
			if (!r && option->rotate_flag && tile->width <= bar_start->y_space && tile->height <= bar_start->x_space) {
				if (tile->height <= bar_start->y_space && tile->width <= bar_start->x_space) {
					if (x_space+tile->height >= bar_start->x_space) {
						r = choose_y_slot(bars_hi, bar_start, option, tile->width, tile->height);
					}
				}
				else {
					x_space -= tile->height;
					r = choose_y_slot(bars_hi, bar_start, option, tile->width, tile->height);
				}
			}
			if (x_space < bar_start->x_space || r || option == last_chance) {
				break;
			}
		}
		return r;
	}
	for (option_idx = mondrian_tiles_n; option_idx--; ) {
		set_option(options+option_idx);
		link_options_x_out(options+option_idx, options+option_idx+1);
	}
	link_options_x_out(options_header, options);
	link_options_x_in(options_header, options_header);
	choices_n = 1+mondrian_tiles_n*2;
	if (choices_n > choices_max) {
		choice_t *choices_tmp = realloc(choices, sizeof(choice_t)*(size_t)(choices_n+1));
		if (!choices_tmp) {
			fputs("Could not reallocate memory for choices\n", stderr);
			fflush(stderr);
			return -1;
		}
		choices = choices_tmp;
		choices_max = choices_n;
	}
	choices_header = choices+choices_n;
	insert_choice(choices, choices_header, choices_header);
	if (verbose_flag && !mp_new(&x_cost)) {
		return -1;
	}
	r = search_x_slot(choices, choices);
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
	bar_t *bar_cur, *bar_cur_next, *bar;
	option->y_slot_lo = bar_start->y_slot;
	option->slot_height = slot_height;
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
	bar_cur_next = bar_cur->next;
	if (slot_height) {
		set_bar(bars+bars_n, bar_cur->y_slot+slot_height, bar_cur->height-slot_height, bar_cur->x_space);
		bar_cur->height = slot_height;
		bar_cur->x_space -= slot_width;
		insert_bar(bars+bars_n, bar_cur, bar_cur_next);
		++bars_n;
	}
	r = search_y_slot(bars_hi-1, bar_start, option->y_next);
	if (slot_height) {
		--bars_n;
		link_bars(bar_cur, bar_cur_next);
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

static int search_x_slot(choice_t *choices_lo, choice_t *choices_hi) {
	option_t *option_in;
	if (verbose_flag && !mp_inc(&x_cost)) {
		return -1;
	}
	if (options_header->x_out_next != options_header) {
		int r;
		option_t *option_out;
		for (; choices_lo != choices_header && !is_valid_choice(choices_lo->y_slot, choices_lo->x_slot); choices_lo = choices_lo->next);
		if (choices_lo == choices_header) {
			return 0;
		}
		r = 0;
		for (option_out = options_header->x_out_next; option_out != options_header; option_out = option_out->x_out_next) {
			if (option_out->y_slot_lo == choices_lo->y_slot) {
				if (option_out == option_sym && choices_lo->x_slot > x_slot_sym_max) {
					break;
				}
				for (option_in = options_header->x_in_next; option_in != options_header && (edges_no_overlap(option_out->y_slot_lo, option_out->y_slot_hi, option_in->y_slot_lo, option_in->y_slot_hi) || edges_no_overlap(choices_lo->x_slot, choices_lo->x_slot+option_out->slot_width, option_in->x_slot_lo, option_in->x_slot_hi)); option_in = option_in->x_in_next);
				if (option_in == options_header) {
					option_t *options_x_in_last = options_header->x_in_last;
					option_out->x_slot_lo = choices_lo->x_slot;
					option_out->x_slot_hi = choices_lo->x_slot+option_out->slot_width;
					link_options_x_out(option_out->x_out_last, option_out->x_out_next);
					link_options_x_in(options_x_in_last, option_out);
					link_options_x_in(option_out, options_header);
					if (option_out->y_slot_hi < square_order) {
						choice_t *choice_last;
						++choices_hi;
						set_choice(choices_hi, option_out->y_slot_hi, option_out->x_slot_lo);
						for (choice_last = choices_header->last; choice_last != choices_header && compare_choices(choice_last, choices_hi) > 0; choice_last = choice_last->last);
						insert_choice(choices_hi, choice_last, choice_last->next);
					}
					if (option_out->x_slot_hi < square_order) {
						choice_t *choice_next;
						++choices_hi;
						set_choice(choices_hi, option_out->y_slot_lo, option_out->x_slot_hi);
						for (choice_next = choices_header->next; choice_next != choices_header && compare_choices(choice_next, choices_hi) < 0; choice_next = choice_next->next);
						insert_choice(choices_hi, choice_next->last, choice_next);
					}
					r = search_x_slot(choices_lo->next, choices_hi);
					if (option_out->x_slot_hi < square_order) {
						link_choices(choices_hi->last, choices_hi->next);
						--choices_hi;
					}
					if (option_out->y_slot_hi < square_order) {
						link_choices(choices_hi->last, choices_hi->next);
						--choices_hi;
					}
					link_options_x_in(options_x_in_last, options_header);
					option_out->x_out_next->x_out_last = option_out;
					option_out->x_out_last->x_out_next = option_out;
					if (r) {
						break;
					}
				}
			}
		}
		return r;
	}
	printf("0 %d %d\n", square_order, mondrian_tiles_n);
	for (option_in = options_header->x_in_next; option_in != options_header; option_in = option_in->x_in_next) {
		printf("%dx%d;%dx%d\n", option_in->y_slot_lo, option_in->x_slot_lo, option_in->slot_height, option_in->slot_width);
	}
	printf("Defect %d\n", mondrian_defect);
	fflush(stdout);
	return 1;
}

static int is_valid_choice(int y_slot, int x_slot) {
	option_t *option_in;
	for (option_in = options_header->x_in_next; option_in != options_header; option_in = option_in->x_in_next) {
		if (option_in->y_slot_lo <= y_slot && option_in->y_slot_hi > y_slot && option_in->x_slot_lo <= x_slot && option_in->x_slot_hi > x_slot) {
			return 0;
		}
	}
	return 1;
}

static int edges_no_overlap(int a1, int a2, int b1, int b2) {
	return a1 >= b2 || a2 <= b1;
}

static void set_tile(tile_t *tile, int height, int width) {
	int y_slots_n = square_order-height+1, x_slots_n = square_order-width+1, weight = rotate_flag && height != width ? square_order:0;
	tile->height = height;
	tile->width = width;
	tile->area = height*width;
	tile->delta = width-height;
	tile->slots_n = y_slots_n*x_slots_n;
	tile->y_priority = weight+y_slots_n;
	tile->x_priority = x_slots_n;
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

static int compare_priorities(const tile_t *tile_a, const tile_t *tile_b) {
	if (tile_a->y_priority != tile_b->y_priority) {
		return tile_b->y_priority-tile_a->y_priority;
	}
	return tile_b->x_priority-tile_a->x_priority;
}

static void set_option(option_t *option) {
	tile_t *tile = option->tile;
	option->y_slot_hi = option->y_slot_lo+option->slot_height;
	option->slot_width = option->slot_height == tile->height ? tile->width:tile->height;
}

static int compare_options(const void *a, const void *b) {
	const option_t *option_a = (const option_t *)a, *option_b = (const option_t *)b;
	tile_t *tile_a = option_a->tile, *tile_b = option_b->tile;
	if (tile_a->slots_n != tile_b->slots_n) {
		return tile_a->slots_n-tile_b->slots_n;
	}
	if (tile_a->area != tile_b->area) {
		return tile_a->area-tile_b->area;
	}
	return tile_b->width-tile_a->width;
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

static void link_options_x_out(option_t *last, option_t *next) {
	last->x_out_next = next;
	next->x_out_last = last;
}

static void link_options_x_in(option_t *last, option_t *next) {
	last->x_in_next = next;
	next->x_in_last = last;
}

static void set_bar(bar_t *bar, int y_slot, int height, int x_space) {
	bar->y_slot = y_slot;
	bar->height = height;
	bar->y_space = square_order-y_slot;
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
