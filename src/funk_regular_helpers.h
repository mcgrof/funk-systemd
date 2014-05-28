#ifndef __FUNKD_REGULAR_HELPERS_H
#define __FUNKD_REGULAR_HELPERS_H

#include <funkd.h>

#if defined(HAVE_SYSTEMD)
int funk_validate_active_socket(int fd);
int funk_claim_active_sockets(void);
#else
static inline int funk_validate_active_socket(int fd)
{
	return -EINVAL;
}

static inline int funk_claim_active_sockets(void)
{
	return -EBADR;
}
#endif /* ! defined(HAVE_SYSTEMD) */
#endif /* __FUNKD_REGULAR_HELPERS_H */
