#include "b2g_platform.h"
#include "b2g_types.h"
#include "b2g_builder.h"

int main()
{
	static const COLOR transparent_color = RGB15(0, 0, 0);
	static const unsigned int brute_force_shuffle_count = 25000;

	unsigned int file_names_count = 0;
	const char** file_names = platform_list_files_within_folder(".", &file_names_count, false, ".bmp");

	BackgroundData* bg_data = builder_create_background_data_from_image_paths(file_names, file_names_count, transparent_color, brute_force_shuffle_count);
	c_free(file_names);

	builder_print_background_data_c_file_to_stdout(bg_data);

	builder_free_background_data(bg_data);

	return 0;
}
