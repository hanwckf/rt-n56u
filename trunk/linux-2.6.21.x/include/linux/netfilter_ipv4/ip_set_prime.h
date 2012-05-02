#ifndef __IP_SET_PRIME_H
#define __IP_SET_PRIME_H

static inline unsigned make_prime_bound(unsigned nr)
{
	unsigned long long nr64 = nr;
	unsigned long long x = 1;
	nr = 1;
	while (x <= nr64) { x <<= 2; nr <<= 1; }
	return nr;
}

static inline int make_prime_check(unsigned nr)
{
	unsigned x = 3;
	unsigned b = make_prime_bound(nr);
	while (x <= b) {
		if (0 == (nr % x)) return 0;
		x += 2;
	}
	return 1;
}

static unsigned make_prime(unsigned nr)
{
	if (0 == (nr & 1)) nr--;
	while (nr > 1) {
		if (make_prime_check(nr)) return nr;
		nr -= 2;
	}
	return 2;
}

#endif /* __IP_SET_PRIME_H */
