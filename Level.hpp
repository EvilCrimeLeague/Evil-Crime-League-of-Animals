#pragma once

#include "Scene.hpp"
#include "Ray.hpp"
#include "WalkMesh.hpp"
#include "UI.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "data_path.hpp"
#include "LitColorTextureProgram.hpp"
#include "TextTextureProgram.hpp"
#include "Driver.hpp"
#include "Sound.hpp"

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
		std::string interaction_description = "";
		std::vector<std::string> interaction_choices = {};
		std::string inventory_description = "";
		std::vector<std::string> inventory_choices = {};
        std::string img_path;
        bool show_description_box = false;
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
    std::unordered_map<std::string, bool> guard_detectables;
    std::shared_ptr<Item> curr_item;

    // scene
    // local copy of the game scene (so code can change it during gameplay):
    Scene scene;
    const WalkMesh* walkmesh;

    Scene::Camera *camera = nullptr;
    Scene::Transform *player_transform = nullptr;
    glm::vec3 player_spawn_point;
    Scene::Transform *target_transform = nullptr;

    // ui
    std::shared_ptr<UI> ui;

    // guards
    std::vector<std::shared_ptr<GuardDog> > guard_dogs;

    // animation
    std::vector<std::shared_ptr<Driver> > drivers;

    Level(std::shared_ptr<UI> ui_);
    virtual ~Level() {}

    std::shared_ptr<Item> get_closest_item(glm::vec3 player_position);

    virtual void handle_enter_key() = 0;
    virtual void handle_interact_key() = 0;
    virtual void update() = 0; // any update particular to that level
    
    void update_guard_detection();
    void update_player_dist_infront();
    void update_animation(const float deltaTime);

    virtual void restart() = 0;
    std::shared_ptr< Sound::PlayingSample > rolling_loop;
    float closest_dist_infront;
};