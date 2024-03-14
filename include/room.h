#ifndef ROOM_H
#define ROOM_H
#include <stdint.h>

typedef void                 room_handle_t;
typedef void                 item_handle_t;
typedef struct item_action_s item_action_t;

typedef enum {
  IA_MESSAGE,
  IA_MOVE_ROOM,
  IA_DEACTIVATE_SELF,
  IA_CHANGE_SPRITE,
  IA_ADD_INV_ITEM,
  IA_REQUIRE_INV_ITEM,
  IA_REMOVE_INV_ITEM,
  IA_REQUIRE_HELD_ITEM,
  IA_REMOVE_HELD_ITEM,
  IA_DEACTIVATE_ITEM,  
  IA_ACTIVATE_ITEM,  
  IA_SHOW,  
  IA_HIDE,  
  IA_SHOW_ITEM,  
  IA_HIDE_ITEM,  
  IA_BREAK,
  IA_SFX,
  IA_N
} item_action_e;

struct item_action_s {
  item_action_e type;
  union {
    uint16_t    item_id;
    uint16_t    room_id;
    const char* message;
    struct {
      float sx, sy, sw, sh;
      float x, y, w, h;
    }           new_gfx_rect;
    struct {
      uint16_t room_id;
      uint16_t item_index;
    }           activate_item;
  } data;
};


room_handle_t* room_create(void* background, void* spritesheet, char* message, room_handle_t** rooms_ptr);
void           room_destroy(room_handle_t* room_handle);
void           room_set_bg(room_handle_t* room, void* bg);
void           room_add_item(room_handle_t* room_handle, float sx, float sy, float sw, float sh, float x, float y, float w, float h, item_action_t* actions, uint8_t actions_n, _Bool active);
void           room_add_item_ns(room_handle_t* room_handle, float x, float y, float w, float h, item_action_t* actions, uint8_t actions_n);
void           room_draw(room_handle_t* room, uint8_t loading);
_Bool          room_click(room_handle_t* room, void* player, float mx, float my);
void           room_enter(room_handle_t* room_handle);
uint8_t        room_cursor_at(room_handle_t* room_handle, float mx, float my);

#endif
