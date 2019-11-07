/*
 * The hashcode handling code below is heavily inspired in libiberty's
 * hashtab code, but with most adaptation points and support for
 * deleting elements removed.
 *
 * Copyright (C) 1999, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
 * Contributed by Vladimir Makarov (vmakarov@cygnus.com).
 */

#ifndef INLINE_HASHTAB_H
# define INLINE_HASHTAB_H 1

static __always_inline unsigned long
higher_prime_number(unsigned long n)
{
	/* These are primes that are near, but slightly smaller than, a power of two. */
	static const unsigned long primes[] = {
		7,
		13,
		31,
		61,
		127,
		251,
		509,
		1021,
		2039,
		4093,
		8191,
		16381,
		32749,
		65521,
		131071,
		262139,
		524287,
		1048573,
		2097143,
		4194301,
		8388593,
		16777213,
		33554393,
		67108859,
		134217689,
		268435399,
		536870909,
		1073741789,
		/* 4294967291 */
		((unsigned long) 2147483647) + ((unsigned long) 2147483644),
	};
	const unsigned long *low = &primes[0];
	const unsigned long *high = &primes[ARRAY_SIZE(primes)];

	while (low != high) {
		const unsigned long *mid = low + (high - low) / 2;
		if (n > *mid)
			low = mid + 1;
		else
			high = mid;
	}

#if 0
	/* If we've run out of primes, abort.  */
	if (n > *low) {
		fprintf(stderr, "Cannot find prime bigger than %lu\n", n);
		abort();
	}
#endif

	return *low;
}

struct funcdesc_ht
{
	/* Table itself */
	void **entries;

	/* Current size (in entries) of the hash table */
	size_t size;

	/* Current number of elements */
	size_t n_elements;
};

static __always_inline struct funcdesc_ht *
htab_create(void)
{
	struct funcdesc_ht *ht = _dl_malloc(sizeof(*ht));
	size_t ent_size;

	if (!ht)
		return NULL;
	ht->size = 3;
	ent_size = sizeof(void *) * ht->size;
	ht->entries = _dl_malloc(ent_size);
	if (!ht->entries)
		return NULL;

	ht->n_elements = 0;
	_dl_memset(ht->entries, 0, ent_size);

	return ht;
}

/*
 * This is only called from _dl_loadaddr_unmap, so it's safe to call
 * _dl_free().  See the discussion below.
 */
static __always_inline void
htab_delete(struct funcdesc_ht *htab)
{
	int i;

	for (i = htab->size - 1; i >= 0; i--)
		if (htab->entries[i])
			_dl_free(htab->entries[i]);

	_dl_free(htab->entries);
	_dl_free(htab);
}

/*
 * Similar to htab_find_slot, but without several unwanted side effects:
 *  - Does not call htab->eq_f when it finds an existing entry.
 *  - Does not change the count of elements/searches/collisions in the
 *    hash table.
 * This function also assumes there are no deleted entries in the table.
 * HASH is the hash value for the element to be inserted.
 */
static __always_inline void **
find_empty_slot_for_expand(struct funcdesc_ht *htab, int hash)
{
	size_t size = htab->size;
	unsigned int index = hash % size;
	void **slot = htab->entries + index;
	int hash2;

	if (!*slot)
		return slot;

	hash2 = 1 + hash % (size - 2);
	for (;;) {
		index += hash2;
		if (index >= size)
			index -= size;

		slot = htab->entries + index;
		if (!*slot)
			return slot;
	}
}

/*
 * The following function changes size of memory allocated for the
 * entries and repeatedly inserts the table elements.  The occupancy
 * of the table after the call will be about 50%.  Naturally the hash
 * table must already exist.  Remember also that the place of the
 * table entries is changed.  If memory allocation failures are allowed,
 * this function will return zero, indicating that the table could not be
 * expanded.  If all goes well, it will return a non-zero value.
 */
static __always_inline int
htab_expand(struct funcdesc_ht *htab, int (*hash_fn) (void *))
{
	void **oentries;
	void **olimit;
	void **p;
	void **nentries;
	size_t nsize;

	oentries = htab->entries;
	olimit = oentries + htab->size;

	/*
	 * Resize only when table after removal of unused elements is either
	 * too full or too empty.
	 */
	if (htab->n_elements * 2 > htab->size)
		nsize = higher_prime_number(htab->n_elements * 2);
	else
		nsize = htab->size;

	nentries = _dl_malloc(sizeof(*nentries) * nsize);
	_dl_memset(nentries, 0, sizeof(*nentries) * nsize);
	if (nentries == NULL)
		return 0;
	htab->entries = nentries;
	htab->size = nsize;

	p = oentries;
	do {
		if (*p)
			*find_empty_slot_for_expand(htab, hash_fn(*p)) = *p;
		p++;
	} while (p < olimit);

#if 0
	/*
	 * We can't tell whether this was allocated by the _dl_malloc()
	 * built into ld.so or malloc() in the main executable or libc,
	 * and calling free() for something that wasn't malloc()ed could
	 * do Very Bad Things (TM).  Take the conservative approach
	 * here, potentially wasting as much memory as actually used by
	 * the hash table, even if multiple growths occur.  That's not
	 * so bad as to require some overengineered solution that would
	 * enable us to keep track of how it was allocated.
	 */
	_dl_free(oentries);
#endif
	return 1;
}

/*
 * This function searches for a hash table slot containing an entry
 * equal to the given element.  To delete an entry, call this with
 * INSERT = 0, then call htab_clear_slot on the slot returned (possibly
 * after doing some checks).  To insert an entry, call this with
 * INSERT = 1, then write the value you want into the returned slot.
 * When inserting an entry, NULL may be returned if memory allocation
 * fails.
 */
static __always_inline void **
htab_find_slot(struct funcdesc_ht *htab, void *ptr, int insert,
	       int (*hash_fn)(void *), int (*eq_fn)(void *, void *))
{
	unsigned int index;
	int hash, hash2;
	size_t size;
	void **entry;

	if (htab->size * 3 <= htab->n_elements * 4 &&
	    htab_expand(htab, hash_fn) == 0)
		return NULL;

	hash = hash_fn(ptr);

	size = htab->size;
	index = hash % size;

	entry = &htab->entries[index];
	if (!*entry)
		goto empty_entry;
	else if (eq_fn(*entry, ptr))
		return entry;

	hash2 = 1 + hash % (size - 2);
	for (;;) {
		index += hash2;
		if (index >= size)
			index -= size;

		entry = &htab->entries[index];
		if (!*entry)
			goto empty_entry;
		else if (eq_fn(*entry, ptr))
			return entry;
	}

 empty_entry:
	if (!insert)
		return NULL;

	htab->n_elements++;
	return entry;
}

#endif
