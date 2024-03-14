#include "font.h"
#include "game.h"
#include "gfx.h"
#include "message.h"
#include "player.h"
#include "rdf.h"
#include "room.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifndef __EMSCRIPTEN__

int
main(int argc, char** argv) {
  pacgame_handler_t* game_handle = game_start("assets/game.rdf");
  _Bool quit = 0;
  while (!quit)
    quit = game_loop(game_handle);

  game_end(game_handle);

  return 0;
}

#else

pacgame_handler_t* game_handle;
_Bool quit;

void
loop(void) {
  quit = game_loop(game_handle);
  if (quit) {
    emscripten_cancel_main_loop();
    game_end(game_handle);
  }
}

int
main(void) {
  game_handle = game_start("assets/game.rdf");
  quit = 0;
  emscripten_set_main_loop(loop, 0, 1);
  return 0;
}

#endif
