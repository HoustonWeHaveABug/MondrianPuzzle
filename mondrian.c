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
void add_option(int, int, int);
int is_mondrian(int);
void set_column_node(node_t *, node_t *);
void set_tile_row_nodes(int, int, int, int, int, int);
void set_slot_row_nodes(int, int, int, int, int);
void set_row_node(int, node_t *);
void link_left(node_t *, node_t *);
void link_top(node_t *, node_t *);
int dlx_search(void);
void cover_column(node_t *);
void uncover_column(node_t *);

int square_order, square_area, tiles_n, tiles_area_sum, delta_min;
tile_t *tiles, **options;
node_t *nodes, **tops, *header, *row_node;

int main(void) {
	int height, width, tile_idx;
	tile_t *tile;
	if (scanf("%d", &square_order) != 1 || square_order < SQUARE_ORDER_MIN) {
		fprintf(stderr, "Invalid square order\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	square_area = square_order*square_order;
	tiles_n = square_order*(square_order+1)/2-1;
	tiles = malloc(sizeof(tile_t)*(size_t)tiles_n);
	if (!tiles) {
		fprintf(stderr, "Could not allocate memory for tiles\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	tile = tiles;
	for (height = 1; height < square_order; height++) {
		for (width = 1; width <= height; width++) {
			set_tile(tile++, height, width);
		}
	}
	for (width = 1; width < square_order; width++) {
		set_tile(tile++, square_order, width);
	}
	qsort(tiles, (size_t)tiles_n, sizeof(tile_t), compare_tiles);
	tiles_area_sum = 0;
	for (tile_idx = 0; tile_idx < tiles_n; tile_idx++) {
		tiles_area_sum += tiles[tile_idx].area;
		tiles[tile_idx].area_sum = tiles_area_sum;
	}
	options = malloc(sizeof(tile_t *)*(size_t)tiles_n);
	if (!options) {
		fprintf(stderr, "Could not allocate memory for options\n");
		fflush(stderr);
		free(tiles);
		return EXIT_FAILURE;
	}
	delta_min = square_area;
	add_option(0, 0, 0);
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
	return tile_a->height-tile_b->height;
}

void add_option(int options_n, int tiles_start, int options_area_sum) {
	int tile_idx;
	for (tile_idx = tiles_start; tile_idx < tiles_n; tile_idx++) {
		int delta;
		options[options_n] = tiles+tile_idx;
		delta = options[0]->area-options[options_n]->area;
		if (delta >= delta_min) {
			break;
		}
		options_area_sum += options[options_n]->area;
		if (options_area_sum == square_area) {
			int option_idx;
			if (is_mondrian(options_n+1) == 1) {
				delta_min = delta;
				printf("%d\n", delta_min);
				for (option_idx = 0; option_idx <= options_n; option_idx++) {
					printf("%dx%d\n", options[option_idx]->height, options[option_idx]->width);
				}
				fflush(stdout);
			}
		}
		if (options_area_sum < square_area && options_area_sum+tiles_area_sum-options[options_n]->area_sum >= square_area) {
			add_option(options_n+1, tile_idx+1, options_area_sum);
		}
		options_area_sum -= options[options_n]->area;
	}
}

int is_mondrian(int options_n) {
	int column_nodes_n = square_area+options_n, nodes_n = column_nodes_n+1, option_idx, node_idx, r;
	for (option_idx = 0; option_idx < options_n; option_idx++) {
		nodes_n += options[option_idx]->row_nodes_n;
		if (options[option_idx]->height != options[option_idx]->width) {
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
	for (option_idx = 0; option_idx < options_n; option_idx++) {
		set_tile_row_nodes(options[option_idx]->height_slots_n, options[option_idx]->width_slots_n, options[option_idx]->height, options[option_idx]->width, options[option_idx]->area, option_idx);
		if (options[option_idx]->height != options[option_idx]->width) {
			set_tile_row_nodes(options[option_idx]->width_slots_n, options[option_idx]->height_slots_n, options[option_idx]->width, options[option_idx]->height, options[option_idx]->area, option_idx);
		}
	}
	for (node_idx = 0; node_idx < column_nodes_n; node_idx++) {
		link_top(nodes+node_idx, tops[node_idx]);
	}
	r = dlx_search();
	free(tops);
	free(nodes);
	return r;
}

void set_column_node(node_t *node, node_t *left) {
	node->rows_n = 0;
	link_left(node, left);
}

void set_tile_row_nodes(int height_slots_n, int width_slots_n, int height, int width, int area, int option_idx) {
	int height_slot_idx;
	for (height_slot_idx = 0; height_slot_idx < height_slots_n; height_slot_idx++) {
		int width_slot_idx;
		for (width_slot_idx = 0; width_slot_idx < width_slots_n; width_slot_idx++) {
			set_slot_row_nodes(height_slot_idx*square_order+width_slot_idx, height, width, area, option_idx);
		}
	}
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

int dlx_search(void) {
	int r;
	node_t *column_min, *column, *row;
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
