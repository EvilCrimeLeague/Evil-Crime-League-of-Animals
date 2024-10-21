#pragma once

#include "Font.hpp"
#include "load_save_png.hpp"

#include <string>
#include <vector>

struct UI {
    struct Box {
        glm::vec4 rect; // x1, y1, x2, y2
        glm::u8vec4 color;
        bool hide = true;
        Box(glm::vec4 rect, glm::u8vec4 color) : rect(rect), color(color) {}
    };
    struct Img {
        glm::vec2 pos;
        glm::uvec2 size;
        std::vector< glm::u8vec4 > data;
        bool hide = true;
        Img(glm::vec2 pos, std::string path) : pos(pos){
            load_png(data_path(path), &size, &data, UpperLeftOrigin);
        }
    };
    std::shared_ptr<Font> font_title = nullptr;
    std::shared_ptr<Font> font_body = nullptr;
    std::shared_ptr<Font> font_manual = nullptr;

    std::shared_ptr<Text> description = nullptr;
    std::shared_ptr<Text> choice1 = nullptr;
    std::shared_ptr<Text> choice2 = nullptr;

    std::shared_ptr<Box> description_box = nullptr;

    std::shared_ptr<Img> I_img = nullptr;
    std::shared_ptr<Img> Y_img = nullptr;
    std::shared_ptr<Img> slot_left = nullptr;
    std::shared_ptr<Img> slot_right = nullptr;

    unsigned int text_texture = -1U;
    unsigned int box_texture = -1U;
    unsigned int img_texture = -1U;

    std::vector<std::shared_ptr<Text> > texts = {};
    std::vector<std::shared_ptr<Box>> boxes = {};
    std::vector<std::shared_ptr<Img>> imgs = {};
    bool need_update_texture = true;
    int32_t width = 1280;
    int32_t height = 720;

    unsigned int choice = 0;
    std::vector<glm::vec2> choice_pos = {glm::vec2(100, 600), glm::vec2(600, 600)};

    UI() {
        // Load font
        std::string font_path = data_path("Open_Sans/static/OpenSans-Regular.ttf");
        font_body = std::make_shared<Font>(font_path, 
                                    /*font_size*/30, 
                                    /*line_height*/35);

        // Create texts
        description = std::make_shared<Text>("Do you want to shoot this vase?", 
                            /*line length*/80, 
                            /*start pos*/glm::vec2(40, 520),
                            font_body);
        choice1 = std::make_shared<Text>("Yes", 
                            /*line length*/85, 
                            /*start pos*/glm::vec2(180, 632),
                            font_body);
        choice2 = std::make_shared<Text>("Skip it", 
                            /*line length*/85, 
                            /*start pos*/glm::vec2(680, 632),
                            font_body);
        texts = {description, choice1, choice2};

        // Create boxes
        description_box = std::make_shared<Box>(glm::vec4(20, 480, 1260, 700), glm::u8vec4(100, 100, 100, 200));
        boxes = {description_box};

        // Create images
        I_img = std::make_shared<Img>(glm::vec2(100, 600), "UI/I_Button.png");
        Y_img = std::make_shared<Img>(glm::vec2(600, 600), "UI/Y_Button.png");
        slot_left = std::make_shared<Img>(choice_pos[0], "UI/Slot.png");
        slot_right = std::make_shared<Img>(choice_pos[1], "UI/Slot.png");
        imgs= {slot_left, slot_right, I_img, Y_img};

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

    void update_texture();

    void update_choice();

    void show_description();
    void hide_description();
};
