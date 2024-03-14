#ifndef FONT_H
#define FONT_H

typedef void font_t;

font_t* font_load(const char* filepath);
void text_draw(const char* text, font_t* font_ptr, float x, float y, float size);
void text_draw_p(const char* text, font_t* font_ptr, float x, float y, float size, float percentage);

#endif
