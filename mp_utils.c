#include <stdio.h>
#include <stdlib.h>
#include "mp_utils.h"

#define P_SHRT_MAX 1000UL
#define P_LONG_MAX (P_SHRT_MAX*P_SHRT_MAX)
#define P_DIGITS_MAX 6

static int mp_resize(mp_t *, unsigned long);
static int mp_operation(mp_t *, mp_t *, unsigned long, void (*)(unsigned long, unsigned long, unsigned long *, unsigned long *));
static unsigned long *p_copy(mp_t *, unsigned long, unsigned long, unsigned long);
static int mp_replace(mp_t *, unsigned long, unsigned long *);
static unsigned long *p_new(unsigned long);
static void hilo_add(unsigned long, unsigned long, unsigned long *, unsigned long *);
static void hilo_sub(unsigned long, unsigned long, unsigned long *, unsigned long *);
static void hilo_mul(unsigned long, unsigned long, unsigned long *, unsigned long *);

int mp_new(mp_t *mp) {
	mp->m = 1UL;
	mp->p = p_new(1UL);
	return mp->p != NULL;
}

int mp_from_val(mp_t *mp, unsigned long val) {
	if (!mp_new(mp)) {
		return 0;
	}
	mp->p[0] = val%P_LONG_MAX;
	val /= P_LONG_MAX;
	while (val) {
		if (!mp_resize(mp, mp->m+1UL)) {
			return 0;
		}
		mp->p[mp->m-1UL] = val%P_LONG_MAX;
		val /= P_LONG_MAX;
	}
	return 1;
}

int mp_inc(mp_t *mp) {
	unsigned long carry, i = 0UL;
	do {
		hilo_add(mp->p[i], 1UL, mp->p+i, &carry);
		++i;
	}
	while (i < mp->m && carry);
	if (carry) {
		if (!mp_resize(mp, mp->m+1UL)) {
			return 0;
		}
		mp->p[mp->m-1UL] = 1UL;
	}
	return 1;
}

int mp_add(mp_t *mp_a, mp_t *mp_b) {
	unsigned long ma = mp_a->m+1UL;
	if (ma < mp_b->m+1UL) {
		ma = mp_b->m+1UL;
	}
	return mp_operation(mp_a, mp_b, ma, hilo_add);
}

int mp_sub(mp_t *mp_a, mp_t *mp_b) {
	if (mp_compare(mp_a, mp_b) < 0) {
		fputs("mp_sub: mp_a cannot be inferior to mp_b\n", stderr);
		fflush(stderr);
		return 0;
	}
	return mp_operation(mp_a, mp_b, mp_a->m, hilo_sub);
}

int mp_mul(mp_t *mp_a, mp_t *mp_b) {
	unsigned long ma = mp_a->m+mp_b->m, *pa = p_new(ma), i;
	if (!pa) {
		return 0;
	}
	for (i = 0UL; i < mp_a->m; ++i) {
		unsigned long j;
		for (j = 0UL; j < mp_b->m; ++j) {
			unsigned long lo, hi, carry, k = i+j;
			hilo_mul(mp_a->p[i], mp_b->p[j], &lo, &hi);
			hilo_add(pa[k], lo, pa+k, &carry);
			++k;
			hilo_add(pa[k], hi+carry, pa+k, &carry);
			while (carry) {
				++k;
				hilo_add(pa[k], carry, pa+k, &carry);
			}
		}
	}
	return mp_replace(mp_a, ma, pa);
}

int mp_divmod(mp_t *mp_a, mp_t *mp_b, mp_t *mp_r) {
	unsigned long mq, *pq = p_new(mp_a->m);
	if (!pq) {
		return 0;
	}
	if (!mp_copy(mp_a, mp_r)) {
		free(pq);
		return 0;
	}
	mq = 0UL;
	while (mp_compare(mp_b, mp_r) >= 0) {
		unsigned long d_lo, d_hi;
		mp_t mp_h;
		mp_h.m = mp_b->m;
		mp_h.p = p_copy(mp_r, mp_h.m, mp_r->m-mp_h.m, mp_r->m);
		if (!mp_h.p) {
			free(pq);
			return 0;
		}
		if (mp_compare(mp_b, &mp_h) > 0) {
			free(mp_h.p);
			++mp_h.m;
			mp_h.p = p_copy(mp_r, mp_h.m, mp_r->m-mp_h.m, mp_r->m);
			if (!mp_h.p) {
				free(pq);
				return 0;
			}
		}
		d_lo = 0UL;
		d_hi = P_LONG_MAX;
		do {
			unsigned long d = (d_lo+d_hi)/2UL;
			mp_t mp_d1;
			if (!mp_from_val(&mp_d1, d)) {
				mp_free(&mp_h);
				free(pq);
				return 0;
			}
			if (!mp_mul(&mp_d1, mp_b)) {
				mp_free(&mp_d1);
				mp_free(&mp_h);
				free(pq);
				return 0;
			}
			if (mp_compare(&mp_h, &mp_d1) < 0) {
				d_hi = d;
			}
			else {
				mp_t mp_d2;
				if (!mp_copy(&mp_h, &mp_d2)) {
					mp_free(&mp_d1);
					mp_free(&mp_h);
					free(pq);
					return 0;
				}
				if (!mp_sub(&mp_d2, &mp_d1)) {
					mp_free(&mp_d2);
					mp_free(&mp_d1);
					mp_free(&mp_h);
					free(pq);
					return 0;
				}
				if (mp_compare(mp_b, &mp_d2) > 0) {
					unsigned long i;
					pq[mq++] = d;
					for (i = 0UL; i < mp_d2.m; ++i) {
						mp_r->p[i+mp_r->m-mp_h.m] = mp_d2.p[i];
					}
					for (; i < mp_h.m; ++i) {
						mp_r->p[i+mp_r->m-mp_h.m] = 0UL;
					}
					if (!mp_resize(mp_r, mp_r->m-mp_h.m+mp_d2.m)) {
						mp_free(&mp_d2);
						mp_free(&mp_d1);
						mp_free(&mp_h);
						free(pq);
					}
					mp_free(&mp_d2);
					mp_free(&mp_d1);
					break;
				}
				d_lo = d;
			}
		}
		while (1);
		mp_free(&mp_h);
	}
	if (!mq) {
		++mq;
	}
	return 1;
}

int mp_copy(mp_t *mp_a, mp_t *mp_b) {
	mp_b->m = mp_a->m;
	mp_b->p = p_copy(mp_a, mp_a->m, 0UL, mp_a->m);
	return mp_b->p != NULL;
}

int mp_compare(const mp_t *mp_a, const mp_t *mp_b) {
	unsigned long i;
	if (mp_a->m < mp_b->m) {
		return -1;
	}
	if (mp_a->m > mp_b->m) {
		return 1;
	}
	for (i = 0UL; i < mp_a->m && mp_a->p[i] == mp_b->p[i]; ++i);
	if (i == mp_a->m) {
		return 0;
	}
	if (mp_a->p[i] < mp_b->p[i]) {
		return -1;
	}
	return 1;
}

int mp_eq_val(const mp_t *mp, unsigned long val) {
	return mp->m == 1UL && mp->p[0] == val;
}

void mp_print(const char *title, const mp_t *mp) {
	unsigned long i;
	printf("%s %lu", title, mp->p[mp->m-1UL]);
	for (i = mp->m-1UL; i; --i) {
		printf(",%0*lu", P_DIGITS_MAX, mp->p[i-1UL]);
	}
	puts("");
}

void mp_free(mp_t *mp) {
	free(mp->p);
}

static int mp_resize(mp_t *mp, unsigned long m) {
	unsigned long *p = realloc(mp->p, sizeof(unsigned long)*m), i;
	if (!p) {
		fputs("mp_resize: could not reallocate memory for mp->p\n", stderr);
		fflush(stderr);
		return 0;
	}
	for (i = mp->m; i < m; ++i) {
		p[i] = 0UL;
	}
	mp->m = m;
	mp->p = p;
	return 1;
}

static int mp_operation(mp_t *mp_a, mp_t *mp_b, unsigned long ma, void (*hilo_operation)(unsigned long, unsigned long, unsigned long *, unsigned long *)) {
	unsigned long *pa = p_copy(mp_a, ma, 0UL, mp_a->m), i;
	for (i = 0UL; i < mp_b->m; ++i) {
		unsigned long carry, j = i;
		hilo_operation(pa[i], mp_b->p[i], pa+i, &carry);
		while (carry) {
			++j;
			hilo_operation(pa[j], 1UL, pa+j, &carry);
		}
	}
	return mp_replace(mp_a, ma, pa);
}

static unsigned long *p_copy(mp_t *mp, unsigned long m, unsigned long lo, unsigned long hi) {
	unsigned long *p = p_new(m), i;
	if (!p) {
		return NULL;
	}
	for (i = lo; i < hi; ++i) {
		p[i] = mp->p[i];
	}
	return p;
}

static int mp_replace(mp_t *mp, unsigned long m, unsigned long *p) {
	free(mp->p);
	mp->m = m;
	mp->p = p;
	for (; m > 1UL && !p[m-1UL]; --m);
	return mp->m == m || mp_resize(mp, m);
}

static unsigned long *p_new(unsigned long size) {
	unsigned long *p = calloc(size, sizeof(unsigned long));
	if (!p) {
		fputs("p_new: could not allocate memory for p\n", stderr);
		fflush(stderr);
		return NULL;
	}
	return p;
}

static void hilo_add(unsigned long a, unsigned long b, unsigned long *lo, unsigned long *hi) {
	*lo = a+b;
	if (*lo < P_LONG_MAX) {
		*hi = 0UL;
	}
	else {
		*lo -= P_LONG_MAX;
		*hi = 1UL;
	}
}

static void hilo_sub(unsigned long a, unsigned long b, unsigned long *lo, unsigned long *hi) {
	if (a < b) {
		*lo = a+P_LONG_MAX-b;
		*hi = 1UL;
	}
	else {
		*lo = a-b;
		*hi = 0UL;
	}
}

static void hilo_mul(unsigned long a2, unsigned long b2, unsigned long *lo, unsigned long *hi) {
	unsigned long a1 = a2%P_SHRT_MAX, b1 = b2%P_SHRT_MAX, m12, m21, carry1, carry2;
	a2 /= P_SHRT_MAX;
	b2 /= P_SHRT_MAX;
	m12 = a1*b2;
	m21 = a2*b1;
	hilo_add(a1*b1, (m12%P_SHRT_MAX)*P_SHRT_MAX, lo, &carry1);
	hilo_add(*lo, (m21%P_SHRT_MAX)*P_SHRT_MAX, lo, &carry2);
	*hi = carry1+carry2+m12/P_SHRT_MAX+m21/P_SHRT_MAX+a2*b2;
}
