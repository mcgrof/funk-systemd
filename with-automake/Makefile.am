include $(CURDIR)/Config.mk

AM_CFLAGS  = -I$(CURDIR)/
AM_CFLAGS  += $(SYSTEMD_CFLAGS)
AM_LDFLAGS = $(SYSTEMD_LIBS)

SHARED_C = funk_shared.c funk_legacy.c

bin_PROGRAMS = funk_regular funk_dynamic funk_math

if CONFIG_SYSTEMD
EXTRA_C_DYN  = funk_shared_active.c
EXTRA_C_DYN += funk_dynamic_helpers.c
endif

funk_dynamic_SOURCES = $(SHARED_C) $(EXTRA_C_DYN) funk_dynamic.c

if CONFIG_SYSTEMD
EXTRA_C  = funk_shared_active.c
EXTRA_C += funk_regular_helpers.c
endif

funk_regular_SOURCES = $(SHARED_C) $(EXTRA_C) funk_regular.c

funk_math_SOURCES = funk_math.c

.PHONY: reload
reload:
	-systemctl daemon-reload

.PHONY: disable
disable:
	systemctl disable funk.service
	systemctl disable funk_rw.socket
	systemctl disable funk_ro.socket

.PHONY: enable
enable:
	make disable
	systemctl daemon-reload
	systemctl enable funk.service
	systemctl enable funk_rw.socket
	systemctl enable funk_ro.socket

install-sockets:
	cp units/*.socket @SYSTEMD_DIR@/

install-data-am:
	make install-sockets
	cp funk_regular $(bindir)/
	cp funk_dynamic $(bindir)/
	cp units/funk.service @SYSTEMD_DIR@/
	make reload
	make enable

maintainer-clean:
	-make clean
	-rm -rf autom4te.cache configure config.status config.log \
		config.h config.h.in config.h.in~ stamp-h1 aclocal.m4 \
		build-aux Makefile.in Makefile Config.mk \
		units/*.service units/*.socket

dist-clean:
	-make maintainer-clean

.PHONY: status
status:
	-systemctl status funk_rw.socket
	-systemctl status funk_ro.socket
	-systemctl status funk.service

.PHONY: start
start:
	-systemctl start funk_rw.socket
	-systemctl start funk_ro.socket
	-systemctl start funk.service
	-make status

.PHONY: stop
stop:
	-systemctl stop funk.service
	-systemctl stop funk_rw.socket
	-systemctl stop funk_ro.socket
	-make status

.PHONY: restart
restart:
	@-make stop
	@-make start

