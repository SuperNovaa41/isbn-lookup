#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <unistd.h>

#include "curl.h"
#include "json.h"
#include "db.h"

#define MAX_BUF_LEN 1024 

void do_ISBN_get(char* argv[])
{
	// want to hold a max of 14 so we can hold up to ISBN13s
	char isbn_buf[14];
	book_t new_book;
	CURLcode res;
	size_t input_len = strlen(argv[1]);


	if (!(13 == input_len || 10 == input_len)) {
		fprintf(stderr, "Invalid ISBN submitted!\n");
		exit(EXIT_FAILURE);
	}

	// We must initialize cURL
	curl_global_init(CURL_GLOBAL_ALL);

	// Grab the ISBN from argv
	memcpy(isbn_buf, argv[1], 14);

	// Setup the output string
	string get_output;
	init_string(&get_output);

	//  Perform the get request
	res = perform_book_get(isbn_buf, &get_output);
	if (0 != res) {
		fprintf(stderr, "Failed to perform the get request!\n");
		exit(EXIT_FAILURE);
	}

	// Now we want to parse the JSON input
	parse_json(&get_output, isbn_buf, &new_book);

	do_db_entry(ADD, &new_book);

	// we need to free these strings
	free(get_output.buf);
	free(new_book.authors);
	free(new_book.image_url);
}

void print_help_menu(char* program)
{
	printf("%s - An ISBN lookup tool.\n", program);
	printf("Author: Nathan Singer\n");

	puts("\n");

	puts("--help - Shows this message.");
	puts("[isbn] -- Attempts to download a book from the given ISBN-10 or ISBN-13 input.");
	puts("remove [id] -- Removes a book with the given ID from the book database.");
}

void process_args(char* argv[])
{
	int id;
	if (0 == strcmp(argv[1], "--help")) {
		print_help_menu(argv[0]);
	} else if (0 == strcmp(argv[1], "remove")) {
		if (NULL == argv[2]) {
			printf("Not enough arguments! Try typing %s --help\n", argv[0]);
			exit(EXIT_FAILURE);
		}

		id = atoi(argv[2]);

		if (0 == id) {
			printf("Invalid book ID given!\n");
			exit(EXIT_FAILURE);
		}
		do_db_entry(REMOVE, id);
	} else {
		// lets assume its an ISBN and let the other functions fail if its not
		do_ISBN_get(argv);
	}
}

int main(int argc, char* argv[])
{
	if (1 == argc) {
		printf("Not enough arguments! Try typing %s --help\n", argv[0]);
		return EXIT_FAILURE;
	}

	process_args(argv);
	

	return 0;
}
