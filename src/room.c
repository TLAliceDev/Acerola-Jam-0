#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "player.h"
#include "room.h"
#include "gfx.h"
#include "message.h"
#define MAX_ITEMS 128
#define MAX_ITEM_ACTIONS 32

typedef struct room_s room_t;
typedef struct item_s item_t;

struct item_s {
  gfx_rect_t    sprite_loc;
  gfx_rect_t    rect;
  item_action_t actions[MAX_ITEM_ACTIONS];
  uint8_t       actions_n;
  uint16_t      actions_flag;
  _Bool         active;
  _Bool         visible;
  uint8_t       action_broken;
  const char*   message;
};

struct room_s {
  uint8_t        loading;
  uint8_t        loading_items;
  gfx_texture_t* background;
  gfx_texture_t* item_spritesheet;
  item_t         items[MAX_ITEMS];
  uint8_t        items_n;
  _Bool          items_flagged[MAX_ITEMS];
  _Bool          visited;
  room_handle_t** rooms_ptr;
  char*          enter_message;
};


room_handle_t*
room_create(gfx_texture_t* background, gfx_texture_t* spritesheet, char* message, room_handle_t** rooms_ptr) {
  room_t* room = calloc(1, sizeof(*room));
  uint8_t i;
  room->background = background;
  room->item_spritesheet = spritesheet;
  room->items_n = 0;
  room->enter_message = message;
  room->loading = 0;
  room->loading_items = 0;
  for (i = 0; i < MAX_ITEMS; i++)
    room->items_flagged[i] = 0;
  room->rooms_ptr = rooms_ptr;
  room->visited = 0;
  return room;
}

void
room_add_item(room_handle_t* room_handle, float sx, float sy, float sw, float sh, float x, float y, float w, float h, item_action_t* actions, uint8_t actions_n, _Bool active) {
  room_t* room = room_handle;
  uint8_t index = room->items_n++;
  uint8_t i;
  room->items[index].sprite_loc = (gfx_rect_t){sx, sy, sw, sh};
  room->items[index].rect = (gfx_rect_t){x, y, w, h};
  room->items[index].actions_flag = 0;
  for (i = 0; i < actions_n; i++) {
    room->items[index].actions[i] = actions[i];
    room->items[index].actions_flag |= 1 << actions[i].type;
  }
  room->items[index].actions_n = actions_n;
  room->items[index].active = active;
  room->items[index].visible = active;
  room->items[index].action_broken = 0;
}
void
room_add_item_ns(room_handle_t* room_handle, float x, float y, float w, float h, item_action_t* actions, uint8_t actions_n) {
  room_t* room = room_handle;
  uint8_t index = room->items_n++;
  uint8_t i;
  room->items[index].sprite_loc = (gfx_rect_t){0, 0, 0, 0};
  room->items[index].rect = (gfx_rect_t){x, y, w, h};
  for (i = 0; i < actions_n; i++)
    room->items[index].actions[i] = actions[i];
  room->items[index].actions_n = actions_n;
  room->items[index].visible = 0;
  room->items[index].active = 1;
}

void
room_draw(room_handle_t* room_handle, uint8_t loading) {
  room_t* room = room_handle;
  gfx_rect_t src = {0, 0, 240, 0};
  gfx_rect_t dst = {120, 0, 240, 0};
  uint8_t i;
  if (room->visited)
    loading *= 2;
  if (room->loading < 160)
    room->loading += loading;
  else {
    if (room->loading_items < 160)
      room->loading_items += loading;
    else
      room->visited = 1;
  }
  src.h = room->loading;
  dst.h = room->loading;
  gfx_texture_draw(room->background, &src, &dst);
  for (i = 0; i < room->items_n ; i++) {
    gfx_rect_t src = room->items[i].sprite_loc;
    gfx_rect_t dst = room->items[i].rect;
    float percentage = room->loading_items / 160.0;
    if (!room->items[i].visible)
      continue;
    src.h *= percentage;
    dst.h *= percentage;
    dst.x += 120.0;
    gfx_texture_draw(room->item_spritesheet, &src, &dst);
  }
}

void
room_enter(room_handle_t* room_handle) {
  room_t* room = room_handle;
  room->loading = 0;
  room->loading_items = 0;
  if (room->enter_message)
    message_push(room->enter_message);
}


static void
item_clicked(room_t* room, void* player_handle, uint8_t item_index, item_t* item) {
  uint8_t i;
  room_t* r;
  if (item->action_broken >= item->actions_n)
    item->action_broken = 0;
  for (i = item->action_broken; i < item->actions_n; i++) {
    switch (item->actions[i].type) {
    case IA_MESSAGE:
      message_push(item->actions[i].data.message);
      break;
    case IA_MOVE_ROOM:
      player_move_to_room(player_handle, item->actions[i].data.room_id);
      break;
    case IA_DEACTIVATE_SELF:
      room->items[item_index].active = 0;
      break;
    case IA_ADD_INV_ITEM:
      player_add_item(player_handle, item->actions[i].data.item_id);
      break;
    case IA_REMOVE_INV_ITEM:
      player_remove_item(player_handle, item->actions[i].data.item_id);
      break;
    case IA_REMOVE_HELD_ITEM:
      player_remove_held_item(player_handle);
      break;
    case IA_REQUIRE_INV_ITEM:
      if (!player_check_for_item(player_handle, item->actions[i].data.item_id))
        return;
      break;
    case IA_REQUIRE_HELD_ITEM:
      if (!player_check_for_item_held(player_handle, item->actions[i].data.item_id))
        return;
      break;
    case IA_DEACTIVATE_ITEM:
      r = room->rooms_ptr[item->actions[i].data.activate_item.room_id];
      r->items[item->actions[i].data.activate_item.item_index].active = 0;
      break;
    case IA_ACTIVATE_ITEM:
      r = room->rooms_ptr[item->actions[i].data.activate_item.room_id];
      r->items[item->actions[i].data.activate_item.item_index].active = 1;
      break;
    case IA_HIDE_ITEM:
      r = room->rooms_ptr[item->actions[i].data.activate_item.room_id];
      r->items[item->actions[i].data.activate_item.item_index].visible = 0;
      break;
    case IA_SHOW_ITEM:
      r = room->rooms_ptr[item->actions[i].data.activate_item.room_id];
      r->items[item->actions[i].data.activate_item.item_index].visible = 1;
      break;
    case IA_HIDE:
      item->visible = 0;
      break;
    case IA_SHOW:
      item->visible = 1;
      break;
    case IA_BREAK:
      item->action_broken = i+1;
      return;
      break;
    default:
      break;
    }

  }
}

_Bool
room_click(room_handle_t* room_handle, void* player_handle, float mx, float my) {
  room_t* room = room_handle;
  uint8_t i;
  for (i = 0; i < room->items_n; i++) {
    uint16_t index = + room->items_n-1 - i;
    item_t* item = room->items + index;
    _Bool x_collision = mx >= item->rect.x && mx <= item->rect.x + item->rect.w;
    _Bool y_collision = my >= item->rect.y && my <= item->rect.y + item->rect.h;
    if (!room->items[index].active)
      continue;
    if (x_collision && y_collision) {
      item_clicked(room, player_handle, index, item);
      break;
    }
  }
  return 0;
}

static uint8_t
room_item_get_cursor(item_t* item) {
  if (item->actions_flag & (1 << IA_MOVE_ROOM))
    return 1;
  if (item->actions_flag & (1 << IA_ADD_INV_ITEM))
    return 3;
  return 2;
}

uint8_t
room_cursor_at(room_handle_t* room_handle, float mx, float my) {
  room_t* room = room_handle;
  uint8_t i;
  for (i = 0; i < room->items_n; i++) {
    uint8_t index = room->items_n-1 - i;
    item_t* item = room->items + index;
    _Bool x_collision = mx >= item->rect.x && mx <= item->rect.x + item->rect.w;
    _Bool y_collision = my >= item->rect.y && my <= item->rect.y + item->rect.h;
    if (!room->items[index].active)
      continue;
    if (x_collision && y_collision) {
      return room_item_get_cursor(item);
    }
  }
  return 0;
  
}

void
room_destroy(room_handle_t* room_handle) {
  room_t* room = room_handle;
  gfx_texture_destroy(room->background);
  free(room->enter_message);
}
