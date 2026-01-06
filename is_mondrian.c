#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MP_SIZE 2
#define TILES_MIN 2
#define TILE_LOCKED 2

typedef struct {
	int height;
	int width;
	int area;
	int delta;
	int slots_n;
	int rotate_flag;
	int area_left;
}
tile_t;

typedef struct option_s option_t;

struct option_s {
	int height;
	int width;
	int area;
	int slots_n;
	int rotate_flag;
	int yh_slot_max;
	int yw_slot_max;
	int slot_height;
	int y_slot_lo;
	int y_slot_hi;
	int slot_width;
	int x_slot_max;
	int x_slot_lo;
	int x_slot_hi;
	option_t *y_last;
	option_t *y_next;
	option_t *x_last;
	option_t *x_next;
};

typedef struct bar_s bar_t;

struct bar_s {
	int y_slot;
	int height;
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

static int run_request(void);
static int is_mondrian(void);
static int can_rotate(const tile_t *);
static int can_be_locked(tile_t *);
static void release_locks(void);
static void print_solution(void);
static void print_lock(const tile_t *);
static int search_y_slot(int, bar_t *, option_t *);
static int check_next_y_slot(bar_t *, int);
static int choose_y_slot(int, bar_t *, option_t *, int, int);
static void rollback_y_slot(bar_t *, bar_t *, bar_t *, int, int);
static int search_x_slot(choice_t *);
static int is_valid_choice(const option_t *, int, int);
static void add_choice(int, int);
static void mp_new(unsigned []);
static void mp_inc(unsigned []);
static void mp_print(const char *, unsigned []);
static int set_tile(tile_t *);
static void copy_tile(option_t *, const tile_t *);
static void set_option(option_t *);
static int compare_options(const void *, const void *);
static void link_options_y(option_t *, option_t *);
static void link_options_x(option_t *, option_t *);
static void print_option(const option_t *);
static void set_bar(bar_t *, int, int, int);
static void insert_bar(bar_t *, bar_t *, bar_t *);
static void link_bars(bar_t *, bar_t *);
static void set_choice(choice_t *, int, int);
static int compare_choices(const choice_t *, const choice_t *);
static void insert_choice(choice_t *, choice_t *, choice_t *);
static void link_choices(choice_t *, choice_t *);
static void flush_log(FILE *, const char *, ...);

static int paint_width, paint_height, rotate_flag, paint_area, tiles_n, height_max, width_max, options_n, bars_n, solutions_n;
static unsigned cost[MP_SIZE];
static tile_t *tiles;
static option_t *options, **solutions, *options_header, *option_sym;
static bar_t *bars, *bars_header;
static choice_t *choices, *choices_header, *choices_hi;

int main(void) {
	rotate_flag = 1;
	while (run_request() >= 0);
	return EXIT_SUCCESS;
}

static int run_request(void) {
	int r = scanf("%d%d%d", &tiles_n, &paint_width, &paint_height), status, choices_n, i;
	if (r == -1) {
		return -1;
	}
	if (r != 3 || tiles_n < TILES_MIN || paint_width < paint_height || paint_height < 1) {
		flush_log(stderr, "Invalid request\n");
		return -1;
	}
	paint_area = paint_height*paint_width;
	tiles = malloc(sizeof(tile_t)*(size_t)tiles_n);
	if (!tiles) {
		flush_log(stderr, "Could not allocate memory for tiles\n");
		return -1;
	}
	for (i = 0; i < tiles_n; ++i) {
		if (!set_tile(tiles+i)) {
			free(tiles);
			return -1;
		}
	}
	if (scanf("%d", &status) != 1 || status < 0 || status > 1) {
		flush_log(stderr, "Invalid status\n");
		free(tiles);
		return -1;
	}
	options = malloc(sizeof(option_t)*(size_t)(tiles_n*2+2));
	if (!options) {
		flush_log(stderr, "Could not allocate memory for options\n");
		free(tiles);
		return -1;
	}
	bars = malloc(sizeof(bar_t)*(size_t)(paint_width+1));
	if (!bars) {
		flush_log(stderr, "Could not allocate memory for bars\n");
		free(options);
		free(tiles);
		return -1;
	}
	bars_header = bars+paint_width;
	insert_bar(bars, bars_header, bars_header);
	solutions = malloc(sizeof(option_t *)*(size_t)tiles_n);
	if (!solutions) {
		flush_log(stderr, "Could not allocate memory for solutions\n");
		free(bars);
		free(options);
		free(tiles);
		return -1;
	}
	choices_n = tiles_n*2;
	choices = malloc(sizeof(choice_t)*(size_t)(choices_n+1));
	if (!choices) {
		flush_log(stderr, "Could not allocate memory for choices\n");
		free(solutions);
		free(bars);
		free(options);
		free(tiles);
		return -1;
	}
	choices_header = choices+choices_n;
	set_choice(choices, 0, 0);
	r = is_mondrian();
	if (r != status) {
		flush_log(stdout, "STATUS MISMATCH\n");
	}
	free(choices);
	free(solutions);
	free(bars);
	free(options);
	free(tiles);
	return r;
}

static int is_mondrian(void) {
	int r, i;
	option_t *option;
	height_max = paint_height;
	width_max = paint_width;
	if (height_max < width_max) {
		for (i = 0; i < tiles_n && can_rotate(tiles+i); ++i);
		if (i == tiles_n) {
			int len = height_max;
			height_max = width_max;
			width_max = len;
		}
	}
	do {
		for (i = 0; i < tiles_n; ++i) {
			r = can_be_locked(tiles+i);
			if (r < 0) {
				release_locks();
				return 0;
			}
			if (r) {
				break;
			}
		}
	}
	while (i < tiles_n);
	options_n = 0;
	for (i = 0; i < tiles_n; ++i) {
		if (tiles[i].rotate_flag < TILE_LOCKED) {
			copy_tile(options+options_n, tiles+i);
			++options_n;
		}
	}
	if (!options_n) {
		print_solution();
		flush_log(stdout, "cost 0\n");
		release_locks();
		return 1;
	}
	qsort(options, (size_t)options_n, sizeof(option_t), compare_options);
	options_header = options+options_n;
	for (i = options_n; i--; ) {
		link_options_y(options+i, options+i+1);
	}
	link_options_y(options_header, options);
	option_sym = options;
	for (option = options->y_next; option != options_header; option = option->y_next) {
		if (option->yh_slot_max > option_sym->yh_slot_max || (option->yh_slot_max == option_sym->yh_slot_max && option->yw_slot_max > option_sym->yw_slot_max)) {
			option_sym = option;
		}
	}
	for (option = options_header->y_next; option != options_header; option = option->y_next) {
		if (option != option_sym) {
			option->yh_slot_max = height_max-option->height;
			option->yw_slot_max = height_max-option->width;
		}
		else {
			option->rotate_flag &= height_max != width_max;
			option->yh_slot_max = (height_max-option->height)/2;
			option->yw_slot_max = (height_max-option->width)/2;
		}
	}
	mp_new(cost);
	set_bar(bars, 0, height_max, width_max);
	set_bar(bars_header, height_max, 0, 0);
	bars_n = 1;
	r = search_y_slot(options_n, bars, options);
	mp_print("cost", cost);
	release_locks();
	return r;
}

static int can_rotate(const tile_t *tile) {
	return tile->width <= height_max && tile->height <= width_max;
}

static int can_be_locked(tile_t *tile) {
	if (tile->rotate_flag) {
		return 0;
	}
	if (tile->height > height_max || tile->width > width_max) {
		return -1;
	}
	if (tile->height == height_max) {
		tile->rotate_flag = TILE_LOCKED;
		width_max -= tile->width;
		return 1;
	}
	if (tile->width == width_max) {
		tile->rotate_flag = TILE_LOCKED;
		height_max -= tile->height;
		return 1;
	}
	return 0;
}

static void release_locks(void) {
	int i;
	for (i = tiles_n; i--; ) {
		if (tiles[i].rotate_flag == TILE_LOCKED) {
			tiles[i].rotate_flag = 0;
		}
	}
}

static void print_solution(void) {
	int i;
	printf("%d %d %d\n", height_max, width_max, solutions_n);
	for (i = 0; i < solutions_n; ++i) {
		print_option(solutions[i]);
	}
	if (height_max < paint_height || width_max < paint_width) {
		puts("Locks");
		for (i = tiles_n; i--; ) {
			print_lock(tiles+i);
		}
	}
	fflush(stdout);
}

static void print_lock(const tile_t *tile) {
	if (tile->rotate_flag == TILE_LOCKED) {
		printf("%dx%d\n", tile->height, tile->width);
	}
}

static int search_y_slot(int bars_hi, bar_t *bar_start, option_t *options_start) {
	int r, i;
	mp_inc(cost);
	if (bars_hi < bars_n) {
		return 0;
	}
	if (bar_start != bars_header) {
		int y_slot = bar_start->y_slot, slot_width = bar_start->x_space, x_max, y_min;
		option_t *option, *last_chance;
		bar_t *bar_cur, *bar_cur_next, *bar_start_next, *bar;
		if (bars_hi == bars_n) {
			for (option = options_start; option != options_header; option = option->y_next) {
				if (option->yh_slot_max < y_slot) {
					return 0;
				}
				if (option->width == slot_width) {
					if (check_next_y_slot(bar_start, y_slot+option->height) && choose_y_slot(bars_hi, bar_start, option, option->height, option->width)) {
						return 1;
					}
				}
				else if (option->rotate_flag && option->yw_slot_max >= y_slot && option->height == slot_width && check_next_y_slot(bar_start, y_slot+option->width) && choose_y_slot(bars_hi, bar_start, option, option->width, option->height)) {
					return 1;
				}
			}
			return 0;
		}
		if (bars_hi == bars_n+1) {
			for (option = options_start; option != options_header; option = option->y_next) {
				if (option->yh_slot_max < y_slot) {
					return 0;
				}
				if (((option->width == slot_width || (option->width < slot_width && check_next_y_slot(bar_start, y_slot+option->height))) && choose_y_slot(bars_hi, bar_start, option, option->height, option->width)) || (option->rotate_flag && option->yw_slot_max >= y_slot && (option->height == slot_width || (option->height < slot_width && check_next_y_slot(bar_start, y_slot+option->width))) && choose_y_slot(bars_hi, bar_start, option, option->width, option->height))) {
					return 1;
				}
			}
			return 0;
		}
		x_max = 0;
		y_min = height_max;
		for (option = options_start; option != options_header; option = option->y_next) {
			if (option->yh_slot_max < y_slot) {
				return 0;
			}
			if (option->width <= slot_width) {
				x_max += option->width;
				if (option->height < y_min) {
					y_min = option->height;
				}
			}
			else if (option->rotate_flag && option->yw_slot_max >= y_slot && option->height <= slot_width) {
				x_max += option->height;
				if (option->width < y_min) {
					y_min = option->width;
				}
			}
		}
		if (x_max < slot_width) {
			return 0;
		}
		for (bar_cur = bar_start; bar_cur != bars_header && bar_cur->height <= y_min; bar_cur = bar_cur->next) {
			y_min -= bar_cur->height;
			bar_cur->x_space -= slot_width;
		}
		bar_cur_next = bar_cur->next;
		if (y_min) {
			set_bar(bars+bars_n, bar_cur->y_slot+y_min, bar_cur->height-y_min, bar_cur->x_space);
			bar_cur->x_space -= slot_width;
			insert_bar(bars+bars_n, bar_cur, bar_cur_next);
		}
		bar_start_next = bar_start->next;
		last_chance = options_header;
		for (option = options_header->y_next; option != options_header; option = option->y_next) {
			if (option->rotate_flag) {
				for (bar = bar_start_next; bar != bars_header && bar->x_space < option->height; bar = bar->next);
				if (bar == bars_header || bar->y_slot > option->yw_slot_max) {
					for (; bar != bars_header && bar->x_space < option->width; bar = bar->next);
					if (bar == bars_header || bar->y_slot > option->yh_slot_max) {
						if (option < options_start || ((option->yw_slot_max < y_slot || option->height > slot_width) && option->width > slot_width)) {
							rollback_y_slot(bar_start, bar_cur, bar_cur_next, y_min, slot_width);
							return 0;
						}
						if (option < last_chance) {
							last_chance = option;
						}
					}
				}
			}
			else {
				for (bar = bar_start_next; bar != bars_header && bar->x_space < option->width; bar = bar->next);
				if (bar == bars_header || bar->y_slot > option->yh_slot_max) {
					if (option < options_start || option->width > slot_width) {
						rollback_y_slot(bar_start, bar_cur, bar_cur_next, y_min, slot_width);
						return 0;
					}
					if (option < last_chance) {
						last_chance = option;
					}
				}
			}
		}
		rollback_y_slot(bar_start, bar_cur, bar_cur_next, y_min, slot_width);
		for (bar = bar_start->next; bar != bars_header; bar = bar->next) {
			for (option = options_header->y_next; option != options_header && (option->yh_slot_max < bar->y_slot || option->width > bar->x_space) && (!option->rotate_flag || option->yw_slot_max < bar->y_slot || option->height > bar->x_space); option = option->y_next);
			if (option == options_header) {
				return 0;
			}
		}
		for (option = options_start; option != options_header; option = option->y_next) {
			if (option->width <= slot_width) {
				if ((choose_y_slot(bars_hi, bar_start, option, option->height, option->width)) || (option->rotate_flag && option->yw_slot_max >= y_slot && option->height <= slot_width && choose_y_slot(bars_hi, bar_start, option, option->width, option->height))) {
					return 1;
				}
				x_max -= option->width;
			}
			else if (option->rotate_flag && option->yw_slot_max >= y_slot && option->height <= slot_width) {
				if (choose_y_slot(bars_hi, bar_start, option, option->width, option->height)) {
					return 1;
				}
				x_max -= option->height;
			}
			if (x_max < slot_width || option == last_chance) {
				return 0;
			}
		}
		return 0;
	}
	for (i = options_n; i--; ) {
		set_option(options+i);
		link_options_x(options+i, options+i+1);
	}
	link_options_x(options_header, options);
	solutions_n = 0;
	insert_choice(choices, choices_header, choices_header);
	choices_hi = choices;
	r = search_x_slot(choices);
	link_choices(choices_header, choices_header);
	return r;
}

static int check_next_y_slot(bar_t *bar_start, int y_slot) {
	bar_t *bar;
	for (bar = bar_start->next; bar != bars_header && bar->y_slot < y_slot; bar = bar->next);
	return bar->y_slot == y_slot;
}

static int choose_y_slot(int bars_hi, bar_t *bar_start, option_t *option, int slot_height, int slot_width) {
	int r;
	bar_t *bar_cur, *bar;
	option->slot_height = slot_height;
	option->y_slot_lo = bar_start->y_slot;
	link_options_y(option->y_last, option->y_next);
	for (bar_cur = bar_start; bar_cur != bars_header && bar_cur->height <= slot_height; bar_cur = bar_cur->next) {
		slot_height -= bar_cur->height;
		bar_cur->x_space -= slot_width;
	}
	bar = bar_cur->next;
	if (slot_height) {
		set_bar(bars+bars_n, bar_cur->y_slot+slot_height, bar_cur->height-slot_height, bar_cur->x_space);
		bar_cur->height = slot_height;
		bar_cur->x_space -= slot_width;
		insert_bar(bars+bars_n, bar_cur, bar);
		++bars_n;
	}
	r = bar_start->x_space ? search_y_slot(bars_hi-1, bar_start, option->y_next):search_y_slot(bars_hi, bar_start->next, options_header->y_next);
	if (slot_height) {
		--bars_n;
		link_bars(bar_cur, bar);
		bar_cur->x_space += slot_width;
		bar_cur->height += bars[bars_n].height;
	}
	for (bar = bar_start; bar != bar_cur; bar = bar->next) {
		bar->x_space += slot_width;
	}
	option->y_next->y_last = option;
	option->y_last->y_next = option;
	return r;
}

static void rollback_y_slot(bar_t *bar_start, bar_t *bar_cur, bar_t *bar_cur_next, int y_min, int slot_width) {
	bar_t *bar;
	if (y_min) {
		link_bars(bar_cur, bar_cur_next);
		bar_cur->x_space += slot_width;
	}
	for (bar = bar_start; bar != bar_cur; bar = bar->next) {
		bar->x_space += slot_width;
	}
}

static int search_x_slot(choice_t *choices_lo) {
	mp_inc(cost);
	if (options_header->x_next != options_header) {
		option_t *option;
		for (; choices_lo != choices_header; choices_lo = choices_lo->next) {
			int i;
			for (i = 0; i < solutions_n && is_valid_choice(solutions[i], choices_lo->y_slot, choices_lo->x_slot); ++i);
			if (i == solutions_n) {
				break;
			}
		}
		if (choices_lo == choices_header) {
			return 0;
		}
		for (option = options_header->x_next; option != options_header; option = option->x_next) {
			if (option->y_slot_lo == choices_lo->y_slot) {
				int i;
				if (option->x_slot_max < choices_lo->x_slot) {
					return 0;
				}
				for (i = 0; i < solutions_n && (choices_lo->y_slot >= solutions[i]->y_slot_hi || choices_lo->x_slot >= solutions[i]->x_slot_hi || choices_lo->x_slot+option->slot_width <= solutions[i]->x_slot_lo); ++i);
				if (i == solutions_n) {
					int r;
					option->x_slot_lo = choices_lo->x_slot;
					option->x_slot_hi = choices_lo->x_slot+option->slot_width;
					link_options_x(option->x_last, option->x_next);
					solutions[solutions_n++] = option;
					if (option->y_slot_hi < height_max) {
						add_choice(option->y_slot_hi, option->x_slot_lo);
					}
					if (option->x_slot_hi < width_max) {
						add_choice(option->y_slot_lo, option->x_slot_hi);
					}
					r = search_x_slot(choices_lo->next);
					if (option->x_slot_hi < width_max) {
						link_choices(choices_hi->last, choices_hi->next);
						--choices_hi;
					}
					if (option->y_slot_hi < height_max) {
						link_choices(choices_hi->last, choices_hi->next);
						--choices_hi;
					}
					--solutions_n;
					option->x_next->x_last = option;
					option->x_last->x_next = option;
					if (r) {
						return r;
					}
				}
			}
		}
		return 0;
	}
	print_solution();
	return 1;
}

static int is_valid_choice(const option_t *option, int y_slot, int x_slot) {
	return option->y_slot_hi <= y_slot || option->x_slot_lo > x_slot || option->x_slot_hi <= x_slot;
}

static void add_choice(int y_slot, int x_slot) {
	choice_t *choice;
	++choices_hi;
	set_choice(choices_hi, y_slot, x_slot);
	for (choice = choices_header->last; choice != choices_header && compare_choices(choice, choices_hi) > 0; choice = choice->last);
	insert_choice(choices_hi, choice, choice->next);
}

static void mp_new(unsigned mp[]) {
	mp[0] = 0;
	mp[1] = 0;
}

static void mp_inc(unsigned mp[]) {
	if (mp[0] < UINT_MAX) {
		++mp[0];
	}
	else {
		mp[0] = 0;
		++mp[1];
	}
}

static void mp_print(const char *label, unsigned mp[]) {
	printf("%s ", label);
	if (mp[1]) {
		printf("%u*%u+", mp[1], UINT_MAX);
	}
	flush_log(stdout, "%u\n", mp[0]);
}

static int set_tile(tile_t *tile) {
	int width, height;
	if (scanf("%d%d", &width, &height) != 2 || width < height || width > paint_width || height < 1 || height > paint_height) {
		fputs("Invalid tile\n", stderr);
		fflush(stderr);
		return 0;
	}
	tile->height = height;
	tile->width = width;
	tile->area = height*width;
	tile->delta = width-height;
	tile->slots_n = (paint_height-height+1)*(paint_width-width+1);
	tile->rotate_flag = rotate_flag && tile->delta && width < paint_width && width <= paint_height;
	if (tile->rotate_flag) {
		tile->slots_n += (paint_height-width+1)*(paint_width-height+1);
	}
	return 1;
}

static void copy_tile(option_t *option, const tile_t *tile) {
	option->height = tile->height;
	option->width = tile->width;
	option->area = tile->area;
	option->slots_n = (height_max-tile->height+1)*(width_max-tile->width+1);
	option->rotate_flag = rotate_flag && tile->delta && tile->width <= height_max;
	option->yh_slot_max = height_max-tile->height;
	option->yw_slot_max = width_max-tile->width;
	if (option->rotate_flag) {
		option->slots_n += (height_max-tile->width+1)*(width_max-tile->height+1);
	}
}

static void set_option(option_t *option) {
	option->y_slot_hi = option->y_slot_lo+option->slot_height;
	option->slot_width = option->slot_height == option->height ? option->width:option->height;
	option->x_slot_max = option != option_sym ? width_max-option->slot_width:(width_max-option->slot_width)/2;
}

static int compare_options(const void *a, const void *b) {
	const option_t *option_a = (const option_t *)a, *option_b = (const option_t *)b;
	if (option_a->slots_n != option_b->slots_n) {
		return option_a->slots_n-option_b->slots_n;
	}
	if (option_a->area != option_b->area) {
		return option_a->area-option_b->area;
	}
	return option_b->width-option_a->width;
}

static void link_options_y(option_t *last, option_t *next) {
	last->y_next = next;
	next->y_last = last;
}

static void link_options_x(option_t *last, option_t *next) {
	last->x_next = next;
	next->x_last = last;
}

static void print_option(const option_t *option) {
	printf("%dx%d;%dx%d\n", option->y_slot_lo, option->x_slot_lo, option->slot_height, option->slot_width);
}

static void set_bar(bar_t *bar, int y_slot, int height, int x_space) {
	bar->y_slot = y_slot;
	bar->height = height;
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

static void flush_log(FILE *fd, const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(fd, format, args);
	va_end(args);
	fflush(fd);
}
