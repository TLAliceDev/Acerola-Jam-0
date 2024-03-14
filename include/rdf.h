#ifndef RDF_H
#define RDF_H
#include <stdint.h>

typedef struct rdf_result_s rdf_result_t;

struct rdf_result_s {
  char*    game_name;
  void**   rooms_pointer;
  void*    inv_item_spritesheet;
  void*    background_texture;
  uint8_t  rooms_n;
  void*    font;
  uint32_t clear_colour;
};

rdf_result_t rdf_load(const char* file);

#endif
