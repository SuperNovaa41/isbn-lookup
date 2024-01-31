#ifndef DH_H
#define DH_H

enum DB_OPTIONS {
	ADD,
	REMOVE
};

/**
 * void do_db_entry
 *
 * enum DB_OPTIONS option - The type of transaction being made to the DB
 * VA args
 * 	- Expects book_t if option is ADD
 * 	- Expects int if option is REMOVE
 */
void do_db_entry(enum DB_OPTIONS option, ...);

void setup_db(sqlite3* db);

void add_to_db(book_t* book, sqlite3* db);

void remove_from_db(int id, sqlite3* db);

#endif
