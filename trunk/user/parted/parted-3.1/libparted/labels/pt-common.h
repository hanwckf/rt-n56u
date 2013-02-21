/* Factor some of the duplication out of *.c.  */

#ifdef DISCOVER_ONLY
# define NULL_IF_DISCOVER_ONLY(val) NULL
#else
# define NULL_IF_DISCOVER_ONLY(val) val
#endif

#define PT_define_limit_functions(PT_type)			\
								\
static bool							\
PT_type##_partition_check (const PedPartition *part)		\
{								\
  return ptt_partition_max_start_len (#PT_type, part);		\
}								\
								\
static PedSector						\
PT_type##_partition_max_start_sector (void)			\
{								\
  PedSector max;						\
  int err = ptt_partition_max_start_sector (#PT_type, &max);	\
  PED_ASSERT (err == 0);					\
  return max;							\
}								\
								\
static PedSector						\
PT_type##_partition_max_length (void)				\
{								\
  PedSector max;						\
  int err = ptt_partition_max_length (#PT_type, &max);		\
  PED_ASSERT (err == 0);					\
  return max;							\
}

#define PT_op_function_initializers(PT_type)			\
probe:                        PT_type##_probe,			\
alloc:                        PT_type##_alloc,			\
duplicate:                    PT_type##_duplicate,		\
free:                         PT_type##_free,			\
read:                         PT_type##_read,			\
partition_new:                PT_type##_partition_new,		\
partition_duplicate:          PT_type##_partition_duplicate,	\
partition_set_flag:           PT_type##_partition_set_flag,	\
partition_get_flag:           PT_type##_partition_get_flag,	\
partition_set_system:         PT_type##_partition_set_system,	\
partition_is_flag_available:  PT_type##_partition_is_flag_available, \
partition_align:              PT_type##_partition_align,	\
partition_destroy:            PT_type##_partition_destroy,	\
partition_enumerate:	      PT_type##_partition_enumerate,	\
alloc_metadata:               PT_type##_alloc_metadata,		\
get_max_primary_partition_count: PT_type##_get_max_primary_partition_count, \
get_max_supported_partition_count:PT_type##_get_max_supported_partition_count,\
partition_check:              PT_type##_partition_check,	\
max_length:                   PT_type##_partition_max_length,	\
max_start_sector:             PT_type##_partition_max_start_sector
