#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <stdarg.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>


#include "curl.h"
#include "json.h"
#include "db.h"

void do_db_entry(enum DB_OPTIONS option, ...)
{
	int rc;
	sqlite3* db;
	va_list args;

	rc = sqlite3_open("books.db", &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);

		exit(EXIT_FAILURE);
	}	

	setup_db(db);

	va_start(args, option);
	switch(option) {
		case ADD:
			add_to_db(va_arg(args, book_t*), db);
			break;
		case REMOVE:
			remove_from_db(va_arg(args, int), db);
			break;
		default:
			fprintf(stderr, "Invalid db command given!\n");
			break;
	}
	va_end(args);

	sqlite3_close(db);
}

void setup_db(sqlite3* db)
{
	int rc;
	char* err_msg = 0;

	rc = sqlite3_exec(db, 
			"CREATE TABLE IF NOT EXISTS books (id UNSIGNED INT PRIMARY KEY, \
			isbn TEXT, title TEXT, authors TEXT, imageurl TEXT, year_of_publication YEAR, \
			page_length UNSIGNED INT, subjects TEXT, date_added TEXT, date_completed TEXT, \
			progress UNSIGNED TINYINT, publication_date TEXT, subtitle TEXT);", 
			0, 0, &err_msg);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", err_msg);

		sqlite3_free(err_msg);
		sqlite3_close(db);

		exit(EXIT_FAILURE);
	}

}

void add_to_db(book_t* book, sqlite3* db)
{
	char* sql;
	int asp_err, rc, entries = 0;
	char* err_msg = 0;

	sqlite3_stmt* getmsg;
	sqlite3_prepare(db, "SELECT * FROM BOOKS;", -1, &getmsg, NULL);
	while (sqlite3_step(getmsg) == SQLITE_ROW)
		entries++;
	sqlite3_finalize(getmsg);

	entries++; // new id!!

	asp_err = asprintf(&sql, "INSERT INTO books \
			(id, isbn, title, authors, imageurl, year_of_publication, \
			 page_length, subjects, date_added, publication_date, subtitle) \
			VALUES(%d, \"%s\", \"%s\", \"%s\", \"%s\", %d, %d, \"%s\", \"%s\", \"%s\", \"%s\");", 
			entries, book->isbn, book->title, book->authors, book->image_url, book->year_of_publication, 
			book->page_len, book->subjects, book->date_added, book->publication_date, book->subtitle);

	if (-1 == asp_err) {
		fprintf(stderr, "asprintf failed!\n");
		exit(EXIT_FAILURE);
	}

	rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(db);
		exit(EXIT_FAILURE);
	}
}

void remove_from_db(int id, sqlite3* db)
{
	char* sql;
	int asp_err, rc;
	char* err_msg = 0;

	asp_err = asprintf(&sql, "DELETE FROM books WHERE id = %d;", id);
	if (-1 == asp_err) {
		fprintf(stderr, "asprintf failed!\n");
		exit(EXIT_FAILURE);
	}

	rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(db);
		exit(EXIT_FAILURE);
	}
}

