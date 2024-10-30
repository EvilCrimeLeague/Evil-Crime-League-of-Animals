#pragma once

#include "Font.hpp"
#include "load_save_png.hpp"

#include <string>
#include <vector>
#include <unordered_map>

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
        std::shared_ptr<std::vector< glm::u8vec4 > > data;
        bool hide = true;
        Img(glm::vec2 pos, std::string path) : pos(pos){
            data = std::make_shared<std::vector< glm::u8vec4 >>();
            load_png(data_path(path), &size, data.get(), UpperLeftOrigin);
        }
        Img(glm::vec2 pos, glm::uvec2 size, std::shared_ptr<std::vector< glm::u8vec4 > > data): pos(pos), size(size), data(data) {}
    };

    std::shared_ptr<Font> font_title = nullptr;
    std::shared_ptr<Font> font_body = nullptr;
    std::shared_ptr<Font> font_manual = nullptr;

    std::shared_ptr<Text> description_text = nullptr;
    std::shared_ptr<Text> choice1_text = nullptr;
    std::shared_ptr<Text> choice2_text = nullptr;
    std::shared_ptr<Text> game_over_text = nullptr;
    std::shared_ptr<Text> manual_text = nullptr;
    std::shared_ptr<Text> inventory_bt_manual_text = nullptr;
    std::shared_ptr<Text> inventory_manual_text = nullptr;
     std::shared_ptr<Text> notification_text = nullptr;

    std::shared_ptr<Box> description_box = nullptr;
    std::shared_ptr<Box> game_over_box = nullptr;
    std::shared_ptr<Box> notification_box = nullptr;

    std::shared_ptr<Img> interact_bt_img = nullptr; // F
    std::shared_ptr<Img> enter_bt_img = nullptr; // enter
    std::shared_ptr<Img> inventory_bt_img = nullptr;
    std::shared_ptr<Img> slot_left_img = nullptr;
    std::shared_ptr<Img> slot_right_img = nullptr;
    std::shared_ptr<Img> inventory_img = nullptr;
    std::shared_ptr<Img> slot_selected_img = nullptr;
    std::shared_ptr<Img> alarm_img = nullptr;

    unsigned int text_texture = -1U;
    unsigned int box_texture = -1U;
    unsigned int img_texture = -1U;

    std::vector<std::shared_ptr<Text> > texts = {};
    std::vector<std::shared_ptr<Box>> boxes = {};
    std::vector<std::shared_ptr<Img>> imgs = {};

    int32_t width = 1280;
    int32_t height = 720;

    // state of the description box when interact with an item
    uint32_t choice_id = 0;
    std::vector<glm::vec2> choice_pos = {glm::vec2(100, 600), glm::vec2(600, 600)};
    
    // state of the inventory
    struct InventoryItem {
        std::string name;
        std::shared_ptr<Img> img;
        uint32_t inventory_slot_id;
    };
    std::vector<glm::vec2> item_pos = {};
    uint32_t inventory_slot_id_start; // index of the first slot in imgs
    uint32_t inventory_slot_selected_id = 0; // index of the selected slot in the inventory
    const uint32_t inventory_slot_num = 10;
    std::vector<InventoryItem> inventory_items;

    bool showing_inventory = false;
    bool showing_inventory_description = false;
    bool showing_notification = false;
    bool showing_description = false;
    bool showing_interactable_button = false;
    bool showing_alarm = false;

    bool need_update_texture = true;

    UI() {
        // Load font
        std::string font_path = data_path("Open_Sans/static/OpenSans-Regular.ttf");
        std::string font_path_bold = data_path("Open_Sans/static/OpenSans-Bold.ttf");
        std::string font_path_italic = data_path("Open_Sans/static/OpenSans-Italic.ttf");
        font_body = std::make_shared<Font>(font_path, 
                                    /*font_size*/30, 
                                    /*line_height*/35);
        font_title = std::make_shared<Font>(font_path_bold, 
                                    /*font_size*/50, 
                                    /*line_height*/55);
        font_manual = std::make_shared<Font>(font_path_italic, 
								/*font_size*/25, 
								/*line_height*/30);

        // Create texts
        description_text = std::make_shared<Text>("Do you want to shoot this vase?", 
                            /*line length*/80, 
                            /*start pos*/glm::vec2(40, 520),
                            font_body);
        choice1_text = std::make_shared<Text>("Yes", 
                            /*line length*/85, 
                            /*start pos*/glm::vec2(180, 632),
                            font_body);
        choice2_text = std::make_shared<Text>("Skip it", 
                            /*line length*/85, 
                            /*start pos*/glm::vec2(680, 632),
                            font_body);
        game_over_text = std::make_shared<Text>("Game Over", 
                            /*line length*/85, 
                            /*start pos*/glm::vec2(500, 350),
                            font_title);
        
        manual_text = std::make_shared<Text>("Press <- or -> to select, press 'return' to continue", 
						/*line length*/85, 
						/*start pos*/glm::vec2(700, 690),
						font_manual);
        inventory_bt_manual_text = std::make_shared<Text>("Show/hide inventory", 
                        /*line length*/85, 
						/*start pos*/glm::vec2(180, 680),
						font_manual);
        inventory_bt_manual_text->color = glm::vec3(0, 32.8, 73.8);
        inventory_manual_text = std::make_shared<Text>("Press <- or -> to select item, press 'return' to use", 
                        /*line length*/85, 
						/*start pos*/glm::vec2(251, 600),
						font_manual);
        inventory_manual_text->color = glm::vec3(0, 32.8, 73.8);
        inventory_bt_manual_text->hide = false;
        notification_text = std::make_shared<Text>("", 
                            /*line length*/80, 
                            /*start pos*/glm::vec2(60, 520),
                            font_body);
        texts = {description_text, choice1_text, choice2_text, game_over_text, manual_text, inventory_bt_manual_text, inventory_manual_text, notification_text};

        // Create boxes
        description_box = std::make_shared<Box>(glm::vec4(20, 480, 1260, 700), glm::u8vec4(100, 100, 100, 200));
        game_over_box = std::make_shared<Box>(glm::vec4(0, 0, 1280, 720), glm::u8vec4(0, 0, 0, 255));
        notification_box = std::make_shared<Box>(glm::vec4(40, 480, 1240, 560), glm::u8vec4(0, 0, 0, 200));
        boxes = {description_box, game_over_box, notification_box};

        // Create images
        interact_bt_img = std::make_shared<Img>(glm::vec2(800, 300), "UI/F.png");
        enter_bt_img = std::make_shared<Img>(choice_pos[0], "UI/return.png");
        inventory_bt_img = std::make_shared<Img>(glm::vec2(100, 640), "UI/E.png");
        inventory_bt_img->hide = false;
        alarm_img = std::make_shared<Img>(glm::vec2(600, 300), "UI/!.png");

        glm::uvec2 size;
        std::shared_ptr<std::vector< glm::u8vec4 > > data = std::make_shared<std::vector< glm::u8vec4 >>();
        load_png(data_path("UI/Slot.png"), &size, data.get(), UpperLeftOrigin);
        slot_left_img = std::make_shared<Img>(choice_pos[0], size, data);
        slot_right_img = std::make_shared<Img>(choice_pos[1], size, data);
        inventory_img = std::make_shared<Img>(glm::vec2(0, 592), "UI/inventory.png");
        imgs= {slot_left_img, slot_right_img, inventory_bt_img, interact_bt_img, enter_bt_img, inventory_img, alarm_img};
        

        // Create slots for inventory
        inventory_slot_id_start = (uint32_t)imgs.size();
        for (uint32_t i = 0; i < inventory_slot_num; i++) {
            glm::vec2 pos = glm::vec2(260 + 78 * i, 626);
            item_pos.push_back(pos);
            imgs.push_back(std::make_shared<Img>(pos, size, data));
        }
        slot_selected_img = std::make_shared<Img>(item_pos[inventory_slot_selected_id], "UI/slot_selected.png");
        imgs.push_back(slot_selected_img);

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

    void update_choice(bool left);

    void show_description(std::string description, std::string choice1 = "", std::string choice2 = "");
    void hide_description();

    void show_game_over(bool won);

    void hide_all();
    void reset();

    void set_interactable_button(bool hide);
    void set_inventory_button(bool hide);

    void set_inventory(bool hide);
    void update_inventory_selection(bool left);

    // Callback functions for left and right arrow keys
    void arrow_key_callback(bool left);

    void add_inventory_item(std::string item_name, std::string img_path);
    void remove_inventory_item(); // remove selected inventory item
    std::string get_selected_inventory_item_name();

    void show_notification(std::string notification);
    void hide_notification();

    void show_alarm();
};
