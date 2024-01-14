#include <curl/curl.h>
#include <curl/easy.h>
#include <stdlib.h>
#include <string.h>

#include "curl.h"

#define BASE_URL_SIZE 38

void init_string(string* s)
{
	s->len = 0;
	s->buf = malloc(s->len + 1);

	if (NULL == s->buf) {
		fprintf(stderr, "malloc() failed!\n");
		exit(EXIT_FAILURE);
	}
	s->buf[0] = '\0';
}

size_t writefunc(void* ptr, size_t size, size_t nmemb, string* s)
{
	size_t new_len = s->len + (size * nmemb);
	s->buf = realloc(s->buf, new_len + 1);
	if (NULL == s->buf) {
		fprintf(stderr, "realloc() failed!\n");
		exit(EXIT_FAILURE);
	}
	memcpy(s->buf + s->len, ptr, size * nmemb);
	s->buf[new_len] = '\0';
	s->len = new_len;
	return size * nmemb;
}

CURLcode perform_book_get(char* isbn, string* s)
{
	CURL* handler;
	char* base_url;
	size_t finalurl_size = BASE_URL_SIZE + 15;
	char finalurl[finalurl_size];
	CURLcode result;

	handler = curl_easy_init();
	if (!handler) {
		fprintf(stderr, "cURL failed to initialize!\n");
		exit(EXIT_FAILURE);
	}

	base_url = "https://openlibrary.org/search.json?q=";

	// Setup the URL for the request
	snprintf(finalurl, finalurl_size, "%s%s", base_url, isbn);

	curl_easy_setopt(handler, CURLOPT_URL, finalurl);
	curl_easy_setopt(handler, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, writefunc);
	curl_easy_setopt(handler, CURLOPT_WRITEDATA, s);

	result = curl_easy_perform(handler);

	// Always clean up
	curl_easy_cleanup(handler);

	return result;

}
