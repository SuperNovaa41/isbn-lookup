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
 * void remove_line_from_file
 * int id_to_remove - The book ID that we don't want anymore
 *
 * Removes a book from the CSV file
 */
void remove_line_from_file(int id_to_remove);

/**
 * void update_line
 * char** line - Pointer to the book entry string
 * int new_id - The new ID that should be placed into this book entry
 *
 * Takes a book entry and changes the ID to the given one
 */
void update_line(char** line, int new_id);

#endif
