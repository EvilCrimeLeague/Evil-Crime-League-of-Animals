#include "Level1.hpp"
#include "Sound.hpp"

#include <iostream>

Load< Sound::Sample > collect_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("collect.wav"));
});

Load< Sound::Sample > pop_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("pop.wav"));
});

Load< Sound::Sample > rolling_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("rolling.wav"));
});

GLuint level1_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > level1_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("level1.pnct"));
	level1_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > level1_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("level1.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = level1_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = level1_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

		// if this doenst work, ask on discord, ping Jim
		drawable.meshBuffer = &(*level1_meshes);
		drawable.mesh = &mesh;

	});
});

Load< WalkMesh > level1_walkmesh(LoadTagDefault, []() -> WalkMesh const * {
	WalkMeshes *ret = new WalkMeshes(data_path("level1.w"));
	WalkMesh const *walkmesh = &ret->lookup("WalkMesh");
	return walkmesh;
});

Load< Animation > level1_animations(LoadTagDefault, []() -> Animation const * {
	Animation *anim = new Animation(data_path("level1.anim"));
	return anim;
});

Level1::Level1(std::shared_ptr<UI> ui_): Level(ui_) {
    scene = *level1_scene;
    walkmesh = level1_walkmesh;

    for (auto &transform : scene.transforms) {
        if (transform.name == "RedPanda") player_transform = &transform;
        else if (transform.name == "Jewel") target_transform = &transform;
		else if (transform.name == "Bone") bone = &transform;
		else if (transform.name == "GuardDog") guardDog = &transform;
		else if (transform.name == "FOV") fov = &transform;
        else if (transform.name == "Vase.001") vase = &transform;
        else if (transform.name == "Painting.002") painting_1 = &transform;
        else if (transform.name == "Painting.003") painting_2 = &transform;
	}

    if (target_transform == nullptr) throw std::runtime_error("Target not found.");
    else if (player_transform == nullptr) throw std::runtime_error("Player not found.");
    else if (bone == nullptr) throw std::runtime_error("Bone not found.");
	else if (guardDog == nullptr) throw std::runtime_error("GuardDog not found.");
	else if (fov == nullptr) throw std::runtime_error("FOV not found.");
	std::cout<<guardDog->position.y - fov->position.y<<std::endl;

    // fov->parent = guardDog;

    player_spawn_point = player_transform->position;

    if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
    camera = &scene.cameras.front();
    guard_detectables["Floor"] = false;

    // initialize items
    auto bone_ptr = std::make_shared<Item>();
    bone_ptr->name = "Bone";
    bone_ptr->interaction_description = "Collect it";
    bone_ptr->inventory_description = "Do you want to use the bone to distract the guard?";
    bone_ptr->inventory_choices = {"Yes", "No"};
    bone_ptr->transform = bone;
    bone_ptr->img_path = "UI/bone.png";
    bone_ptr->spawn_point = bone->position;
    guard_detectables["Bone"] = false;
    items["Bone"] = bone_ptr;

    auto vase_ptr = std::make_shared<Item>();
    vase_ptr->name = "Vase";
    vase_ptr->interaction_description = "It's a bizarre vase. You may want to leave it alone.";
    bone_ptr->inventory_choices = {};
    vase_ptr->transform = vase;
    vase_ptr->show_description_box = true;
    vase_ptr->spawn_point = vase->position;
    items["Vase"] = vase_ptr;

    auto painting_1_ptr = std::make_shared<Item>();
    painting_1_ptr->name = "Painting1";
    painting_1_ptr->interaction_description = "It's an unhappy face.";
    painting_1_ptr->transform = painting_1;
    painting_1_ptr->show_description_box = true;
    painting_1_ptr->spawn_point = painting_1->position;
    items["Painting1"] = painting_1_ptr;

    auto painting_2_ptr = std::make_shared<Item>();
    painting_2_ptr->name = "Painting2";
    painting_2_ptr->interaction_description = "It's a crying face.";
    painting_2_ptr->transform = painting_2;
    painting_2_ptr->show_description_box = true;
    painting_2_ptr->spawn_point = painting_2->position;
    items["Painting2"] = painting_2_ptr;
    
    // initialize guard dogs
    auto guardDog_ptr = std::make_shared<GuardDog>();
    guardDog_ptr->name = "GuardDog";
    guardDog_ptr->transform = guardDog;
    guardDog_ptr->spawn_point = guardDog->position;
    guardDog_ptr->fov = fov;
    guardDog_ptr->fov_spawn_point = fov->position;
    guard_dogs.push_back(guardDog_ptr);

    // initialize drivers
    driver_guardDog_walk = std::make_shared<Driver>("GuardDog-walk", CHANEL_TRANSLATION);
    driver_guardDog_walk->transform = guardDog;
    driver_guardDog_walk->loop = false;
    drivers.push_back(driver_guardDog_walk);

    driver_guardDog_rotate = std::make_shared<Driver>(level1_animations->animations.at("GuardDog-rotation"));
    driver_guardDog_rotate->transform = guardDog;
    driver_guardDog_rotate->start();
    drivers.push_back(driver_guardDog_rotate);

    driver_bone_move = std::make_shared<Driver>("Bone-move", CHANEL_TRANSLATION);
    driver_bone_move->transform = bone;
    driver_bone_move->loop = false;
    drivers.push_back(driver_bone_move);

    driver_bone_rotate = std::make_shared<Driver>(level1_animations->animations.at("Bone-rotation"));
    driver_bone_rotate->transform = bone;
    drivers.push_back(driver_bone_rotate);

    driver_fov_move = std::make_shared<Driver>("FOV-move", CHANEL_TRANSLATION);
    driver_fov_move->transform = fov;
    driver_fov_move->loop = false;
    drivers.push_back(driver_fov_move);

    // sound
    rolling_loop = Sound::loop(*rolling_sample, 0.07f, 0.0f);
    rolling_loop->paused = true;
}

void Level1::handle_enter_key() {
    if(ui->showing_inventory_description) {
        // Interact with inventory item
        handle_inventory_choice(ui->choice_id);
    } 
    else if (ui->showing_description) {
        // Interact with item
        if(ui->showing_choices) {
            handle_description_choice(ui->choice_id);
        } else {
            ui->hide_description();
        }
    } 
    else if (ui->showing_inventory && ui->inventory_items.size() > 0) {
        std::string item_name = ui->get_selected_inventory_item_name();
        Sound::play(*pop_sample, 0.1f, 0.0f);
        if(item_name == "Bone") {
            ui->show_description(items[item_name]->inventory_description, items[item_name]->inventory_choices[0], items[item_name]->inventory_choices[1]);
            ui->showing_inventory_description = true;
        }
        
    }
}

void Level1::handle_interact_key() {
    if(ui->showing_interactable_button) {
        // ui->show_description(curr_item->interaction_description, curr_item->interaction_choices[0], curr_item->interaction_choices[1]);
        if(curr_item->name == "Bone") {
            driver_bone_move->stop();
            driver_bone_rotate->stop();
            ui->add_inventory_item(curr_item->name, curr_item->img_path);
            // hide item
            curr_item->transform->position.x = -1000.0f;
            Sound::play(*pop_sample, 0.05f, 0.0f);
        } else {
            // show description box
            if(curr_item->interaction_choices.size() > 0) {
                // show choices
                ui->show_description(curr_item->interaction_description, curr_item->interaction_choices[0], curr_item->interaction_choices[1]);
            } else {
                // do not show choices since there are none
                ui->show_description(curr_item->interaction_description);
            }
        }
    }
}

void Level1::handle_inventory_choice(uint32_t choice_id) {
    ui->hide_description();
    if(ui->choice_id == 0) {
        // use item
        std::string item_name = ui->get_selected_inventory_item_name();
        ui->remove_inventory_item();
        if(item_name == "Bone") {
            rolling_loop->paused = false;
            // create bone move animation
		    glm::vec3 playerDirectionWorld = glm::normalize(player_transform->make_local_to_world() * glm::vec4(-1.0, 0.0, 0.0, 0.0));
            glm::vec3 bone_target_pos = player_transform->position + playerDirectionWorld*2.0f + glm::vec3(0,0,0.75);
            driver_bone_move->clear();
            driver_bone_move->add_walk_in_straight_line_anim(player_transform->position, bone_target_pos, 3.0f, 5);

            // reset bone rotation animation
            driver_bone_rotate->values4d = level1_animations->animations.at("Bone-rotation").values4d;
            bone->rotation = player_transform->rotation;
            for(int i=0; i<driver_bone_rotate->values4d.size(); i++){
                driver_bone_rotate->values4d[i] = bone->rotation * driver_bone_rotate->values4d[i];
            }

            driver_bone_move->restart();
            driver_bone_rotate->restart();
        }
    } 
    // show inventory again
    ui->set_inventory(false);
    ui->showing_inventory_description = false;
}

void Level1::handle_description_choice(uint32_t choice_id) {
    // ui->hide_description();
    // ui->set_inventory_button(/*hide=*/false);
    // if(ui->choice_id ==0) {
    //     ui->add_inventory_item(curr_item->name, curr_item->img_path);
    //     // hide item
    //     curr_item->transform->position.x = -1000.0f;
    // } else {
    //     // show iteractable button again
    //     ui->set_interactable_button(/*hide=*/false);
    // }
}

void Level1::restart() {
    for(auto &item: items) {
        item.second->transform->position = item.second->spawn_point;
    }

    for(auto &guard_dog: guard_dogs) {
        guard_dog->transform->position = guard_dog->spawn_point;
        guard_dog->fov->position = guard_dog->fov_spawn_point;
    }

    for(auto &driver: drivers) {
        driver->restart();
        driver->stop();
    }

    for(auto &item: guard_detectables) {
        item.second = false;
    }

    driver_guardDog_walk->clear();
    driver_fov_move->clear();

    driver_guardDog_rotate->start();

    driver_bone_move->clear();

    driver_bone_rotate->values4d = level1_animations->animations.at("Bone-rotation").values4d;

    fov->position.x = guardDog->position.x;
    fov->position.y = guardDog->position.y;

    rolling_loop = Sound::loop(*rolling_sample, 0.07f, 0.0f);
    rolling_loop->paused = true;

}

void Level1::update() {
    // Field of view collisions
    update_guard_detection();
    update_player_dist_infront();
    if(guard_detectables["RedPanda"] || driver_guardDog_walk->playing) {
        // stop guard animation
        driver_guardDog_rotate->stop();
    } else {
        driver_guardDog_rotate->start();
    }
    if (guard_detectables["Bone"]) {
        float dist = glm::distance(guardDog->position, bone->position);
        if(dist > 1.25f) {
            driver_guardDog_rotate->stop();
            glm::vec3 guardDirectionWorld = glm::normalize(guardDog->make_local_to_world() * glm::vec4(-1.0, 0.0, 0.0, 0.0));
            driver_guardDog_walk->clear();
            float duration = dist/guard_dog_speed;
            driver_guardDog_walk->add_walk_in_straight_line_anim(guardDog->position, bone->position - guardDirectionWorld, duration, std::max(static_cast<int>(duration),1));
            driver_guardDog_walk->restart();
            driver_fov_move->clear();
            driver_fov_move->add_walk_in_straight_line_anim(fov->position, bone->position - guardDirectionWorld, duration, std::max(static_cast<int>(duration),1));
            driver_fov_move->restart();
        } 
        if(dist <= 1.25f && driver_guardDog_walk->playing) {
            // stop guard when close to bone
            driver_guardDog_walk->stop();
            driver_fov_move->stop();
            driver_bone_move->stop();
            driver_bone_move->finished = true;
            driver_bone_rotate->stop();
        }
        
    }
    
    // animation
    if(driver_bone_move->finished) {
        driver_bone_rotate->stop();
        rolling_loop->paused = true;
    }
}