#pragma once

#include "Level.hpp"

struct Level2 : Level {
	Scene::Transform *guardDog = nullptr;
	Scene::Transform *fov = nullptr;

    Scene::Transform *vase = nullptr;
    Scene::Transform *painting_1 = nullptr;
    Scene::Transform *painting_2 = nullptr;

    std::shared_ptr<Driver> driver_guardDog_walk = nullptr;
    std::shared_ptr<Driver> driver_fov_move = nullptr;

    const float guard_dog_speed = 1.0f;

    Level2(std::shared_ptr<UI> ui_);
    virtual ~Level2() {}

    void handle_enter_key();
    void handle_interact_key();
    void handle_inventory_choice(uint32_t choice_id);
    void handle_description_choice(uint32_t choice_id);
    void update();

    void restart();
};