#ifndef CSV_H
#define CSV_H

#define FILE_NAME "books.csv"

/**
 * int get_next_id
 *
 * Returns the next available ID so that we know what to assign to the book
 */
int get_next_id();

/**
 * void write_to_file
 * boot_t* book - A pointer to the book information struct
 *
 * Writes the book information to a CSV file
 */
void write_to_file(book_t* book);

/**
 * long find_id
 * int id_to_find - The ID we're looking for
 *
 * Returns the position of a FILE to the line with the ID we're looking for
 */
long find_id(int id_to_find);

#endif
