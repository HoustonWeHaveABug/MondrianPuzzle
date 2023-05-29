typedef struct {
	unsigned long m;
	unsigned long *p;
}
mp_t;

int mp_new(mp_t *);
void mp_print(const char *, const mp_t *);
int mp_inc(mp_t *);
int mp_eq_val(const mp_t *, unsigned long);
void mp_free(mp_t *);
