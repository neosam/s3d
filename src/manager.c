/*
 * File manager.c
 *
 * s3d is the legal property of Simon Goller (neosam@gmail.com).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "manager.h"

sqlite3 *db;
char *managererr;

char *stmt_set_2 = "INSERT INTO state (name, time, source, stream, UID) "
"VALUES (:UUID, :time, :source, :stream, :UID);";
char *stmt_get_1 = "SELECT source from state where name = :name "
	"order by time desc limit 1";
char *stmt_get_2 = "SELECT stream from state where name = :name "
	"order by time desc limit 1";

int initManager()
{
	char *home = getenv("HOME");
	char filename[256];
	sprintf(filename, "%s/.s3d/manager.db", home);
	if (sqlite3_open(filename, &db) != SQLITE_OK) {
		managererr = "Could not open database";
		return -1;
	}
}

int quitManager()
{
	sqlite3_close(db);
}

int insertNewState(char *name, char *stream, char *source, int uid, int time)
{
	const char *out;

	printf("Insert state\n");
	sqlite3_stmt *stmt2;
	sqlite3_prepare_v2(db, stmt_set_2, strlen(stmt_set_2), &stmt2, &out);
	sqlite3_bind_text(stmt2, 1, name, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt2, 2, time);
	sqlite3_bind_text(stmt2, 3, source, -1, SQLITE_STATIC);
	sqlite3_bind_blob(stmt2, 4, (void *)stream, 
			  getStreamsize(stream), SQLITE_STATIC);
	sqlite3_bind_int(stmt2, 5, uid);
	if (sqlite3_step(stmt2) != SQLITE_DONE) {
		managererr = "Could not create state";
		return -1;
	}

	return 0;
}

int set(char *name, char *stream, char *source, int uid)
{
	char *out;
	int *istream = (int*)stream;
	int t = time(NULL);

	if (insertNewState(name, stream, source, uid, t) < 0)
		return -1;

	return 0;
}

int setSource(char *id, char *source)
{

}

int setUID(char *id, int uid)
{

}

char *get(char *name)
{
	sqlite3_stmt *stmt;
	const char *out;

	sqlite3_prepare_v2(db, stmt_get_1, strlen(stmt_get_1), &stmt, &out);
	sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
	switch (sqlite3_step(stmt)) {
	case SQLITE_DONE:
		managererr = "Object not in database";
		return NULL;
		break;
	case SQLITE_ROW:
		return sqlite3_column_text(stmt, 0);
		break;
	}
}

char *getCurrent(char *name)
{
	sqlite3_stmt *stmt;
	const char *out;

	sqlite3_prepare_v2(db, stmt_get_2, strlen(stmt_get_1), &stmt, &out);
	sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
	switch (sqlite3_step(stmt)) {
	case SQLITE_DONE:
		managererr = "Object not in database";
		return NULL;
		break;
	case SQLITE_ROW:
		return (char *)sqlite3_column_blob(stmt, 0);
		break;
	}
}

char *getSource(char *id)
{

}

char *getCurrentSource(char *name)
{
	sqlite3_stmt *stmt;
	const char *out;

	sqlite3_prepare_v2(db, stmt_get_1, strlen(stmt_get_1), &stmt, &out);
	sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
	switch (sqlite3_step(stmt)) {
	case SQLITE_DONE:
		managererr = "Object not in database";
		return NULL;
		break;
	case SQLITE_ROW:
		return sqlite3_column_text(stmt, 0);
		break;
	}
}
