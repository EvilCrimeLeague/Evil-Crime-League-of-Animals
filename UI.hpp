#pragma once

#include "Font.hpp"

#include <string>
#include <vector>

struct UI {
    struct Box {
        glm::vec4 rect; // x1, y1, x2, y2
        glm::u8vec4 color;
        Box(glm::vec4 rect, glm::u8vec4 color) : rect(rect), color(color) {}
    };
    std::shared_ptr<Font> font_title = nullptr;
    std::shared_ptr<Font> font_body = nullptr;
    std::shared_ptr<Font> font_manual = nullptr;

    std::shared_ptr<Text> manual = nullptr;
    std::shared_ptr<Text> description = nullptr;

    std::shared_ptr<Box> description_box = nullptr;

    unsigned int text_texture;
    unsigned int box_texture;

    std::vector<std::shared_ptr<Text> > texts = {};
    std::vector<std::shared_ptr<Box>> boxes = {};
    int32_t width = 1280;
    int32_t height = 720;


    UI() {
        // Set up UI
        std::string font_path = data_path("Open_Sans/static/OpenSans-Regular.ttf");
        font_body = std::make_shared<Font>(font_path, 
                                    /*font_size*/30, 
                                    /*line_height*/35);
        description = std::make_shared<Text>("start", 
                            /*line length*/80, 
                            /*start pos*/glm::vec2(40, 520),
                            font_body);
        manual = std::make_shared<Text>("Press 'return' to continue, 1234 to make choices", 
                            /*line length*/85, 
                            /*start pos*/glm::vec2(500, 690),
                            font_body);
        texts = {description};

        description_box = std::make_shared<Box>(glm::vec4(20, 480, 1260, 700), glm::u8vec4(100, 100, 100, 200));
        boxes = {description_box};
    }
    
    void gen_text_texture();
    void gen_box_texture();

    std::vector<std::string> wrapText(const std::string& text, size_t line_length = 85);

    void draw_texture(unsigned int texture);
    void draw();
};
