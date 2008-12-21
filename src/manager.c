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
#include "exception.h"

sqlite3 *db;
char *managererr;

char *stmt_set_2 = "INSERT INTO state (name, time, source, stream, UID) "
"VALUES (:UUID, :time, :source, :stream, :UID);";
char *stmt_get_1 = "SELECT source from state where name = :name "
	"order by time desc limit 1";
char *stmt_get_2 = "SELECT stream from state where name = :name "
	"order by time desc limit 1";

int busyHandler(void *data, int times)
{
	return 1;
}

void initManager()
{
	char *home = getenv("HOME");
	char filename[256];
	sprintf(filename, "%s/.s3d/manager.db", home);
	THROWIF(sqlite3_open_v2(filename, &db, 
				SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_READWRITE, 
				NULL) 
			!= SQLITE_OK, EXC_MAN_OPENDB, 
		"Could not open database");
	sqlite3_busy_handler(db, busyHandler, NULL);
}

void initManagerRO()
{
	char *home = getenv("HOME");
	char filename[256];
	sprintf(filename, "%s/.s3d/manager.db", home);
	THROWIF(sqlite3_open_v2(filename, &db, 
				SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READONLY, 
				NULL) 
			!= SQLITE_OK, EXC_MAN_OPENDB, 
		"Could not open database");
	sqlite3_busy_handler(db, busyHandler, NULL);

}

void quitManager()
{
	int i = 0;
	while (sqlite3_next_stmt(db, NULL) != NULL) {
		i++;
		sqlite3_finalize(sqlite3_next_stmt(db, NULL));
	}
	fprintf(stderr, "%i\n", i);
	if (sqlite3_close(db) != SQLITE_OK)
		fprintf(stderr, "%s\n", sqlite3_errmsg(db));
}

void insertNewState(char *name, char *stream, char *source, int uid, int time)
{
	const char *out;
	int status;

	sqlite3_stmt *stmt2;
	sqlite3_prepare_v2(db, stmt_set_2, -1, &stmt2, &out);
	sqlite3_bind_text(stmt2, 1, name, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt2, 2, time);
	sqlite3_bind_text(stmt2, 3, source, -1, SQLITE_STATIC);
	sqlite3_bind_blob(stmt2, 4, (void *)stream, 
			  getStreamsize(stream), SQLITE_STATIC);
	sqlite3_bind_int(stmt2, 5, uid);
	status = sqlite3_step(stmt2);
	fprintf(stderr, "%i %s\n", status, sqlite3_errmsg(db));
	THROWIF(status != SQLITE_DONE, EXC_MAN_INSERT,
		"Could not create state");
	sqlite3_finalize(stmt2);
}

void set(char *name, char *stream, char *source, int uid)
{
	char *out;
	int *istream = (int*)stream;
	int t = time(NULL);

	insertNewState(name, stream, source, uid, t);
}

void setSource(char *id, char *source)
{

}

void setUID(char *id, int uid)
{

}

char *get(char *name)
{
	sqlite3_stmt *stmt;
	const char *out;
	int code;

	sqlite3_prepare_v2(db, stmt_get_1, -1, &stmt, &out);
	sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
	switch (sqlite3_step(stmt)) {
	case SQLITE_DONE:
		THROWS(EXC_MAN_OBJNOTINDB, "Object not in database");
	case SQLITE_ROW:
		return (char*)sqlite3_column_text(stmt, 0);
		break;
	}
	code = sqlite3_finalize(stmt);
	if (code != SQLITE_OK)
		fprintf(stderr, "%s\n", sqlite3_errmsg(db));
}

char *getCurrent(char *name)
{
	sqlite3_stmt *stmt;
	const char *out;
	int code;

	sqlite3_prepare_v2(db, stmt_get_2, -1, &stmt, &out);
	sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
	switch (sqlite3_step(stmt)) {
	case SQLITE_DONE:
		THROWS(EXC_MAN_OBJNOTINDB, "Object not in database");
	case SQLITE_ROW:
		return (char *)sqlite3_column_blob(stmt, 0);
		break;
	}
	code = sqlite3_finalize(stmt);
	if (code != SQLITE_OK)
		fprintf(stderr, "%s\n", sqlite3_errmsg(db));
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
		THROWS(EXC_MAN_OBJNOTINDB, "Object not in database");
	case SQLITE_ROW:
		return (char *)sqlite3_column_text(stmt, 0);
		break;
	}

	sqlite3_finalize(stmt);
}
