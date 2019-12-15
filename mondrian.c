#include <stdio.h>
#include <stdlib.h>

#define SQUARE_ORDER_MIN 3

typedef struct tile_s tile_t;

struct tile_s {
	int height;
	int width;
	int area;
	int area_sum;
	int height_slots_n;
	int width_slots_n;
	int row_nodes_n;
	int slot_height;
	int slot_width;
	int slot_start;
	tile_t *last;
	tile_t *next;
};

typedef struct node_s node_t;

struct node_s {
	union {
		int rows_n;
		node_t *column;
	};
	int slot_start;
	tile_t *tile;
	node_t *left;
	node_t *right;
	node_t *top;
	node_t *bottom;
};

int best_delta(int);
void set_tile(tile_t *, int, int);
int compare_tiles(const void *, const void *);
int add_option(int, int, int);
int is_mondrian(int);
void set_column_node(node_t *, node_t *);
int set_option_row_nodes(tile_t *, int, int);
void add_option_slots(int, int, int, int, int, int, int);
void generate_slots(int, int);
void generate_heights_set(int, int, tile_t *, int, int, int, int);
void generate_widths_set(int, int, tile_t *, int, int, int);
int is_square(tile_t *);
void set_slot_row_nodes(int, int, int, int, int);
void set_row_node(int, int, int, int, int, node_t *);
void link_left(node_t *, node_t *);
void link_top(node_t *, node_t *);
void insert_height(tile_t *, int, int);
void insert_width(tile_t *, int, int);
void insert_side(tile_t *, int, int, tile_t *, tile_t *);
void remove_side(tile_t *);
int is_unique_column(node_t *);
int compare_columns(node_t *, node_t *);
void remove_column(node_t *);
int dlx_search(int);
void print_option(tile_t *);
int set_column_value(node_t *);
int set_row_value(node_t *);
void cover_column(node_t *);
void uncover_column(node_t *);

int square_order, delta_min, verbose, square_area, *slots, nodes_max, column_nodes_max, tiles_n, tiles_area_sum, cost;
tile_t *tiles, *heights, *widths, **options;
node_t *nodes, **tops, *header, *row_node;

int main(void) {
	int delta_max, tiles_max, height, width, tile_idx, r;
	if (scanf("%d%d%d%d", &square_order, &delta_min, &delta_max, &verbose) != 4 || square_order < SQUARE_ORDER_MIN || delta_min < 0 || delta_max < delta_min) {
		fprintf(stderr, "Invalid parameters\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	square_area = square_order*square_order;
	tiles_max = square_order*(square_order+1)/2-1;
	tiles = malloc(sizeof(tile_t)*(size_t)(tiles_max+2));
	if (!tiles) {
		fprintf(stderr, "Could not allocate memory for tiles\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	heights = tiles+tiles_max;
	heights->slot_height = square_order+1;
	heights->slot_width = square_order+1;
	heights->last = heights;
	heights->next = heights;
	widths = heights+1;
	widths->slot_height = square_order+1;
	widths->slot_width = square_order+1;
	widths->last = widths;
	widths->next = widths;
	options = malloc(sizeof(tile_t *)*(size_t)tiles_max);
	if (!options) {
		fprintf(stderr, "Could not allocate memory for options\n");
		fflush(stderr);
		free(tiles);
		return EXIT_FAILURE;
	}
	slots = malloc(sizeof(int)*(size_t)square_area);
	if (!slots) {
		fprintf(stderr, "Could not allocate memory for slots\n");
		fflush(stderr);
		free(options);
		free(tiles);
		return EXIT_FAILURE;
	}
	nodes = malloc(sizeof(node_t));
	if (!nodes) {
		fprintf(stderr, "Could not allocate memory for nodes\n");
		fflush(stderr);
		free(slots);
		free(options);
		free(tiles);
		return EXIT_FAILURE;
	}
	nodes_max = 1;
	tops = malloc(sizeof(node_t *));
	if (!tops) {
		fprintf(stderr, "Could not allocate memory for tops\n");
		fflush(stderr);
		free(nodes);
		free(slots);
		free(options);
		free(tiles);
		return EXIT_FAILURE;
	}
	column_nodes_max = 1;
	do {
		tiles_n = 0;
		for (height = 1; height < square_order; height++) {
			for (width = 1; width <= height; width++) {
				if (height*width-square_order*(square_order-height) <= delta_min && height*width-height*(square_order-width) <= delta_min && best_delta(height*width) <= delta_min) {
					set_tile(tiles+tiles_n, height, width);
					tiles_n++;
				}
			}
		}
		for (width = 1; width < square_order; width++) {
			if (width > square_order-width) {
				if (square_order*width-square_order*(square_order-width) <= delta_min && best_delta(square_order*width) <= delta_min) {
					set_tile(tiles+tiles_n, square_order, width);
					tiles_n++;
				}
			}
			else {
				if (square_order%2 == 0) {
					if (square_order*width-(square_order/2-1)*(square_order-width) <= delta_min && best_delta(square_order*width) <= delta_min) {
						set_tile(tiles+tiles_n, square_order, width);
						tiles_n++;
					}
				}
				else {
					if (square_order*width-(square_order/2)*(square_order-width) <= delta_min && best_delta(square_order*width) <= delta_min) {
						set_tile(tiles+tiles_n, square_order, width);
						tiles_n++;
					}
				}
			}
		}
		qsort(tiles, (size_t)tiles_n, sizeof(tile_t), compare_tiles);
		tiles_area_sum = 0;
		for (tile_idx = 0; tile_idx < tiles_n; tile_idx++) {
			tiles_area_sum += tiles[tile_idx].area;
			tiles[tile_idx].area_sum = tiles_area_sum;
		}
		printf("%d tiles %d\n", delta_min, tiles_n);
		fflush(stdout);
		r = add_option(0, 0, 0);
		if (!r) {
			delta_min++;
		}
	}
	while (!r && delta_min <= delta_max);
	free(tops);
	free(nodes);
	free(slots);
	free(options);
	free(tiles);
	return EXIT_SUCCESS;
}

int best_delta(int area) {
	int area_mod = square_area%area, best, others_n, others_mod, area_other;
	if (area_mod == 0) {
		return 0;
	}
	best = area-area_mod;
	others_n = square_area/area-1;
	if (others_n == 0) {
		return best;
	}
	others_mod = area_mod%others_n;
	if (others_mod == 0) {
		area_other = area+area_mod/others_n;
	}
	else {
		area_other = area+area_mod/others_n+1;
	}
	if (area_other-area < best) {
		best = area_other-area;
	}
	area_other = (square_area-area)/(others_n+1);
	if (area-area_other < best) {
		best = area-area_other;
	}
	return best;
}

void set_tile(tile_t *tile, int height, int width) {
	tile->height = height;
	tile->width = width;
	tile->area = height*width;
	tile->height_slots_n = square_order-height+1;
	tile->width_slots_n = square_order-width+1;
	tile->row_nodes_n = (tile->area+1)*tile->height_slots_n*tile->width_slots_n;
}

int compare_tiles(const void *a, const void *b) {
	const tile_t *tile_a = (const tile_t *)a, *tile_b = (const tile_t *)b;
	if (tile_a->area != tile_b->area) {
		return tile_b->area-tile_a->area;
	}
	return tile_b->width-tile_a->width;
}

int add_option(int tiles_start, int options_n, int options_area_sum) {
	int r = 0, tile_idx;
	for (tile_idx = tiles_start; tile_idx < tiles_n && !r; tile_idx++) {
		int delta;
		options[options_n] = tiles+tile_idx;
		delta = options[0]->area-options[options_n]->area;
		if (delta > delta_min) {
			break;
		}
		options_area_sum += options[options_n]->area;
		if (options_area_sum == square_area) {
			if (delta == delta_min && is_mondrian(options_n+1) == 1) {
				r = 1;
			}
		}
		else if (options_area_sum < square_area) {
			if (options_area_sum+tiles_area_sum-options[options_n]->area_sum >= square_area) {
				r = add_option(tile_idx+1, options_n+1, options_area_sum);
			}
		}
		options_area_sum -= options[options_n]->area;
	}
	return r;
}

int is_mondrian(int options_n) {
	int column_nodes_n = square_area+options_n, nodes_n = column_nodes_n+1, option_idx, node_idx, removed, r;
	for (option_idx = 0; option_idx < options_n; option_idx++) {
		nodes_n += options[option_idx]->row_nodes_n;
		if (!is_square(options[option_idx])) {
			nodes_n += options[option_idx]->row_nodes_n;
		}
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
	header = nodes+column_nodes_n;
	set_column_node(nodes, header);
	for (node_idx = 0; node_idx < column_nodes_n; node_idx++) {
		set_column_node(nodes+node_idx+1, nodes+node_idx);
		tops[node_idx] = nodes+node_idx;
	}
	if (verbose) {
		printf("set_option_row_nodes options %d\n", options_n);
		fflush(stdout);
	}
	row_node = header+1;
	for (option_idx = 0; option_idx < options_n && set_option_row_nodes(options[option_idx], options_n, option_idx); option_idx++);
	for (node_idx = 0; node_idx < column_nodes_n; node_idx++) {
		link_top(nodes+node_idx, tops[node_idx]);
	}
	if (option_idx < options_n) {
		return 0;
	}
	removed = 0;
	for (node_idx = 1; node_idx < square_area; node_idx++) {
		if (!is_unique_column(nodes+node_idx)) {
			remove_column(nodes+node_idx);
			removed++;
		}
	}
	if (verbose) {
		printf("dlx_search options %d columns removed %d\n", options_n, removed);
		fflush(stdout);
	}
	cost = 0;
	r = dlx_search(options_n);
	if (verbose) {
		printf("dlx_search cost %d\n", cost);
		fflush(stdout);
	}
	return r;
}

void set_column_node(node_t *node, node_t *left) {
	node->rows_n = 0;
	link_left(node, left);
}

int set_option_row_nodes(tile_t *option, int options_n, int option_idx) {
	int rows_n = 0, slot_idx, slot_y_max, slot_x_max, height_slot_idx;
	for (slot_idx = 0; slot_idx < square_area; slot_idx++) {
		slots[slot_idx] = 0;
	}
	add_option_slots(options_n, option_idx, 0, 0, options[option_idx]->height, 0, options[option_idx]->width);
	if (option_idx == 0) {
		slot_y_max = (square_order-options[option_idx]->height)/2+1;
		slot_x_max = (square_order-options[option_idx]->width)/2+1;
	}
	else {
		slot_y_max = square_order;
		slot_x_max = square_order;
	}
	for (height_slot_idx = 0; height_slot_idx < slot_y_max; height_slot_idx++) {
		int slot_min, width_slot_idx;
		if (option_idx == 0 && is_square(option)) {
			slot_min = height_slot_idx;
		}
		else {
			slot_min = 0;
		}
		for (width_slot_idx = slot_min; width_slot_idx < slot_x_max; width_slot_idx++) {
			if (slots[height_slot_idx*square_order+width_slot_idx]) {
				set_slot_row_nodes(height_slot_idx*square_order+width_slot_idx, option->height, option->width, option->area, option_idx);
				rows_n++;
			}
		}
	}
	if (option_idx > 0 && !is_square(option)) {
		for (height_slot_idx = 0; height_slot_idx < square_order; height_slot_idx++) {
			int width_slot_idx;
			for (width_slot_idx = 0; width_slot_idx < square_order; width_slot_idx++) {
				if (slots[height_slot_idx*square_order+width_slot_idx]) {
					set_slot_row_nodes(width_slot_idx*square_order+height_slot_idx, option->width, option->height, option->area, option_idx);
					rows_n++;
				}
			}
		}
	}
	return rows_n;
}

void add_option_slots(int options_n, int option_ref, int option_idx, int heights_n, int heights_sum, int widths_n, int widths_sum) {
	if (heights_sum > square_order || widths_sum > square_order) {
		return;
	}
	if (heights_sum == square_order && widths_sum == square_order) {
		generate_slots(heights_n, widths_n);
		return;
	}
	if (option_idx == options_n) {
		return;
	}
	if (option_idx != option_ref) {
		insert_height(options[option_idx], options[option_idx]->height, options[option_idx]->width);
		add_option_slots(options_n, option_ref, option_idx+1, heights_n+1, heights_sum+options[option_idx]->height, widths_n, widths_sum);
		remove_side(options[option_idx]);
		insert_width(options[option_idx], options[option_idx]->height, options[option_idx]->width);
		add_option_slots(options_n, option_ref, option_idx+1, heights_n, heights_sum, widths_n+1, widths_sum+options[option_idx]->width);
		remove_side(options[option_idx]);
		if (!is_square(options[option_idx])) {
			insert_height(options[option_idx], options[option_idx]->width, options[option_idx]->height);
			add_option_slots(options_n, option_ref, option_idx+1, heights_n+1, heights_sum+options[option_idx]->width, widths_n, widths_sum);
			remove_side(options[option_idx]);
			insert_width(options[option_idx], options[option_idx]->width, options[option_idx]->height);
			add_option_slots(options_n, option_ref, option_idx+1, heights_n, heights_sum, widths_n+1, widths_sum+options[option_idx]->height);
			remove_side(options[option_idx]);
		}
	}
	add_option_slots(options_n, option_ref, option_idx+1, heights_n, heights_sum, widths_n, widths_sum);
}

void generate_slots(int heights_n, int widths_n) {
	int heights_set_max;
	for (heights_set_max = 0; heights_set_max <= heights_n; heights_set_max++) {
		generate_heights_set(heights_n, 0, heights->next, heights_set_max, 0, 0, widths_n);
	}
}

void generate_heights_set(int heights_n, int heights_start_idx, tile_t *heights_start, int heights_set_max, int heights_set_cur, int slot_idx, int widths_n) {
	if (heights_set_cur == heights_set_max) {
		int widths_set_max;
		slot_idx *= square_order;
		for (widths_set_max = 0; widths_set_max <= widths_n; widths_set_max++) {
			generate_widths_set(widths_n, 0, widths->next, widths_set_max, 0, slot_idx);
		}
	}
	else {
		int heights_idx;
		tile_t *tile;
		for (heights_idx = heights_start_idx, tile = heights_start; heights_idx <= heights_n-heights_set_max+heights_set_cur; heights_idx++, tile = tile->next) {
			if (tile == heights_start || tile->slot_height < tile->last->slot_height) {
				generate_heights_set(heights_n, heights_idx+1, tile->next, heights_set_max, heights_set_cur+1, slot_idx+tile->slot_height, widths_n);
			}
		}
	}
}

void generate_widths_set(int widths_n, int widths_start_idx, tile_t *widths_start, int widths_set_max, int widths_set_cur, int slot_idx) {
	if (widths_set_cur == widths_set_max) {
		slots[slot_idx] = 1;
	}
	else {
		int widths_idx;
		tile_t *tile;
		for (widths_idx = widths_start_idx, tile = widths_start; widths_idx <= widths_n-widths_set_max+widths_set_cur; widths_idx++, tile = tile->next) {
			if (tile == widths_start || tile->slot_width < tile->last->slot_width) {
				generate_widths_set(widths_n, widths_idx+1, tile->next, widths_set_max, widths_set_cur+1, slot_idx+tile->slot_width);
			}
		}
	}
}

int is_square(tile_t *tile) {
	return tile->height == tile->width;
}

void set_slot_row_nodes(int slot_start, int height, int width, int area, int option_idx) {
	int x, y;
	set_row_node(slot_start, slot_start, height, width, option_idx, row_node+area);
	for (x = 1; x < width; x++) {
		set_row_node(slot_start+x, slot_start, height, width, option_idx, row_node-1);
	}
	for (y = 1; y < height; y++) {
		for (x = 0; x < width; x++) {
			set_row_node(slot_start+y*square_order+x, slot_start, height, width, option_idx, row_node-1);
		}
	}
	set_row_node(square_area+option_idx, slot_start, height, width, option_idx, row_node-1);
}

void set_row_node(int column, int slot_start, int height, int width, int option_idx, node_t *left) {
	row_node->column = nodes+column;
	if (height < width) {
		row_node->slot_start = square_area+slot_start;
	}
	else {
		row_node->slot_start = slot_start;
	}
	row_node->tile = options[option_idx];
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

void insert_height(tile_t *option, int slot_height, int slot_width) {
	tile_t *tile;
	for (tile = heights->next; tile != heights && tile->slot_height >= slot_height; tile = tile->next);
	insert_side(option, slot_height, slot_width, tile->last, tile);
}

void insert_width(tile_t *option, int slot_height, int slot_width) {
	tile_t *tile;
	for (tile = widths->next; tile != widths && tile->slot_width >= slot_width; tile = tile->next);
	insert_side(option, slot_height, slot_width, tile->last, tile);
}

void insert_side(tile_t *option, int slot_height, int slot_width, tile_t *last, tile_t *next) {
	option->slot_height = slot_height;
	option->slot_width = slot_width;
	option->last = last;
	last->next = option;
	option->next = next;
	next->last = option;
}

void remove_side(tile_t *option) {
	option->last->next = option->next;
	option->next->last = option->last;
}

int is_unique_column(node_t *column) {
	node_t *node;
	for (node = column->left; node != header && compare_columns(column, node); node = node->left);
	return node == header;
}

int compare_columns(node_t *column_a, node_t *column_b) {
	node_t *row_a;
	if (column_a->rows_n != column_b->rows_n) {
		return 1;
	}
	for (row_a = column_a->bottom; row_a != column_a; row_a = row_a->bottom) {
		node_t *node;
		for (node = row_a->left; node != row_a && node->column != column_b; node = node->left);
		if (node->column != column_b) {
			break;
		}
	}
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

int dlx_search(int options_n) {
	int value_max, r;
	node_t *column_min, *column, *row;
	cost++;
	if (header->right == header) {
		int option_idx;
		for (option_idx = 0; option_idx < options_n; option_idx++) {
			print_option(options[option_idx]);
		}
		fflush(stdout);
		return 1;
	}
	if (header->right->rows_n == 0) {
		return 0;
	}
	column_min = header->right;
	for (column = column_min->right; column != header; column = column->right) {
		if (column->rows_n < column_min->rows_n) {
			if (column->rows_n == 0) {
				return 0;
			}
			column_min = column;
		}
	}
	value_max = set_column_value(column_min);
	for (column = column_min->right; column != header; column = column->right) {
		if (column->rows_n == column_min->rows_n) {
			int value = set_column_value(column);
			if (value > value_max) {
				column_min = column;
				value_max = value;
			}
		}
	}
	cover_column(column_min);
	r = 0;
	for (row = column_min->bottom; row != column_min && !r; row = row->bottom) {
		node_t *node;
		for (node = row->right; node != row; node = node->right) {
			cover_column(node->column);
		}
		row->tile->slot_start = row->slot_start;
		r = dlx_search(options_n);
		for (node = row->left; node != row; node = node->left) {
			uncover_column(node->column);
		}
	}
	uncover_column(column_min);
	return r;
}

void print_option(tile_t *option) {
	if (option->slot_start < square_area) {
		printf("%dx%d;%dx%d\n", option->slot_start/square_order, option->slot_start%square_order, option->height, option->width);
	}
	else {
		printf("%dx%d;%dx%d\n", (option->slot_start-square_area)/square_order, (option->slot_start-square_area)%square_order, option->width, option->height);
	}
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

void cover_column(node_t *column) {
	node_t *row;
	column->right->left = column->left;
	column->left->right = column->right;
	for (row = column->bottom; row != column; row = row->bottom) {
		node_t *node;
		for (node = row->right; node != row; node = node->right) {
			node->column->rows_n--;
			node->bottom->top = node->top;
			node->top->bottom = node->bottom;
		}
	}
}

void uncover_column(node_t *column) {
	node_t *row;
	for (row = column->top; row != column; row = row->top) {
		node_t *node;
		for (node = row->left; node != row; node = node->left) {
			node->top->bottom = node;
			node->bottom->top = node;
			node->column->rows_n++;
		}
	}
	column->left->right = column;
	column->right->left = column;
}
