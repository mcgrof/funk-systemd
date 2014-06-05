/* 
 * Funkd - systemd socket activation example - dynamic link loader usage
*/

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

#include <funkd.h>
#include <funk_dynamic_helpers.h>

/*
 * Funk(TM) Dynamic supports both systemd and old init style initialization.
 * Funk(TM) Dynamic supports distributions to ship only one binary to support
 * both systemd and non systemd systems. We accomplish this by not requiring to
 * link this binary (or libary) with -lsystemd-daemon at compile time but
 * instead having libfunksocket.so ask for libsystemd-daemon.so through run
 * time. If libsystemd-daemon.so is present systemd active socket activation
 * will only be used if the system was detected to have booted with systemd
 * by using sd_booted(). Below we explain a bit further how we accomplish
 * runtime loading of systemd libraries in a portable way.
 *
 * dlsym() returns a void pointer, however a void pointer is not required
 * to even have the same size as an object pointer, and therefore a valid
 * conversion between type void* and a pointer to a function may not exist
 * on all platforms. You also may end up with strict-aliasing complaints, so
 * we use a union. For more details refer to:
 *
 * http://en.wikipedia.org/wiki/Dynamic_loading#UNIX_.28POSIX.29
 *
 * The strategy taken here is to use a builder for initial assignment
 * to address issues with different systems with a range of known
 * expected routines from the library, by usng a macro helper we with
 * a builder we can enforce only assignment to expected routines from
 * the library.
 *
 * In order to avoid a sloppy union access we upkeep on our own
 * data structure the actual callbacks that we know exist, and leave
 * the void union trick only for the builder, this requires defining the
 * expected routines twice, on the builder and our own cached copy of
 * the symbol dereferences.
 */

int main(void)
{
	struct funk_sd_ctx *ctx;
	int r, err;
	bool sd_booted = false;

	r = funk_test_open_tmp();
	if (r < 0)
		return r;

	/* ctx can be NULL but that is OK */
	ctx = funk_get_sd_ctx();
	sd_booted = funk_load_sd_required(ctx);

	if (!sd_booted) {
		r = funk_legacy_daemon_init();
		if (r != 0)
			goto out;
	} else {
		r = funk_claim_active_sockets(ctx);
		if (r != 0)
			goto out;
		funk_sd_notify_ready(ctx);
	}

	while (true) {
		r = funk_main_loop(sd_booted);
		if (r != 0)
			goto out;
	}

out:
	err = r < 0 ? -r : r;
	funk_free_sd_ctx(ctx);
	if (err != 0)
		fprintf(stderr, SD_ERR "Error: %d (%s)\n", err, strerror(err));
	return r;
}
