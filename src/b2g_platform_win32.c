#include <stdio.h>
#include <windows.h>

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

const char** platform_list_files_within_folder(const char* folder_path, size_t* string_count, bool recursive, const char* filter_by_extension) {
	cvec_str folders = cvec_str_init();

	internal_platform_list_files_within_folder(&folders, folder_path, recursive, filter_by_extension, "");

	const char** file_names = internal_platform_string_vector_to_c_style(&folders, string_count);

	cvec_str_drop(&folders);

	return file_names;
}
