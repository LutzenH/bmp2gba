#include <stdio.h>
#include <windows.h>

unsigned char* platform_load_file_in_memory(const char* path, unsigned int* file_size)
{
	HANDLE file_handle = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file_handle == INVALID_HANDLE_VALUE)
	{
		*file_size = 0;
		return NULL;
	}

	DWORD win_file_size = GetFileSize(file_handle, NULL);

	unsigned char* buffer = (unsigned char*)malloc(win_file_size);

	DWORD number_of_bytes_read = 0;
	bool success = ReadFile(file_handle, buffer, win_file_size, &number_of_bytes_read,NULL);
	CloseHandle(file_handle);
	if(!success) {
		fprintf(stderr, "Failed to read file: %s , reason: GetLastError=%08lx\n", path, GetLastError());
		*file_size = 0;
		return NULL;
	}

	*file_size = win_file_size;
	return buffer;
}

int internal_platform_list_files_within_folder(cvec_str* folders, const char* folder, int recursive, const char* filter_by_extension, const char* prefix)
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

const char** platform_list_files_within_folder(const char* folder_path, unsigned int* string_count, int recursive, const char* filter_by_extension) {
	cvec_str folders = cvec_str_init();

	internal_platform_list_files_within_folder(&folders, folder_path, recursive, filter_by_extension, "");

	const char** file_names = internal_platform_string_vector_to_c_style(&folders, string_count);

	cvec_str_drop(&folders);

	return file_names;
}
