#include "gfx.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_image.h>

typedef struct gfx_renderer_internal_s gfx_renderer_internal_t;
typedef struct gfx_window_internal_s   gfx_window_internal_t;

struct gfx_window_internal_s {
  SDL_Window* window;
  int         width, height;
};

struct gfx_renderer_internal_s {
  SDL_Renderer* renderer;
  int           width, height;
};

static struct gfx_state_internal_s {
  gfx_window_internal_t   window;
  gfx_renderer_internal_t renderer;
} gfx_state_internal_g;

void
gfx_init(float window_w, float window_h, float render_w, float render_h) {
  printf("TEST HELLO\n");
  gfx_state_internal_g.window = (gfx_window_internal_t) {
    #ifdef __EMSCRIPTEN__
    SDL_CreateWindow("game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, SDL_WINDOW_RESIZABLE),
    #else
    SDL_CreateWindow("game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, SDL_WINDOW_SHOWN),
    #endif
    window_w, window_h
  };

  gfx_state_internal_g.renderer = (gfx_renderer_internal_t) {
    SDL_CreateRenderer(gfx_state_internal_g.window.window, -1, SDL_RENDERER_PRESENTVSYNC),
    render_w, render_h
  };

  SDL_RenderSetLogicalSize(gfx_state_internal_g.renderer.renderer, render_w, render_h);
  SDL_RenderSetIntegerScale(gfx_state_internal_g.renderer.renderer, SDL_TRUE);
}

void
gfx_end(void) {
  SDL_DestroyRenderer(gfx_state_internal_g.renderer.renderer);
  SDL_DestroyWindow(gfx_state_internal_g.window.window);
}

void
gfx_set_window_title(char* new_title) {
  SDL_SetWindowTitle(gfx_state_internal_g.window.window, new_title);
}

void
gfx_clear(gfx_color_t color) {
  SDL_SetRenderDrawColor(gfx_state_internal_g.renderer.renderer, color.r, color.g, color.b, color.a);
  SDL_RenderClear(gfx_state_internal_g.renderer.renderer);
}

void
gfx_present(void) {
  SDL_RenderPresent(gfx_state_internal_g.renderer.renderer);
}



gfx_color_t
gfx_color_from_hex(uint32_t hex) {
  uint8_t r = (hex >> 24) & 0xff;
  uint8_t g = (hex >> 16) & 0xff;
  uint8_t b = (hex >> 8) & 0xff;
  uint8_t a = (hex >> 0) & 0xff;
  return (gfx_color_t){r, g, b, a};
}

gfx_color_t
gfx_color_from_floats(float r, float g, float b, float a) {
  uint8_t fr = (r > 1.0 ? 1.0 : r) * 0xff;
  uint8_t fg = (g > 1.0 ? 1.0 : g) * 0xff;
  uint8_t fb = (b > 1.0 ? 1.0 : b) * 0xff;
  uint8_t fa = (a > 1.0 ? 1.0 : a) * 0xff;
  return (gfx_color_t){fr, fg, fb, fa};
  
}


gfx_texture_t*
gfx_texture_create_and_load(const char* file) {
  SDL_Texture* sdl_texture;
  sdl_texture = IMG_LoadTexture(gfx_state_internal_g.renderer.renderer, file);

  return sdl_texture;
}

void
gfx_texture_draw_ex(gfx_texture_t* texture, gfx_rect_t* src, gfx_rect_t* dst, float angle, uint8_t flip) {
  SDL_Texture* sdl_texture= texture;
  SDL_Rect sdl_src;
  SDL_Rect sdl_dst;
  SDL_Rect* sdl_src_ptr = NULL;
  SDL_Rect* sdl_dst_ptr = NULL;
  if (src) {
    sdl_src.x = src->x;
    sdl_src.y = src->y;
    sdl_src.w = src->w;
    sdl_src.h = src->h;
    sdl_src_ptr = &sdl_src;
  }
  if (dst) {
    sdl_dst.x = dst->x;
    sdl_dst.y = dst->y;
    sdl_dst.w = dst->w;
    sdl_dst.h = dst->h;
    sdl_dst_ptr = &sdl_dst;
  }
  SDL_RenderCopyEx(gfx_state_internal_g.renderer.renderer, sdl_texture, sdl_src_ptr, sdl_dst_ptr, angle, NULL, flip);
}

void
gfx_texture_draw(gfx_texture_t* texture, gfx_rect_t* src, gfx_rect_t* dst) {
  gfx_texture_draw_ex(texture, src, dst, 0, 0);
}

void
gfx_texture_draw_full(gfx_texture_t* texture) {
  gfx_texture_draw_ex(texture, NULL, NULL, 0, 0);
}


void
gfx_texture_destroy(gfx_texture_t* texture) {
  SDL_DestroyTexture((SDL_Texture*)texture);
}
