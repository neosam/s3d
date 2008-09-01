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
#include "manager.h"

sqlite3 *db;
char *managererr;

char *stmt_set_2 = "INSERT INTO state (UUID, time, source, stream, UID) "
"VALUES (:UUID, :time, :source, :stream, :UID);";
char *stmt_set_3 = "INSERT INTO identifier (UUID, time) VALUES (:UUID, :time);";
char *stmt_set_4 = "UPDATE identifier SET time = :time WHERE UUID = :UUID;";

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

int setNewUUID(char *stream, char *source, int uid, int time)
{
	const char *out;

	printf("New uuid\n");
	sqlite3_stmt *stmt2;
	sqlite3_prepare_v2(db, stmt_set_2, strlen(stmt_set_2), &stmt2, &out);
	sqlite3_bind_blob(stmt2, 1, (void *)stream, 16, SQLITE_STATIC);
	sqlite3_bind_int(stmt2, 2, time);
	sqlite3_bind_text(stmt2, 3, source, -1, SQLITE_STATIC);
	sqlite3_bind_blob(stmt2, 4, (void *)stream, 
			  getStreamsize(stream), SQLITE_STATIC);
	sqlite3_bind_int(stmt2, 5, uid);
	if (sqlite3_step(stmt2) != SQLITE_DONE) {
		managererr = "Could not create state";
		return -1;
	}

	sqlite3_stmt *stmt3;
	sqlite3_prepare_v2(db, stmt_set_3, strlen(stmt_set_3), &stmt3, &out);
	sqlite3_bind_blob(stmt3, 1, (void *)stream, 16, SQLITE_STATIC);
	sqlite3_bind_int(stmt3, 2, time);
	if (sqlite3_step(stmt3) != SQLITE_DONE) {
		managererr = "Could not create identifier";
		return -1;
	}

	return 0;
}

int setUpdateUUID(char *stream, char *source, int uid, int time)
{
	const char *out;

	printf("Update uuid\n");
	sqlite3_stmt *stmt2;
	sqlite3_prepare_v2(db, stmt_set_2, strlen(stmt_set_2), &stmt2, &out);
	sqlite3_bind_blob(stmt2, 1, (void *)stream, 16, SQLITE_STATIC);
	sqlite3_bind_int(stmt2, 2, time);
	sqlite3_bind_text(stmt2, 3, source, -1, SQLITE_STATIC);
	sqlite3_bind_blob(stmt2, 4, (void *)stream, 
			  getStreamsize(stream), SQLITE_STATIC);
	sqlite3_bind_int(stmt2, 5, uid);
	if (sqlite3_step(stmt2) != SQLITE_DONE) {
		managererr = "Could not create state";
		return -1;
	}
	
	sqlite3_stmt *stmt3;
	sqlite3_prepare_v2(db, stmt_set_4, strlen(stmt_set_4), &stmt3, &out);
	sqlite3_bind_blob(stmt3, 2, (void *)stream, 16, SQLITE_STATIC);
	sqlite3_bind_int(stmt3, 1, time);
	if (sqlite3_step(stmt3) != SQLITE_DONE) {
		managererr = "Could not update identifier";
		return -1;
	}

	printf("done\n");
	return 0;
}

int set(char *stream, char *source, int uid)
{
	char *uuid = stream, *out;
	int *istream = (int*)stream;
	int time = istream[4];
	sqlite3_stmt *stmt1;

	printf("%i\n", time);
	
	sqlite3_prepare_v2(db, 
			   "select UUID from IDENTIFIER WHERE UUID=:uuid;",
			   45, &stmt1, &out);
	sqlite3_bind_blob(stmt1, 1, (void *)stream, 16, SQLITE_STATIC);
	switch (sqlite3_step(stmt1)) {
	case SQLITE_DONE:
		sqlite3_finalize(stmt1);
		return setNewUUID(stream, source, uid, time);
	case SQLITE_ROW:
		sqlite3_finalize(stmt1);
		return setUpdateUUID(stream, source, uid, time);
	default:
		printf("???\n");
		break;
	}

	return 0;
}

int setSource(char *id, char *source)
{

}

int setUID(char *id, int uid)
{

}

char *get(char *id)
{

}

char *getCurrent(char *uuid)
{

}

char *getSource(char *id)
{

}

char *getCurrentSource(char *uuid)
{

}
