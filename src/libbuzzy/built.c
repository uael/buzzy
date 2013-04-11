/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <libcork/core.h>
#include <libcork/helpers/errors.h>

#include "buzzy/action.h"
#include "buzzy/built.h"
#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/package.h"


/*-----------------------------------------------------------------------
 * Built packages
 */

struct bz_built_package {
    struct bz_env  *env;
    struct bz_builder  *builder;
    struct bz_packager  *packager;
};

static void
bz_built_package__free(void *user_data)
{
    struct bz_built_package  *package = user_data;
    bz_env_free(package->env);
    if (package->builder != NULL) {
        bz_builder_free(package->builder);
    }
    if (package->packager != NULL) {
        bz_packager_free(package->packager);
    }
    free(package);
}

static struct bz_builder *
bz_built_package_builder(struct bz_built_package *package)
{
    if (package->builder == NULL) {
        package->builder = bz_package_builder_new(package->env);
    }
    return package->builder;
}

static struct bz_packager *
bz_built_package_packager(struct bz_built_package *package)
{
    if (package->packager == NULL) {
        struct bz_builder  *builder;
        struct bz_action  *stage;
        struct bz_action  *pkg;
        rpp_check(builder = bz_built_package_builder(package));
        rpp_check(stage = bz_builder_stage_action(builder));
        rpp_check(package->packager = bz_package_packager_new(package->env));
        rpp_check(pkg = bz_packager_package_action(package->packager));
        bz_action_add_pre(pkg, stage);
    }
    return package->packager;
}

static struct bz_action *
bz_built_package__build(void *user_data)
{
    struct bz_built_package  *package = user_data;
    struct bz_builder  *builder;
    rpp_check(builder = bz_built_package_builder(package));
    return bz_builder_build_action(builder);
}

static struct bz_action *
bz_built_package__test(void *user_data)
{
    struct bz_built_package  *package = user_data;
    struct bz_builder  *builder;
    rpp_check(builder = bz_built_package_builder(package));
    return bz_builder_test_action(builder);
}

static struct bz_action *
bz_built_package__install(void *user_data)
{
    struct bz_built_package  *package = user_data;
    struct bz_packager  *packager;
    rpp_check(packager = bz_built_package_packager(package));
    return bz_packager_install_action(packager);
}

struct bz_package *
bz_built_package_new(struct bz_env *env)
{
    struct bz_built_package  *package;
    const char  *name;
    struct bz_version  *version;

    rpp_check(name = bz_env_get_string(env, "name", true));
    rpp_check(version = bz_env_get_version(env, "version", true));

    package = cork_new(struct bz_built_package);
    package->env = env;
    package->builder = NULL;
    package->packager = NULL;
    return bz_package_new
        (name, version, env,
         package, bz_built_package__free,
         bz_built_package__build,
         bz_built_package__test,
         bz_built_package__install);
}