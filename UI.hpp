#pragma once

#include "Font.hpp"
#include "load_save_png.hpp"

#include <string>
#include <vector>

struct UI {
    struct Box {
        glm::vec4 rect; // x1, y1, x2, y2
        glm::u8vec4 color;
        Box(glm::vec4 rect, glm::u8vec4 color) : rect(rect), color(color) {}
    };
    struct Img {
        glm::vec2 pos;
        glm::uvec2 size;
        std::vector< glm::u8vec4 > data;
        Img(glm::vec2 pos, std::string path) : pos(pos){
            load_png(data_path(path), &size, &data, UpperLeftOrigin);
        }
    };
    std::shared_ptr<Font> font_title = nullptr;
    std::shared_ptr<Font> font_body = nullptr;
    std::shared_ptr<Font> font_manual = nullptr;

    std::shared_ptr<Text> manual = nullptr;
    std::shared_ptr<Text> description = nullptr;

    std::shared_ptr<Box> description_box = nullptr;

    unsigned int text_texture = -1U;
    unsigned int box_texture = -1U;
    unsigned int img_texture = -1U;

    std::vector<std::shared_ptr<Text> > texts = {};
    std::vector<std::shared_ptr<Box>> boxes = {};
    std::vector<std::shared_ptr<Img>> imgs = {};
    int32_t width = 1280;
    int32_t height = 720;


    UI() {
        // Set up UI
        std::string font_path = data_path("Open_Sans/static/OpenSans-Regular.ttf");
        font_body = std::make_shared<Font>(font_path, 
                                    /*font_size*/30, 
                                    /*line_height*/35);
        description = std::make_shared<Text>("Do you want to shoot this vase?", 
                            /*line length*/80, 
                            /*start pos*/glm::vec2(40, 520),
                            font_body);
        manual = std::make_shared<Text>("Press 'Space' to shoot, Press 'P' to skip", 
                            /*line length*/85, 
                            /*start pos*/glm::vec2(500, 690),
                            font_body);
        texts = {description, manual};

        description_box = std::make_shared<Box>(glm::vec4(20, 480, 1260, 700), glm::u8vec4(100, 100, 100, 200));
        boxes = {description_box};
        imgs= {std::make_shared<Img>(glm::vec2(0, 0), "UI/I_Button.png")};

        glGenTextures(1, &box_texture);
        glGenTextures(1, &text_texture);
        glGenTextures(1, &img_texture);
    }
    
    void gen_text_texture();
    void gen_box_texture();
    void gen_img_texture();

    std::vector<std::string> wrapText(const std::string& text, size_t line_length = 85);

    void draw_texture(unsigned int texture);
    void draw();
};
