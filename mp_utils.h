typedef struct {
	unsigned long m;
	unsigned long *p;
}
mp_t;

int mp_new(mp_t *);
void mp_print(const char *, mp_t *);
int mp_inc(mp_t *);
int mp_eq_val(mp_t *, unsigned long);
int mp_lt_val(mp_t *, unsigned long);
void mp_free(mp_t *);
