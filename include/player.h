#ifndef PLAYER_H
#define PLAYER_H
#include <stdint.h>

typedef void player_handle_t;

player_handle_t* player_create(void** room_handles, void* inventory_item_sheet);
void             player_move_to_room(player_handle_t* player_handle, uint8_t room);
void             player_add_item(player_handle_t* player_handle, uint8_t inv_item);
void             player_remove_item(player_handle_t* player_handle, uint8_t inv_item);
_Bool            player_check_for_item(player_handle_t* player_handle, uint8_t inv_item);
_Bool            player_check_for_item_held(player_handle_t* player_handle, uint8_t inv_item);
uint8_t          player_last_room(player_handle_t* player);
uint8_t          player_current_room(player_handle_t* player);
void             player_draw_inventory(player_handle_t* player_handle);
void             player_mouse_motion(player_handle_t* player_handle, int mx, int my);
void             player_draw(player_handle_t* player_handle);
void             player_click(player_handle_t* player_handle, int mx, int my);
void             player_remove_held_item(player_handle_t* player_handle);


#endif
