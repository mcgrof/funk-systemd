/* 
 * Funk - math library example using dlopen() and dlsym()
 *
 * Copyright (C) 2014 Luis R. Rodriguez <mcgrof@suse.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

/*
 * Funk Math(TM) is an example usage of dlopen() and dlsym().
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

#define FUNK_MATH_FUNCS		\
	double (*sin)(double);	\
	double (*cos)(double);	\
	double (*tan)(double);	\

union u_math_funcs {
	void *f;
	FUNK_MATH_FUNCS
};

struct funk_math_ctx {
	void *handle;
	FUNK_MATH_FUNCS
};

/*
 * Technically just checking for dlerror() should suffice
 * as libraries may have symbols set to NULL, but our use
 * case will prevent that, we'll use this for libraries that
 * we know don't have symbols set to NULL.
 */
#define funk_load_math_func(__funk_math_lib, __libfunc, __builder) do { \
	__builder.f = NULL;						\
	__builder.f = dlsym(__funk_math_lib->handle, #__libfunc);	\
	if (dlerror() != NULL)						\
		return false;						\
	if (!__builder.f)						\
		return false;						\
	__funk_math_lib->__libfunc = builder.__libfunc;			\
} while (0)

struct funk_math_ctx *funk_get_math_ctx(void)
{
	struct funk_math_ctx *ctx;
	ctx = malloc(sizeof(struct funk_math_ctx));
	if (!ctx)
		return NULL;
	return ctx;
}

void funk_free_math_ctx(struct funk_math_ctx *ctx)
{
	if (!ctx)
		return;
	dlclose(ctx->handle);
	free(ctx);
}

bool funk_load_math(struct funk_math_ctx *ctx)
{
	union u_math_funcs builder;

	if (!ctx)
		return false;

	memset(&builder, 0, sizeof(union u_math_funcs));

	ctx->handle = dlopen("libm.so", RTLD_LAZY);
	if (dlerror() != NULL)
		return false;

	if (!ctx->handle)
		return false;

	funk_load_math_func(ctx, sin, builder);
	funk_load_math_func(ctx, cos, builder);
	funk_load_math_func(ctx, tan, builder);

	return true;
}

int main(void)
{
	struct funk_math_ctx *ctx;
	int r = -ENOMEM;

	ctx = funk_get_math_ctx();
	if (!ctx)
		exit(r);

	if (!funk_load_math(ctx))
		printf("You seem to not have the libm.so library\n");

	printf("Precomputed: sin(1) = 0.8414709848\n");
	printf("Math lb lib: sin(1) = %f\n\n", ctx->sin(1));

	printf("Precomputed: cos(1) = 0.54030230586\n");
	printf("Math lb lib: cos(1) = %f\n\n", ctx->cos(1));

	printf("Precomputed: tan(1) = 1.55740772465\n");
	printf("Math lb lib: tan(1) = %f\n\n", ctx->tan(1));

	funk_free_math_ctx(ctx);

	exit(0);
}
