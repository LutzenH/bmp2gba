#include <stb/stb_image.h>
#include <stdio.h>
#include <stdint.h>

typedef uint16_t COLOR;

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} RGB8;

typedef struct {
	COLOR color[8 * 8];
} ColoredTile;

typedef struct {
	uint32_t id;
} MapTileIndex;

#define i_key COLOR
#define i_tag color
#include <stc/cset.h>

typedef struct {
	uint32_t tile_id;
	cset_color unique_colors;
} TileColorSet;

#define RGB15(red, green, blue) ((red) + ((green) << 5) + ((blue) << 10))

#define i_key ColoredTile
#define i_val uint32_t
#define i_hash(ct) (ct->color[0] ^ ct->color[1] ^ ct->color[2] ^ ct->color[3] ^ ct->color[4] ^ ct->color[5] ^ ct->color[6] ^ ct->color[7] ^ ct->color[8] ^ ct->color[9] ^ ct->color[10] ^ ct->color[11] ^ ct->color[12] ^ ct->color[13] ^ ct->color[14] ^ ct->color[15] ^ ct->color[16] ^ ct->color[17] ^ ct->color[18] ^ ct->color[19] ^ ct->color[20] ^ ct->color[21] ^ ct->color[22] ^ ct->color[23] ^ ct->color[24] ^ ct->color[25] ^ ct->color[26] ^ ct->color[27] ^ ct->color[28] ^ ct->color[29] ^ ct->color[30] ^ ct->color[31] ^ ct->color[32] ^ ct->color[33] ^ ct->color[34] ^ ct->color[35] ^ ct->color[36] ^ ct->color[37] ^ ct->color[38] ^ ct->color[39] ^ ct->color[40] ^ ct->color[41] ^ ct->color[42] ^ ct->color[43] ^ ct->color[44] ^ ct->color[45] ^ ct->color[46] ^ ct->color[47] ^ ct->color[48] ^ ct->color[49] ^ ct->color[50] ^ ct->color[51] ^ ct->color[52] ^ ct->color[53] ^ ct->color[54] ^ ct->color[55] ^ ct->color[56] ^ ct->color[57] ^ ct->color[58] ^ ct->color[59] ^ ct->color[60] ^ ct->color[61] ^ ct->color[62] ^ ct->color[63])
#define i_eq c_memcmp_eq
#define i_tag ct
#include <stc/cmap.h>

typedef struct {
	int overlap_count;
	cset_color* bank;
} PaletteOverlap;

int sort_tile_color_set(const void* a, const void* b) {
	TileColorSet* set_a = (TileColorSet*)a;
	TileColorSet* set_b = (TileColorSet*)b;
	return (int)(set_a->unique_colors.size) - (int)(set_b->unique_colors.size);
}

int sort_palette_overlap(const void* a, const void* b) {
	PaletteOverlap* po_a = (PaletteOverlap*)a;
	PaletteOverlap* po_b = (PaletteOverlap*)b;
	return (int)(po_a->overlap_count) - (int)(po_b->overlap_count);
}

int color_set_overlap(const cset_color* containing_set, const cset_color* contained_set) {
	int count = 0;

	c_foreach(i, cset_color, *contained_set) {
		if (cset_color_contains(containing_set, *i.ref)) {
			count++;
		}
	}

	return count;
}

int main()
{
	static const COLOR transparent_color = RGB15(0, 0, 0);

	const char* file_names[] = {
		"data/img/central_room_bg0.bmp",
		"data/img/four_rooms_bg0.bmp",
		"data/img/four_rooms_bg1.bmp",
		"data/img/lobby_bg0.bmp",
		"data/img/lobby_bg1.bmp",
		"data/img/small_opening_bg0.bmp",
		"data/img/small_opening_bg1.bmp",
		"data/img/small_plus_bg0.bmp",
		"data/img/square_medium_bg0.bmp",
		"data/img/square_medium_bg1.bmp",
		"data/img/thin_hallways_bg0.bmp",
		"data/img/thin_hallways_bg1.bmp",
		"data/img/tight_gap_bg0.bmp",
		"data/img/tight_gap_bg1.bmp",
		"data/img/vertical_side_ways_bg0.bmp",
		"data/img/vertical_side_ways_bg1.bmp",
	};
	size_t file_names_count = sizeof(file_names) / sizeof(file_names[0]);

	cmap_ct tiles = cmap_ct_init();
	MapTileIndex** maps = c_calloc(sizeof(MapTileIndex*), file_names_count);

	// Fill tiles and maps.
	{
		uint32_t total_unique_tiles_count = 0;
		for (int i = 0; i < file_names_count; ++i) {
			const char* file_name = file_names[i];

			int width = 0, height = 0, channels_in_file = 0;

			RGB8* img = (RGB8*)stbi_load(file_name, &width, &height, &channels_in_file, 3);
			if (img == NULL) {
				fprintf(stderr, "Failed to load image file: %s\n", file_name);
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

			maps[i] = c_calloc(sizeof(MapTileIndex), map_width * map_height);
			MapTileIndex* map = maps[i];

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

					map[tile_y * map_width + tile_x] = (MapTileIndex) {
						.id = current_tile_index,
					};
				}
			}

			stbi_image_free(img);
		}
	}

	// Create sorted list of color sets (based on the amount of unique colors on a tile).
	TileColorSet* tile_color_sets = c_calloc(tiles.size, sizeof(TileColorSet));
	{
		int idx = 0;
		c_foreach(i, cmap_ct, tiles) {
			cset_color tile_colors = cset_color_with_capacity(16);
			for (int j = 0; j < sizeof(i.ref->first.color) / sizeof(i.ref->first.color[0]); ++j) {
				cset_color_insert(&tile_colors, i.ref->first.color[j]);
			}
			cset_color_shrink_to_fit(&tile_colors);

			tile_color_sets[idx] = (TileColorSet) {
				.tile_id = idx,
				.unique_colors = tile_colors,
			};
			idx++;
		}
		qsort(tile_color_sets, tiles.size, sizeof(TileColorSet), sort_tile_color_set);
	}

	static const int palette_bank_count = 16;
	cset_color palette_banks[palette_bank_count];
	for (int i = 0; i < sizeof(palette_banks) / sizeof(palette_banks[0]); ++i) {
		palette_banks[i] = cset_color_with_capacity(16);
		cset_color_insert(&palette_banks[i], transparent_color);
	}

	// TODO: Make tile palette reduction possible, by adjusting the palette banks so
	//  that colors of two similar tiles with different colors appear in the same indices in a bank.
	//  meaning that we can have less tiles which only have a different palette bank.

	// Try to fit all tile colors in as little palette banks as possible.
	{
		PaletteOverlap* sortable_palette_array = c_calloc(palette_bank_count, sizeof(PaletteOverlap));
		for (int i = (int)tiles.size - 1; i >= 0; --i) {
			cset_color* tile_colors = &tile_color_sets[i].unique_colors;

			for (int j = 0; j < palette_bank_count; ++j) {
				cset_color* bank = &palette_banks[j];
				int overlap_count = color_set_overlap(bank, tile_colors);

				sortable_palette_array[j] = (PaletteOverlap) {
					.bank = &palette_banks[j],
					.overlap_count = overlap_count,
				};
			}
			qsort(sortable_palette_array, palette_bank_count, sizeof(PaletteOverlap), sort_palette_overlap);

			for (int j = palette_bank_count - 1; j >= 0; --j) {
				if (sortable_palette_array[j].overlap_count == tile_colors->size) {
					// Full overlap palette needs no changing.
					break;
				}

				int bank_space_left = 16 - (int)sortable_palette_array[j].bank->size;
				if (bank_space_left <= 0) {
					// Bank is full...
					continue;
				}

				if ((bank_space_left - ((int)tile_colors->size - sortable_palette_array[j].overlap_count)) >= 0) {
					c_foreach(k, cset_color, *tile_colors) {
						cset_color_insert(sortable_palette_array[j].bank, *k.ref);
					}
					break;
				}
			}
		}
		c_free(sortable_palette_array);
	}

	// Calculate waste.
	cset_color color_dupes = cset_color_init();
	int total_color_count = 0;
	int total_duplicate_count = 0;
	int total_palette_banks_used = 0;
	for (int i = 0; i < 16; ++i) {
		if(palette_banks[i].size > 1) {
			total_palette_banks_used++;
		}

		c_foreach(j, cset_color, palette_banks[i]){
			if (*j.ref == transparent_color) {
				// Transparent color does not count as wasted space.
				continue;
			}

			total_color_count++;

			if (cset_color_contains(&color_dupes, *j.ref)) {
				total_duplicate_count++;
			} else {
				cset_color_insert(&color_dupes, *j.ref);
			}
		}
	}
	cset_color_drop(&color_dupes);

	printf("Percentage of colors in palette that are duplicates: %.2f%% (%i/%i)\n", (float)total_duplicate_count / (float)total_color_count * 100.0f, total_duplicate_count, total_color_count);
	printf("Total palette banks used: %i/%i\n", total_palette_banks_used, palette_bank_count);

	// Cleanup
	{
		for (int i = 0; i < sizeof(palette_banks) / sizeof(palette_banks[0]); ++i) {
			cset_color_drop(&palette_banks[i]);
		}

		for (int i = 0; i < tiles.size; ++i) {
			cset_color_drop(&tile_color_sets[i].unique_colors);
		}
		c_free(tile_color_sets);

		cmap_ct_drop(&tiles);

		for (int i = 0; i < file_names_count; ++i) {
			if (maps[i] != NULL) {
				c_free(maps[i]);
			}
		}
		c_free(maps);
	}

	return 0;
}
