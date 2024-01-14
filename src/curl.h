#ifndef CURL_H
#define CURL_H

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
 * Initializes the string struct
 */
void init_string(string* s);

/**
 * size_t writefunc
 *
 * The arguments from CURLOPT_WRITEFUNCTION
 *
 * Writes the cURL get into the string struct
 */
size_t writefunc(void* ptr, size_t size, size_t nmemb, string* s);

/**
 * CURLcode perform_book_get
 * char* isbn - The ISBN
 * string* s - The string struct to place the GET output in
 *
 * Performs a GET request with the ISBN
 */
CURLcode perform_book_get(char* isbn, string* s);

#endif
