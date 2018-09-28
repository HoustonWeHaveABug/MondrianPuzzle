#include <stdio.h>
#include <stdlib.h>

#define SQUARE_ORDER_MIN 3
#undef LOG_DLX_SEARCH

typedef struct {
	int height;
	int width;
	int area;
	int area_sum;
	int height_slots_n;
	int width_slots_n;
	int row_nodes_n;
}
tile_t;

typedef struct node_s node_t;

struct node_s {
	union {
		int rows_n;
		node_t *column;
	};
	node_t *left;
	node_t *right;
	node_t *top;
	node_t *bottom;
};

void set_tile(tile_t *, int, int);
int compare_tiles(const void *, const void *);
int add_option(int, int, int, int);
int is_mondrian(int);
void set_column_node(node_t *, node_t *);
int set_option_row_nodes(tile_t *, int, int);
int set_rectangle_row_nodes(int, int, int, int, int);
int set_rectangle_slots(int, int, int, int);
int add_rectangle_slots(int, int, int, int, int, int, int, int, int);
void generate_slots(int, int);
void generate_height_sets(int, int, int, int, int, int);
void generate_width_sets(int, int, int, int, int);
int is_square(tile_t *);
void set_slot_row_nodes(int, int, int, int, int);
void set_row_node(int, node_t *);
void link_left(node_t *, node_t *);
void link_top(node_t *, node_t *);
int is_unique_column(node_t *);
int compare_columns(node_t *, node_t *);
void remove_column(node_t *);
int dlx_search(void);
void cover_column(node_t *);
void uncover_column(node_t *);

int square_order, delta_min, square_area, *slots, *heights, *widths, tiles_n, tiles_area_sum;
#ifdef LOG_DLX_SEARCH
int cost;
#endif
tile_t *tiles, **options;
node_t *nodes, **tops, *header, *row_node;

int main(void) {
	int tiles_max, height, width, tile_idx, r;
	if (scanf("%d%d", &square_order, &delta_min) != 2 || square_order < SQUARE_ORDER_MIN || delta_min < 0) {
		fprintf(stderr, "Invalid parameters\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	square_area = square_order*square_order;
	tiles_max = square_order*(square_order+1)/2-1;
	tiles = malloc(sizeof(tile_t)*(size_t)tiles_max);
	if (!tiles) {
		fprintf(stderr, "Could not allocate memory for tiles\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
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
	heights = malloc(sizeof(int)*(size_t)tiles_max);
	if (!heights) {
		fprintf(stderr, "Could not allocate memory for heights\n");
		fflush(stderr);
		free(slots);
		free(options);
		free(tiles);
		return EXIT_FAILURE;
	}
	widths = malloc(sizeof(int)*(size_t)tiles_max);
	if (!widths) {
		fprintf(stderr, "Could not allocate memory for widths\n");
		fflush(stderr);
		free(heights);
		free(slots);
		free(options);
		free(tiles);
		return EXIT_FAILURE;
	}
	do {
		tiles_n = 0;
		for (height = 1; height < square_order; height++) {
			for (width = 1; width <= height; width++) {
				if (height*width-square_order*(square_order-height) <= delta_min) {
					set_tile(tiles+tiles_n, height, width);
					tiles_n++;
				}
			}
		}
		for (width = 1; width < square_order; width++) {
			if (square_order*width-square_order*(square_order-width) <= delta_min) {
				set_tile(tiles+tiles_n, square_order, width);
				tiles_n++;
			}
		}
		qsort(tiles, (size_t)tiles_n, sizeof(tile_t), compare_tiles);
		tiles_area_sum = 0;
		for (tile_idx = 0; tile_idx < tiles_n; tile_idx++) {
			tiles_area_sum += tiles[tile_idx].area;
			tiles[tile_idx].area_sum = tiles_area_sum;
		}
		printf("%d\n", delta_min);
		fflush(stdout);
		r = add_option(0, 0, 0, 0);
		if (!r) {
			delta_min++;
		}
	}
	while (!r);
	free(widths);
	free(heights);
	free(slots);
	free(options);
	free(tiles);
	return EXIT_SUCCESS;
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

int add_option(int tiles_start, int options_n, int width_max, int options_area_sum) {
	int r = 0, tile_idx;
	for (tile_idx = tiles_start; tile_idx < tiles_n && !r; tile_idx++) {
		int delta;
		options[options_n] = tiles+tile_idx;
		delta = options[0]->area-options[options_n]->area;
		if (delta > delta_min) {
			break;
		}
		if (width_max+options[options_n]->width > square_order) {
			continue;
		}
		options_area_sum += options[options_n]->area;
		if (options_area_sum == square_area) {
			if (delta == delta_min && is_mondrian(options_n+1) == 1) {
				int option_idx;
				r = 1;
				for (option_idx = 0; option_idx <= options_n; option_idx++) {
					printf("%dx%d\n", options[option_idx]->height, options[option_idx]->width);
				}
				fflush(stdout);
			}
		}
		else if (options_area_sum < square_area) {
			if (options_area_sum+tiles_area_sum-options[options_n]->area_sum >= square_area) {
				if (options[options_n]->width > width_max) {
					r = add_option(tile_idx+1, options_n+1, options[options_n]->width, options_area_sum);
				}
				else {
					r = add_option(tile_idx+1, options_n+1, width_max, options_area_sum);
				}
			}
		}
		options_area_sum -= options[options_n]->area;
	}
	return r;
}

int is_mondrian(int options_n) {
	int column_nodes_n = square_area+options_n, nodes_n = column_nodes_n+1, option_idx, node_idx, r, removed;
	for (option_idx = 0; option_idx < options_n; option_idx++) {
		nodes_n += options[option_idx]->row_nodes_n;
		if (!is_square(options[option_idx])) {
			nodes_n += options[option_idx]->row_nodes_n;
		}
	}
	nodes = malloc(sizeof(node_t)*(size_t)nodes_n);
	if (!nodes) {
		fprintf(stderr, "Could not allocate memory for nodes\n");
		fflush(stderr);
		return -1;
	}
	tops = malloc(sizeof(node_t *)*(size_t)column_nodes_n);
	if (!tops) {
		fprintf(stderr, "Could not allocate memory for tops\n");
		fflush(stderr);
		free(nodes);
		return -1;
	}
	header = nodes+column_nodes_n;
	set_column_node(nodes, header);
	for (node_idx = 0; node_idx < column_nodes_n; node_idx++) {
		set_column_node(nodes+node_idx+1, nodes+node_idx);
		tops[node_idx] = nodes+node_idx;
	}
	row_node = header+1;
	r = set_option_row_nodes(options[0], options_n, 0);
	for (option_idx = 1; option_idx < options_n && r; option_idx++) {
		r = set_option_row_nodes(options[option_idx], options_n, option_idx);
	}
	if (!r) {
		free(tops);
		free(nodes);
		return -1;
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
#ifdef LOG_DLX_SEARCH
	printf("dlx_search options %d columns removed %d\n", options_n, removed);
	fflush(stdout);
	cost = 0;
#endif
	r = dlx_search();
#ifdef LOG_DLX_SEARCH
	printf("dlx_search cost %d\n", cost);
	fflush(stdout);
#endif
	free(tops);
	free(nodes);
	return r;
}

void set_column_node(node_t *node, node_t *left) {
	node->rows_n = 0;
	link_left(node, left);
}

int set_option_row_nodes(tile_t *option, int options_n, int option_idx) {
	return set_rectangle_row_nodes(option->height, option->width, option->area, options_n, option_idx) && (is_square(option) || option_idx == 0 || set_rectangle_row_nodes(option->width, option->height, option->area, options_n, option_idx));
}

int set_rectangle_row_nodes(int height, int width, int area, int options_n, int option_idx) {
	if (set_rectangle_slots(height, width, options_n, option_idx)) {
		int slot_max, height_slot_idx;
		if (option_idx == 0) {
			if (square_order%2 == 0) {
				slot_max = square_order/2;
			}
			else {
				slot_max = square_order/2+1;
			}
		}
		else {
			slot_max = square_order;
		}
		for (height_slot_idx = 0; height_slot_idx < slot_max; height_slot_idx++) {
			int width_slot_idx;
			for (width_slot_idx = 0; width_slot_idx < slot_max; width_slot_idx++) {
				if (slots[height_slot_idx*square_order+width_slot_idx]) {
					set_slot_row_nodes(height_slot_idx*square_order+width_slot_idx, height, width, area, option_idx);
				}
			}
		}
		return 1;
	}
	return 0;
}

int set_rectangle_slots(int height, int width, int options_n, int option_idx) {
	int slot_idx;
	for (slot_idx = 0; slot_idx < square_area; slot_idx++) {
		slots[slot_idx] = 0;
	}
	return add_rectangle_slots(height, width, options_n, option_idx, 0, 0, 0, 0, 0);
}

int add_rectangle_slots(int height, int width, int options_n, int option_ref, int option_idx, int heights_n, int heights_sum, int widths_n, int widths_sum) {
	int r1, r2, r3, r4, r5;
	if (height+heights_sum > square_order || width+widths_sum > square_order) {
		return 0;
	}
	if (height+heights_sum == square_order && width+widths_sum == square_order) {
		generate_slots(heights_n, widths_n);
		return 1;
	}
	if (option_idx == options_n) {
		return 0;
	}
	if (option_idx != option_ref) {
		heights[heights_n] = options[option_idx]->height;
		r1 = add_rectangle_slots(height, width, options_n, option_ref, option_idx+1, heights_n+1, heights_sum+options[option_idx]->height, widths_n, widths_sum);
		widths[widths_n] = options[option_idx]->width;
		r2 = add_rectangle_slots(height, width, options_n, option_ref, option_idx+1, heights_n, heights_sum, widths_n+1, widths_sum+options[option_idx]->width);
		if (is_square(options[option_idx])) {
			r3 = r1;
			r4 = r2;
		}
		else {
			heights[heights_n] = options[option_idx]->width;
			r3 = add_rectangle_slots(height, width, options_n, option_ref, option_idx+1, heights_n+1, heights_sum+options[option_idx]->width, widths_n, widths_sum);
			widths[widths_n] = options[option_idx]->height;
			r4 = add_rectangle_slots(height, width, options_n, option_ref, option_idx+1, heights_n, heights_sum, widths_n+1, widths_sum+options[option_idx]->height);
		}
	}
	else {
		r1 = 0;
		r2 = 0;
		r3 = 0;
		r4 = 0;
	}
	r5 = add_rectangle_slots(height, width, options_n, option_ref, option_idx+1, heights_n, heights_sum, widths_n, widths_sum);
	if (r1 || r2 || r3 || r4 || r5) {
		return 1;
	}
	return 0;
}

void generate_slots(int heights_n, int widths_n) {
	int heights_set_max;
	for (heights_set_max = 0; heights_set_max <= heights_n; heights_set_max++) {
		generate_height_sets(heights_n, 0, heights_set_max, 0, 0, widths_n);
	}
}

void generate_height_sets(int heights_n, int heights_start, int heights_set_max, int heights_set_idx, int slot_idx, int widths_n) {
	if (heights_set_idx == heights_set_max) {
		int widths_set_max;
		slot_idx *= square_order;
		for (widths_set_max = 0; widths_set_max <= widths_n; widths_set_max++) {
			generate_width_sets(widths_n, 0, widths_set_max, 0, slot_idx);
		}
	}
	else {
		int heights_idx;
		for (heights_idx = heights_start; heights_idx < heights_n; heights_idx++) {
			generate_height_sets(heights_n, heights_idx+1, heights_set_max, heights_set_idx+1, slot_idx+heights[heights_idx], widths_n);
		}
	}
}

void generate_width_sets(int widths_n, int widths_start, int widths_set_max, int widths_set_idx, int slot_idx) {
	if (widths_set_idx == widths_set_max) {
		slots[slot_idx] = 1;
	}
	else {
		int widths_idx;
		for (widths_idx = widths_start; widths_idx < widths_n; widths_idx++) {
			generate_width_sets(widths_n, widths_idx+1, widths_set_max, widths_set_idx+1, slot_idx+widths[widths_idx]);
		}
	}
}

int is_square(tile_t *tile) {
	return tile->height == tile->width;
}

void set_slot_row_nodes(int slot_start, int height, int width, int area, int option_idx) {
	int x, y;
	set_row_node(slot_start, row_node+area);
	for (x = 1; x < width; x++) {
		set_row_node(slot_start+x, row_node-1);
	}
	for (y = 1; y < height; y++) {
		for (x = 0; x < width; x++) {
			set_row_node(slot_start+y*square_order+x, row_node-1);
		}
	}
	set_row_node(square_area+option_idx, row_node-1);
}

void set_row_node(int column, node_t *left) {
	row_node->column = nodes+column;
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
	for (node = column->left; node != header && compare_columns(column, node); node = node->left);
	return node == header;
}

int compare_columns(node_t *column_a, node_t *column_b) {
	node_t *row_a;
	if (column_a->rows_n != column_a->rows_n) {
		return 1;
	}
	for (row_a = column_a->bottom; row_a != column_a; row_a = row_a->bottom) {
		node_t *node;
		for (node = row_a->left; node != row_a && node->column > column_b; node = node->left);
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

int dlx_search(void) {
	int r;
	node_t *column_min, *column, *row;
#ifdef LOG_DLX_SEARCH
	cost++;
#endif
	if (header->right == header) {
		return 1;
	}
	column_min = header->right;
	for (column = column_min->right; column != header; column = column->right) {
		if (column->rows_n < column_min->rows_n) {
			column_min = column;
		}
	}
	cover_column(column_min);
	r = 0;
	for (row = column_min->bottom; row != column_min && !r; row = row->bottom) {
		node_t *node;
		for (node = row->right; node != row; node = node->right) {
			cover_column(node->column);
		}
		r = dlx_search();
		for (node = row->left; node != row; node = node->left) {
			uncover_column(node->column);
		}
	}
	uncover_column(column_min);
	return r;
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
