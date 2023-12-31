#include <stdio.h>
#include <stdlib.h>

#define PAINT_WIDTH_MIN 3U
#define SIZE_T_MAX (size_t)-1
#define TILES_MIN 2U
#define TILES_MAX 52U

typedef struct tile_s tile_t;

struct tile_s {
	unsigned y0;
	unsigned x0;
	unsigned y1;
	unsigned x1;
	unsigned neighbours_n;
	tile_t **neighbours;
	int symbol;
};

static void set_tile(tile_t *, unsigned, unsigned, unsigned, unsigned);
static int are_neighbours(const tile_t *, const tile_t *);
static void try_symbol(unsigned, unsigned);
static void set_symbol(tile_t *, int);
static void print_paint(void);

static int symbols[TILES_MAX] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' }, *paint;
static unsigned paint_height, paint_width, tiles_n, symbols_max;
static tile_t *tiles, **neighbours_cur;

int main(void) {
	int minimize_flag;
	unsigned paint_area, tiles_area_sum, tile_idx;
	tile_t **neighbours;
	if (scanf("%d%u%u%u", &minimize_flag, &paint_height, &paint_width, &tiles_n) != 4 || paint_height < 1 || paint_width < PAINT_WIDTH_MIN || paint_height > paint_width || paint_height > SIZE_T_MAX/paint_width || tiles_n < TILES_MIN || tiles_n > TILES_MAX) {
		fprintf(stderr, "Expected parameters: minimize_flag, paint_height (>= 1), paint_width (>= %d and >= paint_height), tiles_n (>= %d).\n", PAINT_WIDTH_MIN, TILES_MIN);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	paint_area = paint_height*paint_width;
	if (sizeof(int) > SIZE_T_MAX/paint_area) {
		fputs("Will not be able to allocate memory for paint\n", stderr);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	paint = calloc((size_t)paint_area, sizeof(int));
	if (!paint) {
		fputs("Could not allocate memory for paint\n", stderr);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	tiles = malloc(sizeof(tile_t)*(size_t)tiles_n);
	if (!tiles) {
		fputs("Could not allocate memory for tiles\n", stderr);
		fflush(stderr);
		free(paint);
		return EXIT_FAILURE;
	}
	neighbours = malloc(sizeof(tile_t *)*(size_t)(tiles_n*(tiles_n-1U)/2U));
	if (!neighbours) {
		fputs("Could not allocate memory for neighbours\n", stderr);
		fflush(stderr);
		free(paint);
		return EXIT_FAILURE;
	}
	neighbours_cur = neighbours;
	tiles_area_sum = 0U;
	for (tile_idx = 0U; tile_idx < tiles_n; ++tile_idx) {
		unsigned y0, x0, h, w, y;
		if (scanf("%ux%u;%ux%u", &y0, &x0, &h, &w) != 4 || y0+h > paint_height || x0+w > paint_width) {
			fputs("Invalid tile\n", stderr);
			fflush(stderr);
			free(tiles);
			free(paint);
			return EXIT_FAILURE;
		}
		for (y = y0; y < y0+h; ++y) {
			unsigned x;
			for (x = x0; x < x0+w; ++x) {
				unsigned paint_idx = y*paint_width+x;
				if (paint[paint_idx]) {
					fprintf(stderr, "Overlap detected at %ux%u\n", y, x);
					fflush(stderr);
					free(tiles);
					free(paint);
					return EXIT_FAILURE;
				}
				paint[paint_idx] = symbols[tile_idx];
			}
		}
		set_tile(tiles+tile_idx, y0, x0, h, w);
		tiles_area_sum += h*w;
	}
	if (tiles_area_sum < paint_area) {
		fputs("Paint not fully covered\n", stderr);
		fflush(stderr);
		free(tiles);
		free(paint);
		return EXIT_FAILURE;
	}
	if (minimize_flag) {
		symbols_max = tiles_n;
		try_symbol(0U, 0U);
	}
	else {
		print_paint();
	}
	free(tiles);
	free(paint);
	return EXIT_SUCCESS;
}

static void set_tile(tile_t *tile_a, unsigned y0, unsigned x0, unsigned h, unsigned w) {
	tile_t *tile_b;
	tile_a->y0 = y0;
	tile_a->x0 = x0;
	tile_a->y1 = y0+h;
	tile_a->x1 = x0+w;
	tile_a->neighbours_n = 0U;
	tile_a->neighbours = neighbours_cur;
	for (tile_b = tiles; tile_b < tile_a; ++tile_b) {
		if (are_neighbours(tile_a, tile_b)) {
			++tile_a->neighbours_n;
			*neighbours_cur = tile_b;
			++neighbours_cur;
		}
	}
	tile_a->symbol = 0U;
}

static int are_neighbours(const tile_t *tile_a, const tile_t *tile_b) {
	if (tile_a->y0 < tile_b->y0) {
		if (tile_a->x0 < tile_b->x0) {
			return (tile_a->y1 == tile_b->y0 && tile_a->x1 >= tile_b->x0) || (tile_a->y1 >= tile_b->y0 && tile_a->x1 == tile_b->x0);
		}
		return (tile_a->y1 == tile_b->y0 && tile_a->x0 <= tile_b->x1) || (tile_a->y1 >= tile_b->y0 && tile_a->x0 == tile_b->x1);
	}
	if (tile_a->x0 < tile_b->x0) {
		return (tile_a->y0 == tile_b->y1 && tile_a->x1 >= tile_b->x0) || (tile_a->y0 <= tile_b->y1 && tile_a->x1 == tile_b->x0);
	}
	return (tile_a->y0 == tile_b->y1 && tile_a->x0 <= tile_b->x1) || (tile_a->y0 <= tile_b->y1 && tile_a->x0 == tile_b->x1);
}

static void try_symbol(unsigned tiles_hi, unsigned symbols_hi) {
	unsigned symbol_idx;
	if (symbols_hi >= symbols_max) {
		return;
	}
	if (tiles_hi == tiles_n) {
		symbols_max = symbols_hi;
		print_paint();
		return;
	}
	for (symbol_idx = symbols_hi+1U; symbol_idx--; ) {
		unsigned neighbour_idx;
		for (neighbour_idx = 0U; neighbour_idx < tiles[tiles_hi].neighbours_n && tiles[tiles_hi].neighbours[neighbour_idx]->symbol != symbols[symbol_idx]; ++neighbour_idx);
		if (neighbour_idx == tiles[tiles_hi].neighbours_n) {
			set_symbol(tiles+tiles_hi, symbols[symbol_idx]);
			try_symbol(tiles_hi+1U, symbols_hi);
			tiles[tiles_hi].symbol = 0;
		}
	}
	if (tiles_hi > symbols_hi) {
		set_symbol(tiles+tiles_hi, symbols[symbols_hi+1U]);
		try_symbol(tiles_hi+1U, symbols_hi+1U);
		tiles[tiles_hi].symbol = 0;
	}
}

static void set_symbol(tile_t *tile, int symbol) {
	unsigned y;
	for (y = tile->y0; y < tile->y1; ++y) {
		unsigned x;
		for (x = tile->x0; x < tile->x1; ++x) {
			paint[y*paint_width+x] = symbol;
		}
	}
	tile->symbol = symbol;
}

static void print_paint(void) {
	unsigned y;
	for (y = 0U; y < paint_height; ++y) {
		unsigned x;
		for (x = 0U; x < paint_width; ++x) {
			putchar(paint[y*paint_width+x]);
		}
		puts("");
	}
	fflush(stdout);
}
