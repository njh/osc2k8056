# Process this file with autoconf to produce a configure script.
AC_INIT([osc2k8056], [0.1], [njh@aelius.com], osc2k8056)
AC_CONFIG_SRCDIR([osc2k8056.c])
AC_CONFIG_AUX_DIR([build-scripts])
AC_CONFIG_MACRO_DIR([build-scripts])
AM_INIT_AUTOMAKE

# Check for C compiler
AC_PROG_CC
AC_PROG_INSTALL

# Check for liblo
PKG_CHECK_MODULES(LIBLO, liblo >= 0.23)
AC_SUBST(LIBLO_CFLAGS)
AC_SUBST(LIBLO_LIBS)

AC_CHECK_HEADERS([termios.h])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
