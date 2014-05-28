#include <funkd.h>
#include <funk_regular_helpers.h>

int funk_validate_active_socket(int fd)
{
	const struct funk_systemd_active_socket *active_socket;
	unsigned int i;
	int r;

	for (i=0; i<ARRAY_SIZE(funk_active_sockets); i++) {
		active_socket = &funk_active_sockets[i];
		if (fd != active_socket->fd)
			continue;
		r = sd_is_socket_unix(active_socket->fd,
				      active_socket->type,
				      active_socket->listening,
				      active_socket->path,
				      active_socket->length);
		if (r <= 0)
			return -EINVAL;
		return 0;
	}

	return -EINVAL;
}

int funk_claim_active_sockets(void)
{
	int n, r, fd;

	n = sd_listen_fds(0);
	if (n <= 0) {
		sd_notifyf(0, "STATUS=Failed to get any active sockets: %s\n"
			      "ERRNO=%i",
			      strerror(errno),
			      errno);
		return errno;
	} else if (n >= (ARRAY_SIZE(funk_active_sockets))) {
		fprintf(stderr, SD_ERR "Expected %d fds but given %d\n",
				(int) ARRAY_SIZE(funk_active_sockets)-1,
				n);
		sd_notifyf(0, "STATUS=Mismatch on number (%d): %s\n"
			      "ERRNO=%d",
			      (int) ARRAY_SIZE(funk_active_sockets),
			      strerror(EBADR),
			      EBADR);
		return EBADR;
	}

	for (fd = SD_LISTEN_FDS_START; fd < SD_LISTEN_FDS_START + n; fd++) {
		r = funk_validate_active_socket(fd);
		if (r != 0)
			return r;
	}

	fprintf(stderr, SD_NOTICE "%d active sockets have been claimed\n", n);

	return 0;
}
