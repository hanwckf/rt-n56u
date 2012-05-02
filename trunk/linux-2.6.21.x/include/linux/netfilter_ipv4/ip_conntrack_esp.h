#ifndef _IP_CONNTRACK_ESP_H
#define _IP_CONNTRACK_ESP_H

struct esp_hdr {
	u_int32_t spi;
	u_int32_t seq;
};

/* structure for original <-> reply keymap */
struct ip_ct_esp_db {
	struct list_head list;
	u_int32_t src;
	u_int32_t dst;
	u_int32_t ospi;
	u_int32_t rspi;
	u_int16_t seq;
	u_int16_t state;
};

extern struct list_head esp_db_list;
extern rwlock_t ip_esp_lock;

extern void ip_conntrack_find_esp_db_get(u_int32_t src, u_int32_t dst,u_int32_t spi,u_int32_t *ospi,u_int32_t *rspi,u_int16_t *esp_direction, u_int16_t *esp_state,u_int16_t *esp_find);
//extern int esp_find_ospi_cmp_fn(const struct ip_ct_esp_db *i,u_int32_t spi);
extern void ip_conntrack_find_esp_db_spi_get(u_int32_t spi,u_int32_t *ospi,u_int32_t *rspi);
extern void ip_conntrack_esp_destroy_db(struct ip_conntrack *ct);
//extern int esp_find_outgoing_spi_cmp_fn(const struct ip_ct_esp_db *i, u_int32_t src, u_int32_t dst,u_int32_t spi);

inline int
esp_find_ospi_cmp_fn(const struct ip_ct_esp_db *i,
		    u_int32_t spi)
{
	return i->ospi == spi;
}

inline int
esp_find_outgoing_cmp_fn(const struct ip_ct_esp_db *i,
		    u_int32_t src, u_int32_t dst)
{
	return i->src== src
		&& i->dst == dst;
}

inline int
esp_find_outgoing_spi_cmp_fn(const struct ip_ct_esp_db *i,
		    u_int32_t src, u_int32_t dst,u_int32_t spi)
{
	return i->src== src && 
		i->dst == dst &&
		i->ospi == spi;
}

inline int
esp_find_incoming_seen_cmp_fn(const struct ip_ct_esp_db *i,
		    u_int32_t src ,u_int32_t spi)
{
	return  i->dst == src &&
		i->rspi == spi;
}

inline int
esp_find_incoming_noseen_cmp_fn(const struct ip_ct_esp_db *i,
		    u_int32_t src )
{
	return  i->dst == src &&
		i->state == 0;
}



#endif /* _IP_CONNTRACK_ESP_H */
