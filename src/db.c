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
	if (ADD == option) {
		add_to_db(va_arg(args, book_t*), db);
	} else if (REMOVE == option) {
		remove_from_db(va_arg(args, int), db);
	}
	va_end(args);

	sqlite3_close(db);
}

void setup_db(sqlite3* db)
{
	int rc;
	char* err_msg = 0;
	char* sql;

	sql = "CREATE TABLE IF NOT EXISTS books (isbn TEXT, title TEXT, authors TEXT, imageurl TEXT, year_of_publication YEAR, page_length UNSIGNED INT);";

	rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

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
	int asp_err, rc;
	char* err_msg = 0;

	asp_err = asprintf(&sql, "INSERT INTO books (isbn, title, authors, imageurl, year_of_publication, page_length) VALUES(\"%s\", \"%s\", \"%s\", \"%s\", %d, %d);", book->isbn, book->title, book->authors, book->image_url, book->year_of_publication, book->page_len);
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

