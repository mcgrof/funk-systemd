#include <funkd.h>
#include <dlfcn.h>

#define FUNK_SYSTEMD_FUNCS										\
	int (*sd_booted)(void);										\
	int (*sd_listen_fds)(int unset_environment);							\
	int (*sd_is_fifo)(int fd, const char *path);							\
	int (*sd_is_socket)(int fd, int family, int type, int listening);				\
	int (*sd_is_socket_inet)(int fd, int family, int type, int listening, uint16_t port);		\
	int (*sd_is_socket_unix)(int fd, int type, int listening, const char* path, size_t length);	\
	int (*sd_is_mq)(int fd, const char *path);							\
	int (*sd_notify)(int unset_environment, const char *state);					\
	int (*sd_notifyf)(int unset_environment, const char *format, ...);

union u_sd_funcs {
	void *f;
	FUNK_SYSTEMD_FUNCS
};

struct funk_sd_ctx {
	void *handle;
	FUNK_SYSTEMD_FUNCS
};

/*
 * Technically just checking for dlerror() should suffice
 * as libraries may have symbols set to NULL, but our use
 * case will prevent that, we'll use this for libraries that
 * we know don't have symbols set to NULL.
 */
#define funk_load_sd_func(__funk_sd_lib, __libfunc, __builder) do { 	\
	__builder.f = NULL;						\
	__builder.f = dlsym(__funk_sd_lib->handle, #__libfunc);		\
	if (dlerror() != NULL)						\
		return false;						\
	if (!__builder.f)						\
		return false;						\
	__funk_sd_lib->__libfunc = builder.__libfunc;			\
} while (0)

struct funk_sd_ctx *funk_get_sd_ctx(void)
{
	struct funk_sd_ctx *ctx;
	ctx = malloc(sizeof(struct funk_sd_ctx));
	if (!ctx)
		return NULL;
	return ctx;
}

void funk_free_sd_ctx(struct funk_sd_ctx *ctx)
{
	if (!ctx)
		return;
	dlclose(ctx->handle);
	free(ctx);
}

bool funk_load_sd_required(struct funk_sd_ctx *ctx)
{
	union u_sd_funcs builder;

	if (!ctx)
		return false;

	memset(&builder, 0, sizeof(union u_sd_funcs));

	ctx->handle = dlopen("libsystemd-daemon.so", RTLD_LAZY);
	if (dlerror() != NULL)
		return false;

	if (!ctx->handle)
		return false;

	funk_load_sd_func(ctx, sd_booted, builder);

	if (!ctx->sd_booted())
		return false;

	funk_load_sd_func(ctx, sd_listen_fds, builder);
	funk_load_sd_func(ctx, sd_is_socket_unix, builder);
	funk_load_sd_func(ctx, sd_notify, builder);
	funk_load_sd_func(ctx, sd_notifyf, builder);

	return true;
}

int funk_validate_active_socket(struct funk_sd_ctx *ctx, int fd)
{
	const struct funk_systemd_active_socket *active_socket;
	unsigned int i;
	int r;

	/*
	 * we know we must have this library present now, so
	 * ctx cannot be NULL now.
	 */
	if (!ctx)
		return -EINVAL;

	for (i=0; i<ARRAY_SIZE(funk_active_sockets); i++) {
		active_socket = &funk_active_sockets[i];
		if (fd != active_socket->fd)
			continue;
		r = ctx->sd_is_socket_unix(active_socket->fd,
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

int funk_claim_active_sockets(struct funk_sd_ctx *ctx)
{
	const struct funk_systemd_active_socket *active_socket;
	int n, r, fd;

	/*
	 * we know we must have this library present now, so
	 * ctx cannot be NULL now.
	 */
	if (!ctx)
		return -EINVAL;

	n = ctx->sd_listen_fds(0);
	if (n <= 0) {
		ctx->sd_notifyf(0, "STATUS=Failed to get any active sockets: %s\n"
			      "ERRNO=%i",
			      strerror(errno),
			      errno);
		return -errno;
	} else if (n >= (ARRAY_SIZE(funk_active_sockets))) {
		fprintf(stderr, SD_ERR "Expected %d fds but given %d\n",
				(int) ARRAY_SIZE(funk_active_sockets)-1,
				n);
		ctx->sd_notifyf(0, "STATUS=Mismatch on number (%d): %s\n"
			      "ERRNO=%d",
			      (int) ARRAY_SIZE(funk_active_sockets),
			      strerror(EBADR),
			      EBADR);
		return -EBADR;
	}

	fprintf(stderr, SD_NOTICE "%d sockets are expected to be used\n", n);

	for (fd = SD_LISTEN_FDS_START; fd < SD_LISTEN_FDS_START + n; fd++) {
		r = funk_validate_active_socket(ctx, fd);
		if (r != 0) {
			fprintf(stderr, SD_NOTICE "failed validating fd %d\n", fd);
			return r;
		}
		fprintf(stderr, SD_NOTICE "fd %d is OK!\n", fd);
	}

	fprintf(stderr, SD_NOTICE "%d active sockets have been claimed\n", n);

	return 0;
}

void funk_sd_notify_ready(struct funk_sd_ctx *ctx)
{
	if (!ctx)
		return;
	ctx->sd_notify(1, "READY=1");
}
