#include <funkd.h>

int funk_handle_connect(int fd)
{
	int len;
	char c;
	int r;

	while (true) {
		len = recv(fd, &c, 1, 0);
		if (len < 0)
			return -errno;
		if (len == 0)
			return 0;
		c+=1;
		r = send(fd, &c, len, 0);
		if (r < 0)
			return -errno;
	}
	return 0;
}

int funk_main_loop(bool sd_booted)
{
	int r;

	if (!sd_booted)
		r = funk_legacy_socket_loop();
	else
		r = funk_active_socket_loop();

	return r;
}
