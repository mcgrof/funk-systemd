#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Funk Systemd Sample Daemon],  [v1.0],
	[mcgrof@do-not-panic.com],
	[funk],
	[http://github.com/mcgrof/funkd/])
AC_CONFIG_SRCDIR([funk_dynamic.c])
AC_CONFIG_FILES([Makefile
		 Config.mk
		 units/funk.service
		 units/funk.socket])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])

AC_CANONICAL_HOST
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
# FIXME: Replace `main' with a function in `-ldl':
AC_CHECK_LIB([dl], [main])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset mkdir socket strerror])

m4_include([m4/features.m4])
m4_include([m4/systemd.m4])
m4_include([m4/paths.m4])

AX_LOCAL_EXPAND_CONFIG()
AX_AVAILABLE_SYSTEMD()
AM_CONDITIONAL(CONFIG_SYSTEMD, [test x"$systemd" = xy])
AC_OUTPUT
