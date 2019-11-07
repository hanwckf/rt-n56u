/*
 * Copyright (C) 2011  Red Hat, Jeff Layton <jlayton@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*
 * Explanation:
 *
 * This file contains the code to manage the sqlite backend database for the
 * clstated upcall daemon.
 *
 * The main database is called main.sqlite and contains the following tables:
 *
 * parameters: simple key/value pairs for storing database info
 *
 * clients: one column containing a BLOB with the as sent by the client
 * 	    and a timestamp (in epoch seconds) of when the record was
 * 	    established
 *
 * FIXME: should we also record the fsid being accessed?
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <dirent.h>
#include <errno.h>
#include <event.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sqlite3.h>
#include <linux/limits.h>

#include "xlog.h"

#define CLD_SQLITE_SCHEMA_VERSION 1

/* in milliseconds */
#define CLD_SQLITE_BUSY_TIMEOUT 10000

/* private data structures */

/* global variables */

/* reusable pathname and sql command buffer */
static char buf[PATH_MAX];

/* global database handle */
static sqlite3 *dbh;

/* forward declarations */

/* make a directory, ignoring EEXIST errors unless it's not a directory */
static int
mkdir_if_not_exist(const char *dirname)
{
	int ret;
	struct stat statbuf;

	ret = mkdir(dirname, S_IRWXU);
	if (ret && errno != EEXIST)
		return -errno;

	ret = stat(dirname, &statbuf);
	if (ret)
		return -errno;

	if (!S_ISDIR(statbuf.st_mode))
		ret = -ENOTDIR;

	return ret;
}

/* Open the database and set up the database handle for it */
int
sqlite_prepare_dbh(const char *topdir)
{
	int ret;

	/* Do nothing if the database handle is already set up */
	if (dbh)
		return 0;

	ret = snprintf(buf, PATH_MAX - 1, "%s/main.sqlite", topdir);
	if (ret < 0)
		return ret;

	buf[PATH_MAX - 1] = '\0';

	ret = sqlite3_open(buf, &dbh);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to open main database: %d", ret);
		dbh = NULL;
		return ret;
	}

	ret = sqlite3_busy_timeout(dbh, CLD_SQLITE_BUSY_TIMEOUT);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to set sqlite busy timeout: %d", ret);
		sqlite3_close(dbh);
		dbh = NULL;
	}

	return ret;
}

/*
 * Open the "main" database, and attempt to initialize it by creating the
 * parameters table and inserting the schema version into it. Ignore any errors
 * from that, and then attempt to select the version out of it again. If the
 * version appears wrong, then assume that the DB is corrupt or has been
 * upgraded, and return an error. If all of that works, then attempt to create
 * the "clients" table.
 */
int
sqlite_maindb_init(const char *topdir)
{
	int ret;
	char *err = NULL;
	sqlite3_stmt *stmt = NULL;

	ret = mkdir_if_not_exist(topdir);
	if (ret)
		return ret;

	ret = sqlite_prepare_dbh(topdir);
	if (ret)
		return ret;

	/* Try to create table */
	ret = sqlite3_exec(dbh, "CREATE TABLE IF NOT EXISTS parameters "
				"(key TEXT PRIMARY KEY, value TEXT);",
				NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to create parameter table: %d", ret);
		goto out_err;
	}

	/* insert version into table -- ignore error if it fails */
	ret = snprintf(buf, sizeof(buf),
		       "INSERT OR IGNORE INTO parameters values (\"version\", "
		       "\"%d\");", CLD_SQLITE_SCHEMA_VERSION);
	if (ret < 0) {
		goto out_err;
	} else if ((size_t)ret >= sizeof(buf)) {
		ret = -EINVAL;
		goto out_err;
	}

	ret = sqlite3_exec(dbh, (const char *)buf, NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to insert into parameter table: %d",
				ret);
		goto out_err;
	}

	ret = sqlite3_prepare_v2(dbh,
		"SELECT value FROM parameters WHERE key == \"version\";",
		 -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to prepare select statement: %d", ret);
		goto out_err;
	}

	/* check schema version */
	ret = sqlite3_step(stmt);
	if (ret != SQLITE_ROW) {
		xlog(L_ERROR, "Select statement execution failed: %s",
				sqlite3_errmsg(dbh));
		goto out_err;
	}

	/* process SELECT result */
	ret = sqlite3_column_int(stmt, 0);
	if (ret != CLD_SQLITE_SCHEMA_VERSION) {
		xlog(L_ERROR, "Unsupported database schema version! "
			"Expected %d, got %d.",
			CLD_SQLITE_SCHEMA_VERSION, ret);
		ret = -EINVAL;
		goto out_err;
	}

	/* now create the "clients" table */
	ret = sqlite3_exec(dbh, "CREATE TABLE IF NOT EXISTS clients "
				"(id BLOB PRIMARY KEY, time INTEGER);",
				NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to create clients table: %s", err);
		goto out_err;
	}

	sqlite3_free(err);
	sqlite3_finalize(stmt);
	return 0;

out_err:
	if (err) {
		xlog(L_ERROR, "sqlite error: %s", err);
		sqlite3_free(err);
	}
	sqlite3_finalize(stmt);
	sqlite3_close(dbh);
	return ret;
}

/*
 * Create a client record
 *
 * Returns a non-zero sqlite error code, or SQLITE_OK (aka 0)
 */
int
sqlite_insert_client(const unsigned char *clname, const size_t namelen)
{
	int ret;
	sqlite3_stmt *stmt = NULL;

	ret = sqlite3_prepare_v2(dbh, "INSERT OR REPLACE INTO clients VALUES "
				      "(?, strftime('%s', 'now'));", -1,
					&stmt, NULL);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: insert statement prepare failed: %s",
			__func__, sqlite3_errmsg(dbh));
		return ret;
	}

	ret = sqlite3_bind_blob(stmt, 1, (const void *)clname, namelen,
				SQLITE_STATIC);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: bind blob failed: %s", __func__,
				sqlite3_errmsg(dbh));
		goto out_err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_DONE)
		ret = SQLITE_OK;
	else
		xlog(L_ERROR, "%s: unexpected return code from insert: %s",
				__func__, sqlite3_errmsg(dbh));

out_err:
	xlog(D_GENERAL, "%s: returning %d", __func__, ret);
	sqlite3_finalize(stmt);
	return ret;
}

/* Remove a client record */
int
sqlite_remove_client(const unsigned char *clname, const size_t namelen)
{
	int ret;
	sqlite3_stmt *stmt = NULL;

	ret = sqlite3_prepare_v2(dbh, "DELETE FROM clients WHERE id==?", -1,
				 &stmt, NULL);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: statement prepare failed: %s",
				__func__, sqlite3_errmsg(dbh));
		goto out_err;
	}

	ret = sqlite3_bind_blob(stmt, 1, (const void *)clname, namelen,
				SQLITE_STATIC);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: bind blob failed: %s", __func__,
				sqlite3_errmsg(dbh));
		goto out_err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_DONE)
		ret = SQLITE_OK;
	else
		xlog(L_ERROR, "%s: unexpected return code from delete: %d",
				__func__, ret);

out_err:
	xlog(D_GENERAL, "%s: returning %d", __func__, ret);
	sqlite3_finalize(stmt);
	return ret;
}

/*
 * Is the given clname in the clients table? If so, then update its timestamp
 * and return success. If the record isn't present, or the update fails, then
 * return an error.
 */
int
sqlite_check_client(const unsigned char *clname, const size_t namelen)
{
	int ret;
	sqlite3_stmt *stmt = NULL;

	ret = sqlite3_prepare_v2(dbh, "SELECT count(*) FROM clients WHERE "
				      "id==?", -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: unable to prepare update statement: %s",
				__func__, sqlite3_errmsg(dbh));
		goto out_err;
	}

	ret = sqlite3_bind_blob(stmt, 1, (const void *)clname, namelen,
				SQLITE_STATIC);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: bind blob failed: %s",
				__func__, sqlite3_errmsg(dbh));
		goto out_err;
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_ROW) {
		xlog(L_ERROR, "%s: unexpected return code from select: %d",
				__func__, ret);
		goto out_err;
	}

	ret = sqlite3_column_int(stmt, 0);
	xlog(D_GENERAL, "%s: select returned %d rows", __func__, ret);
	if (ret != 1) {
		ret = -EACCES;
		goto out_err;
	}

	sqlite3_finalize(stmt);
	stmt = NULL;
	ret = sqlite3_prepare_v2(dbh, "UPDATE OR FAIL clients SET "
				      "time=strftime('%s', 'now') WHERE id==?",
				 -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: unable to prepare update statement: %s",
				__func__, sqlite3_errmsg(dbh));
		goto out_err;
	}

	ret = sqlite3_bind_blob(stmt, 1, (const void *)clname, namelen,
				SQLITE_STATIC);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: bind blob failed: %s",
				__func__, sqlite3_errmsg(dbh));
		goto out_err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_DONE)
		ret = SQLITE_OK;
	else
		xlog(L_ERROR, "%s: unexpected return code from update: %s",
				__func__, sqlite3_errmsg(dbh));

out_err:
	xlog(D_GENERAL, "%s: returning %d", __func__, ret);
	sqlite3_finalize(stmt);
	return ret;
}

/*
 * remove any client records that were not reclaimed since grace_start.
 */
int
sqlite_remove_unreclaimed(time_t grace_start)
{
	int ret;
	char *err = NULL;

	ret = snprintf(buf, sizeof(buf), "DELETE FROM clients WHERE time < %ld",
			grace_start);
	if (ret < 0) {
		return ret;
	} else if ((size_t)ret >= sizeof(buf)) {
		ret = -EINVAL;
		return ret;
	}

	ret = sqlite3_exec(dbh, buf, NULL, NULL, &err);
	if (ret != SQLITE_OK)
		xlog(L_ERROR, "%s: delete failed: %s", __func__, err);

	xlog(D_GENERAL, "%s: returning %d", __func__, ret);
	sqlite3_free(err);
	return ret;
}
