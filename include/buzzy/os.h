/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013-2014, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_OS_H
#define BUZZY_OS_H

#include <stdarg.h>

#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/os.h>


/*-----------------------------------------------------------------------
 * Executing subprocesses
 */


/* Execute the subprocess, wait for it to finish, and capture its stdout and
 * stderr streams into the given buffers.  (The buffers can be NULL, in which
 * case that output stream is ignored.) */

int
bz_subprocess_v_get_output(struct cork_buffer *out, struct cork_buffer *err,
                           bool *successful, va_list args);

CORK_ATTR_SENTINEL
int
bz_subprocess_get_output(struct cork_buffer *out, struct cork_buffer *err,
                         bool *successful, ...);

int
bz_subprocess_get_output_exec(struct cork_buffer *out, struct cork_buffer *err,
                              bool *successful, struct cork_exec *exec);


/* Execute the subprocess and wait for it to finish.
 *
 * If verbose is true, then the subprocess's stdout and stderr will be the same
 * as the calling process, so the end user will see the subprocess's output.  If
 * verbose is false, then the subprocess's output will be ignored.
 *
 * If successful is non-NULL, we'll fill it in with whether the subprocess's
 * exit code was 0 (indicating success). */

int
bz_subprocess_v_run(bool verbose, bool *successful, va_list args);

CORK_ATTR_SENTINEL
int
bz_subprocess_run(bool verbose, bool *successful, ...);

int
bz_subprocess_run_exec(bool verbose, bool *successful, struct cork_exec *exec);


/*-----------------------------------------------------------------------
 * Creating files and directories
 */

int
bz_create_file(const char *path, struct cork_buffer *src, cork_file_mode mode);

int
bz_create_directory(const char *path, cork_file_mode mode);

int
bz_copy_file(const char *dest, const char *src, int mode);

int
bz_file_exists(const char *path, bool *exists);

int
bz_load_file(const char *path, struct cork_buffer *dest);

int
bz_walk_directory(const char *path, struct cork_dir_walker *walker);


#endif /* BUZZY_OS_H */
