#ifndef __FUNKD_H
#define __FUNKD_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <signal.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <config.h>

#if defined(HAVE_SYSTEMD)
#include <systemd/sd-daemon.h>
#include <dlfcn.h>
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(ar) (sizeof(ar)/sizeof(ar[0]))
#endif

#define FUNK_PID_FILE "/var/run/funk.pid"

struct funk_legacy_socket {
	int fd;
	int domain;
	int type;
	int listening;
	const char *path;
	int mode;
	size_t length;
};

struct funk_legacy_socket funk_legacy_sockets[5];

/* Conforms to what we should send sd_is_socket_unix() */
struct funk_systemd_active_socket {
	int fd;
	int type;
	int listening;
	const char *path;
	int mode;
	size_t length;
};

/* implemented in funk_shared.c */
int funk_handle_connect(int fd);
int funk_main_loop(bool sd_booted);
int funk_create_socket(struct funk_legacy_socket *legacy_socket);
int funk_create_sockets(void);
int funk_wait_ready(int fd);
int funk_write_pidfile(void);
int funk_test_open_tmp(void);

/* implemented in funk_legacy.c */
struct funk_legacy_socket *
funk_get_legacy_socket_by_path(const char *connect_to);
int funk_legacy_daemon_init(void);
int funk_legacy_socket_loop(void);

#if defined(HAVE_SYSTEMD)
/* implemented in either funk_dynamic.c or funk_regular.c */
const struct funk_systemd_active_socket funk_active_sockets[5];

const struct funk_systemd_active_socket *
funk_get_active_socket_by_path(const char *connect_to);
int funk_active_socket_loop(void);
#else
const struct funk_systemd_active_socket *funk_active_sockets = NULL;

static inline struct funk_systemd_active_socket *
funk_get_active_socket_by_path(const char *connect_to)
{
	return NULL;
}
static inline int funk_active_socket_loop(void)
{
	return -1;
}
#endif /* ! defined(HAVE_SYSTEMD) */

#endif /* __FUNKD_H */
