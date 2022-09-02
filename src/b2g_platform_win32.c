#include <stdio.h>
#include <windows.h>

#define i_implement
#include <stc/cstr.h>

#define i_val_str
#include <stc/cvec.h>

static inline bool internal_platform_helper_string_ends_with(const char* string, const char* suffix)
{
	if (!string || !suffix) {
		return false;
	}

	size_t string_length = strlen(string);
	size_t suffix_length = strlen(suffix);
	if (suffix_length >  string_length) {
		return false;
	}

	return strncmp(string + string_length - suffix_length, suffix, suffix_length) == 0;
}

int internal_platform_list_files_within_folder(cvec_str* folders, const char* folder, bool recursive, const char* filter_by_extension, const char* prefix)
{
	int count = 0;

	cstr search_path = cstr_from_fmt("%s/*.*", folder);

	WIN32_FIND_DATA fd;
	HANDLE find_handle = FindFirstFile(cstr_str(&search_path), &fd);
	if(find_handle != INVALID_HANDLE_VALUE) {
		do {
			if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && recursive) {
				if(strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
					cstr inner_folder = cstr_from_fmt("%s/%s", folder, fd.cFileName);
					cstr inner_prefix = cstr_from_fmt("%s%s/", prefix, fd.cFileName);

					count += internal_platform_list_files_within_folder(folders, cstr_str(&inner_folder), recursive, filter_by_extension, cstr_str(&inner_prefix));

					cstr_drop(&inner_folder);
					cstr_drop(&inner_prefix);
				}
			}

			else if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				&& (filter_by_extension == NULL || internal_platform_helper_string_ends_with(fd.cFileName, filter_by_extension)))
			{
				cvec_str_push_back(folders, cstr_from_fmt("%s%s", prefix, fd.cFileName));
				count++;
			}
		} while(FindNextFile(find_handle, &fd));
		FindClose(find_handle);
	}

	return count;
}

const char** internal_platform_string_vector_to_c_style(cvec_str* folders, size_t* string_count) {
	size_t count = cvec_str_size(folders);

	size_t buffer_size = 0;
	buffer_size += count * sizeof(const char*);
	c_foreach (i, cvec_str, *folders) {
		size_t string_size = cstr_size(i.ref);
		buffer_size += (string_size + 1) * sizeof(char);
	}

	unsigned char* buffer = (unsigned char*)calloc(buffer_size, sizeof(unsigned char));
	const char** list_ptr = (const char**)buffer;
	char* string_buffer = (char*)(buffer + count * sizeof(const char*));

	for (int i = 0; i < count; ++i) {
		list_ptr[i] = string_buffer;

		const cstr* str = cvec_str_at(folders, i);

		size_t string_size = cstr_size(cvec_str_at(folders, i));
		memcpy(string_buffer, cstr_str(str), string_size);
		string_buffer[string_size] = '\0';
		string_buffer += (string_size + 1) * sizeof(char);
	}

	*string_count = count;
	return (const char**)buffer;
}

const char** platform_list_files_within_folder(const char* folder_path, size_t* string_count, bool recursive, const char* filter_by_extension) {
	cvec_str folders = cvec_str_init();

	internal_platform_list_files_within_folder(&folders, folder_path, recursive, filter_by_extension, "");

	const char** file_names = internal_platform_string_vector_to_c_style(&folders, string_count);

	cvec_str_drop(&folders);

	return file_names;
}
