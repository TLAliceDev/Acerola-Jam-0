#ifndef GFX_H
#define GFX_H

#include <stdint.h>

typedef void                gfx_texture_t;
typedef struct gfx_point_s  gfx_point_t;
typedef struct gfx_line_s   gfx_line_t;
typedef struct gfx_rect_s   gfx_rect_t;
typedef struct gfx_circ_s   gfx_circ_t;
typedef struct gfx_pointi_s gfx_pointi_t;
typedef struct gfx_linei_s  gfx_linei_t;
typedef struct gfx_recti_s  gfx_recti_t;
typedef struct gfx_circi_s  gfx_circi_t;
typedef struct gfx_color_s  gfx_color_t;

struct gfx_point_s {
  float x, y;
};

struct gfx_line_s {
  float x1, y1;
  float x2, y2;
};

struct gfx_rect_s {
  float x, y;
  float w, h;
};

struct gfx_circ_s {
  float x, y;
  float r;
};

struct gfx_pointi_s {
  int32_t x, y;
};

struct gfx_linei_s {
  int32_t x1, y1;
  int32_t x2, y2;
};

struct gfx_recti_s {
  int32_t x, y;
  int32_t w, h;
};

struct gfx_circi_s {
  int32_t x, y;
  int32_t r;
};

struct gfx_color_s {
  uint8_t r, g, b, a;
};

void gfx_init(float window_w, float window_h, float render_w, float render_h);
void gfx_end(void);
void gfx_clear(gfx_color_t color);
void gfx_present(void);
void gfx_set_window_title(char* new_title);


void gfx_window_resize(float new_w, float new_h);
void gfx_window_move(float new_x, float new_y);

gfx_color_t gfx_color_from_hex(uint32_t hex);
gfx_color_t gfx_color_from_floats(float r, float g, float b, float a);

gfx_texture_t* gfx_texture_create_and_load(const char* file);
void           gfx_texture_draw(gfx_texture_t* texture, gfx_rect_t* src, gfx_rect_t* dst);
void           gfx_texture_draw_ex(gfx_texture_t* texture, gfx_rect_t* src, gfx_rect_t* dst, float angle, uint8_t flip);
void           gfx_texture_draw_full(gfx_texture_t* texture);
void           gfx_texture_destroy(gfx_texture_t* texture);

#endif
