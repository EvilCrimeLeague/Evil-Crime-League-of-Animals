#pragma once

#include "Level.hpp"

struct Level3 : Level {
    Scene::Transform *head = nullptr;

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
};