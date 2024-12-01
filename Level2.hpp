#pragma once

#include <unordered_set>
#include "Level.hpp"

struct Level2 : Level {
    Scene::Transform *head = nullptr;
	Scene::Transform *guardDog_1 = nullptr;
    Scene::Transform *guardDog_2 = nullptr;
	Scene::Transform *fov = nullptr;

    Scene::Transform *vase = nullptr;
    Scene::Transform *painting_1 = nullptr;
    Scene::Transform *painting_2 = nullptr;
    Scene::Transform *painting_3 = nullptr;
    Scene::Transform *painting_4 = nullptr;
    Scene::Transform *painting_5 = nullptr;
    Scene::Transform *painting_6 = nullptr;
    Scene::Transform *painting_7 = nullptr;
    Scene::Transform *painting_8 = nullptr;
    Scene::Transform *painting_9 = nullptr;
    Scene::Transform *painting_10 = nullptr;
    Scene::Transform *painting_11 = nullptr;
    Scene::Transform *painting_12 = nullptr;
    Scene::Transform *control_panel = nullptr;
    Scene::Transform *paper_1 = nullptr;
    Scene::Transform *paper_2 = nullptr;
    Scene::Transform *brochure = nullptr;
    Scene::Transform *fov_1 = nullptr;
    Scene::Transform *fov_2 = nullptr;

    Scene::Transform *laser_1 = nullptr;
    Scene::Transform *laser_2 = nullptr;
    Scene::Transform *laser_3 = nullptr;
    Scene::Transform *laser_4 = nullptr;
    Scene::Transform *laser_5 = nullptr;
    Scene::Transform *laser_6 = nullptr;
    Scene::Transform *laser_7 = nullptr;
    Scene::Transform *laser_8 = nullptr;
    Scene::Transform *laser_9 = nullptr;

    Scene::Transform *podium_1 = nullptr;
    Scene::Transform *podium_2 = nullptr;
    Scene::Transform *podium_3 = nullptr;
    Scene::Transform *podium_4 = nullptr;
    Scene::Transform *podium_5 = nullptr;
    Scene::Transform *podium = nullptr;

    std::shared_ptr<Driver> driver_guardDog1_walk = nullptr;
    std::shared_ptr<Driver> driver_guardDog1_rotate = nullptr;
    std::shared_ptr<Driver> driver_guardDog2_walk = nullptr;
    std::shared_ptr<Driver> driver_guardDog2_rotate = nullptr;
    // std::shared_ptr<Driver> driver_fov_move = nullptr;

    const float guard_dog_speed = 1.0f;

    // UI
    std::shared_ptr<UI::Box> control_panel_box = nullptr;
    std::shared_ptr<Text> control_panel_text = nullptr;
    std::vector<std::shared_ptr<UI::Img> > control_panel_slots = {};
    std::vector<std::shared_ptr<UI::Img> > control_panel_inputs = {};
    bool showing_control_panel = false;
    const std::string password = "386743";
    std::string player_input = "";

    // Achievement paintings
    std::unordered_set<std::string> paintings_seen;

    Level2(std::shared_ptr<UI> ui_, std::shared_ptr<GameInfo> info_);
    virtual ~Level2() {}

    void handle_enter_key();
    void handle_interact_key();
    void handle_numeric_key(uint32_t key);
    void handle_inventory_choice(uint32_t choice_id);
    void handle_description_choice(uint32_t choice_id);
    void update();

    void restart();

    void hide_control_panel();
};