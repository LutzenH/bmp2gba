#include "b2g_platform.h"

#include <string.h>

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

#if defined(B2G_WIN32)
#include "b2g_platform_win32.c"
#elif defined(B2G_LINUX)
#include "b2g_platform_posix.c"
#else
#error The platform you are compiling for is currently not implemented!
#endif
