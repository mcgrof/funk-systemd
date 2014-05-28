#include <funkd.h>

struct funk_legacy_socket funk_legacy_sockets[5] = {
	{
		.fd = STDIN_FILENO,
		.domain = 0,
		.type = 0,
		.listening = 0,
		.path = "stdin",
		.mode = 0600,
		.length = 0,
	},
	{
		.fd = STDOUT_FILENO,
		.type = 0,
		.domain = 0,
		.listening = 0,
		.path = "stderr",
		.mode = 0600,
		.length = 0,
	},
	{
		.fd = STDERR_FILENO,
		.domain = 0,
		.type = 0,
		.listening = 0,
		.path = "stderr",
		.mode = 0600,
		.length = 0,
	},
	{
		.fd = 0,
		.domain = PF_UNIX,
		.type = SOCK_STREAM,
		.listening = 0,
		.path = "/var/run/funk/socket",
		.mode = 0600,
		.length = 0,
	},
	{
		.fd = 0,
		.domain = PF_UNIX,
		.type = SOCK_STREAM,
		.listening = 0,
		.path = "/var/run/funk/socket_ro",
		.mode = 0660,
		.length = 0,
	},
};

struct funk_legacy_socket *
funk_get_legacy_socket_by_path(const char *connect_to)
{
	unsigned int i;

	for (i=0; i<ARRAY_SIZE(funk_legacy_sockets); i++) {
		if (!strcmp(connect_to, funk_legacy_sockets[i].path)) {
			if (!funk_legacy_sockets[i].type)
				return NULL;
			return &funk_legacy_sockets[i];
		}
	}

	return NULL;
}

int funk_create_socket(struct funk_legacy_socket *legacy_socket)
{
	int r;

	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;

	r = unlink(legacy_socket->path);
	if (r < 0 && errno != ENOENT)
		return -errno;

	legacy_socket->fd = socket(legacy_socket->domain,
				   legacy_socket->type,
				   0);
	if (legacy_socket->fd < 0)
		return -errno;

	if (strlen(legacy_socket->path) >= sizeof(addr.sun_path))
		return -EINVAL;

	strncpy(addr.sun_path,
		legacy_socket->path,
		strlen(legacy_socket->path));

	r = bind(legacy_socket->fd, (struct sockaddr *)&addr, sizeof(addr));
	if (r != 0)
		return -errno;

	r = chmod(legacy_socket->path, legacy_socket->mode);
	if (r != 0)
		return -errno;

	r = listen(legacy_socket->fd, 1);
	if (r != 0)
		return -errno;

	return 0;
}

int funk_create_sockets(void)
{
	unsigned int i;
	struct funk_legacy_socket *legacy_socket;
	int r;

	r = mkdir("/var/run/funk", 0755);
	if (r !=0 && errno != EEXIST)
		return -errno;

	for (i=0; i<ARRAY_SIZE(funk_active_sockets); i++) {
		legacy_socket = &funk_legacy_sockets[i];

		if (!legacy_socket->type)
			continue;
		if (legacy_socket->listening)
			continue;

		r = funk_create_socket(legacy_socket);
		if (r !=0)
			return r;
	}

	return 0;
}

int funk_wait_ready(int fd)
{
	FILE *stream;
	int r;

	stream = fdopen(fd, "r");
	if (!stream)
		return errno;

	r = fgetc(stream);
	if (ferror(stream))
		return errno;

	fclose(stream);

	if (r != 0)
		return r;

	return 0;
}

int funk_write_pidfile(void)
{
	char buf[100];
	int len;
	int fd;
	int r;

	fd = open(FUNK_PID_FILE, O_RDWR | O_CREAT, 0600);
	if (fd == -1)
		return errno;

	len = snprintf(buf, sizeof(buf), "%ld\n", (long)getpid());
	r = write(fd, buf, len) != len;
	if (r != len)
		return errno;

	close(fd);

	return 0;
}

int funk_test_open_tmp(void)
{
	int fd;

	fd = open("/tmp/funk.0", O_RDWR | O_CREAT, 0600);
	if (fd == -1)
		return errno;
	fprintf(stderr, "opened fd: %d\n", fd);

	fd = open("/tmp/funk.1", O_RDWR | O_CREAT, 0600);
	if (fd == -1)
		return errno;
	fprintf(stderr, "opened fd: %d\n", fd);

	fd = open("/tmp/funk.2", O_RDWR | O_CREAT, 0600);
	if (fd == -1)
		return errno;
	fprintf(stderr, "opened fd: %d\n", fd);

	fd = open("/tmp/funk.3", O_RDWR | O_CREAT, 0600);
	if (fd == -1)
		return errno;
	fprintf(stderr, "opened fd: %d\n", fd);

	return 0;
}

int funk_legacy_daemon_init(void)
{
	pid_t pid;
	pid_t sid;
	int p[2];
	FILE *stream;
	int r;

	signal(SIGCHLD, SIG_IGN);
	r = pipe(p);
	if (r < 0)
		return r;

	pid = fork();
	if (pid < 0)
		return -errno;
	if (pid != 0)
		return funk_wait_ready(p[0]);

	close(p[0]);

	stream = fdopen(p[1], "w");
	if (!stream)
		return -errno;

	sid = setsid();
	if (sid < 0)
		return -errno;

	/*
	 * Some daemons tend to follow the practice of forking twice.
	 * The first fork is done before calling setsid to ensure that
	 * you are not a process group leader, the second time is done
	 * to ensure that children cannot regain the terminal. A more
	 * reasonable alternative to forking twice seems to be to let
	 * use signal(SIGCHLD, SIG_IGN); so that the parent in no way
	 * needs to wait for its children.
	 */

	pid = fork();
	if (pid < 0)
		return -errno;
	if (pid != 0)
		return 0;

	r = funk_write_pidfile();
	if (r != 0)
		return r;

	umask(0);

	chdir("/");

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	r = funk_create_sockets();
	if (r != 0)
		return r;

	/*
	 * Useful for telling old init we are ready, the equivalent of
	 * what sd_notify() implicates we should be doing on typical
	 * daemons but never do.
	 */
	fputc(0, stream);
	if (ferror(stream))
		return errno;
	fclose(stream);

	return 0;
}

int funk_legacy_socket_loop(void)
{
	const struct funk_legacy_socket *legacy_socket;
	int fd;
	int r;

	legacy_socket = funk_get_legacy_socket_by_path("/var/run/funk/socket");
	if (!legacy_socket)
		return -EINVAL;

	fd = accept(legacy_socket->fd, NULL, NULL);
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
