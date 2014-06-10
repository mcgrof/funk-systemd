#include <funkd.h>

/*
 * We list stdin, stdout and stderr simply for documentation purposes
 * and to help our array size fit the number of expected sockets we
 * as sd_listen_fds() will return 5 for example if you set the socket
 * service with 2 sockets.
 */
const struct funk_systemd_active_socket funk_active_sockets[5] = {
	{
		.fd = SD_LISTEN_FDS_START -3,
		.type = 0,
		.listening = 0,
		.path = "stdin",
		.length = 0,
	},
	{
		.fd = SD_LISTEN_FDS_START - 2,
		.type = 0,
		.listening = 0,
		.path = "stderr",
		.length = 0,
	},
	{
		.fd = SD_LISTEN_FDS_START - 1,
		.type = 0,
		.listening = 0,
		.path = "stderr",
		.length = 0,
	},
	{
		.fd = SD_LISTEN_FDS_START,
		.type = SOCK_STREAM,
		.listening = 1,
		.path = "/var/run/funk/socket",
		.length = 0,
	},
	{
		.fd = SD_LISTEN_FDS_START + 1,
		.type = SOCK_STREAM,
		.listening = 1,
		.path = "/var/run/funk/socket_ro",
		.length = 0,
	},
};

const struct funk_systemd_active_socket *
funk_get_active_socket_by_path(const char *connect_to)
{
	unsigned int i;

	for (i=0; i<ARRAY_SIZE(funk_active_sockets); i++) {
		if (!strcmp(connect_to, funk_active_sockets[i].path)) {
			if (!funk_active_sockets[i].type)
				return NULL;
			return &funk_active_sockets[i];
		}
	}

	return NULL;
}

int funk_active_socket_loop(void)
{
	const struct funk_systemd_active_socket *active_socket;
	int fd;
	int r;

	active_socket = funk_get_active_socket_by_path("/var/run/funk/socket");
	if (!active_socket)
		return -EINVAL;

	fd = accept(active_socket->fd, NULL, NULL);
	if (fd < 0) {
		fprintf(stderr, "Invalid connection, skipping\n");
		goto out;
	}

	r = funk_handle_connect(fd);
	if (r < 0) {
		fprintf(stderr, "Could not handle incomming connection\n");
		goto out;
	}
out:
	return 0;
}
