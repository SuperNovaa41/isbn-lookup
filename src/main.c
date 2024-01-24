#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "curl.h"
#include "json.h"
#include "csv.h"

#define MAX_BUF_LEN 1024 

/**
 * TODO: we need to check the csv file for duplicates
 * TODO: allow us to remove a book from the csv and update the ids
 */
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

	write_to_file(&new_book);

	// we need to free these strings
	free(get_output.buf);
	free(new_book.authors);
}

void process_args(char* argv[])
{
	if (0 == strcmp(argv[1], "--help")) {
		printf("%s - An ISBN lookup tool.\n", argv[0]);
		printf("Author: Nathan Singer\n");

		puts("\n");

		puts("--help - Shows this message.");
		puts("[isbn] -- Attempts to download a book from the given ISBN-10 or ISBN-13 input.");
		puts("remove [id] -- Removes a book with the given ID from the book database.");
	} else if (0 == strcmp(argv[1], "remove")) {
		// remove a book here
		return;
	} else {
		// lets assume its an ISBN and let the other functions fail if its not
		do_ISBN_get(argv);
	}
}



int main(int argc, char* argv[])
{
	return 0;
	
	if (1 == argc) {
		printf("Not enough arguments! Try typing %s --help\n", argv[0]);
		return EXIT_FAILURE;
	}

	process_args(argv);

	return 0;
}
