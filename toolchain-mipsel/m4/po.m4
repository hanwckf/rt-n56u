# po.m4 serial 24
dnl Dummy version of po.m4 from gettext. Crosstool-NG currently doesn't have
dnl any localications for kconfig (and if we decide to, we'd need much more than
dnl that). So ignore PO_SUBDIRS and any possible dependencies the real po.m4 would
dnl have pulled in.
AC_DEFUN([AM_PO_SUBDIRS], [])
