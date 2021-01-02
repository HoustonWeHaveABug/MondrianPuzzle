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
	int y_slots_sym;
	double y_factor_sym;
	int x_slots_sym;
	double x_factor_sym;
}
tile_t;

typedef struct option_s option_t;

struct option_s {
	tile_t *tile;
	int y_slot;
	int slot_height;
	int slot_width;
	int slot_start;
	option_t *h_last;
	option_t *h_next;
	option_t *d_last;
	option_t *d_next;
};

typedef struct bar_s bar_t;

struct bar_s {
	int y_slot;
	int height;
	int width;
	bar_t *last;
	bar_t *next;
};

typedef struct {
	int slot_start;
	option_t *option;
}
choice_t;

typedef struct node_s node_t;

struct node_s {
	union {
		int rows_n;
		node_t *column;
	};
	choice_t *choice;
	node_t *left;
	node_t *right;
	node_t *top;
	node_t *bottom;
};

void set_bar(bar_t *, int, int, int);
void link_bar(bar_t *, bar_t *);
int set_tiles(void);
void add_tiles(int);
int best_defect(int);
void set_tile(tile_t *, int, int);
int add_option(int, int, int, int, int);
int is_mondrian(int, int);
void link_option(option_t *, option_t *);
void set_dominance(option_t *);
int is_dominated(tile_t *, tile_t *);
void insert_dominance(option_t *, option_t *, option_t *);
int search_y_slot(int, int, int, int, int, bar_t *, option_t *);
int choose_y_slot(int, int, int, int, int, bar_t *, option_t *, int, int);
void set_column_node(node_t *, node_t *);
void set_option_row_nodes(option_t *, int);
void set_slot_row_nodes(int, int, int, int, int);
void set_row_node(int, node_t *);
void link_left(node_t *, node_t *);
void link_top(node_t *, node_t *);
int is_unique_column(node_t *);
int compare_columns(node_t *, node_t *);
void remove_column(node_t *);
void print_option(option_t *);
int search_x_slot(void);
int set_column_value(node_t *);
int set_row_value(node_t *);
void assign_choice(choice_t *);
void cover_column(node_t *);
node_t *cover_rows(node_t *);
void uncover_rows(node_t *, node_t *);
void uncover_column(node_t *);
int is_square(tile_t *);
int compare_tiles(const void *, const void *);
void free_data(void);

int square_order, rotate_flag, defect_a, defect_b, options_min, verbose_flag, square_area, *counts, tiles_max, options_max, dominances_max, choices_max, nodes_max, tops_max, tiles_best_n, defect_cur, tiles_n, dominances_n;
mp_t y_cost, x_cost;
bar_t *bars, *bars_header;
tile_t *tiles, tile_max, *tiles_best;
option_t *options, *options_header, *option_sym, *dominances;
choice_t *choices, *choice_cur;
node_t *nodes, **tops, *columns_header, *row_node;

int main(void) {
	int r;
	if (scanf("%d%d%d%d%d%d", &square_order, &rotate_flag, &defect_a, &defect_b, &options_min, &verbose_flag) != 6 || square_order < SQUARE_ORDER_MIN || square_order > INT_MAX/square_order || defect_a < 0 || defect_b < 0 || options_min < TILES_MIN) {
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
	options = malloc(sizeof(option_t)*(size_t)(options_min+1));
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
	link_bar(bars, bars_header);
	link_bar(bars_header, bars);
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
	choices_max = 1;
	nodes = malloc(sizeof(node_t));
	if (!nodes) {
		fprintf(stderr, "Could not allocate memory for nodes\n");
		fflush(stderr);
		free(choices);
		free(dominances);
		free(bars);
		free(options);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	nodes_max = 1;
	tops = malloc(sizeof(node_t *));
	if (!tops) {
		fprintf(stderr, "Could not allocate memory for tops\n");
		fflush(stderr);
		free(nodes);
		free(choices);
		free(dominances);
		free(bars);
		free(options);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	tops_max = 1;
	tiles_best = malloc(sizeof(tile_t)*(size_t)options_min);
	if (!tiles_best) {
		fprintf(stderr, "Could not allocate memory for tiles_best\n");
		fflush(stderr);
		free(tops);
		free(nodes);
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

void set_bar(bar_t *bar, int y_slot, int height, int width) {
	bar->y_slot = y_slot;
	bar->height = height;
	bar->width = width;
}

void link_bar(bar_t *bar, bar_t *last) {
	bar->last = last;
	last->next = bar;
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
	if (!rotate_flag) {
		for (width = 1; width < square_order; width++) {
			area = square_order*width;
			if (width > square_order-width) {
				if (area-square_order*(square_order-width) <= defect_cur && best_defect(area) <= defect_cur) {
					counts[area-1]++;
				}
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

int best_defect(int area) {
	int options_n = square_area/area, area_mod, best, others_n, others_mod, defect;
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
	others_mod = area_mod%others_n;
	if (others_mod > 0) {
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

void set_tile(tile_t *tile, int height, int width) {
	tile->height = height;
	tile->width = width;
	tile->area = height*width;
	tile->y_slots_sym = (square_order-height)/2+1;
	tile->y_factor_sym = (square_order-height+1)/(double)tile->y_slots_sym;
	tile->x_slots_sym = (square_order-width)/2+1;
	tile->x_factor_sym = (square_order-width+1)/(double)tile->x_slots_sym;
}

int add_option(int tiles_start, int options_n, int same_tiles, int all_squares, int options_area_sum) {
	int r, tile_idx1;
	if (options_n == options_max) {
		option_t *options_tmp = realloc(options, sizeof(option_t)*(size_t)(options_n+2));
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
	if (options_n > dominances_max) {
		option_t *dominances_tmp = realloc(dominances, sizeof(option_t)*(size_t)options_n);
		if (!dominances_tmp) {
			fprintf(stderr, "Could not reallocate memory for dominances\n");
			fflush(stderr);
			return -1;
		}
		dominances = dominances_tmp;
		dominances_max = options_n;
	}
	if (verbose_flag) {
		printf("is_mondrian options %d defect %d\n", options_n, defect);
		fflush(stdout);
	}
	options_header = options+options_n;
	link_option(options, options_header);
	for (option_idx = 0; option_idx < options_n; option_idx++) {
		link_option(options+option_idx+1, options+option_idx);
		set_dominance(dominances+option_idx);
	}
	option_sym = options;
	for (option = options->h_next; option != options_header; option = option->h_next) {
		if (option->tile->y_factor_sym > option_sym->tile->y_factor_sym || (option->tile->y_factor_sym == option_sym->tile->y_factor_sym && option->tile->x_factor_sym > option_sym->tile->x_factor_sym)) {
			option_sym = option;
		}
	}
	insert_dominance(option_sym, dominances->d_last, dominances);
	dominances_n = 1;
	for (option = options_header->h_next; option != options_header; option = option->h_next) {
		if (option != option_sym) {
			int dominance_idx;
			for (dominance_idx = 1; dominance_idx < dominances_n && !is_dominated(option->tile, dominances[dominance_idx].d_last->tile); dominance_idx++);
			insert_dominance(option, dominances[dominance_idx].d_last, dominances+dominance_idx);
			if (dominance_idx == dominances_n) {
				dominances_n++;
			}
		}
	}
	if (!mp_new(&y_cost)) {
		return -1;
	}
	r = search_y_slot(options_n, 0, defect, 1, 0, bars, options);
	if (verbose_flag) {
		printf("search_y_slot ");
		mp_print("cost", &y_cost);
		fflush(stdout);
	}
	mp_free(&y_cost);
	return r;
}

void link_option(option_t *option, option_t *last) {
	option->h_last = last;
	last->h_next = option;
}

void set_dominance(option_t *dominance) {
	dominance->tile = &tile_max;
	dominance->d_last = dominance;
	dominance->d_next = dominance;
}

int is_dominated(tile_t *tile_a, tile_t *tile_b) {
	return tile_a->height <= tile_b->height && tile_a->width <= tile_b->width;
}

void insert_dominance(option_t *option, option_t *last, option_t *next) {
	option->d_last = last;
	last->d_next = option;
	option->d_next = next;
	next->d_last = option;
}

int search_y_slot(int options_n, int options_hi, int defect, int bars_n, int bars_hi, bar_t *bar_start_prev, option_t *options_start) {
	int dominance_idx, r;
	option_t *option;
	bar_t *bar_start, *bar;
	if (!mp_inc(&y_cost)) {
		return -1;
	}
	for (bar_start = bar_start_prev; bar_start != bars_header && bar_start->width == square_order; bar_start = bar_start->next) {
		bars_hi++;
	}
	if (bar_start == bars_header) {
		int choices_n = 0, tops_n = square_area+options_n, nodes_n = tops_n+1, option_idx, node_idx, removed;
		for (option_idx = 0; option_idx < options_n; option_idx++) {
			int x_slots_n;
			option = options+option_idx;
			if (option != option_sym) {
				x_slots_n = square_order-option->slot_width+1;
			}
			else {
				if (!is_square(option->tile)) {
					x_slots_n = option->tile->x_slots_sym;
				}
				else {
					x_slots_n = option->tile->x_slots_sym-option->y_slot;
				}
			}
			choices_n += x_slots_n;
			if (option->tile->area+1 > INT_MAX/x_slots_n || (option->tile->area+1)*x_slots_n > INT_MAX-nodes_n) {
				fprintf(stderr, "Too many nodes\n");
				fflush(stderr);
				return -1;
			}
			nodes_n += (option->tile->area+1)*x_slots_n;
			if (sizeof(node_t) > (size_t)-1/(size_t)nodes_n) {
				fprintf(stderr, "Will not be able to reallocate memory for nodes\n");
				fflush(stderr);
				return -1;
			}
		}
		if (choices_n > choices_max) {
			choice_t *choices_tmp = realloc(choices, sizeof(choice_t)*(size_t)choices_n);
			if (!choices_tmp) {
				fprintf(stderr, "Could not reallocate memory for choices\n");
				fflush(stderr);
				return -1;
			}
			choices = choices_tmp;
			choices_max = choices_n;
		}
		if (nodes_n > nodes_max) {
			node_t *nodes_tmp = realloc(nodes, sizeof(node_t)*(size_t)nodes_n);
			if (!nodes_tmp) {
				fprintf(stderr, "Could not reallocate memory for nodes\n");
				fflush(stderr);
				return -1;
			}
			nodes = nodes_tmp;
			nodes_max = nodes_n;
		}
		if (tops_n > tops_max) {
			node_t **tops_tmp = realloc(tops, sizeof(node_t *)*(size_t)tops_n);
			if (!tops_tmp) {
				fprintf(stderr, "Could not reallocate memory for tops\n");
				fflush(stderr);
				return -1;
			}
			tops = tops_tmp;
			tops_max = tops_n;
		}
		columns_header = nodes+tops_n;
		set_column_node(nodes, columns_header);
		for (node_idx = 0; node_idx < tops_n; node_idx++) {
			set_column_node(nodes+node_idx+1, nodes+node_idx);
			tops[node_idx] = nodes+node_idx;
		}
		choice_cur = choices;
		row_node = columns_header+1;
		for (option_idx = 0; option_idx < options_n; option_idx++) {
			set_option_row_nodes(options+option_idx, option_idx);
		}
		for (node_idx = 0; node_idx < tops_n; node_idx++) {
			link_top(nodes+node_idx, tops[node_idx]);
		}
		removed = 0;
		for (node_idx = 1; node_idx < square_area; node_idx++) {
			if (!is_unique_column(nodes+node_idx)) {
				remove_column(nodes+node_idx);
				removed++;
			}
		}
		if (verbose_flag) {
			printf("search_x_slot columns %d removed %d choices %d\n", tops_n, removed, choices_n);
			fflush(stdout);
		}
		if (!mp_new(&x_cost)) {
			return -1;
		}
		r = search_x_slot();
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
	if (options_n-options_hi < bars_n-bars_hi) {
		return 0;
	}
	option = dominances->d_next;
	if (option != dominances) {
		for (bar = bar_start; bar != bars_header && square_order-bar->width < option->tile->width; bar = bar->next);
		if (bar == bars_header || bar->y_slot >= option->tile->y_slots_sym || square_order-bar->y_slot < option->tile->height) {
			return 0;
		}
	}
	for (dominance_idx = 1; dominance_idx < dominances_n; dominance_idx++) {
		option = dominances[dominance_idx].d_next;
		if (option != dominances+dominance_idx) {
			if (rotate_flag) {
				for (bar = bar_start; bar != bars_header && square_order-bar->width < option->tile->height; bar = bar->next);
				if (bar == bars_header || square_order-bar->y_slot < option->tile->width) {
					if (is_square(option->tile)) {
						return 0;
					}
					for (; bar != bars_header && square_order-bar->width < option->tile->width; bar = bar->next);
					if (bar == bars_header || square_order-bar->y_slot < option->tile->height) {
						return 0;
					}
				}
			}
			else {
				for (bar = bar_start; bar != bars_header && square_order-bar->width < option->tile->width; bar = bar->next);
				if (bar == bars_header || square_order-bar->y_slot < option->tile->height) {
					return 0;
				}
			}
		}
	}
	for (bar = bar_start->next; bar != bars_header; bar = bar->next) {
		option = dominances->d_last;
		if (option != dominances && bar->y_slot < option->tile->y_slots_sym && option->tile->height <= square_order-bar_start->y_slot && option->tile->width <= square_order-bar_start->width) {
			continue;
		}
		for (dominance_idx = 1; dominance_idx < dominances_n; dominance_idx++) {
			option = dominances[dominance_idx].d_last;
			if (option != dominances+dominance_idx && ((option->tile->height <= square_order-bar->y_slot && option->tile->width <= square_order-bar->width) || (rotate_flag && option->tile->width <= square_order-bar->y_slot && option->tile->height <= square_order-bar->width))) {
				break;
			}
		}
		if (dominance_idx == dominances_n) {
			return 0;
		}
	}
	if (bar_start != bar_start_prev) {
		options_start = options_header->h_next;
	}
	r = 0;
	for (option = options_start; option != options_header && !r; option = option->h_next) {
		r = choose_y_slot(options_n, options_hi, defect, bars_n, bars_hi, bar_start, option, option->tile->height, option->tile->width);
		if (rotate_flag && option != option_sym && !is_square(option->tile) && !r) {
			r = choose_y_slot(options_n, options_hi, defect, bars_n, bars_hi, bar_start, option, option->tile->width, option->tile->height);
		}
	}
	return r;
}

int choose_y_slot(int options_n, int options_hi, int defect, int bars_n, int bars_hi, bar_t *bar_start, option_t *option, int slot_height, int slot_width) {
	int r = 0;
	if (slot_height <= square_order-bar_start->y_slot && slot_width <= square_order-bar_start->width) {
		int bar_height;
		bar_t *bar_cur, *bar_start_last, *bar_cur_last;
		option->y_slot = bar_start->y_slot;
		option->slot_height = slot_height;
		option->slot_width = slot_width;
		option->h_last->h_next = option->h_next;
		option->h_next->h_last = option->h_last;
		option->d_last->d_next = option->d_next;
		option->d_next->d_last = option->d_last;
		bar_height = slot_height;
		for (bar_cur = bar_start; bar_cur != bars_header && bar_cur->height <= bar_height; bar_cur = bar_cur->next) {
			bar_height -= bar_cur->height;
			bar_cur->width += slot_width;
		}
		bar_start_last = bar_start->last;
		bar_cur_last = bar_cur->last;
		if (bar_height > 0) {
			set_bar(bars+bars_n, bar_cur->y_slot, bar_height, bar_cur->width+slot_width);
			bar_cur->y_slot += bar_height;
			bar_cur->height -= bar_height;
			bar_cur_last->next = bars+bars_n;
			bars[bars_n].last = bar_cur_last;
			bars[bars_n].next = bar_cur;
			bar_cur->last = bars+bars_n;
			bars_n++;
		}
		if (bar_start->last == bar_start_last) {
			r = search_y_slot(options_n, options_hi+1, defect, bars_n, bars_hi, bar_start, option->h_next);
		}
		else {
			r = search_y_slot(options_n, options_hi+1, defect, bars_n, bars_hi, bar_start->last, option->h_next);
		}
		if (bar_height > 0) {
			bars_n--;
			bar_cur->last = bar_cur_last;
			bar_cur_last->next = bar_cur;
			bar_cur->height += bar_height;
			bar_cur->y_slot -= bar_height;
		}
		for (bar_cur = bar_start; bar_cur != bars_header && bar_height < slot_height; bar_cur = bar_cur->next) {
			bar_cur->width -= slot_width;
			bar_height += bar_cur->height;
		}
		option->d_next->d_last = option;
		option->d_last->d_next = option;
		option->h_next->h_last = option;
		option->h_last->h_next = option;
	}
	return r;
}

void set_column_node(node_t *node, node_t *left) {
	node->rows_n = 0;
	node->choice = NULL;
	link_left(node, left);
}

void set_option_row_nodes(option_t *option, int option_idx) {
	int x_slot_start, x_slots_n, x_slot_idx;
	if (option != option_sym) {
		x_slot_start = 0;
		x_slots_n = square_order-option->slot_width+1;
	}
	else {
		if (!is_square(option->tile)) {
			x_slot_start = 0;
		}
		else {
			x_slot_start = option->y_slot;
		}
		x_slots_n = option->tile->x_slots_sym;
	}
	for (x_slot_idx = x_slot_start; x_slot_idx < x_slots_n; x_slot_idx++) {
		set_slot_row_nodes(option->y_slot*square_order+x_slot_idx, option->slot_height, option->slot_width, option->tile->area, option_idx);
	}
}

void set_slot_row_nodes(int slot_start, int slot_height, int slot_width, int slot_area, int option_idx) {
	int x, y;
	if (rotate_flag && slot_height <= slot_width) {
		choice_cur->slot_start = slot_start;
	}
	else {
		choice_cur->slot_start = square_area+slot_start;
	}
	choice_cur->option = options+option_idx;
	set_row_node(slot_start, row_node+slot_area);
	for (x = 1; x < slot_width; x++) {
		set_row_node(slot_start+x, row_node-1);
	}
	for (y = 1; y < slot_height; y++) {
		for (x = 0; x < slot_width; x++) {
			set_row_node(slot_start+y*square_order+x, row_node-1);
		}
	}
	set_row_node(square_area+option_idx, row_node-1);
	choice_cur++;
}

void set_row_node(int column, node_t *left) {
	row_node->column = nodes+column;
	row_node->choice = choice_cur;
	link_left(row_node, left);
	link_top(row_node, tops[column]);
	tops[column] = row_node++;
	nodes[column].rows_n++;
}

void link_left(node_t *node, node_t *left) {
	node->left = left;
	left->right = node;
}

void link_top(node_t *node, node_t *top) {
	node->top = top;
	top->bottom = node;
}

int is_unique_column(node_t *column) {
	node_t *node;
	for (node = column->left; node != columns_header && compare_columns(column, node); node = node->left);
	return node == columns_header;
}

int compare_columns(node_t *column_a, node_t *column_b) {
	node_t *row_a, *row_b;
	if (column_a->rows_n != column_b->rows_n) {
		return 1;
	}
	for (row_a = column_a->bottom, row_b = column_b->bottom; row_a != column_a && row_a->choice == row_b->choice; row_a = row_a->bottom, row_b = row_b->bottom);
	return row_a != column_a;
}

void remove_column(node_t *column) {
	node_t *row;
	column->right->left = column->left;
	column->left->right = column->right;
	for (row = column->bottom; row != column; row = row->bottom) {
		row->right->left = row->left;
		row->left->right = row->right;
	}
}

void print_option(option_t *option) {
	if (option->slot_start < square_area) {
		printf("%dx%d;%dx%d\n", option->slot_start/square_order, option->slot_start%square_order, option->tile->height, option->tile->width);
	}
	else {
		printf("%dx%d;%dx%d\n", (option->slot_start-square_area)/square_order, (option->slot_start-square_area)%square_order, option->tile->width, option->tile->height);
	}
}

int search_x_slot(void) {
	int mins_n, r;
	node_t *column_min, *column, *row;
	if (!mp_inc(&x_cost)) {
		return -1;
	}
	if (columns_header->right == columns_header) {
		return 1;
	}
	column_min = columns_header->right;
	mins_n = 1;
	for (column = column_min->right; column != columns_header; column = column->right) {
		if (column->rows_n < column_min->rows_n) {
			column_min = column;
			mins_n = 1;
		}
		else if (column->rows_n == column_min->rows_n) {
			mins_n++;
		}
	}
	if (mins_n > 1) {
		int value_max = set_column_value(column_min);
		for (column = column_min->right; column != columns_header; column = column->right) {
			if (column->rows_n == column_min->rows_n) {
				int value = set_column_value(column);
				if (value > value_max) {
					column_min = column;
					value_max = value;
				}
			}
		}
	}
	r = 0;
	cover_column(column_min);
	cover_rows(column_min);
	for (row = column_min->bottom; row != column_min && !r; row = row->bottom) {
		node_t *node;
		for (node = row->right; node != row; node = node->right) {
			cover_column(node->column);
		}
		for (node = row->right; node != row; node = node->right) {
			node_t *stop = cover_rows(node->column);
			if (stop != node->column) {
				uncover_rows(node->column, stop);
				break;
			}
		}
		if (node == row) {
			assign_choice(row->choice);
			r = search_x_slot();
		}
		for (node = node->left; node != row; node = node->left) {
			uncover_rows(node->column, node->column->top);
		}
		for (node = row->left; node != row; node = node->left) {
			uncover_column(node->column);
		}
	}
	uncover_rows(column_min, column_min->top);
	uncover_column(column_min);
	return r;
}

int set_column_value(node_t *column) {
	int value = 0;
	node_t *row;
	for (row = column->bottom; row != column; row = row->bottom) {
		value += set_row_value(row);
	}
	return value;
}

int set_row_value(node_t *row) {
	int value = 0;
	node_t *node;
	for (node = row->right; node != row; node = node->right) {
		value += node->column->rows_n;
	}
	return value;
}

void assign_choice(choice_t *choice) {
	choice->option->slot_start = choice->slot_start;
}

void cover_column(node_t *column) {
	column->choice = choices;
	column->right->left = column->left;
	column->left->right = column->right;
}

node_t *cover_rows(node_t *column) {
	node_t *row;
	for (row = column->bottom; row != column; row = row->bottom) {
		int stop = 0;
		node_t *node;
		for (node = row->right; node != row; node = node->right) {
			node->column->rows_n--;
			if (!node->column->choice && node->column->rows_n == 0) {
				stop++;
			}
			node->bottom->top = node->top;
			node->top->bottom = node->bottom;
		}
		if (stop) {
			break;
		}
	}
	return row;
}

void uncover_rows(node_t *column, node_t *start) {
	node_t *row;
	for (row = start; row != column; row = row->top) {
		node_t *node;
		for (node = row->left; node != row; node = node->left) {
			node->top->bottom = node;
			node->bottom->top = node;
			node->column->rows_n++;
		}
	}
}

void uncover_column(node_t *column) {
	column->left->right = column;
	column->right->left = column;
	column->choice = NULL;
}

int is_square(tile_t *tile) {
	return tile->height == tile->width;
}

int compare_tiles(const void *a, const void *b) {
	const tile_t *tile_a = (const tile_t *)a, *tile_b = (const tile_t *)b;
	if (tile_a->area != tile_b->area) {
		return tile_b->area-tile_a->area;
	}
	return tile_b->width-tile_a->width;
}

void free_data(void) {
	free(tiles_best);
	free(tops);
	free(nodes);
	free(choices);
	free(dominances);
	free(bars);
	free(options);
	free(tiles);
	free(counts);
}
