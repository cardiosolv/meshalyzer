#ifndef TEST_TEXT_H
#define TEST_TEXT_H

#include "types.h"

int text_creates_new();
int text_gets_info();
int text_appends();
int text_reads();

int _text_compare_info(ch5_text_file *expected, ch5_text_file *given);

#endif
