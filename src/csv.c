#include <cjson/cJSON.h>
#include <curl/curl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "curl.h"
#include "json.h"
#include "csv.h"

#define MAX_BUFFER_SIZE 1024

int get_next_id()
{
	size_t i;
	char buffer[MAX_BUFFER_SIZE];
	FILE* csv;
	char id[MAX_BUFFER_SIZE];

	csv = fopen(FILE_NAME, "r");

	while (fgets(buffer, sizeof(buffer), csv));

	i = 0;
	while(buffer[i] != ',')
		i++;

	strncpy(id, buffer, i);

	return atoi(id) + 1;
}

long find_id(int id_to_find)
{
	FILE* csv;
	char* buffer;
	size_t line_size = MAX_BUFFER_SIZE - 1;
	int i, file_exists;
	long file_pos;


	file_exists = access(FILE_NAME, F_OK);
	if (0 != file_exists) {
		fprintf(stderr, "%s does not exist!", FILE_NAME);
		exit(EXIT_FAILURE);
	}

	csv = fopen(FILE_NAME, "r");
	if (NULL == csv) {
		fprintf(stderr, "Failed to open %s!\n", FILE_NAME);
		exit(EXIT_FAILURE);
	}

	for(i = 0; i < id_to_find + 1; i++)
		getline(&buffer, &line_size, csv);

	file_pos = ftell(csv);
	fclose(csv);

	return file_pos;
}


void write_to_file(book_t* book)
{
	FILE* file;
	int file_exists;
	int book_id;

	/**
	 * We want to check if the file exists
	 * if it doesnt, we create a new one
	 * otherwise, we write to the existing one
	 */
	file_exists = access(FILE_NAME, F_OK);
	if (0 != file_exists) {
		file = fopen(FILE_NAME, "w");
		fprintf(file, "id,isbn,title,authors,imageurl,year of publication,page length\n");
		book_id = 1;
	} else {
		file = fopen(FILE_NAME, "a");
		book_id = get_next_id();
	}

	if (NULL == file) {
		fprintf(stderr, "Failed to open %s!\n", FILE_NAME);
		exit(EXIT_FAILURE);
	}

	// now we write the information
	fprintf(file, "%d,\"%s\",\"%s\",\"%s\",\"%s\",%d,%d\n", 
			book_id, book->isbn, book->title, book->authors, book->image_url, book->year_of_publication, book->page_len);

	fclose(file);
}
