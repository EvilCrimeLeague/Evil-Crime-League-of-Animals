#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

#include <glm/glm.hpp>

#include <iostream>
#include <string>
#include <memory>

#include "data_path.hpp"
#include "GL.hpp"

struct Font {
    Font(std::string const &font_path, int font_size, float line_height);
    ~Font();

    void load_text(std::string const &text);

    void load_glyph(hb_codepoint_t gid);

    FT_Library ft_library;
	FT_Face ft_face;
    hb_font_t *hb_font;
    hb_buffer_t *hb_buffer;
    int font_size;
    float line_height;
};

struct Text {
    std::string text;
    float line_length; // max length of a line
    glm::vec2 start_pos; // start at start_pos.x and start_pos.y
    std::shared_ptr<Font> font;
    glm::u8vec3 color = glm::u8vec3(255, 255, 255);
    bool hide = true;

    Text(std::string text, float line_length, glm::vec2 start_pos,std::shared_ptr<Font> font ) : text(text), line_length(line_length), start_pos(start_pos), font(font) {}
    Text(){}
};