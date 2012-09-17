/* We want an #include_next, but we are the main source file.
   So, #include ourselves and in that incarnation we can use #include_next.  */
#ifndef INCLUDED_SELF
# define INCLUDED_SELF
# include <clone.S>
#else
# define RESET_PID
# include_next <clone.S>
#endif
