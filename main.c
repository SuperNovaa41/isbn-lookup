#include <curl/curl.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cjson/cJSON.h>

#define BASE_URL_SIZE 38
#define FILE_NAME "books.csv"

#define MAX_BUF_LEN 1024 

/**
 * struct string_t
 * char* buf - The buffer containing the string data
 * size_t len - The length of the buffer
 */
typedef struct string_t {
	char* buf;
	size_t len;	
} string;

/**
 * void init_string
 * string* s - The string struct
 *
 * Initializes the string struct */
void init_string (string* s)
{
	s->len = 0;
	s->buf = malloc(s->len + 1);

	if (NULL == s->buf) {
		fprintf(stderr, "malloc() failed\n"); exit(EXIT_FAILURE);
	}
	s->buf[0] = '\0';
}

/**
 * size_t writefunc
 *
 * The arguments come from CURLOPT_WRITEFUNCTION
 *
 * Writes the cURL get into the string struct
 */
size_t writefunc(void* ptr, size_t size, size_t nmemb, string* s)
{
	size_t new_len = s->len + (size * nmemb);
	s->buf = realloc(s->buf, new_len + 1); 
	if (NULL == s->buf) {
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}	

	memcpy(s->buf + s->len, ptr, size * nmemb);
	s->buf[new_len] = '\0';
	s->len = new_len;

	return size * nmemb;
}

/**
 * CURLcode perform_book_get
 * char isbn_buf[14] - The ISBN
 * string* s - The string struct to place the GET output in
 *
 * Perforrms a GET request with the ISBN
 */
CURLcode perform_book_get(char isbn_buf[14], string* s)
{
	CURL* handler;
	char *base_url;
	size_t finalurl_size = BASE_URL_SIZE + 15;
	char finalurl[finalurl_size];
	CURLcode result;

	handler = curl_easy_init();
	if (!handler) {
		fprintf(stderr, "cURL failed to initialize!\n");
		exit(EXIT_FAILURE);
	}

	base_url = "https://openlibrary.org/search.json?q=";
	/**
	 * Setup the URL for the request
	 */
	snprintf(finalurl, finalurl_size, "%s%s", base_url, isbn_buf);

	curl_easy_setopt(handler, CURLOPT_URL, finalurl);
	curl_easy_setopt(handler, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, writefunc);
	curl_easy_setopt(handler, CURLOPT_WRITEDATA, s);

	result = curl_easy_perform(handler);

	// Always clean up	
	curl_easy_cleanup(handler);

	return result;
}

void print_book(char* isbn, char* title, char* authors, int year, int page_len)
{
	printf("ISBN: %s\n", isbn);
	printf("Title: %s\n", title);
	printf("Author(s): %s\n", authors);
	printf("(First) Year of Publication: %d\n", year);
	printf("Page length: %d\n", page_len);
}

void write_to_file(char* isbn, char* title, char* authors, int year, int page_len)
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
	fprintf(file, "\"%s\",\"%s\",\"%s\",%d,%d\n", isbn, title, authors, year, page_len);

	fclose(file);
}

/**
 * void parse_json
 * string* s - Pointer to the string struct
 *
 * Parses the JSON inside of s.buf and prints the information we're looking for
 */
void parse_json(string* s, char* options, char* isbn_buf)
{
	char *title, *temp_author;
	int year, page_len, new_len;
	char authors[MAX_BUF_LEN];

	cJSON* json = cJSON_Parse(s->buf);
	if (NULL == json) {
		const char* error_ptr = cJSON_GetErrorPtr();
		if (NULL != error_ptr)
			fprintf(stderr, "JSON error: %s\n", error_ptr);
		cJSON_Delete(json);
		exit(EXIT_FAILURE);	
	}

	/**
	 * if there are no results, we should exit
	 * if theres too many, we should just treat it normally
 	 */
	cJSON* numFound = cJSON_GetObjectItemCaseSensitive(json, "numFound");
	if (0 == numFound->valueint) {
		fprintf(stderr, "No ISBN found!\n");
		exit(EXIT_FAILURE);
	}

	cJSON* docs = cJSON_GetObjectItemCaseSensitive(json, "docs");
	cJSON* child = docs->child; // this is the JSON object that stores all of the books information

	title = cJSON_GetObjectItemCaseSensitive(child, "title")->valuestring;
	year = cJSON_GetObjectItemCaseSensitive(child, "first_publish_year")->valueint;
	page_len = cJSON_GetObjectItemCaseSensitive(child, "number_of_pages_median")->valueint;

	/**
	 * The author value is a linked list, so we want to loop through each value
	 */

	cJSON* authors_json = cJSON_GetObjectItemCaseSensitive(child, "author_name");
	cJSON* authorarr = authors_json->child;

	// we want to do this first out of the loop, because of formatting	
	snprintf(authors, strlen(authorarr->valuestring) + 1, "%s", authorarr->valuestring);
	authorarr = authorarr->next;
	while (NULL != authorarr) {
		// The plus one is for the \0, the plus two is for the ", "
		new_len = strlen(authors) + strlen(authorarr->valuestring) + 1 + 2;

		/**
		 * So first we make a new string to hold the new string temporarily
		 *
		 * Then we want to add formatting by using the existing authors string
		 * along with the next string
		 *
		 * Then we memcpy the temp string into the original string
		 */
		temp_author = malloc(sizeof(char) * new_len);
		snprintf(temp_author, new_len, "%s, %s", authors, authorarr->valuestring);
		memcpy(authors, temp_author, new_len);

		// Free since we malloc'd
		free(temp_author);
		authorarr = authorarr->next;
	}

	if (0 == strcmp(options, "r")) 
		print_book(isbn_buf, title, authors, year, page_len);
	else if (0 == strcmp(options, "w"))
		write_to_file(isbn_buf, title, authors, year, page_len);
	else
		fprintf(stderr, "Improper option provided!");
}



int main(int argc, char* argv[])
{
	char isbn_buf[14]; // want to hold a max of 14 so we can hold up to ISBN13s
	char options[2];
	CURLcode res;

	if (3 != argc) {
		printf("Usage: isbn [isbn] [options]\n");
		return EXIT_FAILURE;
	}

	size_t input_len = strlen(argv[1]);
	if (!(13 == input_len || 10 == input_len)) {
		fprintf(stderr, "Invalid ISBN submitted!");
		return EXIT_FAILURE;
	}

	/**
	 * We must initialize cURL
	 */
	curl_global_init(CURL_GLOBAL_ALL);

	/**
	 * Grab the ISBN from argv
	 */
	snprintf(isbn_buf, 14, "%s", argv[1]);

	/**
	 * Grab the formatting options from argv
	 */
	snprintf(options, 2, "%s", argv[2]);

	/**
	 * Setup the output string
	 */
	string get_output;
	init_string(&get_output);

	/**
	 *  Perform the get request
	 */ 
	res = perform_book_get(isbn_buf, &get_output);
	if (0 != res) {
		fprintf(stderr, "Failed to perform the get request!\n");
		return EXIT_FAILURE;
	}

	/**
	 * Now we want to parse the JSON input
	 */
	parse_json(&get_output, options, isbn_buf);

	/**
	 * We need to free this string
	 */
	free(get_output.buf);


	return 0;
}
