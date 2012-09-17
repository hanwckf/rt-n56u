#define TLS_LE(x)					\
  ({ int *__result;					\
     void *tp = __builtin_thread_pointer ();		\
     __asm__ ("ldr %0, 1f; "				\
	  "add %0, %1, %0; "				\
	  "b 2f; "					\
	  ".align 2; "					\
	  "1: .word " #x "(tpoff); "			\
	  "2: "						\
	  : "=&r" (__result) : "r" (tp));		\
     __result; })

#define TLS_IE(x)					\
  ({ int *__result;					\
     int tmp;						\
     void *tp = __builtin_thread_pointer ();		\
     __asm__ ("ldr %0, 1f; "				\
	  "adr %1, 1f; "				\
	  "ldr %0, [%1, %0]; "				\
	  "add %0, %2, %0; "				\
	  "b 2f; "					\
	  ".align 2; "					\
	  "1: .word " #x "(gottpoff); "			\
	  "2: "						\
	  : "=&r" (__result), "=&r"(tmp) : "r" (tp));	\
     __result; })

#define TLS_LD(x)					\
  ({ char *__result;					\
     int __offset;					\
     extern void *__tls_get_addr (void *);		\
     __asm__ ("ldr %0, 2f; "				\
	  ".align 2; "					\
	  "1: add %0, pc, %0; "				\
	  "b 3f; "					\
	  "2: .word " #x "(tlsldm) + (. - 1b - 4); "	\
	  "3: "						\
	  : "=r" (__result));				\
     __result = (char *)__tls_get_addr (__result);	\
     __asm__ ("ldr %0, 1f; "				\
	  "b 2f; "					\
	  "1: .word " #x "(tlsldo); "			\
	  "2: "						\
	  : "=r" (__offset));				\
     (int *) (__result + __offset); })

#define TLS_GD(x)					\
  ({ int *__result;					\
     extern void *__tls_get_addr (void *);		\
     __asm__ ("ldr %0, 2f; "				\
	  ".align 2; "					\
	  "1: add %0, pc, %0; "				\
	  "b 3f; "					\
	  "2: .word " #x "(tlsgd) + (. - 1b - 4); "	\
	  "3: "						\
	  : "=r" (__result));				\
     (int *)__tls_get_addr (__result); })
