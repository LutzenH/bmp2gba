#include "b2g_builder.h"
#include "b2g_platform.h"

#include <stdbool.h>
#include <stdio.h>
#include <stb_image.c>

BackgroundData* builder_create_background_data_from_image_paths(const char** paths, size_t paths_count, COLOR transparent_color, unsigned int brute_force_shuffle_count)
{
	BackgroundData* background_data = c_calloc(1, sizeof(BackgroundData));
	background_data->transparent_color = transparent_color;

	// Fill tiles and maps.
	cmap_ct tiles = cmap_ct_init();
	{
		background_data->maps = c_calloc(sizeof(Map*), paths_count);

		size_t map_count = 0;

		uint32_t total_unique_tiles_count = 0;
		for (int i = 0; i < paths_count; ++i) {
			const char* file_name = paths[i];

			int width = 0, height = 0, channels_in_file = 0;

			unsigned int file_size = 0;
			unsigned char* file = platform_load_file_in_memory(file_name, &file_size);
			if (file == NULL) {
				fprintf(stderr, "Failed to load image file: %s\n", file_name);
				continue;
			}

			RGB8* img = (RGB8*)stbi_load_from_memory(file, (int)file_size, &width, &height, &channels_in_file, 3);
			c_free(file);
			if (img == NULL) {
				fprintf(stderr, "Failed to process image file: %s\n", file_name);
				continue;
			}

			if (channels_in_file != 3) {
				fprintf(stderr, "Image file (%s) has invalid channel count: required=%i has=%i\n", file_name, 3, channels_in_file);
				stbi_image_free(img);
				continue;
			}

			if ((width % 8) != 0) {
				fprintf(stderr, "Image file (%s) width needs to be divisible by 8\n", file_name);
				stbi_image_free(img);
				continue;
			}

			if ((height % 8) != 0) {
				fprintf(stderr, "Image file (%s) height needs to be divisible by 8\n", file_name);
				stbi_image_free(img);
				continue;
			}

			int map_width = width / 8;
			int map_height = height / 8;

			background_data->maps[map_count] = c_calloc(sizeof(Map) + (sizeof(MapTileIndex) * map_width * map_height), 1);
			Map* map = background_data->maps[map_count];
			map->map_width = map_width;
			map->map_height = map_height;

			const char* map_name = strrchr(file_name, '/');
			if (map_name != NULL) {
				map_name++;
			} else {
				map_name = file_name;
			}
			snprintf(map->map_name, sizeof(map->map_name), "%s", map_name);
			for (int j = 0; j < strlen(map->map_name); ++j) {
				if (map->map_name[j] == ' ' || map->map_name[j] == '/' || map->map_name[j] == '\\') {
					map->map_name[j] = '_';
				}
				if (map->map_name[j] == '.') {
					map->map_name[j] = '\0';
				}
			}

			MapTileIndex* map_tiles = map->tiles;

			for (int tile_y = 0; tile_y < map_height; ++tile_y) {
				for (int tile_x = 0; tile_x < map_width; ++tile_x) {
					ColoredTile tile = {};

					for (int y = 0; y < 8; ++y) {
						for (int x = 0; x < 8; ++x) {
							RGB8 pixel = img[(tile_y * 8 + y) * width + (tile_x * 8 + x)];
							COLOR color = RGB15(pixel.r / 8, pixel.g / 8, pixel.b / 8);

							tile.color[y * 8 + x] = color;
						}
					}

					uint32_t current_tile_index;
					if (cmap_ct_contains(&tiles, tile)) {
						current_tile_index = cmap_ct_get(&tiles, tile)->second;
					} else {
						current_tile_index = total_unique_tiles_count;
						cmap_ct_insert(&tiles, tile, total_unique_tiles_count++);
					}

					map_tiles[tile_y * map_width + tile_x] = current_tile_index & 0x03FF;
				}
			}

			stbi_image_free(img);

			map_count++;
		}

		background_data->map_count = map_count;

		if (background_data->map_count == 0) {
			fprintf(stderr, "Failed to find any .bmp images in this folder!\n");
			c_free(background_data);
			c_free(background_data->maps);
			return NULL;
		}
	}

	// Turn every tile into a set and sort based on the amount of unique colors in a tile.
	TileColorSet* tile_color_sets = c_calloc(tiles.size, sizeof(TileColorSet));
	{
		int idx = 0;
		c_foreach(i, cmap_ct, tiles)
		{
			tile_color_sets[idx].unique_colors = cset_color_init();
			for (int j = 0; j < sizeof(i.ref->first.color) / sizeof(i.ref->first.color[0]); ++j) {
				cset_color_insert(&tile_color_sets[idx].unique_colors, i.ref->first.color[j]);
			}

			tile_color_sets[idx].tile_idx = idx;
			tile_color_sets[idx].bank_idx = 0;

			idx++;
		}
		qsort(tile_color_sets, tiles.size, sizeof(TileColorSet), sort_tile_color_set_using_unique_color_count);
	}

	// Reduce the amount of sets by removing overlapping sets.
	size_t bank_color_sets_used = 0;
	BankColorSet* bank_color_sets = c_calloc(tiles.size, sizeof(BankColorSet));
	cset_color** tile_idx_to_color_set = c_calloc(tiles.size, sizeof(cset_color*));
	for (int i = (int)tiles.size - 1; i >= 0; --i) {
		size_t tile_idx = tile_color_sets[i].tile_idx;
		cset_color* tile_colors = &tile_color_sets[i].unique_colors;

		bool found_color_set_containing_color_set = false;
		for (int set_idx = 0; set_idx < bank_color_sets_used; ++set_idx) {
			if (color_set_overlap(bank_color_sets[set_idx].colors, tile_colors) == tile_colors->size) {
				tile_idx_to_color_set[tile_idx] = bank_color_sets[set_idx].colors;
				found_color_set_containing_color_set = true;
			}
		}

		if (!found_color_set_containing_color_set) {
			size_t set_idx = bank_color_sets_used;

			cset_color* bank_color_set = c_calloc(1, sizeof(cset_color));
			*(bank_color_set) = cset_color_init();

			c_foreach(color, cset_color, *tile_colors)
			{
				cset_color_insert(bank_color_set, *color.ref);
			}
			tile_idx_to_color_set[tile_idx] = bank_color_set;

			bank_color_sets[set_idx].colors = bank_color_set;
			bank_color_sets[set_idx].bank_idx = 0;

			bank_color_sets_used++;
		}
	}
	background_data->tile_color_sets_reduction_count = bank_color_sets_used;

	// TODO: Add tile reduction using H and V flipping.

	// TODO: Make tile palette reduction possible, by adjusting the palette banks so
	//  that colors of two similar tiles with different colors appear in the same indices in a bank.
	//  meaning that we can have less tiles which only have a different palette bank.

	unsigned int best_found_palette_bank_color_sum = UINT_MAX;
	cset_color best_found_palette_banks[PALETTE_BANK_COUNT];
	for (int i = 0; i < sizeof(best_found_palette_banks) / sizeof(best_found_palette_banks[0]); ++i) {
		best_found_palette_banks[i] = cset_color_with_capacity(PALETTE_BANK_SIZE);
	}
	BankColorSet* best_found_palette_bank_color_sets = c_calloc(bank_color_sets_used, sizeof(BankColorSet));

	cset_color tmp_palette_banks[PALETTE_BANK_COUNT];
	for (int i = 0; i < sizeof(tmp_palette_banks) / sizeof(tmp_palette_banks[0]); ++i) {
		tmp_palette_banks[i] = cset_color_with_capacity(PALETTE_BANK_SIZE);
	}

	for (int shuffle = 0; shuffle < brute_force_shuffle_count; ++shuffle) {
		for (int i = 0; i < sizeof(tmp_palette_banks) / sizeof(tmp_palette_banks[0]); ++i) {
			cset_color_clear(&tmp_palette_banks[i]);
			cset_color_insert(&tmp_palette_banks[i], transparent_color);
		}

		size_t used_sets_count = 0;
		for (int color_set_idx = 0; color_set_idx < bank_color_sets_used; ++color_set_idx) {
			BankColorSet* bank_color_set = &bank_color_sets[color_set_idx];
			cset_color* set = bank_color_set->colors;

			int bank_idx;
			for (bank_idx = 0; bank_idx < PALETTE_BANK_COUNT; ++bank_idx) {
				cset_color* bank = &tmp_palette_banks[bank_idx];

				int overlap_count = color_set_overlap(bank, set);
				if (overlap_count == set->size) {
					used_sets_count++;
					break;
				}

				int bank_space_left = PALETTE_BANK_SIZE - (int)bank->size;
				if (bank_space_left <= 0) {
					// Bank is full...
					continue;
				}

				if ((bank_space_left - ((int)set->size - overlap_count)) >= 0) {
					c_foreach(k, cset_color, *set)
					{
						cset_color_insert(bank, *k.ref);
					}
					used_sets_count++;
					break;
				}
			}

			bank_color_set->bank_idx = bank_idx;
		}

		unsigned int total_color_sum;
		if (used_sets_count < bank_color_sets_used) {
			total_color_sum = UINT_MAX; // If we fail to place all color sets we have not found a better palette.
		} else {
			total_color_sum = 0;
			for (int i = 0; i < sizeof(tmp_palette_banks) / sizeof(tmp_palette_banks[0]); ++i) {
				if (tmp_palette_banks[i].size > 1) {
					total_color_sum += tmp_palette_banks[i].size;
				}
			}
		}

		if (total_color_sum < best_found_palette_bank_color_sum) {
			fprintf(stderr, "Found smaller palette: %u -> %u\n", best_found_palette_bank_color_sum, total_color_sum);
			best_found_palette_bank_color_sum = total_color_sum;

			for (int i = 0; i < sizeof(tmp_palette_banks) / sizeof(tmp_palette_banks[0]); ++i) {
				cset_color_clear(&best_found_palette_banks[i]);
				c_foreach(color, cset_color, tmp_palette_banks[i])
				{
					cset_color_insert(&best_found_palette_banks[i], *color.ref);
				}
			}

			memcpy(best_found_palette_bank_color_sets, bank_color_sets, bank_color_sets_used * sizeof(BankColorSet));
		}

		array_shuffle(bank_color_sets, bank_color_sets_used, sizeof(BankColorSet));
	}

	for (int i = 0; i < sizeof(tmp_palette_banks) / sizeof(tmp_palette_banks[0]); ++i) {
		cset_color_drop(&tmp_palette_banks[i]);
	}

	// Map tile to palette bank
	qsort(tile_color_sets, tiles.size, sizeof(TileColorSet), sort_tile_color_set_using_tile_idx);
	c_foreach(t, cmap_ct, tiles)
	{
		uint32_t tile_idx = t.ref->second;
		cset_color* tile_color_set = tile_idx_to_color_set[tile_idx];

		for (int cs_idx = 0; cs_idx < bank_color_sets_used; ++cs_idx) {
			if (tile_color_set == best_found_palette_bank_color_sets[cs_idx].colors) {
				tile_color_sets[tile_idx].bank_idx = best_found_palette_bank_color_sets[cs_idx].bank_idx;
				break;
			}
		}
	}

	// Fill final palette array
	background_data->palette_banks_used = 0;
	for (int bank_idx = 0; bank_idx < PALETTE_BANK_COUNT; ++bank_idx) {
		background_data->palette_banks[bank_idx][0] = transparent_color;
		int color_idx = 1;

		c_foreach(j, cset_color, best_found_palette_banks[bank_idx])
		{
			uint16_t value = *j.ref;
			if (value != transparent_color) {
				background_data->palette_banks[bank_idx][color_idx] = value;
				color_idx++;
			}
		}

		if (best_found_palette_banks[bank_idx].size > 1) {
			background_data->palette_banks_used++;
		}
	}

	// Fill tiles array.
	{
		background_data->tiles = calloc(tiles.size, sizeof(Tile));
		background_data->tile_count = tiles.size;

		c_foreach(tile, cmap_ct, tiles)
		{
			size_t tile_idx = tile.ref->second;
			ColoredTile ct = tile.ref->first;
			size_t pb_idx = tile_color_sets[tile_idx].bank_idx;

			for (int j = 0; j < 32; ++j) {
				COLOR l = ct.color[j * 2 + 0];
				COLOR r = ct.color[j * 2 + 1];

				int l_cpal_idx = -1;
				int r_cpal_idx = -1;
				for (int k = 0; k < PALETTE_BANK_SIZE; ++k) {
					if (l_cpal_idx == -1 && l == background_data->palette_banks[pb_idx][k]) {
						l_cpal_idx = k;
					}

					if (r_cpal_idx == -1 && r == background_data->palette_banks[pb_idx][k]) {
						r_cpal_idx = k;
					}
				}

				background_data->tiles[tile_idx].color_idxs[j] = (l_cpal_idx | r_cpal_idx << 4) & 0xFF;
			}
		}
	}

	// Update Map Arrays
	for (int idx = 0; idx < background_data->map_count; ++idx) {
		Map* map = background_data->maps[idx];
		int height = map->map_height;
		int width = map->map_width;

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				int tile_idx = map->tiles[y * width + x];

				if (tile_idx != tile_color_sets[tile_idx].tile_idx) {
					fprintf(stderr, "FATAL ERROR: tile id's do not match! file: %s line: %i\n", __FILE__, __LINE__);
					abort();
				}

				uint16_t palette_bits = tile_color_sets[tile_idx].bank_idx << 12;
				map->tiles[y * width + x] = palette_bits | tile_idx;
			}
		}
	}

	// Cleanup
	{
		c_free(best_found_palette_bank_color_sets);
		c_free(tile_idx_to_color_set);

		for (int i = 0; i < bank_color_sets_used; ++i) {
			cset_color_drop(bank_color_sets[i].colors);
			c_free(bank_color_sets[i].colors);
		}
		c_free(bank_color_sets);

		for (int i = 0; i < tiles.size; ++i) {
			cset_color_drop(&tile_color_sets[i].unique_colors);
		}
		c_free(tile_color_sets);

		for (int i = 0; i < sizeof(best_found_palette_banks) / sizeof(best_found_palette_banks[0]); ++i) {
			cset_color_drop(&best_found_palette_banks[i]);
		}

		cmap_ct_drop(&tiles);
	}

	return background_data;
}

void builder_free_background_data(BackgroundData* background_data)
{
	c_free(background_data->tiles);

	for (int map_idx = 0; map_idx < background_data->map_count; ++map_idx) {
		c_free(background_data->maps[map_idx]);
	}
	c_free(background_data->maps);

	c_free(background_data);
}

static inline void builder_internal_calculate_and_print_statistics(const BackgroundData* data)
{
	cset_color color_dupes = cset_color_init();
	int total_color_count = 0;
	int total_duplicate_count = 0;
	for (int bank_idx = 0; bank_idx < data->palette_banks_used; ++bank_idx) {
		for (int color_idx = 0; color_idx < PALETTE_BANK_SIZE; ++color_idx) {
			COLOR value = data->palette_banks[bank_idx][color_idx];

			if (value == data->transparent_color) {
				// Transparent color does not count as wasted space.
				continue;
			}

			total_color_count++;

			if (cset_color_contains(&color_dupes, value)) {
				total_duplicate_count++;
			} else {
				cset_color_insert(&color_dupes, value);
			}
		}
	}

	cset_color_drop(&color_dupes);

	printf("// ------------------------ statistics ------------------------\n");
	printf("//   Sets of color versus unique tiles: %zu / %zu = %.4f\n", data->tile_color_sets_reduction_count, data->tile_count, (float)data->tile_color_sets_reduction_count / (float)data->tile_count);
	printf("//   Colors in palette that are duplicates: %i/%i (%.2f%%)\n", total_duplicate_count, total_color_count, (float)total_duplicate_count / (float)total_color_count * 100.0f);
	printf("//   Total palette banks used: %zu/%d (%.2f%%)\n", data->palette_banks_used, PALETTE_BANK_COUNT, (float)data->palette_banks_used / (float)PALETTE_BANK_COUNT * 100.0f);
	printf("//   Tiles used: %zu/%i (%.2f%%)\n", data->tile_count, 1024, (float)data->tile_count / 1024.0f * 100.0f);
	printf("// ------------------------------------------------------------\n");
}

void builder_internal_print_palette_data(const BackgroundData* background_data)
{
	printf("const unsigned short bg_data_palette[] __attribute__((aligned(4))) __attribute__((visibility(\"hidden\"))) = {\n");

	for (int bank_idx = 0; bank_idx < background_data->palette_banks_used; ++bank_idx) {
		printf("\t");
		int idx = 0;

		for (int color_idx = 0; color_idx < PALETTE_BANK_SIZE; ++color_idx) {
			printf("0x%04X, ", background_data->palette_banks[bank_idx][color_idx]);
		}

		printf("\n");
	}

	printf("};\n");
}

void builder_internal_print_tile_data(const BackgroundData* background_data)
{
	printf("const unsigned char bg_data_tiles[] __attribute__((aligned(4))) __attribute__((visibility(\"hidden\"))) = {\n");

	for (int tile_idx = 0; tile_idx < background_data->tile_count; ++tile_idx) {
		printf("\t");
		for (int cidx = 0; cidx < sizeof(background_data->tiles[tile_idx].color_idxs) / sizeof(background_data->tiles[tile_idx].color_idxs[0]); ++cidx) {
			printf("0x%.02X, ", background_data->tiles[tile_idx].color_idxs[cidx]);
		}
		printf("\n");
	}

	printf("};\n");
}

void builder_internal_print_map_data(const Map* map)
{
	printf("const unsigned short %s_map[%i] __attribute__((aligned(4))) __attribute__((visibility(\"hidden\"))) = { \n", map->map_name, map->map_width * map->map_height);

	for (int y = 0; y < map->map_height; ++y) {
		printf("\t");

		for (int x = 0; x < map->map_width; ++x) {
			int tile_id = map->tiles[y * map->map_width + x];
			printf("0x%.04X, ", map->tiles[y * map->map_width + x]);
		}

		printf("\n");
	}

	printf("};\n");
}

void builder_print_background_data_c_file_to_stdout(const BackgroundData* background_data)
{
	builder_internal_calculate_and_print_statistics(background_data);
	printf("\n");
	builder_internal_print_palette_data(background_data);
	printf("\n");
	builder_internal_print_tile_data(background_data);
	for (int map_idx = 0; map_idx < background_data->map_count; ++map_idx) {
		printf("\n");
		builder_internal_print_map_data(background_data->maps[map_idx]);
	}

	fflush(stdout);
}
