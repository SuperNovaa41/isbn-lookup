#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>

#define BASE_URL_SIZE 38

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
		fprintf(stderr, "malloc() failed\n");
		exit(EXIT_FAILURE);
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

/**
 * void parse_json
 * string* s - Pointer to the string struct
 *
 * Parses the JSON inside of s.buf and prints the information we're looking for
 */
void parse_json(string* s)
{
	cJSON* json = cJSON_Parse(s->buf);
	if (NULL == json) {
		const char* error_ptr = cJSON_GetErrorPtr();
		if (NULL != error_ptr)
			fprintf(stderr, "JSON error: %s\n", error_ptr);
		cJSON_Delete(json);
		exit(EXIT_FAILURE);	
	}

	/**
	 * if there are no results, or too many, we should exit
 	 */
	cJSON* numFound = cJSON_GetObjectItemCaseSensitive(json, "numFound");
	if (0 == numFound->valueint) {
		fprintf(stderr, "No ISBN found!\n");
		exit(EXIT_FAILURE);
	} else if (numFound->valueint > 1) {
		fprintf(stderr, "Multipled ISBNs found, exiting!\n");
		exit(EXIT_FAILURE);
	}


	cJSON* docs = cJSON_GetObjectItemCaseSensitive(json, "docs");
	cJSON* child = docs->child; // this is the JSON object that stores all of the books information

	printf("Title: %s\n", cJSON_GetObjectItemCaseSensitive(child, "title")->valuestring);


	/**
	 * The author value is a linked list, so we want to loop through each value
	 */
	cJSON* authors = cJSON_GetObjectItemCaseSensitive(child, "author_name");
	cJSON* authorarr = authors->child;

	printf("Author(s): ");
	while (NULL != authorarr->next) {
		printf("%s, ", authorarr->valuestring);
		authorarr = authorarr->next;
	}
	printf("%s\n", authorarr->valuestring);

	printf("(First) Year of Publication: %d\n", cJSON_GetObjectItemCaseSensitive(child, "first_publish_year")->valueint);
	printf("Page length: %d\n", cJSON_GetObjectItemCaseSensitive(child, "number_of_pages_median")->valueint);
}

int main(int argc, char* argv[])
{
	char isbn_buf[14]; // want to hold a max of 14 so we can hold up to ISBN13s
	CURLcode res;

	if (2 != argc) {
		printf("Usage: isbn [isbn]\n");
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
	 * Setup the output string
	 */
	string get_output;
	init_string(&get_output);

	/**
	 *  Perform the get request
	 */ res = perform_book_get(isbn_buf, &get_output);
	if (0 != res) {
		fprintf(stderr, "Failed to perform the get request!\n");
		return EXIT_FAILURE;
	}

	/**
	 * Now we want to parse the JSON input
	 */
	parse_json(&get_output);

	/**
	 * We need to free this string
	 */
	free(get_output.buf);


	return 0;
}
