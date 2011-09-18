#include "utils.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

char* util_join_path(int num_parts, char const *glue, char const *ext, ...) {
  int length = num_parts - 1;
  va_list vargs;
  va_start(vargs, ext);
  for (int i = 0; i < num_parts; i++) {
    char *x = va_arg(vargs, char*);
    length += strlen(x);
  }
  if (ext != NULL) length += strlen(ext);
  
  va_start(vargs, ext);
  char *result = (char*) calloc(sizeof(char), length+1);
  char *index = result;
  for (int i = 0; i < num_parts; i++) {
    char *x = va_arg(vargs, char*);
    strcpy(index, x);
    index += strlen(x);
    if (i < num_parts - 1) {
      strcpy(index, glue);
      index += strlen(glue);
    }
  }
  if (ext != NULL) strcpy(index, ext);
  va_end(vargs);
  return result;
}

int util_file_exists(char const *path) {
  FILE *data_file = fopen(path, "r");
  if (data_file != NULL) {
    fclose(data_file);
    return 1;
  }
  return 0;
}

void util_consume_witespace(FILE *file) {
  fpos_t last_pos;
  char buff;
  while (!feof(file)) {
    fgetpos(file, &last_pos);
    fscanf(file, "%c", &buff);
    if (buff != '\n' && buff != '\r' && buff != '\t' && buff != ' ') {
      fsetpos(file, &last_pos);
      return;
    }
  }
}
