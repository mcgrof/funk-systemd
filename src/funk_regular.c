/* 
 * Funk - systemd socket activation example - dynamic library usage
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
#include <funk_regular_helpers.h>

/*
 * Funk(TM) supports both systemd and old init style initialization.
 * Systemd support is provided only if you have systemd libraries, and if
 * binaries are built with systemd enabled you will need the systemd libraries
 * present in order for those binaries to work. For binaries that will function
 * on any system use Funk(TM) Dynamic.
 *
 * This example assumes you'll compile with -lsystemd-daemon for the case
 * that systemd is desired.
 */

int main(void)
{
	int r, err;
	bool _sd_booted = false;

	r = funk_test_open_tmp();
	if (r < 0)
		return r;

#if defined(HAVE_SYSTEMD)
	_sd_booted = sd_booted();
#endif

	if (!_sd_booted) {
		r = funk_legacy_daemon_init();
		if (r != 0)
			goto out;
	} else {
		r = funk_claim_active_sockets();
		if (r != 0)
			goto out;
#if defined(HAVE_SYSTEMD)
		sd_notify(1, "READY=1");
#endif
	}

	while (true) {
		r = funk_main_loop(_sd_booted);
		if (r != 0)
			goto out;
	}

	return 0;
out:
	err = r < 0 ? -r : r;
	fprintf(stderr, SD_ERR "Error: %d (%s)\n", err, strerror(err));
	return r;
}
