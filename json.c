#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "curl.h"
#include "json.h"

#define MAX_BUF_LEN 1024

void check_valid_query(cJSON* numfound)
{
	if (0 != numfound->valueint)
		return;
	fprintf(stderr, "No ISBN found!\n");
	exit(EXIT_FAILURE);
}

void get_authors(cJSON* bookinfo, char authors[256])
{
	char* temp_author;
	size_t new_len;
	cJSON* author_arr = cJSON_GetObjectItemCaseSensitive(bookinfo, "author_name")->child;

	snprintf(authors, strlen(author_arr->valuestring) + 1, "%s", author_arr->valuestring);
	author_arr = author_arr->next;

	while (NULL != author_arr) {
		// The plus 1 is for the \0, the plus 2 is for the ", "
		new_len = strlen(authors) + strlen(author_arr->valuestring) + 1 + 2;

		temp_author = malloc(sizeof(char) * new_len);
		snprintf(temp_author, new_len, "%s, %s", authors, author_arr->valuestring);
		memcpy(authors, temp_author, new_len);

		free(temp_author);

		author_arr = author_arr->next;
	}
}

void parse_json(string* s, char* isbn, book_t* book)
{
	char authors[MAX_BUF_LEN];

	cJSON* json = cJSON_Parse(s->buf);
	if (NULL == json) {
		const char* error_ptr = cJSON_GetErrorPtr();
		if (NULL != error_ptr)
			fprintf(stderr, "JSON error: %s\n", error_ptr);
		cJSON_Delete(json);
		exit(EXIT_FAILURE);
	}

	check_valid_query(cJSON_GetObjectItemCaseSensitive(json, "numFound"));

	cJSON* bookinfo = cJSON_GetObjectItemCaseSensitive(json, "docs")->child;

	book->isbn = isbn;
	book->title = cJSON_GetObjectItemCaseSensitive(bookinfo, "title")->valuestring;
	book->year_of_publication = cJSON_GetObjectItemCaseSensitive(bookinfo, "first_publish_year")->valueint;
	book->page_len = cJSON_GetObjectItemCaseSensitive(bookinfo, "number_of_pages_median")->valueint;

	get_authors(bookinfo, authors);

	// Need to malloc, because we need to copy authors into the book struct
	book->authors = (char*) malloc(sizeof(char) * (strlen(authors) + 1));
	memcpy(book->authors, authors, strlen(authors) + 1);
}
