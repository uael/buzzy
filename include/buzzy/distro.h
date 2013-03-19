/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_DISTRO_H
#define BUZZY_DISTRO_H

#include <libcork/core.h>
#include <libcork/os.h>

#include "buzzy/action.h"
#include "buzzy/package.h"


/*-----------------------------------------------------------------------
 * Creating packages
 */

typedef struct bz_action *
(*bz_create_package_f)(struct bz_package_spec *spec,
                       struct cork_path *package_path,
                       struct cork_path *staging_path,
                       struct bz_action *stage_action,
                       bool verbose);

/* If name is NULL, we try to auto-detect. */
int
bz_packager_choose(const char *name);

/* Returns an action that builds a new pacman binary package from a staging
 * directory that you've already set up.  If stage_action is non-NULL, we assume
 * that's the action that fills in the staging directory; we'll make sure to
 * mark that as a prereq of the packaging action.  The new binary package will
 * be placed in package_path.
 *
 * Takes control of package_path and staging_path. */
struct bz_action *
bz_create_package(struct bz_package_spec *spec,
                  struct cork_path *package_path,
                  struct cork_path *staging_path,
                  struct bz_action *stage_action,
                  bool verbose);


#endif /* BUZZY_DISTRO_H */
