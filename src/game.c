#include "game.h"
#include "gfx.h"
#include "message.h"
#include "player.h"
#include "rdf.h"
#include "font.h"
#include "room.h"
#include <SDL2/SDL_events.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef __EMSCRIPTEN__
#include <SDL2/SDL_mixer.h>
#endif

typedef struct pacgame_s pacgame_t;

struct pacgame_s {
  gfx_texture_t*   background_texture;
  font_t*          text_font;
  player_handle_t* player_handle;
  room_handle_t**  rooms;
  uint32_t         rooms_n;
  uint32_t         clear_colour;
};

pacgame_handler_t*
game_start(const char* game_def_file) {
  rdf_result_t rdf;
  pacgame_t* game = malloc(sizeof(*game));

  gfx_init(1366,768,240*1.5,160*1.5);
  #ifndef __EMSCRIPTEN__
  Mix_Init(MIX_INIT_MP3);
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 1, 2048);
  #endif
  rdf = rdf_load(game_def_file);

  gfx_set_window_title(rdf.game_name);
  game->player_handle = player_create(rdf.rooms_pointer, rdf.inv_item_spritesheet);
  game->text_font = rdf.font;

  game->background_texture = rdf.background_texture;
  SDL_ShowCursor(SDL_DISABLE);

  game->clear_colour = rdf.clear_colour;
  message_set_font(game->text_font);
  message_push("");
  game->rooms_n = rdf.rooms_n;
  return game;
} 

uint8_t
game_loop(pacgame_handler_t* handle) {
  pacgame_t* game = handle;
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
    case SDL_QUIT:
      return 1;
      break;
    case SDL_MOUSEMOTION:
      player_mouse_motion(game->player_handle, e.motion.x, e.motion.y);
      break;
    case SDL_MOUSEBUTTONDOWN:
      if (e.button.button == SDL_BUTTON_LEFT)
        player_click(game->player_handle, e.button.x, e.button.y);
      break;
    }
  }
  gfx_clear(gfx_color_from_hex(game->clear_colour));
  gfx_texture_draw_full(game->background_texture);
  player_draw(game->player_handle);
  message_display();
  gfx_present();
  return 0;
}

void
game_end(pacgame_handler_t* handle) {
  /*
  pacgame_t* game = handle;
  uint8_t i;
  for (i = 0; i < game->rooms_n; i++)
    room_destroy(game->rooms[i]);
  free(game->rooms);
  gfx_texture_destroy(game->background_texture);
  */
  gfx_end();
}
