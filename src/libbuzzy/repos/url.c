/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>
#include <yaml.h>

#include "buzzy/error.h"
#include "buzzy/repo.h"
#include "buzzy/yaml.h"


/*-----------------------------------------------------------------------
 * URL repositories
 */

static struct cork_hash_table  url_repos;
static bool  url_repos_initialized = false;

static enum cork_hash_table_map_result
url_repo_free(struct cork_hash_table_entry *entry, void *user_data)
{
    const char  *url = entry->key;
    cork_strfree(url);
    return CORK_HASH_TABLE_MAP_DELETE;
}

static void
url_repos_done(void)
{
    cork_hash_table_map(&url_repos, url_repo_free, NULL);
    cork_hash_table_done(&url_repos);
}

static void
url_repos_init(void)
{
    if (CORK_UNLIKELY(!url_repos_initialized)) {
        cork_string_hash_table_init(&url_repos, 0);
        cork_cleanup_at_exit(0, url_repos_done);
        url_repos_initialized = true;
    }
}

static struct bz_repo *
bz_url_repo_create(const char *url)
{
    /* For now, we assume the URL is a local filesystem path. */
    return bz_local_filesystem_repo_new(url);
}

struct bz_repo *
bz_url_repo_new(const char *url)
{
    struct cork_hash_table_entry  *entry;
    bool  is_new;

    url_repos_init();
    entry = cork_hash_table_get_or_create(&url_repos, (void *) url, &is_new);

    if (is_new) {
        struct bz_repo  *repo;
        entry->key = (void *) cork_strdup(url);
        rpp_check(repo = bz_url_repo_create(url));
        bz_repo_register(repo);
        entry->value = repo;
    }

    return entry->value;
}


/*-----------------------------------------------------------------------
 * YAML repo links
 */

struct bz_repo *
bz_yaml_repo_new(yaml_document_t *doc, int node_id)
{
    yaml_node_t  *node = yaml_document_get_node(doc, node_id);
    const char  *tag = (const char *) node->tag;

    /* Simple strings are treated as URLs. */
    if (strcmp(tag, YAML_STR_TAG) == 0) {
        const char  *url = (const char *) node->data.scalar.value;
        return bz_url_repo_new(url);
    }

    /* Otherwise we don't know what to do. */
    bz_bad_config("Unknown repository type %s", tag);
    return NULL;
}

int
bz_repo_parse_yaml_links(struct bz_repo *repo, const char *path)
{
    yaml_document_t  doc;
    yaml_node_t  *root;
    yaml_node_item_t  *item;

    rii_check(bz_load_yaml_file(&doc, path));
    root = yaml_document_get_root_node(&doc);
    if (CORK_UNLIKELY(root->type != YAML_SEQUENCE_NODE)) {
        bz_bad_config("Repo links file must contain a sequence");
        goto error;
    }

    for (item = root->data.sequence.items.start;
         item < root->data.sequence.items.top; item++) {
        struct bz_repo  *link;
        ep_check(link = bz_yaml_repo_new(&doc, *item));
        bz_repo_add_link(repo, link);
    }

    yaml_document_delete(&doc);
    return 0;

error:
    yaml_document_delete(&doc);
    return -1;
}
