#pragma once

#include "Level.hpp"

struct Level0 : Level { // A level that displays the bronze head player has collected
    Scene::Transform *head_dragon = nullptr;
	Scene::Transform *head_dog = nullptr;
	Scene::Transform *head_chicken = nullptr;
	Scene::Transform *head_snake = nullptr;
    Scene::Transform *head_sheep = nullptr;

    std::vector<Scene::Transform *> heads;

    Level0(std::shared_ptr<UI> ui_, std::shared_ptr<GameInfo> info_);
    virtual ~Level0() {}

    void handle_enter_key();
    void handle_interact_key();
    void handle_numeric_key(uint32_t key) {}
    void handle_inventory_choice(uint32_t choice_id) {}
    void handle_description_choice(uint32_t choice_id) {}
    void update();

    void restart();
    void exit();
};