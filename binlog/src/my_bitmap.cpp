#include "my_bitmap.h"
void create_last_word_mask(MY_BITMAP *map) {
  unsigned int const used= 1U + ((map->n_bits-1U) & 0x7U);
  unsigned char const mask= (~((1 << used) - 1)) & 255;

  unsigned char *ptr= (unsigned char*)&map->last_word_mask;

  map->last_word_ptr= map->n_bits == 0 ? map->bitmap :
    map->bitmap + no_words_in_map(map) - 1;

  switch (no_bytes_in_map(map) & 3) {
  case 1:
    map->last_word_mask= ~0U;
    ptr[0]= mask;
    return;
  case 2:
    map->last_word_mask= ~0U;
    ptr[0]= 0;
    ptr[1]= mask;
    return;
  case 3:
    map->last_word_mask= 0U;
    ptr[2]= mask;
    ptr[3]= 0xFFU;
    return;
  case 0:
    map->last_word_mask= 0U;
    ptr[3]= mask;
    return;
  }
}

uint bitmap_bits_set(const MY_BITMAP *map)
{
	my_bitmap_map *data_ptr= map->bitmap;
	my_bitmap_map *end= map->last_word_ptr;
	uint res= 0;

	for (; data_ptr < end; data_ptr++)
		res+= my_count_bits_uint32(*data_ptr);

	/*Reset last bits to zero*/
	res+= my_count_bits_uint32(*map->last_word_ptr & ~map->last_word_mask);
	return res;
}


my_bool bitmap_init(MY_BITMAP *map, my_bitmap_map *buf, uint n_bits)
{
	int size_in_bytes= bitmap_buffer_size(n_bits);
	buf = (my_bitmap_map*) malloc(size_in_bytes);
	map->bitmap = buf;
	map->n_bits= n_bits;
	create_last_word_mask(map);
	bitmap_clear_all(map);
	return 0;
}

void bitmap_free(MY_BITMAP *map)
{
	if (map->bitmap) {
		free(map->bitmap);
		map->bitmap=0;
	}
	return;
}
