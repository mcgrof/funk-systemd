include $(CURDIR)/Config.mk

AM_CFLAGS  = -I$(CURDIR)/
AM_CFLAGS  += $(SYSTEMD_CFLAGS)
AM_LDFLAGS = $(SYSTEMD_LIBS)

SHARED_C = funk_shared.c funk_legacy.c

bin_PROGRAMS = funk_regular funk_dynamic funk_math
funk_dynamic_SOURCES = $(SHARED_C) funk_dynamic.c funk_dynamic_helpers.c funk_shared_active.c
if CONFIG_SYSTEMD
EXTRA_C = funk_shared_active.c
endif
funk_regular_SOURCES = $(SHARED_C) $(EXTRA_C) funk_regular.c funk_regular_helpers.c
funk_math_SOURCES = funk_math.c

.PHONY: reload
reload:
	-systemctl daemon-reload

.PHONY: disable
disable:
	systemctl disable funk.service
	systemctl disable funk.socket

.PHONY: enable
enable:
	make disable
	systemctl daemon-reload
	systemctl enable funk.service
	systemctl enable funk.socket

install-sockets:
	cp units/*.socket @SYSTEMD_DIR@/

install-reg:
	make install-sockets
	cp funk_regular $(bindir)/
	cp units/funk_regular.service @SYSTEMD_DIR@/funk.service
	make enable

install-dyn:
	make install-sockets
	cp funk_regular $(bindir)/
	cp units/funk_dynamic.service @SYSTEMD_DIR@/funk.service
	make enable

install-data-am:
	make install-sockets
	@echo
	@echo You must run either of:
	@echo
	@echo    make install-reg
	@echo    make install-dyn
	@echo
	@echo Since you didn't specify we'll run make install-dyn
	make install-dyn
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
	-systemctl status funk.socket
	-systemctl status funk.service

.PHONY: start
start:
	-systemctl start funk.socket
	-systemctl start funk.service
	-make status

.PHONY: stop
stop:
	-systemctl stop funk.service
	-systemctl stop funk.socket
	-make status

.PHONY: restart
restart:
	@-make stop
	@-make start

