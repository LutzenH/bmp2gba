#ifndef B2G_PLATFORM_H
#define B2G_PLATFORM_H

unsigned char* platform_load_file_in_memory(const char* path, unsigned int* file_size);

const char** platform_list_files_within_folder(const char* folder_path, unsigned int* string_count, int recursive, const char* filter_by_extension);

#endif //B2G_PLATFORM_H
