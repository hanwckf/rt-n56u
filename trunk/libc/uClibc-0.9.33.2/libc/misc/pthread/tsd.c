/* libpthread sets _dl_error_catch_tsd to point to this function.
   We define it here instead of in libpthread so t here instead of in libpthread so that it doesn't
   need to have a TLS segment of its own just for this one pointer.  */

void **__libc_dl_error_tsd(void) __attribute__ ((const));
void ** __attribute__ ((const))
__libc_dl_error_tsd (void)
{
  static __thread void *data __attribute__ ((tls_model ("initial-exec")));
  return &data;
}
