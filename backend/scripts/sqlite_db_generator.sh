#!/bin/bash
mkdir "$PWD/data/"
touch "$PWD/data/database.db"
chmod 664 "$PWD/data/database.db"
sqlite3 -batch "$PWD/data/database.db" < "$PWD/backend/db/sql/users.sql"