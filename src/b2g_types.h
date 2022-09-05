#ifndef B2G_TYPES_H
#define B2G_TYPES_H

#include <stdint.h>
#include <time.h>

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
	uint8_t color_idxs[32]; // 4bpp
} Tile;

typedef uint16_t MapTileIndex;

typedef struct {
	char name[128];

	int width;
	int height;

	MapTileIndex tiles[];
} Map;

#define i_key COLOR
#define i_tag color
#include <stc/cset.h>

typedef struct {
	size_t tile_idx;
	cset_color unique_colors;
	size_t bank_idx;
} TileColorSet;

typedef struct {
	size_t bank_idx;
	cset_color* colors;
} BankColorSet;

#define RGB15(red, green, blue) ((red) + ((green) << 5) + ((blue) << 10))

#define i_key ColoredTile
#define i_val uint32_t
#define i_hash(ct) (ct->color[0] ^ ct->color[1] ^ ct->color[2] ^ ct->color[3] ^ ct->color[4] ^ ct->color[5] ^ ct->color[6] ^ ct->color[7] ^ ct->color[8] ^ ct->color[9] ^ ct->color[10] ^ ct->color[11] ^ ct->color[12] ^ ct->color[13] ^ ct->color[14] ^ ct->color[15] ^ ct->color[16] ^ ct->color[17] ^ ct->color[18] ^ ct->color[19] ^ ct->color[20] ^ ct->color[21] ^ ct->color[22] ^ ct->color[23] ^ ct->color[24] ^ ct->color[25] ^ ct->color[26] ^ ct->color[27] ^ ct->color[28] ^ ct->color[29] ^ ct->color[30] ^ ct->color[31] ^ ct->color[32] ^ ct->color[33] ^ ct->color[34] ^ ct->color[35] ^ ct->color[36] ^ ct->color[37] ^ ct->color[38] ^ ct->color[39] ^ ct->color[40] ^ ct->color[41] ^ ct->color[42] ^ ct->color[43] ^ ct->color[44] ^ ct->color[45] ^ ct->color[46] ^ ct->color[47] ^ ct->color[48] ^ ct->color[49] ^ ct->color[50] ^ ct->color[51] ^ ct->color[52] ^ ct->color[53] ^ ct->color[54] ^ ct->color[55] ^ ct->color[56] ^ ct->color[57] ^ ct->color[58] ^ ct->color[59] ^ ct->color[60] ^ ct->color[61] ^ ct->color[62] ^ ct->color[63])
#define i_eq c_memcmp_eq
#define i_tag ct
#include <stc/cmap.h>

#define i_key ColoredTile
#define i_hash(ct) (ct->color[0] ^ ct->color[1] ^ ct->color[2] ^ ct->color[3] ^ ct->color[4] ^ ct->color[5] ^ ct->color[6] ^ ct->color[7] ^ ct->color[8] ^ ct->color[9] ^ ct->color[10] ^ ct->color[11] ^ ct->color[12] ^ ct->color[13] ^ ct->color[14] ^ ct->color[15] ^ ct->color[16] ^ ct->color[17] ^ ct->color[18] ^ ct->color[19] ^ ct->color[20] ^ ct->color[21] ^ ct->color[22] ^ ct->color[23] ^ ct->color[24] ^ ct->color[25] ^ ct->color[26] ^ ct->color[27] ^ ct->color[28] ^ ct->color[29] ^ ct->color[30] ^ ct->color[31] ^ ct->color[32] ^ ct->color[33] ^ ct->color[34] ^ ct->color[35] ^ ct->color[36] ^ ct->color[37] ^ ct->color[38] ^ ct->color[39] ^ ct->color[40] ^ ct->color[41] ^ ct->color[42] ^ ct->color[43] ^ ct->color[44] ^ ct->color[45] ^ ct->color[46] ^ ct->color[47] ^ ct->color[48] ^ ct->color[49] ^ ct->color[50] ^ ct->color[51] ^ ct->color[52] ^ ct->color[53] ^ ct->color[54] ^ ct->color[55] ^ ct->color[56] ^ ct->color[57] ^ ct->color[58] ^ ct->color[59] ^ ct->color[60] ^ ct->color[61] ^ ct->color[62] ^ ct->color[63])
#define i_eq c_memcmp_eq
#define i_tag ct
#include <stc/cset.h>

static int sort_tile_color_set_using_unique_color_count(const void* a, const void* b)
{
	TileColorSet* set_a = (TileColorSet*)a;
	TileColorSet* set_b = (TileColorSet*)b;
	return (int)(set_a->unique_colors.size) - (int)(set_b->unique_colors.size);
}

static int sort_tile_color_set_using_tile_idx(const void* a, const void* b)
{
	TileColorSet* set_a = (TileColorSet*)a;
	TileColorSet* set_b = (TileColorSet*)b;
	return (int)(set_a->tile_idx) - (int)(set_b->tile_idx);
}

static int color_set_overlap(const cset_color* containing_set, const cset_color* contained_set)
{
	int count = 0;

	c_foreach(i, cset_color, *contained_set)
	{
		if (cset_color_contains(containing_set, *i.ref)) {
			count++;
		}
	}

	return count;
}

static void array_shuffle(void* base, size_t count, size_t size_of_element)
{
	srand(time(NULL));

	unsigned char* uc_base = base;

	unsigned char tmp[size_of_element];
	if (count > 1) {
		for (size_t i = 0; i < count - 1; i++) {
			size_t j = i + rand() / (RAND_MAX / (count - i) + 1);

			memcpy(tmp, uc_base + (j * size_of_element), size_of_element);
			memcpy(uc_base + (j * size_of_element), uc_base + (i * size_of_element), size_of_element);
			memcpy(uc_base + (i * size_of_element), tmp, size_of_element);
		}
	}
}

#endif //B2G_TYPES_H
