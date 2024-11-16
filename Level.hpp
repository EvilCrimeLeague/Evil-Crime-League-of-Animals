#pragma once

#include "Scene.hpp"
#include "Ray.hpp"
#include "WalkMesh.hpp"
#include "UI.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "data_path.hpp"
#include "read_write_chunk.hpp"
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
        std::string img_path; //inventory slot mini image 64x64
        std::string description_img_path; //description image
        std::shared_ptr<UI::Img> img = nullptr;
        bool show_description_box = false;
        bool added = false;//added to inventory
        bool interactable = true;
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
    glm::vec3 camera_spawn_point;
    Scene::Transform *player_transform = nullptr;
    glm::vec3 player_spawn_point;
    glm::quat player_spawn_rotation;
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
    virtual void handle_numeric_key(uint32_t key) = 0;
    virtual void update() = 0; // any update particular to that level
    
    void update_guard_detection();
    void update_guard_fov();
    void update_animation(const float deltaTime);

    virtual void restart() = 0;
    std::shared_ptr< Sound::PlayingSample > rolling_loop;
    float closest_dist_infront = 5.0f;
};

struct GameInfo {
    std::string file_path;
    uint32_t highest_level;

    GameInfo() {
        file_path = data_path("game.info");
        // if file does not exist, create it
        std::ifstream file(file_path, std::ios::binary);
        std::vector<uint32_t> highest_level_v;
        if(!file.good()) {
            std::ofstream new_file(file_path, std::ios::binary);
            highest_level_v = {0};
            highest_level = 0;
            write_chunk("int0", highest_level_v, &new_file);
            new_file.close();
        } else {
            read_chunk(file, "int0", &highest_level_v);
            highest_level = highest_level_v[0];
        }
	    file.close();
    }

    void update_highest_level(int level) {
        highest_level = level;
        std::ofstream file(file_path, std::ios::binary);
        std::vector<uint32_t> highest_level_v = {highest_level};
        write_chunk("int0", highest_level_v, &file);
        file.close();
    }

};

struct Triangle {
    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 c;
    std::string name;
};

extern Load< Sound::Sample > collect_sample;
extern Load< Sound::Sample > pop_sample;
extern Load< Sound::Sample > rolling_sample;