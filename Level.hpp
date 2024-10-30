#pragma once

#include "Scene.hpp"
#include "Ray.hpp"
#include "UI.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

struct Level {
    struct Item {
		// Interactable item
		std::string name;
		Scene::Transform * transform;
		glm::vec3 spawn_point;
		std::string interaction_description;
		std::vector<std::string> interaction_choices;
		std::string inventory_description;
		std::vector<std::string> inventory_choices;
        std::string img_path;
	};

    struct GuardDog {
		// Interactable item
		std::string name;
		Scene::Transform * transform;
		glm::vec3 spawn_point;
        Scene::Transform * fov;
        glm::vec3 fov_spawn_point;
	};

    const float interactable_distance = 2.0f;
    std::unordered_map<std::string, std::shared_ptr<Item> > items;
    std::shared_ptr<Item> curr_item;

    // scene information
    // local copy of the game scene (so code can change it during gameplay):
    Scene scene;

    Scene::Camera *camera = nullptr;
    Scene::Transform *player_transform = nullptr;
    Scene::Transform *target_transform = nullptr;

    std::shared_ptr<UI> ui;

    std::vector<std::shared_ptr<GuardDog> > guard_dogs;

    Level(Scene const * scene_, std::shared_ptr<UI> ui_);
    virtual ~Level() {}

    std::shared_ptr<Item> get_closest_item(glm::vec3 player_position);

    virtual void handle_enter_key() = 0;
    virtual void handle_interact_key() = 0;

    bool update_guard(); // return seen_by_guard

    virtual void restart() = 0;
};