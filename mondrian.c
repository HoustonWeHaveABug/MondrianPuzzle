#include <stdio.h>
#include <stdlib.h>

#define SQUARE_ORDER_MIN 3

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
int set_side_slots(int, int, int, int *);
int add_side_slots(int, int, int, int, int, int *);
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

int square_order, delta_min, square_area, *height_slots, *width_slots, tiles_n, tiles_area_sum, cost;
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
	height_slots = malloc(sizeof(int)*(size_t)square_order);
	if (!height_slots) {
		fprintf(stderr, "Could not allocate memory for height_slots\n");
		fflush(stderr);
		free(options);
		free(tiles);
		return EXIT_FAILURE;
	}
	width_slots = malloc(sizeof(int)*(size_t)square_order);
	if (!width_slots) {
		fprintf(stderr, "Could not allocate memory for width_slots\n");
		fflush(stderr);
		free(height_slots);
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
	free(width_slots);
	free(height_slots);
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
	printf("is_mondrian options %d columns removed %d\n", options_n, removed);
	fflush(stdout);
	cost = 0;
	r = dlx_search();
	printf("is_mondrian cost %d\n", cost);
	fflush(stdout);
	free(tops);
	free(nodes);
	return r;
}

void set_column_node(node_t *node, node_t *left) {
	node->rows_n = 0;
	link_left(node, left);
}

int set_option_row_nodes(tile_t *option, int options_n, int option_idx) {
	return set_rectangle_row_nodes(option->height, option->width, option->area, options_n, option_idx) && (option->height == option->width || option_idx == 0 || set_rectangle_row_nodes(option->width, option->height, option->area, options_n, option_idx));
}

int set_rectangle_row_nodes(int height, int width, int area, int options_n, int option_idx) {
	if (set_side_slots(height, options_n, option_idx, height_slots) && set_side_slots(width, options_n, option_idx, width_slots)) {
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
			if (height_slots[height_slot_idx]) {
				int width_slot_idx;
				for (width_slot_idx = 0; width_slot_idx < slot_max; width_slot_idx++) {
					if (width_slots[width_slot_idx]) {
						set_slot_row_nodes(height_slot_idx*square_order+width_slot_idx, height, width, area, option_idx);
					}
				}
			}
		}
		return 1;
	}
	return 0;
}

int set_side_slots(int side, int options_n, int option_idx, int *slots) {
	int slot_idx;
	for (slot_idx = 0; slot_idx < square_order; slot_idx++) {
		slots[slot_idx] = 0;
	}
	return add_side_slots(side, options_n, option_idx, 0, 0, slots);
}

int add_side_slots(int side, int options_n, int option_ref, int option_idx, int options_side_sum, int *slots) {
	int r1, r2, r3;
	if (side+options_side_sum > square_order) {
		return 0;
	}
	if (side+options_side_sum == square_order) {
		slots[options_side_sum] = 1;
		return 1;
	}
	if (option_idx == options_n) {
		return 0;
	}
	if (option_idx != option_ref) {
		r1 = add_side_slots(side, options_n, option_ref, option_idx+1, options_side_sum+options[option_idx]->height, slots);
		if (!is_square(options[option_idx])) {
			r2 = add_side_slots(side, options_n, option_ref, option_idx+1, options_side_sum+options[option_idx]->width, slots);
		}
		else {
			r2 = r1;
		}
	}
	else {
		r1 = 0;
		r2 = 0;
	}
	r3 = add_side_slots(side, options_n, option_ref, option_idx+1, options_side_sum, slots);
	if (r1 || r2 || r3) {
		slots[options_side_sum] = 1;
		return 1;
	}
	return 0;
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
	cost++;
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
