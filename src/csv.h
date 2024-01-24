#ifndef CSV_H
#define CSV_H

#define FILE_NAME "books.csv"

int get_next_id();

void write_to_file(book_t* book);

long find_id(int id_to_find);

#endif
