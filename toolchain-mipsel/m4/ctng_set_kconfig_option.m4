# Set the kconfig option.
AC_DEFUN([CTNG_SET_KCONFIG_OPTION],
    [AS_IF(
         [test -n "$$1" -o "$2" = "y" -o "$2" = "1" ],
         [AC_SUBST([KCONFIG_$1], ["def_bool y"])],
         [AC_SUBST([KCONFIG_$1], ["bool"])])
    ])
