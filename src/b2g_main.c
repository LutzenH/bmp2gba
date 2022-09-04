#include "b2g_builder.h"
#include "b2g_platform.h"
#include "b2g_types.h"
#include "b2g_version.h"

#include <stc/coption.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
	static const char* str_usage = "Usage: bmp2gba --transparent-color FF00FF --brute-force-count 250000 > bg_data.c";

	coption_long long_opts[] = {
		{ "transparent-color", coption_required_argument, 't' },
		{ "brute-force-count", coption_required_argument, 'b' },
		{ "help", coption_no_argument, 'h' },
		{ "version", coption_no_argument, 'v' },
		{ 0 }
	};

	unsigned int opt_transparent_color = 0xFF00FF;
	unsigned int opt_brute_force_shuffle_count = 250000;

	coption opt = coption_init();

	int c;
	while ((c = coption_get(&opt, argc, argv, "t:b:vh", long_opts)) != -1) {
		switch (c) {
			case '?': {
				printf("error: unknown option: %s\n", opt.optstr);
			} break;
			case ':': {
				printf("error: missing argument for %s\n", opt.optstr);
			} break;
			case 't': {
				if (opt.arg) {
					int count = sscanf(opt.arg, "%x", &opt_transparent_color);
					if (count == 0) {
						printf("error: invalid argument '%s' for %s\tvalid format: %s FF00FF\n", opt.arg, opt.optstr, opt.optstr);
						return 1;
					}
				} else {
					printf("error: missing argument for %s\n", opt.optstr);
					return 1;
				}
			} break;
			case 'b': {
				if (opt.arg) {
					int count = sscanf(opt.arg, "%u", &opt_brute_force_shuffle_count);
					if (count == 0) {
						printf("error: invalid argument '%s' for %s\tvalid format: %s 250000\n", opt.arg, opt.optstr, opt.optstr);
						return 1;
					}
				} else {
					printf("error: missing argument for %s\n", opt.optstr);
					return 1;
				}
			} break;
			case 'h': {
				printf("%s\n", str_usage);
				return 0;
			} break;
			case 'v': {
				printf("bmp2gba v%d.%d\n", B2G_VERSION_MAJOR, B2G_VERSION_MINOR);
				return 0;
			} break;
			default: {
			} break;
		}
	}

	unsigned int r = (opt_transparent_color & 0xFF) / 8;
	unsigned int g = ((opt_transparent_color >> 8) & 0xFF) / 8;
	unsigned int b = ((opt_transparent_color >> 16) & 0xFF) / 8;
	COLOR transparent_color = RGB15(r, g, b);

	unsigned int file_names_count = 0;
	const char** file_names = platform_list_files_within_folder(".", &file_names_count, false, ".bmp");

	BackgroundData* bg_data = builder_create_background_data_from_image_paths(file_names, file_names_count, transparent_color, opt_brute_force_shuffle_count);
	c_free(file_names);

	builder_print_background_data_c_file_to_stdout(bg_data);

	builder_free_background_data(bg_data);

	return 0;
}
