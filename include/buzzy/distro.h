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
#include "buzzy/env.h"
#include "buzzy/package.h"


/*-----------------------------------------------------------------------
 * Detect built-in package databases
 */

int
bz_pdb_discover(void);


/*-----------------------------------------------------------------------
 * Detect packagers
 */

typedef struct bz_action *
(*bz_create_package_f)(struct bz_env *env, struct bz_action *stage_action);

struct bz_value_provider *
bz_packager_detector_new(void);


#endif /* BUZZY_DISTRO_H */
