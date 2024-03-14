#include "rdf.h"
#include "font.h"
#include "room.h"
#include "gfx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
GAME_INFO:(game_title);(number_of_rooms);(map_item_spritesheet_file);(inventory_item_spritesheet_file)
ROOM_INFO:(room_name);(background_image_filename);(number_of_items)\t(enter_message)
ITEM:(texture_x);(texture_y);(texture_w);(texture_h);(x);(y);(w);(h);(number_of_actions)
ITEM_ACTION:(action_id);(action_data)
*/

static char*
strfstnw(char* str) {
  while (*str == ' ' || *str == '\t')
    ++str;
  return str;
}

static item_action_t
item_action_parse(char* str) {
  static char* action_names[IA_N] = {
    [IA_MESSAGE] = "MESSAGE",
    [IA_MOVE_ROOM] = "MOVE_ROOM",
    [IA_DEACTIVATE_SELF] = "DEACTIVATE_SELF",
    [IA_CHANGE_SPRITE] = "CHANGE_SPRITE",
    [IA_ADD_INV_ITEM] = "ADD_INV_ITEM",
    [IA_REQUIRE_INV_ITEM] = "REQUIRE_INV_ITEM",
    [IA_REMOVE_INV_ITEM] = "REMOVE_INV_ITEM",
    [IA_REQUIRE_HELD_ITEM] = "REQUIRE_HELD_ITEM",
    [IA_REMOVE_HELD_ITEM] = "REMOVE_HELD_ITEM",
    [IA_DEACTIVATE_ITEM] = "DEACTIVATE_ITEM",
    [IA_ACTIVATE_ITEM] = "ACTIVATE_ITEM",
    [IA_HIDE_ITEM] = "HIDE_ITEM",
    [IA_SHOW_ITEM] = "SHOW_ITEM",
    [IA_HIDE] = "HIDE",
    [IA_SHOW] = "SHOW",
    [IA_BREAK] = "BREAK",
    [IA_SFX] = "SFX",
  };
  char action_name[128];
  item_action_e action_e;
  char* message;
  uint8_t id, id2;
  str = strfstnw(str);
  sscanf(str, "ITEM_ACTION:%[^;];", action_name);
  for (action_e = IA_MESSAGE; action_e <= IA_N; action_e++) {
    if (action_e == IA_N) {
      printf("error parsing: %s\n", str);
    }
    if (!strcmp(action_name, action_names[action_e]))
      break;
  }
  switch (action_e) {
  case IA_MESSAGE:
    message = calloc(512, sizeof(*message));
    sscanf(str, "ITEM_ACTION:MESSAGE;\"%[^\"]\"", message);
    return (item_action_t){IA_MESSAGE, .data.message=message};
    break;
  case IA_MOVE_ROOM:
    sscanf(str, "ITEM_ACTION:MOVE_ROOM;%hhu", &id);
    return (item_action_t){IA_MOVE_ROOM, .data.room_id=id};
    break;
  case IA_ADD_INV_ITEM:
    sscanf(str, "ITEM_ACTION:ADD_INV_ITEM;%hhu", &id);
    return (item_action_t){IA_ADD_INV_ITEM, .data.item_id=id};
    break;
  case IA_REQUIRE_INV_ITEM:
    sscanf(str, "ITEM_ACTION:REQUIRE_INV_ITEM;%hhu", &id);
    return (item_action_t){IA_REQUIRE_INV_ITEM, .data.item_id=id};
    break;
  case IA_REQUIRE_HELD_ITEM:
    sscanf(str, "ITEM_ACTION:REQUIRE_HELD_ITEM;%hhu", &id);
    return (item_action_t){IA_REQUIRE_HELD_ITEM, .data.item_id=id};
    break;
  case IA_REMOVE_INV_ITEM:
    sscanf(str, "ITEM_ACTION:REMOVE_INV_ITEM;%hhu", &id);
    return (item_action_t){IA_REMOVE_INV_ITEM, .data.item_id=id};
    break;
  case IA_REMOVE_HELD_ITEM:
    sscanf(str, "ITEM_ACTION:REMOVE_HELD_ITEM;%hhu", &id);
    return (item_action_t){IA_REMOVE_HELD_ITEM, .data.item_id=id};
    break;
  case IA_DEACTIVATE_SELF:
    return (item_action_t){IA_DEACTIVATE_SELF, .data.message=NULL};
    break;
  case IA_DEACTIVATE_ITEM:
    sscanf(str, "ITEM_ACTION:DEACTIVATE_ITEM;%hhu;%hhu", &id, &id2);
    return (item_action_t){IA_DEACTIVATE_ITEM, .data.activate_item = {.room_id=id, .item_index=id2}};
  case IA_ACTIVATE_ITEM:
    sscanf(str, "ITEM_ACTION:ACTIVATE_ITEM;%hhu;%hhu", &id, &id2);
    return (item_action_t){IA_ACTIVATE_ITEM, .data.activate_item = {.room_id=id, .item_index=id2}};
  case IA_SHOW_ITEM:
    sscanf(str, "ITEM_ACTION:SHOW_ITEM;%hhu;%hhu", &id, &id2);
    return (item_action_t){IA_SHOW_ITEM, .data.activate_item = {.room_id=id, .item_index=id2}};
    break;
  case IA_HIDE_ITEM:
    sscanf(str, "ITEM_ACTION:HIDE_ITEM;%hhu;%hhu", &id, &id2);
    return (item_action_t){IA_HIDE_ITEM, .data.activate_item = {.room_id=id, .item_index=id2}};
    break;
  case IA_SHOW:
    return (item_action_t){IA_SHOW, .data.message=NULL};
    break;
  case IA_HIDE:
    return (item_action_t){IA_HIDE, .data.message=NULL};
    break;
  case IA_BREAK:
    return (item_action_t){IA_BREAK, .data.message=NULL};
    break;
  default:
    return (item_action_t){IA_DEACTIVATE_SELF, .data.message=NULL};
    break;
  }
}

static room_handle_t*
room_load(FILE* fptr, gfx_texture_t* spritesheet, char* room_name_out, room_handle_t* rooms_ptr) {
  uint8_t items_n, i;
  char buffer[2048];
  char bg_image_file[2048];
  char* enter_message;
  room_handle_t* room;
  gfx_texture_t* bg_image;
  int16_t item_sx, item_sy, item_sw, item_sh, item_x, item_y, item_w, item_h, item_nactions;
  uint8_t item_actions_i;
  item_action_t* item_actions;
  _Bool active;
  char* str;
  enter_message = calloc(512, sizeof(*enter_message));
  fgets(buffer, 2048, fptr);
  str = strfstnw(buffer);
  sscanf(str, "ROOM_INFO:\"%[^\"]\";\"%[^\"]\";%hhu;\"%[^\"]\"\n", room_name_out, bg_image_file, &items_n, enter_message);
  bg_image = gfx_texture_create_and_load(bg_image_file);
  room = room_create(bg_image, spritesheet, enter_message, rooms_ptr);
  for (i = 0; i < items_n; i++) {
    fgets(buffer, 2048, fptr);
    str = strfstnw(buffer);
    sscanf(str, "ITEM:%hi;%hi;%hi;%hi;%hi;%hi;%hi;%hi;%hi;%hhu\n", &item_sx, &item_sy, &item_sw, &item_sh, &item_x, &item_y, &item_w, &item_h, &item_nactions, &active);
    item_actions = calloc(item_nactions, sizeof(*item_actions));
    for (item_actions_i = 0; item_actions_i < item_nactions; item_actions_i++) {
      fgets(buffer, 2048, fptr);
      item_actions[item_actions_i] = item_action_parse(buffer);
    }
    room_add_item(room, item_sx, item_sy, item_sw, item_sh, item_x, item_y, item_w, item_h, item_actions, item_nactions, !active);
    free(item_actions);
  }
  return room;
}

rdf_result_t
rdf_load(const char* file) {
  uint8_t i;
  FILE* fptr = fopen(file, "r");
  char buffer[2048];
  char map_item_spritesheet_file[2048];
  char inv_item_spritesheet_file[2048];
  char background_texture_file[2048];
  char font_file[2048];
  gfx_texture_t* map_item_spritesheet;
  rdf_result_t results = {
    calloc(64, sizeof(*results.game_name)),
    NULL, NULL, NULL,
    0, NULL, 0
  };
  fgets(buffer, 2048, fptr);
  char* str = strfstnw(buffer);
  sscanf(str, "GAME_INFO:\"%[^\"]\";%hhu;\"%[^\"]\";\"%[^\"]\";\"%[^\"]\";\"%[^\"]\";%x\n", results.game_name, &results.rooms_n, map_item_spritesheet_file, inv_item_spritesheet_file, background_texture_file, font_file, &results.clear_colour);
  results.inv_item_spritesheet = gfx_texture_create_and_load(inv_item_spritesheet_file);
  results.background_texture = gfx_texture_create_and_load(background_texture_file);
  results.font = font_load(font_file);
  map_item_spritesheet = gfx_texture_create_and_load(map_item_spritesheet_file);
  results.rooms_pointer = calloc(results.rooms_n,sizeof(*results.rooms_pointer));
  for (i = 0; i < results.rooms_n; i++) {
    char room_name[2048];
    results.rooms_pointer[i] = room_load(fptr, map_item_spritesheet, room_name, results.rooms_pointer);
  }
  fclose(fptr);
  return results;
}
