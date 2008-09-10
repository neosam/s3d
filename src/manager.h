/*
 * File manager.h
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

/*
 * Manages importing and exporting of the sqlite database.
 * While a server stores all the objects a client can use it for caching.
 */

#ifndef MANAGER_H
#define MANAGER_H

#include "sqlite3.h"

/* Exceptions */
#define EXC_MAN_OPENDB -1
#define EXC_MAN_INSERT -2
#define EXC_MAN_OBJNOTINDB -3

extern sqlite3 *db;
extern char *managererr;

void initManager();
void quitManager();

/*
 * Add a new object or update it in the database.  
 * Source can be NULL and uid can be -1.  This can be useful for clients which 
 * streams just the binary stream data.  You can update these informations
 * later by calling setSource or setUID.
 * Returns 0 on success and -1 on error.
 */
void set(char *name, char *stream, char *source, int uid);

/*
 * Set the sourcecode of the given id.
 * Returns 0 on success or -1 on error.
 */
void setSource(char *id, char *source);

/*
 * Set the uid of the given id.
 * Returns 0 on success or -1 on error.
 */
void setUID(char *id, int uid);

/*
 * Returns the stream of the given ID or NULL on error (maybe id doesn't 
 * exist).
 */
char *get(char *name);

/*
 * Returns the stream of the given uuid or NULL on error (maybe uuid doesn't
 * exist).
 */
char *getCurrent(char *uuid);

/*
 * Returns the sourcecode of the given ID or NULL on error (maybe id doesn't
 * exist.
 */
char *getSource(char *id);

/*
 * Returns the sourcecode of the given uuid or NULL on error (maybe uuid
 * doesn't exist.
 */
char *getCurrentSource(char *name);

#endif /* MANAGER_H */
