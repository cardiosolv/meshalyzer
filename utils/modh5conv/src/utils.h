#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <hdf5.h>

/**
 * Calculates a 2D -> 1D array offset where y is the "row", x is the "column"
 * and nx is the number of "columns" in the array.
 */
#define ARR_OFFSET(y,x,nx) (((y)*(nx))+(x))

/**
 * Joins a together path fragments with the glue string.
 * Example:
 *     util_join_path(3, "/", ".mod", "/usr/local/mods", "fairlight", "intro");
 *     --> "/usr/local/mods/fairlight/intro.mod"
 */
char* util_join_path(int num_parts, char const *glue, char const *ext, ...);

/**
 * Returns 1 if the given file exists, 0 otherwise.
 */
int util_file_exists(char const *path);

/**
 * Takes a file pointer and consumes newlines, carriage returns, tabs and
 * spaces until EOF or a character other than whitespace is encountered.
 */
void util_consume_witespace(FILE *file);

#endif
