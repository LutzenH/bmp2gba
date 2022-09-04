#ifndef B2G_BUILDER_H
#define B2G_BUILDER_H

#include "b2g_types.h"

#define PALETTE_BANK_COUNT 16
#define PALETTE_BANK_SIZE 16

typedef struct {
	COLOR transparent_color;
	size_t tile_color_sets_reduction_count; // The amount of unique color sets in all backgrounds.

	size_t palette_banks_used;
	COLOR palette_banks[PALETTE_BANK_COUNT][PALETTE_BANK_SIZE];

	size_t tile_count;
	Tile* tiles;

	size_t map_count;
	Map** maps;
} BackgroundData;

BackgroundData* builder_create_background_data_from_image_paths(const char** paths, size_t paths_count, COLOR transparent_color, unsigned int brute_force_shuffle_count);
void builder_free_background_data(BackgroundData* background_data);

void builder_print_background_data_c_file_to_stdout(const BackgroundData* background_data);

#endif //B2G_BUILDER_H
