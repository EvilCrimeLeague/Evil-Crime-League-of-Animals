#pragma once

#include "Level.hpp"

struct Level1 : Level {
    Scene::Transform *head = nullptr;
	Scene::Transform *guardDog = nullptr;
	Scene::Transform *bone = nullptr;
	Scene::Transform *fov = nullptr;

    Scene::Transform *vase = nullptr;
    Scene::Transform *painting_1 = nullptr;
    Scene::Transform *painting_2 = nullptr;
    Scene::Transform *shell = nullptr;

    std::shared_ptr<Driver> driver_guardDog_walk = nullptr;
    std::shared_ptr<Driver> driver_fov_move = nullptr;
    std::shared_ptr<Driver> driver_guardDog_rotate = nullptr;
    std::shared_ptr<Driver> driver_fov_rotate = nullptr;
    std::shared_ptr<Driver> driver_bone_rotate = nullptr;
    std::shared_ptr<Driver> driver_bone_move = nullptr;

    const float guard_dog_speed = 1.0f;

    Level1(std::shared_ptr<UI> ui_, std::shared_ptr<GameInfo> info_);
    virtual ~Level1() {}

    void handle_enter_key();
    void handle_interact_key();
    void handle_numeric_key(uint32_t key) {}
    void handle_inventory_choice(uint32_t choice_id);
    void handle_description_choice(uint32_t choice_id);
    float update_bone_dist_infront();
    void update();

    void restart();
    void exit();
};
