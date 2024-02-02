#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

void get_list(cJSON* bookinfo, char* in_str, char* json_value)
{
	char* temp_str;
	size_t new_len;
	cJSON* str_arr = cJSON_GetObjectItemCaseSensitive(bookinfo, json_value)->child;

	snprintf(in_str, strlen(str_arr->valuestring) + 1, "%s", str_arr->valuestring);
	str_arr = str_arr->next;

	while (NULL != str_arr) {
		// The plus 1 is for the \0, the plus 2 is for the ", "
		new_len = strlen(in_str) + strlen(str_arr->valuestring) + 1 + 2;

		temp_str = malloc(sizeof(char) * new_len);
		snprintf(temp_str, new_len, "%s, %s", in_str, str_arr->valuestring);
		memcpy(in_str, temp_str, new_len);

		free(temp_str);

		str_arr = str_arr->next;
	}
}


void get_image_link(cJSON* bookinfo, book_t* book)
{
	int image_id = cJSON_GetObjectItemCaseSensitive(bookinfo, "cover_i")->valueint;

	const char* begin = "https://covers.openlibrary.org/b/id/";
	const char* end = "-L.jpg";

	book->image_url = malloc(sizeof(char) * MAX_BUF_LEN);

	sprintf(book->image_url, "%s%d%s", begin, image_id, end);
}

void set_list_values(cJSON* bookinfo, book_t* book)
{
	char authors[MAX_BUF_LEN];
	char subjects[MAX_BUF_LEN];

	get_list(bookinfo, authors, "author_name");
	get_list(bookinfo, subjects, "subject");

	// Need to malloc, because we need to copy authors into the book struct
	book->authors = (char*) malloc(sizeof(char) * (strlen(authors) + 1));
	memcpy(book->authors, authors, strlen(authors) + 1);

	book->subjects = (char*) malloc(sizeof(char) * (strlen(subjects) + 1));
	memcpy(book->subjects, subjects, strlen(subjects) + 1);
}

void set_actual_values(cJSON* bookinfo, book_t* book, char* isbn)
{
	book->isbn = isbn;
	book->title = cJSON_GetObjectItemCaseSensitive(bookinfo, "title")->valuestring;
	book->year_of_publication = cJSON_GetObjectItemCaseSensitive(bookinfo, "first_publish_year")->valueint;
	book->page_len = cJSON_GetObjectItemCaseSensitive(bookinfo, "number_of_pages_median")->valueint;
	book->subtitle = cJSON_GetObjectItemCaseSensitive(bookinfo, "subtitle")->valuestring;
	book->publication_date = cJSON_GetObjectItemCaseSensitive(bookinfo, "publish_date")->child->valuestring;

	get_image_link(bookinfo, book);
}

void set_current_date(book_t* book)
{
	char date[MAX_BUF_LEN];

	time_t t;
	time(&t);

	sprintf(date, "%s", ctime(&t));

	book->date_added = (char*) malloc(sizeof(char) * (strlen(date) + 1));
	memcpy(book->date_added, date, strlen(date) + 1);
}

void parse_json(string* s, char* isbn, book_t* book)
{
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

	set_current_date(book);
	set_actual_values(bookinfo, book, isbn);
	set_list_values(bookinfo, book);
}
