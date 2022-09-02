#ifndef B2G_PLATFORM_H
#define B2G_PLATFORM_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

const char** platform_list_files_within_folder(const char* folder_path, size_t* string_count, bool recursive, const char* filter_by_extension);

#endif//B2G_PLATFORM_H
