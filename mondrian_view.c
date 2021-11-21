#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define SQUARE_ORDER_MIN 3
#define TILES_MIN 2
#define TILES_MAX 52

typedef struct tile_s tile_t;

struct tile_s {
	int y0;
	int x0;
	int y1;
	int x1;
	int neighbours_n;
	tile_t **neighbours;
	int symbol;
};

void set_tile(tile_t *, int, int, int, int);
int are_neighbours(tile_t *, tile_t *);
void try_symbol(int, int);
void set_symbol(tile_t *, int);
void print_square(void);

int symbols[TILES_MAX] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' }, square_order, tiles_n, *square, symbols_max;
tile_t *tiles, **neighbours_cur;

int main(void) {
	int minimize, square_area, tiles_area_sum, tile_idx;
	tile_t **neighbours;
	if (scanf("%d%d%d", &minimize, &square_order, &tiles_n) != 3 || square_order < SQUARE_ORDER_MIN || square_order > INT32_MAX/square_order || tiles_n < TILES_MIN || tiles_n > TILES_MAX) {
		fprintf(stderr, "Invalid square_order\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	square_area = square_order*square_order;
	if (sizeof(int) > (size_t)-1/(size_t)square_area) {
		fprintf(stderr, "Will not be able to allocate memory for square\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	square = calloc((size_t)square_area, sizeof(int));
	if (!square) {
		fprintf(stderr, "Could not allocate memory for square\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	tiles = malloc(sizeof(tile_t)*(size_t)tiles_n);
	if (!tiles) {
		fprintf(stderr, "Could not allocate memory for tiles\n");
		fflush(stderr);
		free(square);
		return EXIT_FAILURE;
	}
	neighbours = malloc(sizeof(tile_t *)*(size_t)(tiles_n*(tiles_n-1)/2));
	if (!neighbours) {
		fprintf(stderr, "Could not allocate memory for neighbours\n");
		fflush(stderr);
		free(square);
		return EXIT_FAILURE;
	}
	neighbours_cur = neighbours;
	tiles_area_sum = 0;
	for (tile_idx = 0; tile_idx < tiles_n; tile_idx++) {
		int y0, x0, h, w, y;
		if (scanf("%dx%d;%dx%d", &y0, &x0, &h, &w) != 4 || y0 < 0 || y0+h > square_order || x0 < 0 || x0+w > square_order) {
			fprintf(stderr, "Invalid tile\n");
			fflush(stderr);
			free(tiles);
			free(square);
			return EXIT_FAILURE;
		}
		for (y = y0; y < y0+h; y++) {
			int x;
			for (x = x0; x < x0+w; x++) {
				int square_idx = y*square_order+x;
				if (square[square_idx]) {
					fprintf(stderr, "Overlap detected at %dx%d\n", y, x);
					fflush(stderr);
					free(tiles);
					free(square);
					return EXIT_FAILURE;
				}
				square[square_idx] = symbols[tile_idx];
			}
		}
		set_tile(tiles+tile_idx, y0, x0, h, w);
		tiles_area_sum += h*w;
	}
	if (tiles_area_sum < square_area) {
		fprintf(stderr, "Square not fully covered\n");
		fflush(stderr);
		free(tiles);
		free(square);
		return EXIT_FAILURE;
	}
	if (minimize) {
		symbols_max = tiles_n;
		try_symbol(0, 0);
	}
	else {
		print_square();
	}
	free(tiles);
	free(square);
	return EXIT_SUCCESS;
}

void set_tile(tile_t *tile_a, int y0, int x0, int h, int w) {
	tile_t *tile_b;
	tile_a->y0 = y0;
	tile_a->x0 = x0;
	tile_a->y1 = y0+h;
	tile_a->x1 = x0+w;
	tile_a->neighbours_n = 0;
	tile_a->neighbours = neighbours_cur;
	for (tile_b = tiles; tile_b < tile_a; tile_b++) {
		if (are_neighbours(tile_a, tile_b)) {
			tile_a->neighbours_n++;
			*neighbours_cur = tile_b;
			neighbours_cur++;
		}
	}
	tile_a->symbol = 0;
}

int are_neighbours(tile_t *tile_a, tile_t *tile_b) {
	if (tile_a->y0 < tile_b->y0) {
		if (tile_a->x0 < tile_b->x0) {
			return (tile_a->y1 == tile_b->y0 && tile_a->x1 >= tile_b->x0) || (tile_a->y1 >= tile_b->y0 && tile_a->x1 == tile_b->x0);
		}
		else {
			return (tile_a->y1 == tile_b->y0 && tile_a->x0 <= tile_b->x1) || (tile_a->y1 >= tile_b->y0 && tile_a->x0 == tile_b->x1);
		}
	}
	else {
		if (tile_a->x0 < tile_b->x0) {
			return (tile_a->y0 == tile_b->y1 && tile_a->x1 >= tile_b->x0) || (tile_a->y0 <= tile_b->y1 && tile_a->x1 == tile_b->x0);
		}
		else {
			return (tile_a->y0 == tile_b->y1 && tile_a->x0 <= tile_b->x1) || (tile_a->y0 <= tile_b->y1 && tile_a->x0 == tile_b->x1);
		}
	}
}

void try_symbol(int tiles_hi, int symbols_hi) {
	int symbol_idx;
	if (symbols_hi >= symbols_max) {
		return;
	}
	if (tiles_hi == tiles_n) {
		symbols_max = symbols_hi;
		print_square();
		return;
	}
	for (symbol_idx = 0; symbol_idx <= symbols_hi; symbol_idx++) {
		int neighbour_idx;
		for (neighbour_idx = 0; neighbour_idx < tiles[tiles_hi].neighbours_n && tiles[tiles_hi].neighbours[neighbour_idx]->symbol != symbols[symbol_idx]; neighbour_idx++);
		if (neighbour_idx == tiles[tiles_hi].neighbours_n) {
			set_symbol(tiles+tiles_hi, symbols[symbol_idx]);
			try_symbol(tiles_hi+1, symbols_hi);
			tiles[tiles_hi].symbol = 0;
		}
	}
	if (tiles_hi > symbols_hi) {
		set_symbol(tiles+tiles_hi, symbols[symbols_hi+1]);
		try_symbol(tiles_hi+1, symbols_hi+1);
		tiles[tiles_hi].symbol = 0;
	}
}

void set_symbol(tile_t *tile, int symbol) {
	int y;
	for (y = tile->y0; y < tile->y1; y++) {
		int x;
		for (x = tile->x0; x < tile->x1; x++) {
			square[y*square_order+x] = symbol;
		}
	}
	tile->symbol = symbol;
}

void print_square(void) {
	int y;
	for (y = 0; y < square_order; y++) {
		int x;
		for (x = 0; x < square_order; x++) {
			putchar(square[y*square_order+x]);
		}
		puts("");
	}
	fflush(stdout);
}
