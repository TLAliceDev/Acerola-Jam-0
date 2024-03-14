#ifndef MESSAGE_H
#define MESSAGE_H

typedef const char* message_t;

void message_set_font(void* font);
void message_push(message_t message);
void message_display();

#endif
