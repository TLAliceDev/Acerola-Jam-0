#include "font.h"
#include "stdint.h"
#include "gfx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct fnt_header_s {
  uint16_t size;
  _Bool    bold;
  _Bool    italic;
  _Bool    unicode;
  uint8_t  stretch_h;
  _Bool    smooth;
  _Bool    aa;
  int16_t  padding[4];
  int16_t  spacing[2];
  int16_t  line_height;
  int16_t  base;
  int16_t  scale_w;
  int16_t  scale_h;
  uint8_t  pages;
  _Bool    packed;
} fnt_header_t;

typedef struct fnt_char_s {
  uint16_t x, y, w, h;
  int      xoff, yoff, xadv;
  uint8_t  page;
} fnt_char_t;

typedef struct font_s {
  fnt_header_t   header;
  gfx_texture_t* pages[8];
  int            char_count;
  fnt_char_t     chars[256];
  int8_t         kernings[256][256];
} font_internal_t;


static gfx_texture_t*
read_page(char* str, uint8_t* id) {
  char filepath[512];
  sscanf(str, "page id=%hhu file=\"%[^\"]\"\n", id, filepath);
  return gfx_texture_create_and_load(filepath);
}

static int8_t
read_kerning(char* str, uint8_t* first, uint8_t* second) {
  int8_t kerning;
  sscanf(str, "kerning first=%hhu second=%hhu amount=%hhd", first, second, &kerning);
  return kerning;
}

static fnt_char_t
read_char(char* str, uint8_t* id) {
  uint16_t x, y, w, h;
  int xoff, yoff, xadv;
  uint8_t page;
  sscanf(str, "char id=%hhu x=%hu y=%hu width=%hu height=%hu xoffset=%d yoffset=%d xadvance=%d page=%hhu chnl=15\n", id, &x, &y, &w, &h, &xoff, &yoff, &xadv, &page);
  return (fnt_char_t){x, y, w, h, xoff, yoff, xadv, page};
}

static fnt_header_t
read_header(char* first, char* second) {
  char face[512];
  int size;
  int bold;
  int italic;
  int unicode;
  int stretch_h;
  int smooth;
  int aa;
  int padding[4];
  int spacing[2];
  int line_height;
  int base;
  int scale_w;
  int scale_h;
  int pages;
  int packed;
  sscanf(first, "info face=\"%[^\"]\" size=%d bold=%d italic=%d charset=\"\" unicode=%d stretch_h=%d smooth=%d aa=%d padding=%d,%d,%d,%d spacing=%d,%d\n", face, &size, &bold, &italic, &unicode, &stretch_h, &smooth, &aa, &padding[0], &padding[1], &padding[2], &padding[3], &spacing[0], &spacing[1]);
  sscanf(second, "common lineHeight=%d base=%d scaleW=%d scaleH=%d pages=%d packed=%d\n", &line_height, &base, &scale_w, &scale_h, &pages, &packed);
  return (fnt_header_t) {
    .size = size,
    .bold = bold,
    .italic = italic,
    .unicode = unicode,
    .stretch_h = stretch_h,
    .smooth = smooth,
    .aa = aa,
    .padding = {padding[0], padding[1], padding[2], padding[3]},
    .spacing = {spacing[0], spacing[1]},
    .line_height = line_height,
    .base = base,
    .scale_w = scale_w,
    .scale_h = scale_h,
    .pages = pages,
    .packed  = packed 
  };
}

font_t*
font_load(const char* filepath) {
  char buffer1[4096];
  char buffer2[4096];
  FILE* file;
  font_internal_t* fnt = malloc(sizeof(*fnt));
  int count;
  uint16_t i, j;

  file = fopen(filepath, "r");
  fgets(buffer1, 4096, file);
  fgets(buffer2, 4096, file);
  fnt->header = read_header(buffer1, buffer2);
  for (i = 0; i < 256; i++)
    for (j = 0; j < 256; j++)
      fnt->kernings[i][j] = 0;

  for (i = 0; i < fnt->header.pages; i++) {
    uint8_t id;
    gfx_texture_t* tex;
    fgets(buffer1, 4096, file);
    tex = read_page(buffer1, &id);
    fnt->pages[id] = tex;
  }
  
  fscanf(file, "chars count=%d\n", &count);
  for (i = 0; i < count; i++) {
    fnt_char_t f_char;
    uint8_t id;
    fgets(buffer1, 4096, file);
    f_char = read_char(buffer1, &id);
    fnt->chars[id] = f_char;
  }
  if (!fscanf(file, "kernings count=%d\n", &count)) {
    return fnt;
  }
  for (i = 0; i < count; i++) {
    uint8_t first, second;
    int8_t kerning;
    fgets(buffer1, 4096, file);
    kerning = read_kerning(buffer1, &first, &second);
    fnt->kernings[first][second] = kerning;
  }
  
  return fnt;
}


void
char_draw(gfx_texture_t* tex, fnt_char_t* fchar, float x, float y) {
  float w = fchar->w;
  float h = fchar->h;
  gfx_rect_t dst = {x, y, w, h};
  gfx_rect_t src = {fchar->x, fchar->y, fchar->w, fchar->h};
  gfx_texture_draw(tex, &src, &dst);
}


void
text_draw(const char* text, font_t* font_ptr, float x, float y, float size) {
  text_draw_p(text, font_ptr, x, y, size, 1.0);
}

void
text_draw_p(const char* text, font_t* font_ptr, float x, float y, float size, float percentage) {
  percentage = percentage > 1.0 ? 1.0 : percentage;
  font_internal_t* font = font_ptr;
  uint8_t ind = 0;
  unsigned char curr = text[0];
  unsigned char prev = '0';
  float cx = x, cy = y;
  int length = strlen(text) * percentage;
  if (size < 0)
    size = font->header.size;
  for (ind = 0; ind < length; ind++) {
    fnt_char_t* fchar;
    float kerning;
    float lx;
    float ly;
    curr = text[ind];
    if (curr == '\n' || curr == '\t' || curr == '^') {
      cy += font->header.line_height ;
      cx = x;
      continue;
    }
    fchar = &font->chars[curr];
    kerning = font->kernings[prev][curr];
    lx = cx + (fchar->xoff + kerning);
    ly = cy + fchar->yoff;
    char_draw(font->pages[fchar->page], fchar, lx, ly);
    cx += fchar->xadv;
  }
}
