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

struct GameInfo {
    std::string file_path;
    uint32_t highest_level;
    std::vector<uint32_t> targets_obtained; //bronze head obtained: dog, dragon, chicken, sheep, snake

    GameInfo() {
        file_path = data_path("game.info");
        // if file does not exist, create it
        std::ifstream file(file_path, std::ios::binary);
        std::vector<uint32_t> highest_level_v;
        if(!file.good()) {
            std::ofstream new_file(file_path, std::ios::binary);
            highest_level_v = {0};
            highest_level = 0;
            targets_obtained = {0, 0, 0, 0, 0};
            write_chunk("int0", highest_level_v, &new_file);
            write_chunk("bool", targets_obtained, &new_file);
            new_file.close();
        } else {
            read_chunk(file, "int0", &highest_level_v);
            highest_level = highest_level_v[0];
            read_chunk(file, "bool", &targets_obtained);
        }
	    file.close();
    }

    void update_game_info() {
        std::ofstream file(file_path, std::ios::binary);
        std::vector<uint32_t> highest_level_v = {highest_level};
        write_chunk("int0", highest_level_v, &file);
        write_chunk("bool", targets_obtained, &file);
        file.close();
    }

    void update_target_obtained(std::vector<uint32_t>& level_targets) {
        for(uint32_t i = 0; i < level_targets.size(); i++) {
            if(level_targets[i]>0) {
                targets_obtained[i] = level_targets[i];
            }
        }
    }
};

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

    struct Laser {
        std::string name;
        Scene::Transform *transform;
        glm::vec3 spawn_point;
        bool on = true;
        float timer = 0;
        float target_time = 0;
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
    Scene::Transform *exit_transform = nullptr;

    std::vector<uint32_t> level_targets = {0, 0, 0, 0, 0};
    bool is_target_obtained();

    // ui
    std::shared_ptr<UI> ui;

    // guards
    std::vector<std::shared_ptr<GuardDog> > guard_dogs;
    
    // lasers
    std::vector<std::shared_ptr<Laser>> lasers;

    // fovs
    MeshBuffer *guard_fov_meshes = nullptr;
    Scene::Transform *guard_fov_transform = nullptr;


    // animation
    std::vector<std::shared_ptr<Driver> > drivers;
    std::shared_ptr<Driver> driver_rope_descend = nullptr;
    std::shared_ptr<Driver> driver_rope_ascend = nullptr;
    std::shared_ptr<Driver> driver_player_ascend = nullptr;
    float rope_move_time = 3.0f;

    // game info
    std::shared_ptr<GameInfo> info;

    Level(std::shared_ptr<UI> ui_, std::shared_ptr<GameInfo> info_);
    virtual ~Level() {}

    std::shared_ptr<Item> get_closest_item(glm::vec3 player_position);

    virtual void handle_enter_key() = 0;
    virtual void handle_interact_key() = 0;
    virtual void handle_numeric_key(uint32_t key) = 0;
    virtual void update() = 0; // any update particular to that level

    std::vector< Vertex > guard_fov_data;
    
    void update_guard_detection();
    void update_guard_fov();
    void update_animation(const float deltaTime);

    bool check_laser_hits();
    bool disable_lasers = false;
    void move_lasers();

    virtual void restart() = 0;
    std::shared_ptr< Sound::PlayingSample > rolling_loop;
    float closest_dist_infront = 5.0f;

    void exit();
    bool is_exit_finished();
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

GLint gen_texture_from_img(const std::string img_path);