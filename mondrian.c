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
	option_t *last;
	option_t *next;
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

int set_tiles(void);
void add_tiles(int);
int best_defect(int);
void set_tile(tile_t *, int, int);
int compare_tiles(const void *, const void *);
int add_option(int, int, int);
void set_option(option_t *, tile_t *);
int is_mondrian(int, int);
void link_last(option_t *, option_t *);
int search_y_slot(int, int, int, option_t *);
int choose_y_slot(int, int, int, option_t *, int, int);
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
void free_data(void);

int square_order, rotate_flag, defect_a, defect_b, options_min, verbose_flag, square_area, *counts, tiles_max, *widths, choices_max, nodes_max, column_nodes_max, defect_cur, tiles_n;
mp_t y_cost, x_cost;
tile_t *tiles;
option_t *options, *options_header, *option_sym;
choice_t *choices, *choice_cur;
node_t *nodes, **tops, *columns_header, *row_node;

int main(void) {
	if (scanf("%d%d%d%d%d%d", &square_order, &rotate_flag, &defect_a, &defect_b, &options_min, &verbose_flag) != 6 || square_order < SQUARE_ORDER_MIN || square_order > INT_MAX/square_order || defect_a < 0 || defect_b < 0 || options_min < 0) {
		fprintf(stderr, "Invalid parameters\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	square_area = square_order*square_order;
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
	tiles_max = 1;
	options = malloc(sizeof(option_t)*(size_t)(tiles_max+1));
	if (!options) {
		fprintf(stderr, "Could not allocate memory for options\n");
		fflush(stderr);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	widths = calloc((size_t)square_order, sizeof(int));
	if (!widths) {
		fprintf(stderr, "Could not allocate memory for widths\n");
		fflush(stderr);
		free(options);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	choices = malloc(sizeof(choice_t));
	if (!choices) {
		fprintf(stderr, "Could not allocate memory for choices\n");
		fflush(stderr);
		free(widths);
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
		free(widths);
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
		free(widths);
		free(options);
		free(tiles);
		free(counts);
		return EXIT_FAILURE;
	}
	column_nodes_max = 1;
	defect_cur = defect_a;
	if (defect_a <= defect_b) {
		int r;
		do {
			if (!set_tiles()) {
				free_data();
				return EXIT_FAILURE;
			}
			if (tiles_n >= TILES_MIN) {
				r = add_option(0, 0, 0);
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
		if (!set_tiles()) {
			free_data();
			return EXIT_FAILURE;
		}
		if (tiles_n >= TILES_MIN) {
			add_option(0, 0, 0);
		}
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
		option_t *options_tmp;
		if (!tiles_tmp) {
			fprintf(stderr, "Could not reallocate memory for tiles\n");
			fflush(stderr);
			return 0;
		}
		tiles = tiles_tmp;
		tiles_max = tiles_n;
		options_tmp = realloc(options, sizeof(option_t)*(size_t)(tiles_max+1));
		if (!options_tmp) {
			fprintf(stderr, "Could not reallocate memory for options\n");
			fflush(stderr);
			return 0;
		}
		options = options_tmp;
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
	int area_mod = square_area%area, best, others_n, others_mod, defect;
	if (area_mod == 0) {
		return 0;
	}
	best = area-area_mod;
	others_n = square_area/area-1;
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

int compare_tiles(const void *a, const void *b) {
	const tile_t *tile_a = (const tile_t *)a, *tile_b = (const tile_t *)b;
	if (tile_a->area != tile_b->area) {
		return tile_b->area-tile_a->area;
	}
	return tile_b->width-tile_a->width;
}

int add_option(int tiles_start, int options_n, int options_area_sum) {
	int r = 0, tile_idx1;
	for (tile_idx1 = tiles_start; tile_idx1 < tiles_n && !r; tile_idx1++) {
		int defect;
		if (options_n == 0 && tiles[tile_idx1].height > tiles[tile_idx1].width) {
			continue;
		}
		set_option(options+options_n, tiles+tile_idx1);
		defect = options[0].tile->area-tiles[tile_idx1].area;
		if (defect > defect_cur) {
			break;
		}
		options_area_sum += tiles[tile_idx1].area;
		if (options_area_sum < square_area) {
			int options_area_max = options_area_sum, tile_idx2;
			for (tile_idx2 = tile_idx1+1; tile_idx2 < tiles_n && options[0].tile->area-tiles[tile_idx2].area <= defect_cur && options_area_max < square_area; tile_idx2++) {
				options_area_max += tiles[tile_idx2].area;
			}
			if (options_area_max >= square_area) {
				r = add_option(tile_idx1+1, options_n+1, options_area_sum);
			}
		}
		else if (options_area_sum == square_area) {
			if (defect_a <= defect_b) {
				if (defect == defect_cur) {
					r = is_mondrian(options_n+1, defect);
				}
			}
			else {
				r = is_mondrian(options_n+1, defect);
				if (r == 1) {
					defect_cur = defect-1;
					r = defect_cur < defect_b;
				}
			}
		}
		options_area_sum -= tiles[tile_idx1].area;
	}
	return r;
}

void set_option(option_t *option, tile_t *tile) {
	option->tile = tile;
	option->y_slot = square_order;
}

int is_mondrian(int options_n, int defect) {
	int option_idx, r;
	option_t *option;
	if (options_n < options_min) {
		return 0;
	}
	if (verbose_flag) {
		printf("is_mondrian options %d defect %d\n", options_n, defect);
		fflush(stdout);
	}
	options_header = options+options_n;
	link_last(options, options_header);
	for (option_idx = 0; option_idx < options_n; option_idx++) {
		link_last(options+option_idx+1, options+option_idx);
	}
	option_sym = options;
	for (option = options->next; option != options_header; option = option->next) {
		if (option->tile->y_factor_sym > option_sym->tile->y_factor_sym || (option->tile->y_factor_sym == option_sym->tile->y_factor_sym && option->tile->x_factor_sym > option_sym->tile->x_factor_sym)) {
			option_sym = option;
		}
	}
	if (!mp_new(&y_cost)) {
		return -1;
	}
	r = search_y_slot(options_n, defect, 0, options);
	if (verbose_flag) {
		printf("search_y_slot ");
		mp_print("cost", &y_cost);
		fflush(stdout);
	}
	mp_free(&y_cost);
	return r;
}

void link_last(option_t *option, option_t *last) {
	option->last = last;
	last->next = option;
}

int search_y_slot(int options_n, int defect, int y_last, option_t *options_start) {
	int y_start, r;
	option_t *option;
	if (!mp_inc(&y_cost)) {
		return -1;
	}
	for (y_start = y_last; y_start < square_order && widths[y_start] == square_order; y_start++);
	if (y_start == square_order) {
		int choices_n = 0, column_nodes_n = square_area+options_n, nodes_n = column_nodes_n+1, option_idx, node_idx, removed;
		for (option_idx = 0; option_idx < options_n; option_idx++) {
			int x_slots_n;
			if (options+option_idx != option_sym) {
				x_slots_n = square_order-options[option_idx].slot_width+1;
			}
			else {
				if (!is_square(options[option_idx].tile)) {
					x_slots_n = options[option_idx].tile->x_slots_sym;
				}
				else {
					x_slots_n = options[option_idx].tile->x_slots_sym-options[option_idx].y_slot;
				}
			}
			choices_n += x_slots_n;
			nodes_n += (options[option_idx].tile->area+1)*x_slots_n;
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
		if (column_nodes_n > column_nodes_max) {
			node_t **tops_tmp = realloc(tops, sizeof(node_t *)*(size_t)column_nodes_n);
			if (!tops_tmp) {
				fprintf(stderr, "Could not reallocate memory for tops\n");
				fflush(stderr);
				return -1;
			}
			tops = tops_tmp;
			column_nodes_max = column_nodes_n;
		}
		columns_header = nodes+column_nodes_n;
		set_column_node(nodes, columns_header);
		for (node_idx = 0; node_idx < column_nodes_n; node_idx++) {
			set_column_node(nodes+node_idx+1, nodes+node_idx);
			tops[node_idx] = nodes+node_idx;
		}
		choice_cur = choices;
		row_node = columns_header+1;
		for (option_idx = 0; option_idx < options_n; option_idx++) {
			set_option_row_nodes(options+option_idx, option_idx);
		}
		for (node_idx = 0; node_idx < column_nodes_n; node_idx++) {
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
			printf("search_x_slot columns %d removed %d choices %d\n", column_nodes_n, removed, choices_n);
			fflush(stdout);
		}
		if (!mp_new(&x_cost)) {
			return -1;
		}
		r = search_x_slot();
		if (r == 1) {
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
	for (option = options_header->next; option != options_header; option = option->next) {
		if (option != option_sym) {
			if (widths[square_order-option->tile->height] > square_order-option->tile->width && rotate_flag && widths[square_order-option->tile->width] > square_order-option->tile->height) {
				return 0;
			}
		}
		else {
			if (y_start >= option->tile->y_slots_sym || widths[option->tile->y_slots_sym-1] > square_order-option->tile->width) {
				return 0;
			}
		}
	}
	if (y_start > y_last) {
		options_start = options_header->next;
	}
	r = 0;
	for (option = options_start; option != options_header && !r; option = option->next) {
		r = choose_y_slot(options_n, defect, y_start, option, option->tile->height, option->tile->width);
		if (rotate_flag && option != option_sym && !is_square(option->tile) && !r) {
			r = choose_y_slot(options_n, defect, y_start, option, option->tile->width, option->tile->height);
		}
	}
	return r;
}

int choose_y_slot(int options_n, int defect, int y_start, option_t *option, int slot_height, int slot_width) {
	int y_stop = y_start+slot_height, r = 0;
	if (y_stop <= square_order && widths[y_start]+slot_width <= square_order) {
		int width_idx;
		for (width_idx = y_start; width_idx < y_stop; width_idx++) {
			widths[width_idx] += slot_width;
		}
		if (width_idx == y_stop) {
			option->y_slot = y_start;
			option->slot_height = slot_height;
			option->slot_width = slot_width;
			option->last->next = option->next;
			option->next->last = option->last;
			r = search_y_slot(options_n, defect, y_start, option->next);
			option->next->last = option;
			option->last->next = option;
			option->y_slot = square_order;
		}
		for (width_idx = y_start; width_idx < y_stop; width_idx++) {
			widths[width_idx] -= slot_width;
		}
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

void free_data(void) {
	free(tops);
	free(nodes);
	free(choices);
	free(widths);
	free(options);
	free(tiles);
	free(counts);
}
