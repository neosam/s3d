#!/bin/bash

# Creates a folder in the homedirectory which contains a sqlite database.

USAGE="$0 USER EMAIL"

if test -e $HOME/.s3d; then
    echo $HOME/.s3d already exists
    exit 1
fi
if test -z $1; then
    echo $USAGE
    exit 2
fi
if test -z $2; then
    echo $USAGE
    exit 2
fi

mkdir $HOME/.s3d
sqlite3 $HOME/.s3d/manager.db \
    "CREATE TABLE IDENTIFIER (UUID, time);
     CREATE TABLE STATE (UUID, time, source, stream, UID);
     CREATE TABLE AUTOR (UID PRIMARY KEY, name, email);
     INSERT INTO AUTOR (name, email) VALUES (\"$1\", \"$2\");
     INSERT INTO AUTOR (name) VALUES (\"guest\");
     INSERT INTO AUTOR (name) VALUES (\"unknown\");"
chmod a-rwx $HOME/.s3d/manager.db
chmod u+rw $HOME/.s3d/manager.db