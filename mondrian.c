#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "mp_utils.h"

#define SQUARE_ORDER_MIN 3
#define TILES_MIN 2

typedef struct {
	int height;
	int width;
	int area;
	int y_h_slot_sym_max;
	int y_w_slot_sym_max;
	int x_h_slot_sym_max;
	int x_w_slot_sym_max;
	double y_factor_sym;
	double x_factor_sym;
}
tile_t;

typedef struct option_s option_t;

struct option_s {
	tile_t *tile;
	int y_slot;
	int x_slot;
	int slot_height;
	int slot_width;
	int x_slot_max;
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
	int width;
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

int set_tiles(void);
void add_tiles(int);
int best_defect(int);
int add_option(int, int, int, int, int);
int is_mondrian(int, int);
void set_dominance(option_t *);
void insert_dominance(option_t *, option_t *, option_t *);
int is_dominated(tile_t *, tile_t *);
int search_y_slot(int, int, int, int, int, bar_t *, option_t *, int, option_t *);
int choose_y_slot(int, int, int, int, int, bar_t *, int, option_t *, option_t *, int, int);
void set_tile(tile_t *, int, int);
int compare_tiles(const void *, const void *);
void set_factor_sym(int, int, tile_t *);
double compare_factor_syms(tile_t *, tile_t *);
int search_x_slot(choice_t *, choice_t *);
int is_valid_choice(int, int);
int is_valid_slot(option_t *);
int edges_overlap(int, int, int, int);
void link_options_y(option_t *, option_t *);
void link_options_d(option_t *, option_t *);
void link_options_x_out(option_t *, option_t *);
void link_options_x_in(option_t *, option_t *);
void print_option(option_t *);
void set_bar(bar_t *, int, int, int);
void link_bars(bar_t *, bar_t *);
void set_choice(choice_t *, int, int);
int compare_choices(choice_t *, choice_t *);
void insert_choice(choice_t *, choice_t *, choice_t *);
void link_choices(choice_t *, choice_t *);
int is_square(tile_t *);
void free_data(void);

int square_order, rotate_flag, defect_a, defect_b, options_min, verbose_flag, square_area, *counts, tiles_max, options_max, dominances_max, choices_max, tiles_best_n, defect_cur, tiles_n, dominances_n;
mp_t y_cost, x_cost;
tile_t *tiles, tile_max, *tiles_best;
option_t *options, *options_y_header, *dominances, *options_x_out_header, *options_x_in_header;
bar_t *bars, *bars_header;
choice_t *choices, *choices_header;

int main(void) {
	int r;
	if (scanf("%d%d%d%d%d%d", &square_order, &rotate_flag, &defect_a, &defect_b, &options_min, &verbose_flag) != 6 || square_order < SQUARE_ORDER_MIN || square_order > LONG_MAX/square_order || defect_a < 0 || defect_b < 0 || options_min < TILES_MIN) {
		fprintf(stderr, "Invalid parameters\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	square_area = square_order*square_order;
	if (sizeof(int) > (size_t)-1/(size_t)square_area) {
		fprintf(stderr, "Will not be able to allocate memory for counts\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	counts = malloc(sizeof(int)*(size_t)square_area);
	if (!counts) {
		fprintf(stderr, "Could not allocate memory for counts\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	tiles = malloc(sizeof(tile_t));
	if (!tiles) {
		fprintf(stderr, "Could not allocate memory for tiles\n");
		fflush(stderr);
		free(counts);
		return EXIT_FAILURE;
	}
	set_tile(&tile_max, square_order, square_order);
	tiles_max = 1;
	options = malloc(sizeof(option_t)*(size_t)(options_min+3));
	if (!options) {
		fprintf(stderr, "Could not allocate memory for options\n");
		fflush(stderr);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	options_max = options_min;
	bars = malloc(sizeof(bar_t)*(size_t)(square_order+1));
	if (!bars) {
		fprintf(stderr, "Could not allocate memory for bars\n");
		fflush(stderr);
		free(options);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	set_bar(bars, 0, square_order, 0);
	bars_header = bars+square_order;
	set_bar(bars_header, square_order, 0, square_order);
	link_bars(bars_header, bars);
	link_bars(bars, bars_header);
	dominances = malloc(sizeof(option_t));
	if (!dominances) {
		fprintf(stderr, "Could not allocate memory for dominances\n");
		fflush(stderr);
		free(bars);
		free(options);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	dominances_max = 1;
	choices = malloc(sizeof(choice_t));
	if (!choices) {
		fprintf(stderr, "Could not allocate memory for choices\n");
		fflush(stderr);
		free(dominances);
		free(bars);
		free(options);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	set_choice(choices, 0, 0);
	choices_max = 1;
	tiles_best = malloc(sizeof(tile_t)*(size_t)options_min);
	if (!tiles_best) {
		fprintf(stderr, "Could not allocate memory for tiles_best\n");
		fflush(stderr);
		free(choices);
		free(dominances);
		free(bars);
		free(options);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	tiles_best_n = 0;
	defect_cur = defect_a;
	if (defect_a <= defect_b) {
		do {
			if (!set_tiles()) {
				free_data();
				return EXIT_FAILURE;
			}
			if (tiles_n >= TILES_MIN) {
				r = add_option(0, 0, 1, 1, 0);
			}
			else {
				r = 0;
			}
			if (!r) {
				defect_cur++;
			}
		}
		while (defect_cur <= defect_b && !r);
	}
	else {
		do {
			if (!set_tiles()) {
				free_data();
				return EXIT_FAILURE;
			}
			if (tiles_n >= TILES_MIN) {
				r = add_option(0, 0, 1, 1, 0);
			}
			else {
				r = 0;
			}
		}
		while (defect_cur >= defect_b && r == 1);
	}
	free_data();
	return EXIT_SUCCESS;
}

int set_tiles(void) {
	int count_idx1, height, width, area;
	for (count_idx1 = 0; count_idx1 < square_area; count_idx1++) {
		counts[count_idx1] = 0;
	}
	for (width = 1; width < square_order; width++) {
		for (height = 1; height <= width; height++) {
			area = height*width;
			if (area-square_order*(square_order-width) <= defect_cur && area-(square_order-height)*width <= defect_cur && best_defect(area) <= defect_cur) {
				counts[area-1]++;
			}
		}
		if (!rotate_flag) {
			for (; height < square_order; height++) {
				area = height*width;
				if (area-(square_order-height)*square_order <= defect_cur && area-height*(square_order-width) <= defect_cur && best_defect(area) <= defect_cur) {
					counts[area-1]++;
				}
			}
		}
	}
	for (height = 1; height < square_order; height++) {
		area = height*square_order;
		if (height > square_order-height) {
			if (area-(square_order-height)*square_order <= defect_cur && best_defect(area) <= defect_cur) {
				counts[area-1]++;
			}
		}
		else if (height == square_order-height) {
			if (area-(square_order-height)*(square_order/2-1) <= defect_cur && best_defect(area) <= defect_cur) {
				counts[area-1]++;
			}
		}
		else {
			if ((square_order-height)*square_order-area <= defect_cur && best_defect(area) <= defect_cur) {
				counts[area-1]++;
			}
			else {
				if (square_order%2 == 0) {
					if (area-(square_order-height)*(square_order/2-1) <= defect_cur && best_defect(area) <= defect_cur) {
						counts[area-1]++;
					}
				}
				else {
					if (area-(square_order-height)*(square_order/2) <= defect_cur && best_defect(area) <= defect_cur) {
						counts[area-1]++;
					}
				}
			}
		}
	}
	if (!rotate_flag) {
		for (width = 1; width < square_order; width++) {
			area = square_order*width;
			if (width > square_order-width) {
				if (area-square_order*(square_order-width) <= defect_cur && best_defect(area) <= defect_cur) {
					counts[area-1]++;
				}
			}
			else if (width == square_order-width) {
				if (area-(square_order/2-1)*(square_order-width) <= defect_cur && best_defect(area) <= defect_cur) {
					counts[area-1]++;
				}
			}
			else {
				if (square_order*(square_order-width)-area <= defect_cur && best_defect(area) <= defect_cur) {
					counts[area-1]++;
				}
				else {
					if (square_order%2 == 0) {
						if (area-(square_order/2-1)*(square_order-width) <= defect_cur && best_defect(area) <= defect_cur) {
							counts[area-1]++;
						}
					}
					else {
						if (area-(square_order/2)*(square_order-width) <= defect_cur && best_defect(area) <= defect_cur) {
							counts[area-1]++;
						}
					}
				}
			}
		}
	}
	for (count_idx1 = 0; count_idx1 < square_area; count_idx1++) {
		if (counts[count_idx1] > 0) {
			int offset;
			if (count_idx1 < defect_cur) {
				offset = count_idx1;
			}
			else {
				offset = defect_cur;
			}
			while (offset >= 0) {
				int tiles_area_sum = 0, count_idx2;
				for (count_idx2 = count_idx1-offset; count_idx2 < square_area && count_idx2 <= defect_cur+count_idx1-offset && tiles_area_sum < square_area; count_idx2++) {
					tiles_area_sum += (count_idx2+1)*counts[count_idx2];
				}
				if (tiles_area_sum >= square_area) {
					break;
				}
				offset--;
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
			fprintf(stderr, "Could not reallocate memory for tiles\n");
			fflush(stderr);
			return 0;
		}
		tiles = tiles_tmp;
		tiles_max = tiles_n;
	}
	add_tiles(1);
	qsort(tiles, (size_t)tiles_n, sizeof(tile_t), compare_tiles);
	printf("Current %d Tiles %d\n", defect_cur, tiles_n);
	fflush(stdout);
	return 1;
}

void add_tiles(int set_call) {
	int height, width, area;
	tiles_n = 0;
	for (width = 1; width < square_order; width++) {
		for (height = 1; height <= width; height++) {
			area = height*width;
			if (counts[area-1] > 0 && area-square_order*(square_order-width) <= defect_cur && area-(square_order-height)*width <= defect_cur && best_defect(area) <= defect_cur) {
				if (set_call) {
					set_tile(tiles+tiles_n, height, width);
				}
				tiles_n++;
			}
		}
		if (!rotate_flag) {
			for (; height < square_order; height++) {
				area = height*width;
				if (counts[area-1] > 0 && area-(square_order-height)*square_order <= defect_cur && area-height*(square_order-width) <= defect_cur && best_defect(area) <= defect_cur) {
					if (set_call) {
						set_tile(tiles+tiles_n, height, width);
					}
					tiles_n++;
				}
			}
		}
	}
	for (height = 1; height < square_order; height++) {
		area = height*square_order;
		if (counts[area-1] > 0) {
			if (height > square_order-height) {
				if (area-(square_order-height)*square_order <= defect_cur && best_defect(area) <= defect_cur) {
					if (set_call) {
						set_tile(tiles+tiles_n, height, square_order);
					}
					tiles_n++;
				}
			}
			else if (height == square_order-height) {
				if (area-(square_order-height)*(square_order/2-1) <= defect_cur && best_defect(area) <= defect_cur) {
					if (set_call) {
						set_tile(tiles+tiles_n, height, square_order);
					}
					tiles_n++;
				}
			}
			else {
				if ((square_order-height)*square_order-area <= defect_cur && best_defect(area) <= defect_cur) {
					if (set_call) {
						set_tile(tiles+tiles_n, height, square_order);
					}
					tiles_n++;
				}
				else {
					if (square_order%2 == 0) {
						if (area-(square_order-height)*(square_order/2-1) <= defect_cur && best_defect(area) <= defect_cur) {
							if (set_call) {
								set_tile(tiles+tiles_n, height, square_order);
							}
							tiles_n++;
						}
					}
					else {
						if (area-(square_order-height)*(square_order/2) <= defect_cur && best_defect(area) <= defect_cur) {
							if (set_call) {
								set_tile(tiles+tiles_n, height, square_order);
							}
							tiles_n++;
						}
					}
				}
			}
		}
	}
	if (!rotate_flag) {
		for (width = 1; width < square_order; width++) {
			area = square_order*width;
			if (counts[area-1] > 0) {
				if (width > square_order-width) {
					if (area-square_order*(square_order-width) <= defect_cur && best_defect(area) <= defect_cur) {
						if (set_call) {
							set_tile(tiles+tiles_n, square_order, width);
						}
						tiles_n++;
					}
				}
				else if (width == square_order-width) {
					if (area-(square_order/2-1)*(square_order-width) <= defect_cur && best_defect(area) <= defect_cur) {
						if (set_call) {
							set_tile(tiles+tiles_n, square_order, width);
						}
						tiles_n++;
					}
				}
				else {
					if (square_order*(square_order-width)-area <= defect_cur && best_defect(area) <= defect_cur) {
						if (set_call) {
							set_tile(tiles+tiles_n, square_order, width);
						}
						tiles_n++;
					}
					else {
						if (square_order%2 == 0) {
							if (area-(square_order/2-1)*(square_order-width) <= defect_cur && best_defect(area) <= defect_cur) {
								if (set_call) {
									set_tile(tiles+tiles_n, square_order, width);
								}
								tiles_n++;
							}
						}
						else {
							if (area-(square_order/2)*(square_order-width) <= defect_cur && best_defect(area) <= defect_cur) {
								if (set_call) {
									set_tile(tiles+tiles_n, square_order, width);
								}
								tiles_n++;
							}
						}
					}
				}
			}
		}
	}
}

int best_defect(int area) {
	int options_n = square_area/area, area_mod, best, others_n, defect;
	if (options_n < options_min) {
		return area-(square_area-area)/(options_min-1);
	}
	area_mod = square_area%area;
	if (area_mod == 0) {
		return 0;
	}
	best = area-area_mod;
	others_n = options_n-1;
	if (others_n == 0) {
		return best;
	}
	if (area_mod%others_n > 0) {
		defect = area_mod/others_n+1;
	}
	else {
		defect = area_mod/others_n;
	}
	if (defect < best) {
		best = defect;
	}
	defect = area-(square_area-area)/(others_n+1);
	if (defect < best) {
		best = defect;
	}
	return best;
}

int add_option(int tiles_start, int options_n, int same_tiles, int all_squares, int options_area_sum) {
	int r, tile_idx1;
	if (options_n == options_max) {
		option_t *options_tmp = realloc(options, sizeof(option_t)*(size_t)(options_n+4));
		if (!options_tmp) {
			fprintf(stderr, "Could not reallocate memory for options\n");
			fflush(stderr);
			return -1;
		}
		options = options_tmp;
		options_max = options_n+1;
	}
	r = 0;
	for (tile_idx1 = tiles_start; tile_idx1 < tiles_n && !r; tile_idx1++) {
		int defect;
		options[options_n].tile = tiles+tile_idx1;
		defect = options[0].tile->area-tiles[tile_idx1].area;
		if (defect > defect_cur) {
			break;
		}
		if (same_tiles) {
			if (options_n < tiles_best_n) {
				int compare = compare_tiles(tiles+tile_idx1, tiles_best+options_n);
				if (compare < 0) {
					continue;
				}
				same_tiles = compare == 0;
			}
			else {
				same_tiles = 0;
			}
		}
		if (all_squares && tiles[tile_idx1].height > tiles[tile_idx1].width) {
			continue;
		}
		options_n++;
		options_area_sum += tiles[tile_idx1].area;
		if (options_area_sum < square_area) {
			int options_n_max = options_n, options_area_sum_max = options_area_sum, tile_idx2;
			for (tile_idx2 = tile_idx1+1; tile_idx2 < tiles_n && options[0].tile->area-tiles[tile_idx2].area <= defect_cur && (options_n_max < options_min || options_area_sum_max < square_area); tile_idx2++) {
				options_n_max++;
				options_area_sum_max += tiles[tile_idx2].area;
			}
			if (options_n_max >= options_min && options_area_sum_max >= square_area) {
				r = add_option(tile_idx1+1, options_n, same_tiles, all_squares && tiles[tile_idx1].height == tiles[tile_idx1].width, options_area_sum);
			}
		}
		else if (options_area_sum == square_area) {
			if (defect_a <= defect_b) {
				if (defect == defect_cur) {
					r = is_mondrian(options_n, defect);
				}
			}
			else {
				if (defect >= defect_b) {
					r = is_mondrian(options_n, defect);
					if (r == 1 && options_n > options_min) {
						tile_t *tiles_best_tmp = realloc(tiles_best, sizeof(tile_t)*(size_t)options_n);
						if (tiles_best_tmp) {
							tiles_best = tiles_best_tmp;
							options_min = options_n;
							tiles_best_n = options_n;
						}
						else {
							fprintf(stderr, "Could not reallocate memory for tiles_best\n");
							fflush(stderr);
							r = -1;
						}
					}
					if (r == 1) {
						int option_idx;
						for (option_idx = 0; option_idx < options_n; option_idx++) {
							tiles_best[option_idx] = *(options[option_idx].tile);
						}
						defect_cur = defect-1;
					}
				}
			}
		}
		options_area_sum -= tiles[tile_idx1].area;
		options_n--;
	}
	return r;
}

int is_mondrian(int options_n, int defect) {
	int option_idx, r;
	option_t *option;
	if (options_n < options_min) {
		return 0;
	}
	if (options_n+1 > dominances_max) {
		option_t *dominances_tmp = realloc(dominances, sizeof(option_t)*(size_t)(options_n+1));
		if (!dominances_tmp) {
			fprintf(stderr, "Could not reallocate memory for dominances\n");
			fflush(stderr);
			return -1;
		}
		dominances = dominances_tmp;
		dominances_max = options_n+1;
	}
	if (verbose_flag) {
		printf("is_mondrian options %d defect %d\n", options_n, defect);
		fflush(stdout);
	}
	options_y_header = options+options_n;
	link_options_y(options_y_header, options);
	for (option_idx = 0; option_idx < options_n; option_idx++) {
		link_options_y(options+option_idx, options+option_idx+1);
		set_dominance(dominances+option_idx);
	}
	set_dominance(dominances+options_n);
	dominances_n = 1;
	for (option = options_y_header->y_next; option != options_y_header; option = option->y_next) {
		int dominance_idx;
		for (dominance_idx = 1; dominance_idx < dominances_n && !is_dominated(option->tile, dominances[dominance_idx].d_last->tile); dominance_idx++);
		insert_dominance(option, dominances[dominance_idx].d_last, dominances+dominance_idx);
		if (dominance_idx == dominances_n) {
			dominances_n++;
		}
	}
	if (!mp_new(&y_cost)) {
		return -1;
	}
	r = search_y_slot(options_n, 0, defect, 1, 0, bars, options, 0, NULL);
	if (verbose_flag) {
		printf("search_y_slot ");
		mp_print("cost", &y_cost);
		fflush(stdout);
	}
	mp_free(&y_cost);
	return r;
}

void set_dominance(option_t *dominance) {
	dominance->tile = &tile_max;
	link_options_d(dominance, dominance);
}

void insert_dominance(option_t *option, option_t *last, option_t *next) {
	link_options_d(last, option);
	link_options_d(option, next);
}

int is_dominated(tile_t *tile_a, tile_t *tile_b) {
	return tile_a->height <= tile_b->height && tile_a->width <= tile_b->width;
}

int search_y_slot(int options_n, int options_hi, int defect, int bars_n, int bars_hi, bar_t *bar_start_prev, option_t *options_start, int rotate_sym_flag, option_t *option_sym) {
	int r, choices_n, option_idx;
	bar_t *bar_start;
	if (!mp_inc(&y_cost)) {
		return -1;
	}
	for (bar_start = bar_start_prev; bar_start != bars_header && bar_start->width == square_order; bar_start = bar_start->next) {
		bars_hi++;
	}
	if (bar_start != bars_header) {
		int dominance_idx;
		tile_t *tile;
		option_t *option;
		bar_t *bar;
		if (options_n-options_hi < bars_n-bars_hi) {
			return 0;
		}
		option = dominances->d_next;
		if (option != dominances) {
			tile = option->tile;
			if (rotate_sym_flag) {
				for (bar = bar_start; bar != bars_header && square_order-bar->width < tile->height; bar = bar->next);
				if (bar == bars_header || bar->y_slot > tile->y_w_slot_sym_max || square_order-bar->y_slot < tile->width) {
					if (is_square(tile)) {
						return 0;
					}
					for (; bar != bars_header && square_order-bar->width < tile->width; bar = bar->next);
					if (bar == bars_header || bar->y_slot > tile->y_h_slot_sym_max || square_order-bar->y_slot < tile->height) {
						return 0;
					}
				}
			}
			else {
				for (bar = bar_start; bar != bars_header && square_order-bar->width < tile->width; bar = bar->next);
				if (bar == bars_header || bar->y_slot > tile->y_h_slot_sym_max || square_order-bar->y_slot < tile->height) {
					return 0;
				}
			}
		}
		for (dominance_idx = 1; dominance_idx < dominances_n; dominance_idx++) {
			option = dominances[dominance_idx].d_next;
			if (option != dominances+dominance_idx) {
				tile = option->tile;
				if (rotate_flag) {
					for (bar = bar_start; bar != bars_header && square_order-bar->width < tile->height; bar = bar->next);
					if (bar == bars_header || square_order-bar->y_slot < tile->width) {
						if (is_square(tile)) {
							return 0;
						}
						for (; bar != bars_header && square_order-bar->width < tile->width; bar = bar->next);
						if (bar == bars_header || square_order-bar->y_slot < tile->height) {
							return 0;
						}
					}
				}
				else {
					for (bar = bar_start; bar != bars_header && square_order-bar->width < tile->width; bar = bar->next);
					if (bar == bars_header || square_order-bar->y_slot < tile->height) {
						return 0;
					}
				}
			}
		}
		for (bar = bar_start->next; bar != bars_header; bar = bar->next) {
			option = dominances->d_last;
			if (option != dominances) {
				tile = option->tile;
				if (((tile->y_h_slot_sym_max >= bar->y_slot && tile->height <= square_order-bar->y_slot && tile->width <= square_order-bar->width) || (rotate_sym_flag && tile->y_w_slot_sym_max >= bar->y_slot && tile->width <= square_order-bar->y_slot && tile->height <= square_order-bar->width))) {
					continue;
				}
			}
			for (dominance_idx = 1; dominance_idx < dominances_n; dominance_idx++) {
				option = dominances[dominance_idx].d_last;
				if (option != dominances+dominance_idx) {
					tile = option->tile;
					if (((tile->height <= square_order-bar->y_slot && tile->width <= square_order-bar->width) || (rotate_flag && tile->width <= square_order-bar->y_slot && tile->height <= square_order-bar->width))) {
						break;
					}
				}
			}
			if (dominance_idx == dominances_n) {
				return 0;
			}
		}
		r = -2;
		if (!option_sym) {
			if (rotate_flag) {
				for (option = options_y_header->y_next; option != options_y_header && r == -2; option = option->y_next) {
					tile = option->tile;
					if (tile->width >= square_order-bar_start->y_slot && (tile->height == square_order-bar_start->y_slot || tile->width == square_order-bar_start->width)) {
						r = choose_y_slot(options_n, options_hi, defect, bars_n, bars_hi, bar_start, rotate_sym_flag, option_sym, option, tile->height, tile->width);
					}
					if (r == -2 && tile->width >= square_order-bar_start->width && (tile->height == square_order-bar_start->width || tile->width == square_order-bar_start->y_slot)) {
						r = choose_y_slot(options_n, options_hi, defect, bars_n, bars_hi, bar_start, rotate_sym_flag, option_sym, option, tile->width, tile->height);
					}
				}
			}
			else {
				for (option = options_y_header->y_next; option != options_y_header && r == -2; option = option->y_next) {
					tile = option->tile;
					if (tile->height == square_order-bar_start->y_slot || tile->width == square_order-bar_start->width) {
						r = choose_y_slot(options_n, options_hi, defect, bars_n, bars_hi, bar_start, rotate_sym_flag, option_sym, option, tile->height, tile->width);
					}
				}
			}
		}
		if (r == -2) {
			option_t *option_sym_last, *option_sym_next;
			if (option_sym) {
				option_sym_last = NULL;
				option_sym_next = NULL;
			}
			else {
				rotate_sym_flag = bar_start->y_slot != bar_start->width;
				set_factor_sym(bar_start->y_slot, rotate_sym_flag, options->tile);
				option_sym = options;
				for (option = options->y_next; option != options_y_header; option = option->y_next) {
					set_factor_sym(bar_start->y_slot, rotate_sym_flag, option->tile);
					if (compare_factor_syms(option->tile, option_sym->tile) < 0) {
						option_sym = option;
					}
				}
				option_sym_last = option_sym->d_last;
				option_sym_next = option_sym->d_next;
				link_options_d(option_sym_last, option_sym_next);
				insert_dominance(option_sym, dominances->d_last, dominances);
			}
			if (bar_start != bar_start_prev || option_sym_last) {
				options_start = options_y_header->y_next;
			}
			r = 0;
			for (option = options_start; option != options_y_header && !r; option = option->y_next) {
				r = choose_y_slot(options_n, options_hi, defect, bars_n, bars_hi, bar_start, rotate_sym_flag, option_sym, option, option->tile->height, option->tile->width);
				if (rotate_flag && (option != option_sym || rotate_sym_flag) && !is_square(option->tile) && !r) {
					r = choose_y_slot(options_n, options_hi, defect, bars_n, bars_hi, bar_start, rotate_sym_flag, option_sym, option, option->tile->width, option->tile->height);
				}
			}
			if (option_sym_last) {
				link_options_d(dominances->d_last, dominances);
				insert_dominance(option_sym, option_sym_last, option_sym_next);
			}
		}
		return r;
	}
	options_x_out_header = options_y_header+1;
	link_options_x_out(options_x_out_header, options);
	for (option_idx = 0; option_idx < options_n-1; option_idx++) {
		link_options_x_out(options+option_idx, options+option_idx+1);
	}
	link_options_x_out(options+options_n-1, options_x_out_header);
	options_x_in_header = options_x_out_header+1;
	link_options_x_in(options_x_in_header, options_x_in_header);
	choices_n = 1+options_n*2;
	if (choices_n > choices_max) {
		choice_t *choices_tmp = realloc(choices, sizeof(choice_t)*(size_t)(choices_n+1));
		if (!choices_tmp) {
			fprintf(stderr, "Could not reallocate memory for choices\n");
			fflush(stderr);
			return -1;
		}
		choices = choices_tmp;
		choices_max = choices_n;
	}
	choices_header = choices+choices_n;
	link_choices(choices_header, choices);
	link_choices(choices, choices_header);
	if (!mp_new(&x_cost)) {
		return -1;
	}
	r = search_x_slot(choices, choices);
	if (r == 1) {
		printf("0 %d %d\n", square_order, options_n);
		for (option_idx = 0; option_idx < options_n; option_idx++) {
			print_option(options+option_idx);
		}
		printf("Defect %d\n", defect);
		fflush(stdout);
	}
	if (verbose_flag) {
		printf("search_x_slot ");
		mp_print("cost", &x_cost);
		fflush(stdout);
	}
	mp_free(&x_cost);
	return r;
}

int choose_y_slot(int options_n, int options_hi, int defect, int bars_n, int bars_hi, bar_t *bar_start, int rotate_sym_flag, option_t *option_sym, option_t *option, int slot_height, int slot_width) {
	int r = 0;
	if (slot_height <= square_order-bar_start->y_slot && slot_width <= square_order-bar_start->width) {
		bar_t *bar_cur, *bar_cur_last;
		option->y_slot = bar_start->y_slot;
		option->slot_height = slot_height;
		option->slot_width = slot_width;
		if (option == option_sym) {
			if (slot_width < slot_height) {
				option->x_slot_max = option->tile->x_h_slot_sym_max;
			}
			else {
				option->x_slot_max = option->tile->x_w_slot_sym_max;
			}
		}
		else {
			option->x_slot_max = square_order;
		}
		link_options_y(option->y_last, option->y_next);
		link_options_d(option->d_last, option->d_next);
		for (bar_cur = bar_start; bar_cur != bars_header && bar_cur->height <= slot_height; bar_cur = bar_cur->next) {
			slot_height -= bar_cur->height;
			bar_cur->width += slot_width;
		}
		bar_cur_last = bar_cur->last;
		if (slot_height > 0) {
			set_bar(bars+bars_n, bar_cur->y_slot, slot_height, bar_cur->width+slot_width);
			bar_cur->y_slot += slot_height;
			bar_cur->height -= slot_height;
			link_bars(bar_cur_last, bars+bars_n);
			link_bars(bars+bars_n, bar_cur);
			bars_n++;
			if (bar_cur != bar_start) {
				r = search_y_slot(options_n, options_hi+1, defect, bars_n, bars_hi, bar_start, option->y_next, rotate_sym_flag, option_sym);
			}
			else {
				r = search_y_slot(options_n, options_hi+1, defect, bars_n, bars_hi, bar_start->last, option->y_next, rotate_sym_flag, option_sym);
			}
		}
		else {
			r = search_y_slot(options_n, options_hi+1, defect, bars_n, bars_hi, bar_start, option->y_next, rotate_sym_flag, option_sym);
		}
		if (slot_height > 0) {
			bars_n--;
			link_bars(bar_cur_last, bar_cur);
			bar_cur->height += slot_height;
			bar_cur->y_slot -= slot_height;
		}
		for (bar_cur = bar_start; slot_height < option->slot_height; bar_cur = bar_cur->next) {
			bar_cur->width -= slot_width;
			slot_height += bar_cur->height;
		}
		option->d_next->d_last = option;
		option->d_last->d_next = option;
		option->y_next->y_last = option;
		option->y_last->y_next = option;
	}
	return r;
}

void set_tile(tile_t *tile, int height, int width) {
	tile->height = height;
	tile->width = width;
	tile->area = height*width;
}

int compare_tiles(const void *a, const void *b) {
	const tile_t *tile_a = (const tile_t *)a, *tile_b = (const tile_t *)b;
	if (tile_a->area != tile_b->area) {
		return tile_b->area-tile_a->area;
	}
	return tile_b->width-tile_a->width;
}

void set_factor_sym(int y_offset, int rotate_sym_flag, tile_t *tile) {
	int y_h_slots_sym = (square_order-y_offset-tile->height)/2+1, y_h_slots_n = square_order-y_offset-tile->height+1, y_w_slots_sym = (square_order-y_offset-tile->width)/2+1, x_h_slots_sym = (square_order-tile->height)/2+1, x_w_slots_sym = (square_order-tile->width)/2+1, x_w_slots_n = square_order-tile->width+1;
	tile->y_h_slot_sym_max = y_offset+y_h_slots_sym-1;
	tile->y_w_slot_sym_max = y_offset+y_w_slots_sym-1;
	tile->x_h_slot_sym_max = x_h_slots_sym-1;
	tile->x_w_slot_sym_max = x_w_slots_sym-1;
	if (rotate_flag && !is_square(tile)) {
		if (rotate_sym_flag) {
			int y_slots_sym = 0, y_slots_n = 0, x_slots_sym = 0, x_slots_n = 0;
			if (y_h_slots_sym > 0) {
				y_slots_sym += y_h_slots_sym;
				y_slots_n += y_h_slots_n;
			}
			if (y_w_slots_sym > 0) {
				y_slots_sym += y_w_slots_sym;
				y_slots_n += square_order-y_offset-tile->width+1;
			}
			if (x_h_slots_sym > 0) {
				x_slots_sym += x_h_slots_sym;
				x_slots_n += square_order-tile->height+1;
			}
			if (x_w_slots_sym > 0) {
				x_slots_sym += x_w_slots_sym;
				x_slots_n += x_w_slots_n;
			}
			tile->y_factor_sym = (double)y_slots_sym/y_slots_n;
			tile->x_factor_sym = (double)x_slots_sym/x_slots_n;
		}
		else {
			tile->y_factor_sym = (double)y_h_slots_sym/y_h_slots_n/2;
			tile->x_factor_sym = (double)x_w_slots_sym/x_w_slots_n;
		}
	}
	else {
		tile->y_factor_sym = (double)y_h_slots_sym/y_h_slots_n;
		tile->x_factor_sym = (double)x_w_slots_sym/x_w_slots_n;
	}
}

double compare_factor_syms(tile_t *tile_a, tile_t *tile_b) {
	if (tile_a->y_factor_sym != tile_b->y_factor_sym) {
		return tile_a->y_factor_sym-tile_b->y_factor_sym;
	}
	if (tile_a->x_factor_sym != tile_b->x_factor_sym) {
		return tile_a->x_factor_sym-tile_b->x_factor_sym;
	}
	return 0;
}

int search_x_slot(choice_t *choices_lo, choice_t *choices_hi) {
	int r;
	option_t *option;
	if (!mp_inc(&x_cost)) {
		return -1;
	}
	if (options_x_out_header->x_out_next == options_x_out_header) {
		return 1;
	}
	while (choices_lo != choices_header && !is_valid_choice(choices_lo->y_slot, choices_lo->x_slot)) {
		choices_lo = choices_lo->next;
	}
	if (choices_lo == choices_header) {
		return 0;
	}
	r = 0;
	for (option = options_x_out_header->x_out_next; option != options_x_out_header && !r; option = option->x_out_next) {
		if (option->y_slot == choices_lo->y_slot) {
			if (option->x_slot_max < choices_lo->x_slot) {
				break;
			}
			option->x_slot = choices_lo->x_slot;
			if (is_valid_slot(option)) {
				option_t *options_x_in_last;
				choice_t *choice_last, *choice_next;
				link_options_x_out(option->x_out_last, option->x_out_next);
				options_x_in_last = options_x_in_header->x_in_last;
				link_options_x_in(options_x_in_last, option);
				link_options_x_in(option, options_x_in_header);
				if (option->y_slot+option->slot_height < square_order) {
					choices_hi++;
					set_choice(choices_hi, option->y_slot+option->slot_height, option->x_slot);
					for (choice_last = choices_header->last; choice_last != choices_header && compare_choices(choices_hi, choice_last) < 0; choice_last = choice_last->last);
					insert_choice(choices_hi, choice_last, choice_last->next);
				}
				if (option->x_slot+option->slot_width < square_order) {
					choices_hi++;
					set_choice(choices_hi, option->y_slot, option->x_slot+option->slot_width);
					for (choice_next = choices_header->next; choice_next != choices_header && compare_choices(choices_hi, choice_next) > 0; choice_next = choice_next->next);
					insert_choice(choices_hi, choice_next->last, choice_next);
				}
				r = search_x_slot(choices_lo->next, choices_hi);
				if (option->x_slot+option->slot_width < square_order) {
					link_choices(choices_hi->last, choices_hi->next);
					choices_hi--;
				}
				if (option->y_slot+option->slot_height < square_order) {
					link_choices(choices_hi->last, choices_hi->next);
					choices_hi--;
				}
				link_options_x_in(options_x_in_last, options_x_in_header);
				option->x_out_next->x_out_last = option;
				option->x_out_last->x_out_next = option;
			}
		}
	}
	return r;
}

int is_valid_choice(int y_slot, int x_slot) {
	option_t *option_in;
	for (option_in = options_x_in_header->x_in_next; option_in != options_x_in_header; option_in = option_in->x_in_next) {
		if (y_slot >= option_in->y_slot && y_slot < option_in->y_slot+option_in->slot_height && x_slot >= option_in->x_slot && x_slot < option_in->x_slot+option_in->slot_width) {
			return 0;
		}
	}
	return 1;
}

int is_valid_slot(option_t *option_out) {
	option_t *option_in;
	if (option_out->x_slot+option_out->slot_width > square_order) {
		return 0;
	}
	for (option_in = options_x_in_header->x_in_next; option_in != options_x_in_header; option_in = option_in->x_in_next) {
		if (edges_overlap(option_out->y_slot, option_out->y_slot+option_out->slot_height-1, option_in->y_slot, option_in->y_slot+option_in->slot_height-1) && edges_overlap(option_out->x_slot, option_out->x_slot+option_out->slot_width-1, option_in->x_slot, option_in->x_slot+option_in->slot_width-1)) {
			return 0;
		}
	}
	return 1;
}

int edges_overlap(int a1, int a2, int b1, int b2) {
	return (a1 >= b1 && a1 <= b2) || (a2 >= b1 && a2 <= b2) || (b1 >= a1 && b1 <= a2) || (b2 >= a1 && b2 <= a2) || (a1 < b1 && a2 > b2) || (b1 < a1 && b2 > a2);
}

void link_options_y(option_t *last, option_t *next) {
	last->y_next = next;
	next->y_last = last;
}

void link_options_d(option_t *last, option_t *next) {
	last->d_next = next;
	next->d_last = last;
}

void link_options_x_out(option_t *last, option_t *next) {
	last->x_out_next = next;
	next->x_out_last = last;
}

void link_options_x_in(option_t *last, option_t *next) {
	last->x_in_next = next;
	next->x_in_last = last;
}

void print_option(option_t *option) {
	printf("%dx%d;%dx%d\n", option->y_slot, option->x_slot, option->slot_height, option->slot_width);
}

void set_bar(bar_t *bar, int y_slot, int height, int width) {
	bar->y_slot = y_slot;
	bar->height = height;
	bar->width = width;
}

void link_bars(bar_t *last, bar_t *next) {
	last->next = next;
	next->last = last;
}

void set_choice(choice_t *choice, int y_slot, int x_slot) {
	choice->y_slot = y_slot;
	choice->x_slot = x_slot;
}

int compare_choices(choice_t *choice_a, choice_t *choice_b) {
	if (choice_a->y_slot != choice_b->y_slot) {
		return choice_a->y_slot-choice_b->y_slot;
	}
	if (choice_a->x_slot != choice_b->x_slot) {
		return choice_a->x_slot-choice_b->x_slot;
	}
	if (choice_a < choice_b) {
		return -1;
	}
	return 1;
}

void insert_choice(choice_t *choice, choice_t *last, choice_t *next) {
	link_choices(last, choice);
	link_choices(choice, next);
}

void link_choices(choice_t *last, choice_t *next) {
	last->next = next;
	next->last = last;
}

int is_square(tile_t *tile) {
	return tile->height == tile->width;
}

void free_data(void) {
	free(tiles_best);
	free(choices);
	free(dominances);
	free(bars);
	free(options);
	free(tiles);
	free(counts);
}
