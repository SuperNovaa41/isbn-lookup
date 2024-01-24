#ifndef JSON_H
#define JSON_H

#define MAX_BUF_LEN 1024

typedef struct book_t {
	char* isbn;
	char* title;
	char* authors;
	char* image_url;
	int year_of_publication;
	int page_len;
} book_t;

/**
 * void check_valid_query
 * cJSON* numfound - The json entry that contains the numFound value
 *
 * Checks if there are any search results in this json entry
 */
void check_valid_query(cJSON* numfound);

/**
 * void get_authors
 * cJSON* bookinfo - The JSON entry that has all of the information about the book
 * char* authors - The string to fill
 *
 * Takes the JSON entry from bookinfo and combines all of the authors into one final string
 */
void get_authors(cJSON* bookinfo, char* authors);

/**
 * void parse_json
 * string* s - The string struct
 * char* isbn - The ISBN
 * book_t* book - The struct to fill with book information
 *
 * Parses through the given JSON entry and gathers all of the information we need
 */
void parse_json(string* s, char* isbn, book_t* book);
#endif
