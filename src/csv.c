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

void update_line(char** line, int new_id)
{
	int i;
	char* buffer;
	for (i = 0; i < MAX_BUFFER_SIZE; i++) {
		if ((*line)[i] == ',')
			break;
	}
	
	if (i == MAX_BUFFER_SIZE - 1) {
		fprintf(stderr, "There was an error in %s file!\n", FILE_NAME);
		exit(EXIT_FAILURE);
	}

	buffer = malloc(sizeof(char) * MAX_BUFFER_SIZE);
	strncpy(buffer, (*line) + i, MAX_BUFFER_SIZE);

	snprintf(*line, MAX_BUFFER_SIZE, "%d%s", new_id, buffer);
	free(buffer);
}

void remove_line_from_file(int id_to_remove)
{
	int file_exists, line_count;
	FILE* csv;
	FILE* new_csv;
	char* line;
	size_t line_size = MAX_BUFFER_SIZE;

	file_exists = access(FILE_NAME, F_OK);
	if (0 != file_exists) {
		fprintf(stderr, "%s does not exist!\n", FILE_NAME);
		exit(EXIT_FAILURE);
	}

	csv = fopen(FILE_NAME, "r");
	if (NULL == csv) {
		fprintf(stderr, "Failed to open %s!\n", FILE_NAME);
		exit(EXIT_FAILURE);
	}

	new_csv = fopen("temp.csv", "w");

	line_count = 0;
	while(getline(&line, &line_size, csv) != -1) {
		if (id_to_remove > line_count) {
			fprintf(new_csv, "%s", line);
		} else if (id_to_remove < line_count) {
			update_line(&line, line_count - 1);
			fprintf(new_csv, "%s", line);
		}
		line_count++;
	}
	fclose(new_csv);
	fclose(csv);

	remove(FILE_NAME);
	rename("temp.csv", FILE_NAME); // new csv is now the original file, without that line
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
		// write the csv headers to the file since we're making it
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
