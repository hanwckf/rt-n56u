#ifndef __NVRAM_LINUX__
#define __NVRAM_LINUX__

#define NVRAM_MAX_PARAM_LEN	64
#define NVRAM_MAX_VALUE_LEN	4096

struct nvram_pair {
	char *name;
	char *value;
};

extern char *nvram_get(const char *name);
extern char *nvram_safe_get(const char *name);
extern int nvram_get_int(const char *name);
extern int nvram_safe_get_int(const char* name, int val_def, int val_min, int val_max);
extern int nvram_getall(char *buf, int count, int include_temp);

extern int nvram_set(const char *name, const char *value);
extern int nvram_set_int(const char *name, int value);
extern int nvram_unset(const char *name);

extern int nvram_set_temp(const char *name, const char *value);
extern int nvram_set_int_temp(const char *name, int value);

extern int nvram_match(const char *name, char *match);
extern int nvram_invmatch(const char *name, char *invmatch);

extern int nvram_commit(void);
extern int nvram_clear(void);


#endif

