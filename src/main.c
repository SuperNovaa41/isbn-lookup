#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "curl.h"
#include "json.h"

#define FILE_NAME "books.csv"

#define MAX_BUF_LEN 1024 

/**
 * TODO: we need to check the csv file for duplicates
 * TODO: assign an ID to each book
 * TODO: allow us to remove a book from the csv and update the ids
 */


enum book_options {
	PRINT_TO_STDIN,
	WRITE_TO_FILE	
};

void print_book(book_t* book)
{
	printf("ISBN: %s\n", book->isbn);
	printf("Title: %s\n", book->title);
	printf("Author(s): %s\n", book->authors);
	printf("(First) Year of Publication: %d\n", book->year_of_publication);
	printf("Page length: %d\n", book->page_len);
}

void write_to_file(book_t* book)
{
	FILE* file;
	int file_exists;

	/**
	 * We want to check if the file exists
	 * if it doesnt, we create a new one
	 * otherwise, we write to the existing one
	 */
	file_exists = access(FILE_NAME, F_OK);
	if (0 != file_exists) {
		file = fopen(FILE_NAME, "w");
		fprintf(file, "isbn,title,authors,year of publication,page length\n");
	} else {
		file = fopen(FILE_NAME, "a");
	}

	// now we write the information
	fprintf(file, "\"%s\",\"%s\",\"%s\",%d,%d\n", 
			book->isbn, book->title, book->authors, book->year_of_publication, book->page_len);

	fclose(file);
}

void do_output(book_t* book, enum book_options output_type)
{
	if (WRITE_TO_FILE == output_type) {
		write_to_file(book);
		return;
	}
	print_book(book);
}

int main(int argc, char* argv[])
{
	// want to hold a max of 14 so we can hold up to ISBN13s
	char isbn_buf[14];
	char options[2];
	CURLcode res;
	book_t new_book;
	enum book_options output_type;

	if (3 != argc) {
		printf("Usage: isbn [isbn] [options]\n");
		return EXIT_FAILURE;
	}

	size_t input_len = strlen(argv[1]);
	if (!(13 == input_len || 10 == input_len)) {
		fprintf(stderr, "Invalid ISBN submitted!");
		return EXIT_FAILURE;
	}


	// We must initialize cURL
	curl_global_init(CURL_GLOBAL_ALL);

	// Grab the ISBN from argv
	memcpy(isbn_buf, argv[1], 14);

	// Grab the formatting options from argv
	memcpy(options, argv[2], 2);
	switch(options[0]) {
		case 'w':
			output_type = WRITE_TO_FILE;
			break;
		case 'r':
			output_type = PRINT_TO_STDIN;
			break;
		default:
			fprintf(stderr, "Invalid option provided!\n");
			return EXIT_FAILURE;
	}


	// Setup the output string
	string get_output;
	init_string(&get_output);

	//  Perform the get request
	res = perform_book_get(isbn_buf, &get_output);
	if (0 != res) {
		fprintf(stderr, "Failed to perform the get request!\n");
		return EXIT_FAILURE;
	}


	// Now we want to parse the JSON input
	parse_json(&get_output, isbn_buf, &new_book);

	// We need to free this string
	free(get_output.buf);

	//  NOW we either print or save the book
	do_output(&new_book, output_type);

	// Need to tree this string
	free(new_book.authors);

	return 0;
}
