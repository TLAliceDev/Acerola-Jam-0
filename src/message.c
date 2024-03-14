#include "message.h"
#include "gfx.h"
#include "font.h"

struct message_display_internal_s {
  font_t* font;
  message_t current_message;
  float percentage;
} message_display_internal_g;

void
message_set_font(font_t* font) {
  message_display_internal_g.font = font;
}

void
message_push(message_t message) {
  message_display_internal_g.percentage = 0;
  message_display_internal_g.current_message = message;
}

void
message_display() {
  if (!message_display_internal_g.current_message)
    return;
  text_draw_p(message_display_internal_g.current_message, message_display_internal_g.font, 4, 4, 8, message_display_internal_g.percentage);
  if (message_display_internal_g.percentage < 1.0)
    message_display_internal_g.percentage += 2.0 / 160.0;
  else
    message_display_internal_g.percentage = 1.0;
}

