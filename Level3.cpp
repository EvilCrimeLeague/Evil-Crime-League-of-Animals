#include "Level3.hpp"

#include <iostream>


GLuint level3_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > level3_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("level3.pnct"));
	level3_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > level3_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("level3.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = level3_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = level3_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

		// if this doenst work, ask on discord, ping Jim
		drawable.meshBuffer = &(*level3_meshes);
		drawable.mesh = &mesh;

	});
});

Load< WalkMesh > level3_walkmesh(LoadTagDefault, []() -> WalkMesh const * {
	WalkMeshes *ret = new WalkMeshes(data_path("level3.w"));
	WalkMesh const *walkmesh = &ret->lookup("WalkMesh");
	return walkmesh;
});

// Load< Animation > level3_animations(LoadTagDefault, []() -> Animation const * {
// 	Animation *anim = new Animation(data_path("level3.anim"));
// 	return anim;
// });

Level3::Level3(std::shared_ptr<UI> ui_, std::shared_ptr<GameInfo> info_): Level(ui_, info_) {
    scene = *level3_scene;
    walkmesh = level3_walkmesh;

    for (auto &transform : scene.transforms) {
        if (transform.name == "RedPanda") player_transform = &transform;
        else if (transform.name == "Rope") exit_transform = &transform;
        else if (transform.name == "Diamond") diamond = &transform;
        else if (transform.name == "Quartz") quartz = &transform;
        else if (transform.name == "Corundum") corundum = &transform;
        else if (transform.name == "DiamondPodium") diamond_podium = &transform;
        else if (transform.name == "QuartzPodium") quartz_podium = &transform;
        else if (transform.name == "CorundumPodium") corundum_podium = &transform;
        else if (transform.name == "Paper") paper_passwd = &transform;
        else if (transform.name == "Paper.001") paper_scale = &transform;
	}

    if (player_transform == nullptr) throw std::runtime_error("Player not found.");
    else if (exit_transform == nullptr) throw std::runtime_error("Exit not found."); 
    else if (diamond == nullptr) throw std::runtime_error("Diamond not found.");
    else if (quartz == nullptr) throw std::runtime_error("Quartz not found.");
    else if (corundum == nullptr) throw std::runtime_error("Corundum not found.");
    else if (diamond_podium == nullptr) throw std::runtime_error("DiamondPodium not found.");
    else if (quartz_podium == nullptr) throw std::runtime_error("QuartzPodium not found.");
    else if (corundum_podium == nullptr) throw std::runtime_error("CorundumPodium not found.");
    else if (paper_passwd == nullptr) throw std::runtime_error("Paper password not found.");
    else if (paper_scale == nullptr) throw std::runtime_error("Paper scale not found.");

    player_spawn_point = player_transform->position;
    player_spawn_rotation = player_transform->rotation;

    if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
    camera = &scene.cameras.front();
    camera_spawn_point = camera->transform->position;
    guard_detectables["Wall"] = false;

    podiums = {diamond_podium, quartz_podium, corundum_podium};

    // initialize items
    auto diamond_ptr = std::make_shared<Item>();
    diamond_ptr->name = "Diamond";
    diamond_ptr->interaction_description = "Pick it up";
    diamond_ptr->transform = diamond;
    diamond_ptr->img_path = "UI/Level3/diamond.png";
    diamond_ptr->inventory_description = "This is a diamond. Do you want to place it here?";
    diamond_ptr->inventory_choices = {"Yes", "No"};
    diamond_ptr->spawn_point = diamond->position;
    items[diamond_ptr->name] = diamond_ptr;

    auto quartz_ptr = std::make_shared<Item>();
    quartz_ptr->name = "Quartz";
    quartz_ptr->interaction_description = "Pick it up";
    quartz_ptr->transform = quartz;
    quartz_ptr->img_path = "UI/Level3/quartz.png";
    quartz_ptr->inventory_description = "This is a quartz. Do you want to place it here?";
    quartz_ptr->inventory_choices = {"Yes", "No"};
    quartz_ptr->spawn_point = quartz->position;
    items[quartz_ptr->name] = quartz_ptr;

    auto corundum_ptr = std::make_shared<Item>();
    corundum_ptr->name = "Corundum";
    corundum_ptr->interaction_description = "Pick it up";
    corundum_ptr->transform = corundum;
    corundum_ptr->img_path = "UI/Level3/corundum.png";
    corundum_ptr->inventory_description = "This is a corundum. Do you want to place it here?";
    corundum_ptr->inventory_choices = {"Yes", "No"};
    corundum_ptr->spawn_point = corundum->position;
    items[corundum_ptr->name] = corundum_ptr;

    auto paper_scale_ptr = std::make_shared<Item>();
    paper_scale_ptr->name = "PaperScale";
    paper_scale_ptr->interaction_description = "Pick it up";
    paper_scale_ptr->transform = paper_scale;
    paper_scale_ptr->img_path = "UI/Level3/paper_scale.png";
    paper_scale_ptr->description_img_path = "UI/Level3/hardness_scale.png";
    paper_scale_ptr->spawn_point = paper_scale->position;
    items[paper_scale_ptr->name] = paper_scale_ptr;

    auto paper_passwd_ptr = std::make_shared<Item>();
    paper_passwd_ptr->name = "PaperPasswd";
    paper_passwd_ptr->interaction_description = "Pick it up";
    paper_passwd_ptr->transform = paper_passwd;
    paper_passwd_ptr->img_path = "UI/Level3/paper_passwd.png";
    paper_passwd_ptr->description_img_path = "UI/Level3/password.png";
    paper_passwd_ptr->spawn_point = paper_passwd->position;
    items[paper_passwd_ptr->name] = paper_passwd_ptr;
    
    // initialize guard dogs

    // initialize animation drivers
    driver_rope_descend = std::make_shared<Driver>("Rope-descend", CHANEL_TRANSLATION);
    driver_rope_descend->transform = exit_transform;
    glm::vec3 rope_up_pos = exit_transform->position;
    rope_up_pos.z = 5.0f;
    glm::vec3 rope_down_pos = rope_up_pos;
    rope_down_pos.z = 0.0f;
    driver_rope_descend->add_move_in_straight_line_anim(rope_up_pos, rope_down_pos, 5.0f, 3);
    driver_rope_descend->loop = false;
    drivers.push_back(driver_rope_descend);

    driver_rope_ascend = std::make_shared<Driver>("Rope-ascend", CHANEL_TRANSLATION);
    driver_rope_ascend->transform = exit_transform;
    driver_rope_ascend->add_move_in_straight_line_anim(rope_down_pos, rope_up_pos, 5.0f, 3);
    driver_rope_ascend->loop = false;
    drivers.push_back(driver_rope_ascend);

    driver_player_ascend = std::make_shared<Driver>("Player-ascend", CHANEL_TRANSLATION);
    driver_player_ascend->transform = player_transform;
    driver_player_ascend->loop = false;
    drivers.push_back(driver_player_ascend);

    // sound
}

void Level3::handle_enter_key() {
    if(ui->showing_inventory_description) {
        handle_inventory_choice(ui->choice_id);
    } else if (ui->showing_image) {
        ui->hide_img();
    } else if (ui->showing_description) {
        // Interact with item
        if(ui->showing_choices) {
            handle_description_choice(ui->choice_id);
        } else {
            ui->hide_description();
        }
    } 
    else if (ui->showing_inventory && ui->inventory_slot_selected_id < ui->inventory_items.size()) {
        Sound::play(*pop_sample, 0.1f, 0.0f);
        std::string item_name = ui->get_selected_inventory_item_name();
        ui->handle_inventory_selection(items[item_name]->inventory_description, items[item_name]->inventory_choices);
        
    }
}

void Level3::handle_interact_key() {
    if(ui->showing_interactable_button) {
        if (curr_item->name == "Head") {
            ui->add_inventory_item(curr_item->name, curr_item->img_path);
            curr_item->transform->position.x = -1000.0f;
            Sound::play(*pop_sample, 0.05f, 0.0f);
            level_targets[1] = 1;
            driver_rope_descend->start();
        } else if (curr_item->name == "Diamond" || curr_item->name == "Quartz" || curr_item->name == "Corundum") {
            if(gem_to_podium.find(curr_item->name) != gem_to_podium.end()) {
                // remove from podium
                podium_occupied[gem_to_podium[curr_item->name]] = false;
                gem_to_podium.erase(curr_item->name);
            }
            ui->add_inventory_item(curr_item->name, curr_item->img_path);
            ui->set_inventory(false);
            curr_item->transform->position.x = -1000.0f;
            Sound::play(*pop_sample, 0.05f, 0.0f);
        } else if (curr_item->name.find("Paper") != std::string::npos) {
            ui->add_inventory_item(curr_item->name, curr_item->img_path, curr_item->description_img_path);
            // uint32_t id = ui->get_inventory_item_id(curr_item->name);
            // ui->show_inventory_description_img(id);
            ui->set_inventory(false);
            Sound::play(*pop_sample, 0.05f, 0.0f);
            curr_item->transform->position.x = -1000.0f;
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

void Level3::handle_inventory_choice(uint32_t choice_id) {
    ui->hide_inventory_description_img();
    ui->hide_description();
    if(choice_id == 1) return;
    std::string item_name = ui->get_selected_inventory_item_name();
    if (item_name == "Diamond" || item_name == "Quartz" || item_name == "Corundum") {
        int closest_podium_idx = -1;
        float closest_dist = 1000.0f;
        for(int i=0; i<3; i++) {
            auto distance = glm::distance(podiums[i]->position, player_transform->position);
            if (distance < 2.0f && distance < closest_dist) {
                closest_podium_idx = i;
                closest_dist = distance;
            }
        }
        if(closest_podium_idx != -1) {
            if(podium_occupied[closest_podium_idx]) {
                ui->show_description("There is already an item on this podium");
            } else {
                items[item_name]->transform->position = podiums[closest_podium_idx]->position;
                items[item_name]->transform->position.z = 1.24f;
                podium_occupied[closest_podium_idx] = true;
                gem_to_podium[item_name] = closest_podium_idx;
                ui->remove_inventory_item();
                // check if all gems are placed to correct position
                if(podium_occupied[0] && podium_occupied[1] && podium_occupied[2]) {
                    if(gem_to_podium["Diamond"] == 0 && gem_to_podium["Quartz"] == 2 && gem_to_podium["Corundum"] == 3) {
                        // TODO: open door
                    }
                }
            }
            
        } else {
            // no podium near by
            ui->show_description("Maybe it should be placed somewhere else");
        }
    }
}

void Level3::handle_description_choice(uint32_t choice_id) {
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

void Level3::restart() {
    for(auto &item: items) {
        item.second->transform->position = item.second->spawn_point;
    }

    for(auto &guard_dog: guard_dogs) {
        guard_dog->transform->position = guard_dog->spawn_point;
        // guard_dog->fov->position = guard_dog->fov_spawn_point;
    }

    for(auto &driver: drivers) {
        driver->restart();
        driver->stop();
    }

    driver_player_ascend->clear();

    for(auto &item: guard_detectables) {
        item.second = false;
    }

    podium_occupied = {false, false, false};
    gem_to_podium.clear();

}

void Level3::update() {
    // Field of view collisions
    update_guard_detection();
    
    
    // animation
}

void Level3::handle_numeric_key(uint32_t key) {
}