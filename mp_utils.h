typedef struct {
	unsigned long m;
	unsigned long *p;
}
mp_t;

int mp_new(mp_t *);
int mp_from_val(mp_t *, unsigned long);
int mp_inc(mp_t *);
int mp_add(mp_t *, mp_t *);
int mp_sub(mp_t *, mp_t *);
int mp_mul(mp_t *, mp_t *);
int mp_divmod(mp_t *, mp_t *, mp_t *);
int mp_copy(mp_t *, mp_t *);
int mp_compare(const mp_t *, const mp_t *);
int mp_eq_val(const mp_t *, unsigned long);
void mp_print(const char *, const mp_t *);
void mp_free(mp_t *);
