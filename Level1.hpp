#pragma once

#include "Level.hpp"

struct Level1 : Level {
	Scene::Transform *guardDog = nullptr;
	Scene::Transform *bone = nullptr;
	Scene::Transform *fov = nullptr;

    std::shared_ptr<Driver> driver_guardDog_walk = nullptr;
    std::shared_ptr<Driver> driver_guardDog_rotate = nullptr;

    Level1(std::shared_ptr<UI> ui_);
    virtual ~Level1() {}

    void handle_enter_key();
    void handle_interact_key();
    void handle_inventory_choice(uint32_t choice_id);
    void handle_description_choice(uint32_t choice_id);

    void restart();
};