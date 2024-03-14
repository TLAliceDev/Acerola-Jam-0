#ifndef GAME_H
#define GAME_H

typedef void pacgame_handler_t;

pacgame_handler_t* game_start(const char* game_def);
unsigned char      game_loop(pacgame_handler_t* game_handle);
void               game_end(pacgame_handler_t* game_handle);

#endif
