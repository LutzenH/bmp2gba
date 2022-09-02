#include <stdio.h>
#include <dirent.h>

int internal_platform_list_files_within_folder(cvec_str* folders, const char* folder, bool recursive, const char* filter_by_extension, const char* prefix) {
	int count = 0;

	DIR* directory_stream = NULL;
	struct dirent* dir = NULL;
	directory_stream = opendir(folder);
	if (directory_stream != NULL) {
		while ((dir = readdir(directory_stream)) != NULL) {
			if(dir->d_type == DT_REG && (filter_by_extension == NULL || internal_platform_helper_string_ends_with(dir->d_name, filter_by_extension))) {
				cvec_str_push_back(folders, cstr_from_fmt("%s%s", prefix, dir->d_name));
			}
			else if(dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
				if(recursive) {
					cstr inner_folder = cstr_from_fmt("%s/%s", folder, dir->d_name);
					cstr inner_prefix = cstr_from_fmt("%s%s/", prefix, dir->d_name);

					count += internal_platform_list_files_within_folder(folders, cstr_str(&inner_folder), recursive, filter_by_extension, cstr_str(&inner_prefix));

					cstr_drop(&inner_folder);
					cstr_drop(&inner_prefix);
				}
			}
		}

		closedir(directory_stream);
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
