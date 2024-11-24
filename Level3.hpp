#pragma once

#include "Level.hpp"

struct Level3 : Level {
    Scene::Transform *head = nullptr;

    Scene::Transform *diamond = nullptr;
    Scene::Transform *quartz = nullptr;
    Scene::Transform *corundum = nullptr;
    Scene::Transform *diamond_podium = nullptr;
    Scene::Transform *quartz_podium = nullptr;
    Scene::Transform *corundum_podium = nullptr;
    std::map<std::string, int> gem_to_podium;;
    std::vector<Scene::Transform *> podiums;
    std::vector<bool> podium_occupied;

    Scene::Transform *paper_scale = nullptr;
    Scene::Transform *paper_passwd = nullptr;

    Scene::Transform *control_panel = nullptr;
    std::shared_ptr<UI::Box> control_panel_box = nullptr;
    std::shared_ptr<Text> control_panel_text = nullptr;
    std::vector<std::shared_ptr<UI::Img> > control_panel_slots = {};
    std::vector<std::shared_ptr<UI::Img> > control_panel_inputs = {};
    bool showing_control_panel = false;
    const std::string password = "153241";
    std::string player_input = "";

    const float guard_dog_speed = 1.0f;

    // UI

    Level3(std::shared_ptr<UI> ui_, std::shared_ptr<GameInfo> info_);
    virtual ~Level3() {}

    void handle_enter_key();
    void handle_interact_key();
    void handle_numeric_key(uint32_t key);
    void handle_inventory_choice(uint32_t choice_id);
    void handle_description_choice(uint32_t choice_id);
    void update();

    void restart();

    void hide_control_panel();
};