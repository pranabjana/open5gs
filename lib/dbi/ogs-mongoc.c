/*
 * Copyright (C) 2019,2020 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <mongoc.h>

#include "ogs-dbi.h"

int __ogs_dbi_domain;

static ogs_mongoc_t self;

/*
 * We've added it 
 * Because the following function is deprecated in the mongo-c-driver
 */
static bool
ogs_mongoc_mongoc_client_get_server_status (mongoc_client_t *client, /* IN */
                                 mongoc_read_prefs_t *read_prefs, /* IN */
                                 bson_t *reply,                   /* OUT */
                                 bson_error_t *error)             /* OUT */
{
   bson_t cmd = BSON_INITIALIZER;
   bool ret = false;

   BSON_ASSERT (client);

   BSON_APPEND_INT32 (&cmd, "ping", 1);
   ret = mongoc_client_command_simple (
      client, "admin", &cmd, read_prefs, reply, error);
   bson_destroy (&cmd);

   return ret;
}

static int db_uri_creds_from_env(mongoc_uri_t *uri)
{
    char *username;
    char *password;

    username = getenv("DB_USER");
    password = getenv("DB_PASS");

    if (username && password && strlen(username) && strlen(password)) {
        if (!mongoc_uri_set_username(uri, username) ||
            !mongoc_uri_set_password(uri, password)) {
            ogs_error("Failed to set DB username and password from environment");
            return OGS_ERROR;
        }
        ogs_info("Setting DB username and password from environment");
    }

    return OGS_OK;
}

static char *masked_db_uri(const char *db_uri)
{
    char *tmp;
    char *array[2], *saveptr = NULL;
    char *masked = NULL;

    ogs_assert(db_uri);

    tmp = ogs_strdup(db_uri);
    ogs_assert(tmp);

    memset(array, 0, sizeof(array));
    array[0] = ogs_strtok_r(tmp, "@", &saveptr);
    if (array[0])
        array[1] = ogs_strtok_r(NULL, "@", &saveptr);

    if (array[1]) {
        masked = ogs_msprintf("mongodb://*****:*****@%s", array[1]);
        ogs_assert(masked);
    } else {
        masked = ogs_strdup(array[0]);
        ogs_assert(masked);
    }

    ogs_free(tmp);

    return masked;
}

int ogs_mongoc_init(const char *db_uri)
{
    bson_t reply;
    bson_error_t error;
    bson_iter_t iter;

    mongoc_uri_t *uri;

    if (!db_uri) {
        ogs_error("No DB_URI");
        return OGS_ERROR;
    }

    memset(&self, 0, sizeof(ogs_mongoc_t));

    self.masked_db_uri = masked_db_uri(db_uri);

    mongoc_init();

    self.initialized = true;

    uri = mongoc_uri_new(db_uri);
    if (!uri) {
        ogs_error("Failed to parse DB URI [%s]", self.masked_db_uri);
        return OGS_ERROR;
    }

    if (db_uri_creds_from_env(uri) != OGS_OK)
        return OGS_ERROR;

    self.client = mongoc_client_new_from_uri(uri);

    ogs_assert(self.client);

#if MONGOC_MAJOR_VERSION >= 1 && MONGOC_MINOR_VERSION >= 4
    mongoc_client_set_error_api(self.client, 2);
#endif

    self.name = mongoc_uri_get_database(uri);
    ogs_assert(self.name);

    self.database = mongoc_client_get_database(self.client, self.name);
    ogs_assert(self.database);

    if (!ogs_mongoc_mongoc_client_get_server_status(
                self.client, NULL, &reply, &error)) {
        ogs_warn("Failed to connect to server [%s]", self.masked_db_uri);
        return OGS_RETRY;
    }

    ogs_assert(bson_iter_init_find(&iter, &reply, "ok"));

    bson_destroy(&reply);

    ogs_info("MongoDB URI: '%s'", self.masked_db_uri);

    return OGS_OK;
}

void ogs_mongoc_final(void)
{
    if (self.database) {
        mongoc_database_destroy(self.database);
        self.database = NULL;
    }
    if (self.client) {
        mongoc_client_destroy(self.client);
        self.client = NULL;
    }
    if (self.masked_db_uri) {
        ogs_free(self.masked_db_uri);
        self.masked_db_uri = NULL;
    }

    if (self.initialized) {
        mongoc_cleanup();
        self.initialized = false;
    }
}

ogs_mongoc_t *ogs_mongoc(void)
{
    return &self;
}

int ogs_dbi_init(const char *db_uri)
{
    int rv;

    ogs_assert(db_uri);

    rv = ogs_mongoc_init(db_uri);
    if (rv != OGS_OK) return rv;

    if (ogs_mongoc()->client && ogs_mongoc()->name) {
        self.collection.subscriber = mongoc_client_get_collection(
            ogs_mongoc()->client, ogs_mongoc()->name, "subscribers");
        ogs_assert(self.collection.subscriber);
    }

    return OGS_OK;
}

void ogs_dbi_final()
{
    if (self.collection.subscriber) {
        mongoc_collection_destroy(self.collection.subscriber);
    }

#if MONGOC_MAJOR_VERSION >= 1 && MONGOC_MINOR_VERSION >= 9
    if (self.stream) {
        mongoc_change_stream_destroy(self.stream);
    }
#endif

    ogs_mongoc_final();
}

int ogs_dbi_collection_watch_init(void)
{
#if MONGOC_MAJOR_VERSION >= 1 && MONGOC_MINOR_VERSION >= 9
    bson_t empty = BSON_INITIALIZER;    
    const bson_t *err_doc;
    bson_error_t error;
    bson_t *options = BCON_NEW("fullDocument", "updateLookup");
   
    ogs_mongoc()->stream = mongoc_collection_watch(self.collection.subscriber,
        &empty, options);

    if (mongoc_change_stream_error_document(ogs_mongoc()->stream, &error,
            &err_doc)) {
        if (!bson_empty (err_doc)) {
            ogs_error("Change Stream Error.  Enable replica sets to "
                "enable database updates to be sent to MME.");
        } else {
            ogs_error("Client Error: %s\n", error.message);
        }
        return OGS_ERROR;
    } else {
        ogs_info("Change Streams are Enabled.");
    }

    return OGS_OK;
# else
    return OGS_ERROR;
#endif
}

int ogs_dbi_poll_change_stream(void)
{
#if MONGOC_MAJOR_VERSION >= 1 && MONGOC_MINOR_VERSION >= 9
    int rv;
    
    const bson_t *document;
    const bson_t *err_document;
    bson_error_t error;

    while (mongoc_change_stream_next(ogs_mongoc()->stream, &document)) {
        rv = ogs_dbi_process_change_stream(document);
        if (rv != OGS_OK) return rv;
    }

    if (mongoc_change_stream_error_document(ogs_mongoc()->stream, &error, 
            &err_document)) {
        if (!bson_empty (err_document)) {
            ogs_debug("Server Error: %s\n",
            bson_as_relaxed_extended_json(err_document, NULL));
        } else {
            ogs_debug("Client Error: %s\n", error.message);
        }
        return OGS_ERROR;
    }

    return OGS_OK;
# else
    return OGS_ERROR;
#endif
}
