/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <string.h>

#include <libcork/core.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/logging.h"
#include "buzzy/os.h"
#include "buzzy/package.h"


/*-----------------------------------------------------------------------
 * Standard messages
 */

int
bz_build_message(struct bz_env *env, const char *builder_name)
{
    const char  *package_name;
    const char  *version;
    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(version = bz_env_get_string(env, "version", true));
    bz_log_action("Build %s %s (%s)", package_name, version, builder_name);
    return 0;
}

int
bz_test_message(struct bz_env *env, const char *builder_name)
{
    const char  *package_name;
    const char  *version;
    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(version = bz_env_get_string(env, "version", true));
    bz_log_action("Test %s %s (%s)", package_name, version, builder_name);
    return 0;
}

int
bz_stage_message(struct bz_env *env, const char *builder_name)
{
    const char  *package_name;
    const char  *version;
    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(version = bz_env_get_string(env, "version", true));
    bz_log_action("Stage %s %s (%s)", package_name, version, builder_name);
    return 0;
}


/*-----------------------------------------------------------------------
 * Builders
 */

struct bz_builder {
    struct bz_env  *env;
    struct bz_package  *pkg;
    const char  *builder_name;

    void  *user_data;
    cork_free_f  free_user_data;
    bz_package_is_needed_f  build_needed;
    bz_package_step_f  build;
    bz_package_is_needed_f  test_needed;
    bz_package_step_f  test;
    bz_package_is_needed_f  stage_needed;
    bz_package_step_f  stage;
    bool  built;
    bool  tested;
    bool  staged;
};


struct bz_builder *
bz_builder_new(struct bz_env *env, const char *builder_name,
               void *user_data, cork_free_f free_user_data,
               bz_package_is_needed_f build_needed,
               bz_package_step_f build,
               bz_package_is_needed_f test_needed,
               bz_package_step_f test,
               bz_package_is_needed_f stage_needed,
               bz_package_step_f stage)
{
    struct bz_builder  *builder = cork_new(struct bz_builder);
    builder->env = env;
    builder->pkg = NULL;
    builder->builder_name = cork_strdup(builder_name);
    builder->user_data = user_data;
    builder->free_user_data = free_user_data;
    builder->build_needed = build_needed;
    builder->build = build;
    builder->test_needed = test_needed;
    builder->test = test;
    builder->stage_needed = stage_needed;
    builder->stage = stage;
    builder->built = false;
    builder->tested = false;
    builder->staged = false;
    return builder;
}

void
bz_builder_free(struct bz_builder *builder)
{
    cork_strfree(builder->builder_name);
    cork_free_user_data(builder);
    free(builder);
}

void
bz_builder_set_package(struct bz_builder *builder, struct bz_package *pkg)
{
    builder->pkg = pkg;
}


int
bz_builder_build(struct bz_builder *builder)
{
    if (!builder->built) {
        bool  is_needed;
        builder->built = true;
        rii_check(builder->build_needed(builder->user_data, &is_needed));
        if (is_needed) {
            if (builder->pkg != NULL) {
                rii_check(bz_package_install_build_deps(builder->pkg));
                rii_check(bz_package_install_deps(builder->pkg));
            }
            return builder->build(builder->user_data);
        }
    }
    return 0;
}

int
bz_builder_test(struct bz_builder *builder)
{
    if (!builder->tested) {
        bool  is_needed;
        builder->tested = true;
        rii_check(builder->test_needed(builder->user_data, &is_needed));
        if (is_needed) {
            rii_check(bz_builder_build(builder));
            return builder->test(builder->user_data);
        }
    }
    return 0;
}

int
bz_builder_stage(struct bz_builder *builder)
{
    if (!builder->staged) {
        bool  is_needed;
        builder->staged = true;
        rii_check(builder->stage_needed(builder->user_data, &is_needed));
        if (is_needed) {
            rii_check(bz_builder_build(builder));
            return builder->stage(builder->user_data);
        }
    }
    return 0;
}


/*-----------------------------------------------------------------------
 * Available builders
 */

struct bz_builder_reg {
    const char  *name;
    struct bz_builder *(*new_builder)(struct bz_env *env);
};

static struct bz_builder_reg  builders[] = {
    { "autotools", bz_autotools_builder_new },
    { "cmake", bz_cmake_builder_new },
    { "noop", bz_noop_builder_new },
    { NULL }
};

struct bz_builder *
bz_package_builder_new(struct bz_env *env)
{
    const char  *builder_name;
    struct bz_builder_reg  *builder;
    rpp_check(builder_name = bz_env_get_string(env, "builder", true));
    for (builder = builders; builder->name != NULL; builder++) {
        if (strcmp(builder_name, builder->name) == 0) {
            return builder->new_builder(env);
        }
    }
    bz_bad_config("Unknown builder \"%s\"", builder_name);
    return NULL;
}


/*-----------------------------------------------------------------------
 * Builder detection
 */

static bool
bz_builder_check_file_(struct bz_value *ctx, const char *var_name)
{
    bool  found;
    struct cork_path  *path;
    xp_check(false, path = bz_value_get_path(ctx, var_name, true));
    xi_check(false, bz_file_exists(cork_path_get(path), &found));
    return found;
}

#define bz_builder_check_file(ctx, var_name) \
    do { \
        bool  found; \
        rie_check(found = bz_builder_check_file_((ctx), (var_name))); \
        if (found) { \
            return true; \
        } \
    } while (0)

static bool
bz_builder_is_autotools(struct bz_value *ctx)
{
    bz_builder_check_file(ctx, "autotools.configure.configure");
    bz_builder_check_file(ctx, "autotools.configure.configure_in");
    return false;
}

static bool
bz_builder_is_cmake(struct bz_value *ctx)
{
    bz_builder_check_file(ctx, "cmake.cmakelists");
    return false;
}

#define try(name) \
    do { \
        bool  found; \
        rpe_check(found = bz_builder_is_##name(ctx)); \
        if (found) { \
            return #name; \
        } \
    } while (0)

static const char *
bz_builder__detect(void *user_data, struct bz_value *ctx)
{
    const char  *package_name;
    try(autotools);
    try(cmake);

    rpe_check(package_name = bz_value_get_string(ctx, "name", false));
    if (package_name == NULL) {
        package_name = "an unknown package";
    }
    bz_bad_config("Don't know what builder to use for %s", package_name);
    return NULL;
}

struct bz_value *
bz_builder_detector_new(void)
{
    return bz_scalar_value_new(NULL, NULL, bz_builder__detect);
}
