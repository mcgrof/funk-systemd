# systemd.m4 - Macros to check for and enable systemd          -*- Autoconf -*-
#
# Copyright (C) 2014,2019 Luis Chamberlain <mcgrof@kernel.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

dnl Some optional path options when your system is detected as systemd being
dnl the init process.
AC_DEFUN([AX_SYSTEMD_INIT_OPTIONS], [
	AC_ARG_WITH(systemdisinit, [  --with-systemd-user     set user directory for systemd service files],
		SYSTEMD_USER_DIR="$withval", SYSTEMD_USER_DIR="")
	AC_SUBST(SYSTEMD_USER_DIR)

	AC_ARG_WITH(systemdisinit, [  --with-systemd          set directory for systemd service files],
		SYSTEMD_DIR="$withval", SYSTEMD_DIR="")
	AC_SUBST(SYSTEMD_DIR)

	AC_ARG_WITH(systemdisinit, [  --with-systemd-modules-load          set directory for systemd modules load files],
		SYSTEMD_MODULES_LOAD="$withval", SYSTEMD_MODULES_LOAD="")
	AC_SUBST(SYSTEMD_MODULES_LOAD)
])

AC_DEFUN([AX_ENABLE_SYSTEMD_SYSTEM_OPTS], [
	AX_ARG_DEFAULT_ENABLE([systemdisinit], [Confirms systemd is your init process])
	AX_SYSTEMD_INIT_OPTIONS()
])

AC_DEFUN([AX_ALLOW_SYSTEMD_SYSTEM_OPTS], [
	AX_ARG_DEFAULT_DISABLE([systemdisinit], [Disables systemd as your init process])
	AX_SYSTEMD_INIT_OPTIONS()
])

AC_DEFUN([AX_ENABLE_SYSTEMD_OPTS], [
	AX_ARG_DEFAULT_ENABLE([systemd], [Disable systemd development support])
])

AC_DEFUN([AX_ENABLE_SYSTEMD_INIT_OPTS], [
	AX_ARG_DEFAULT_ENABLE([systemdisinit], [Disable systemd init support])
	AX_SYSTEMD_INIT_OPTIONS()
])

AC_DEFUN([AX_ALLOW_SYSTEMD_OPTS], [
	AX_ARG_DEFAULT_DISABLE([systemd], [Enable systemd development support])
])

AC_DEFUN([AX_ALLOW_SYSTEMD_INIT_OPTS], [
	AX_ARG_DEFAULT_DISABLE([systemdisinit], [Enable systemd init support])
	AX_SYSTEMD_INIT_OPTIONS()
])

AC_DEFUN([AX_CHECK_SYSTEMD_LIBS], [
	AC_CHECK_HEADER([systemd/sd-daemon.h], [
	    AC_CHECK_LIB([systemd-daemon], [sd_listen_fds], [libsystemddaemon="y"])
	])
	AS_IF([test "x$libsystemddaemon" = x], [
	    AC_MSG_ERROR([Unable to find a suitable libsystemd-daemon library])
	])

	PKG_CHECK_MODULES([SYSTEMD], [libsystemd-daemon])
	dnl pkg-config older than 0.24 does not set these for
	dnl PKG_CHECK_MODULES() worth also noting is that as of version 208
	dnl of systemd pkg-config --cflags currently yields no extra flags yet.
	AC_SUBST([SYSTEMD_CFLAGS])
	AC_SUBST([SYSTEMD_LIBS])

])

AC_DEFUN([AX_CHECK_SYSTEMD_VARS], [
	AS_IF([test "x$SYSTEMD_DIR" = x], [
	    SYSTEMD_DIR='${prefix}/lib/systemd/system/'
	    AC_SUBST(SYSTEMD_DIR)
	], [])

	AS_IF([test "x$SYSTEMD_USER_DIR" = x], [
	    SYSTEMD_USER_DIR='${prefix}/lib/systemd/user'
	    AC_SUBST(SYSTEMD_USER_DIR)
	], [])

	AS_IF([test "x$SYSTEMD_DIR" = x], [
	    AC_MSG_ERROR([SYSTEMD_DIR is unset])
	], [])

	AS_IF([test "x$SYSTEMD_USER_DIR" = x], [
	    AC_MSG_ERROR([SYSTEMD_USER_DIR is unset])
	], [])

	dnl There is no variable for this yet for some reason
	AS_IF([test "x$SYSTEMD_MODULES_LOAD" = x], [
	    SYSTEMD_MODULES_LOAD='${prefix}/lib/modules-load.d/'
	], [])

	AS_IF([test "x$SYSTEMD_MODULES_LOAD" = x], [
	    AC_MSG_ERROR([SYSTEMD_MODULES_LOAD is unset])
	], [])
])


AC_DEFUN([AX_CHECK_SYSTEMD], [
	dnl Respect user override to disable
	AS_IF([test "x$enable_systemd" != "xno"], [
	     AS_IF([test "x$systemd" = "xy" ], [
		AC_DEFINE([HAVE_SYSTEMD], [1], [Systemd development libraries available and enabled])
			systemd=y
			AX_CHECK_SYSTEMD_LIBS()
	    ],[systemd=n])
	],[systemd=n])
])

AC_DEFUN([AX_CHECK_SYSTEMD_INIT], [
	dnl Respect user override to disable
	AS_IF([test "x$enable_systemdinit" != "xno"], [
	     AS_IF([test "x$systemdisinit" = "xy" ], [
		AC_DEFINE([HAVE_SYSTEMD_INIT], [1], [Systemd init running])
			systemdisinit=y
			AX_CHECK_SYSTEMD_VARS()
	    ],[systemdisinit=n])
	],[systemdisinit=n])
])

AC_DEFUN([AX_CHECK_SYSTEMD_INIT_ENABLE_AVAILABLE], [
	AC_CACHE_CHECK([if systemd is your init], [systemd_cv_init],
	     [systemd_cv_init=no
	      systemdpid=x`pidof -s systemd`
	      if test "$systemdpid" != x; then
		systemd_cv_init=yes
		systemdisinit=y
	      fi])
])

AC_DEFUN([AX_CHECK_SYSTEMD_ENABLE_AVAILABLE], [
	AC_CHECK_HEADER([systemd/sd-daemon.h], [
	    AC_CHECK_LIB([systemd-daemon], [sd_listen_fds], [systemd="y"])
	])
])

dnl Enables systemd by default and requires a --disable-systemd option flag
dnl to configure if you want to disable.
AC_DEFUN([AX_ENABLE_SYSTEMD], [
	AX_ENABLE_SYSTEMD_INIT_OPTS()
	AX_ENABLE_SYSTEMD_OPTS()
	AX_CHECK_SYSTEMD_INIT()
	AX_CHECK_SYSTEMD()
])

dnl Systemd will be disabled by default and requires you to run configure with
dnl --enable-systemd to look for and enable systemd.
AC_DEFUN([AX_ALLOW_SYSTEMD], [
	AX_ALLOW_SYSTEMD_INIT_OPTS()
	AX_ALLOW_SYSTEMD_OPTS()
	AX_CHECK_SYSTEMD_INIT()
	AX_CHECK_SYSTEMD()
])

dnl Systemd will be disabled by default but if your build system is detected
dnl to have systemd build libraries it will be enabled. You can always force
dnl disable with --disable-systemd
AC_DEFUN([AX_AVAILABLE_SYSTEMD], [
	AX_ALLOW_SYSTEMD_INIT_OPTS()
	AX_ALLOW_SYSTEMD_OPTS()
	AX_CHECK_SYSTEMD_INIT_ENABLE_AVAILABLE()
	AX_CHECK_SYSTEMD_ENABLE_AVAILABLE()
	AX_CHECK_SYSTEMD_INIT()
	AX_CHECK_SYSTEMD()
])
