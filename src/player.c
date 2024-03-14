#include "player.h"
#include "room.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gfx.h"
#ifndef __EMSCRIPTEN__
#include <SDL2/SDL_mixer.h>
#endif

typedef struct player_s player_t;

struct player_s {
  room_handle_t** room_array;
  gfx_texture_t* inventory_item_sheet;
  gfx_texture_t* cursor_sheet;
  uint8_t last_room;
  uint8_t current_room;
  uint8_t inventory[9];
  uint8_t curr_slot;
  uint8_t curr_cursor;
  int     mx, my;
#ifndef __EMSCRIPTEN__
  Mix_Chunk* move_sfx;
  Mix_Chunk* click_sfx;
  Mix_Chunk* item_sfx;
#endif
};

player_handle_t*
player_create(void** room_handles, void* inventory_item_sheet) {
  uint8_t i;
  player_t* player = malloc(sizeof(*player));
  player->current_room = 0;
  player->last_room = 0;
  player->room_array = room_handles;
  player->inventory_item_sheet = inventory_item_sheet;
  player->cursor_sheet = gfx_texture_create_and_load("assets/final/textures/cursor.png");
  player->curr_cursor = 0;
  player->curr_slot = 0xff;
  for (i = 0; i < 9; i++)
    player->inventory[i] = 0;
#ifndef __EMSCRIPTEN__
  player->move_sfx = Mix_LoadWAV("assets/final/sfx/move.wav");
  player->item_sfx = Mix_LoadWAV("assets/final/sfx/item.wav");
  player->click_sfx = Mix_LoadWAV("assets/final/sfx/click.wav");
#endif
  return player;
}

void
player_move_to_room(player_handle_t* player_handle, uint8_t room) {
  player_t* player = player_handle;
  player->last_room = player->current_room;
  player->current_room = room;
  room_enter(player->room_array[player->current_room]);
#ifndef __EMSCRIPTEN__
  Mix_PlayChannel(-1, player->move_sfx, 0);
#endif
}

_Bool
player_check_for_item(player_handle_t* player_handle, uint8_t inv_item) {
  player_t* player = player_handle;
  uint8_t i;
  for (i = 0; i < 9; i++)
    if (player->inventory[i] == inv_item)
      return 1;
  return 0;
}

_Bool
player_check_for_item_held(player_handle_t* player_handle, uint8_t inv_item) {
  player_t* player = player_handle;
  if (player->curr_slot == 0xff)
    return 0;
  return player->inventory[player->curr_slot] == inv_item;
}

void
player_add_item(player_handle_t* player_handle, uint8_t inv_item) {
  player_t* player = player_handle;
  uint8_t index;
  for (index = 0; index <= 9; index++)
    if (!player->inventory[index])
      break;
  if (index == 9) 
    return;
  player->inventory[index] = inv_item;
#ifndef __EMSCRIPTEN__
  Mix_PlayChannel(-1, player->item_sfx, 0);
#endif
}

void
player_remove_item(player_handle_t* player_handle, uint8_t inv_item) {
  player_t* player = player_handle;
  uint8_t index;
  for (index = 0; index <= 9; index++) {
    if (player->inventory[index] == inv_item)
      player->inventory[index] = 0;
  }
}

void
player_remove_held_item(player_handle_t* player_handle) {
  player_t* player = player_handle;
  if (player->curr_slot == 0xff)
    return;
  player->inventory[player->curr_slot] = 0;
}

uint8_t
player_current_room(player_handle_t* player) {
  return ((player_t*)player)->current_room;
}

uint8_t
player_last_room(player_handle_t* player) {
  return ((player_t*)player)->current_room;
}

void
player_mouse_motion(player_handle_t* player_handle, int mx, int my) {
  player_t* player = player_handle;
  player->mx = mx;
  player->my = my;
  if (mx >= 120)
    player->curr_cursor = room_cursor_at(player->room_array[player->current_room], mx-120, my);
  else
    player->curr_cursor = 0;
}

void
player_click(player_handle_t* player_handle, int mx, int my) {
  player_t* player = player_handle;
  uint8_t slot;
#ifndef __EMSCRIPTEN__
  Mix_PlayChannel(-1, player->click_sfx, 0);
#endif
  if (mx >= 120 && my <= 160) {
    room_click(player->room_array[player->current_room], player, mx-120, my);
    player->curr_slot = 0xff;
    player_mouse_motion(player, mx, my);
    return;
  }
  if (my >= 176 && my <= 208 && mx >= 32 && mx <= 320 && (player->inventory[(mx-32)/32]))
    player->curr_slot = (mx-32)/32;
  else 
    player->curr_slot = 0xff;
  player_mouse_motion(player, mx, my);
}

void
player_draw_inventory(player_handle_t* player_handle) {
  player_t* player = player_handle;
  uint8_t i = 0;
  for (i = 0; i < 9; i++) {
    if (player->inventory[i]) {
      uint8_t x = (player->inventory[i] - 1) % 8;
      uint8_t y = (player->inventory[i] - 1) / 8;
      gfx_rect_t src = {x*24, y*24, 24, 24};
      gfx_rect_t dst = {36 + i * 32, 180, 24, 24};
      gfx_texture_draw(player->inventory_item_sheet, &src, &dst);
    }
  }
}

void
player_draw_cursor(player_handle_t* player_handle) {
  player_t* player = player_handle;
  gfx_rect_t src = {player->curr_cursor*24, 0, 24, 24};
  gfx_rect_t dst = {player->mx-12, player->my-12, 24, 24};
  if (player->curr_slot != 0xff) {
    uint8_t x = (player->inventory[player->curr_slot]-1) % 8;
    uint8_t y = (player->inventory[player->curr_slot]-1) / 8;
    src.x = x * 24;
    src.y = y * 24;
    gfx_texture_draw(player->inventory_item_sheet, &src, &dst);
  } else {
    gfx_texture_draw(player->cursor_sheet, &src, &dst);
  }
}

void
player_draw(player_handle_t* player_handle) {
  player_t* player = player_handle;

  room_draw(player->room_array[player->last_room], 0);
  room_draw(player->room_array[player->current_room], 2);
  player_draw_inventory(player);
  player_draw_cursor(player);
}

