#include <config.h>
#include <funkd.h>

struct xs_sd_ctx;

#if defined(HAVE_SYSTEMD)
struct funk_sd_ctx *funk_get_sd_ctx(void);
void funk_free_sd_ctx(struct funk_sd_ctx *ctx);
bool funk_load_sd_required(struct funk_sd_ctx *ctx);
int funk_validate_active_socket(struct funk_sd_ctx *ctx, int fd);
int funk_claim_active_sockets(struct funk_sd_ctx *ctx);
void funk_sd_notify_ready(struct funk_sd_ctx *ctx);
#else
static inline struct funk_sd_ctx *funk_get_sd_ctx(void)
{
	return NULL;
}

static inline void funk_free_sd_ctx(struct funk_sd_ctx *ctx)
{
	return;
}

static inline bool funk_load_sd_required(struct funk_sd_ctx *ctx)
{
	return false;
}

static inline int funk_validate_active_socket(struct funk_sd_ctx *ctx, int fd)
{
	return -EINVAL;
}

static inline int funk_claim_active_sockets(struct funk_sd_ctx *ctx)
{
	return -EBADR;
}

static inline void funk_sd_notify_ready(struct funk_sd_ctx *ctx)
{
	return;
}
#endif /* ! defined(HAVE_SYSTEMD) */
