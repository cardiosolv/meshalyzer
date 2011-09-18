#include "test_utils.h"
#include "ch5.h"

#include <string.h>

int util_computes_joined_path_length() {
  int length1 = ch5_util_join_path_strlen(4, "/", NULL, "one", "two", "three", "four");
  int length2 = ch5_util_join_path_strlen(2, "/", ".jpg", "goatse.cx", "art");
  return (length1 == 19) && (length2 == 18);
}

int util_joins_static_path() {
  char str1[19];
  char str2[18];
  char expected1[19] = "one/two/three/four";
  char expected2[19] = "goatse.cx/art.jpg";
  ch5_util_join_path(str1, 4, "/", NULL, "one", "two", "three", "four");
  ch5_util_join_path(str2, 2, "/", ".jpg", "goatse.cx", "art");
  int result1 = strcmp(str1, expected1);
  int result2 = strcmp(str2, expected2);
  return (result1 == 0) && (result2 == 0) &&
         (strlen(str1) == 18) && (strlen(str2) == 17);
}
