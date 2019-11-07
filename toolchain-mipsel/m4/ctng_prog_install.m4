# Additional checks for install(1)

# Check if install(1) supports --strip-program=...
AC_DEFUN(
    [CTNG_INSTALL_STRIP_PROGRAM],
    [AC_CACHE_CHECK([whether install takes --strip-program option],
        [ctng_cv_install_with_strip_program],
        [touch conftest
         mkdir conftest.dir
         AS_IF([$INSTALL --strip-program=true -s conftest conftest.dir/conftest 2>/dev/null],
            [ctng_cv_install_with_strip_program=yes],
            [ctng_cv_install_with_strip_program=no])
         rm -rf conftest.dir
         rm -f conftest
        ])
     AS_IF([test "$ctng_cv_install_with_strip_program" = yes], [$1], [$2])
    ])
     
AC_DEFUN([CTNG_PROG_INSTALL],
    [CTNG_WITH_DEPRECATED([install], [INSTALL])
     AC_ARG_VAR([INSTALL], [Specify the full path to a BSD-compatible install])
     AC_PROG_INSTALL
     CTNG_INSTALL_STRIP_PROGRAM(
        [CTNG_SET_KCONFIG_OPTION([install_with_strip_program], [y])],
        [CTNG_SET_KCONFIG_OPTION([install_with_strip_program])])
    ])
