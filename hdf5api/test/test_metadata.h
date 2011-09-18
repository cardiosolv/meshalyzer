#ifndef TEST_META_H
#define TEST_META_H

int meta_writes_version();
int meta_writes_name();
int meta_reads_version();
int meta_bad_version_returns_zero();
int meta_reads_name();
int meta_bad_name_returns_zero();

#endif
