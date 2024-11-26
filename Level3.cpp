#include "Level3.hpp"
#include "PlayMode.hpp"

#include <iostream>


GLuint level3_meshes_for_lit_color_texture_program = 0;
GLuint guard_fov_3_meshes_for_lit_color_texture_program = 0;
GLuint guard_fov_4_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > level3_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("level3.pnct"));
	level3_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});


MeshBuffer *guard_fov_3 = nullptr;
MeshBuffer *guard_fov_4 = nullptr;

Load< MeshBuffer > guard_fov_3_load(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer *ret = new MeshBuffer(data_path("guard_fov_1.pnct"));
    guard_fov_3 = ret;
	guard_fov_3_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< MeshBuffer > guard_fov_4_load(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer *ret = new MeshBuffer(data_path("guard_fov_2.pnct"));
    guard_fov_4 = ret;
	guard_fov_4_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > level3_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("level3.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = (mesh_name == "fov1") ? guard_fov_3->lookup("Cube") : ((mesh_name == "fov2") ? guard_fov_4->lookup("Cube") : level3_meshes->lookup(mesh_name));

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = (mesh_name == "fov1") ? guard_fov_3_meshes_for_lit_color_texture_program : ((mesh_name == "fov2") ? guard_fov_4_meshes_for_lit_color_texture_program : level3_meshes_for_lit_color_texture_program);
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

		// if this doenst work, ask on discord, ping Jim
		drawable.meshBuffer = (mesh_name == "fov1") ? &(*guard_fov_3) : ((mesh_name == "fov2") ? &(*guard_fov_4) : &(*level3_meshes));
		drawable.mesh = &mesh;

	});
});

Load< WalkMesh > level3_walkmesh(LoadTagDefault, []() -> WalkMesh const * {
	WalkMeshes *ret = new WalkMeshes(data_path("level3.w"));
	WalkMesh const *walkmesh = &ret->lookup("WalkMesh");
	return walkmesh;
});

Load< Animation > level3_animations(LoadTagDefault, []() -> Animation const * {
	Animation *anim = new Animation(data_path("level3.anim"));
	return anim;
});

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
        else if (transform.name == "ControlPanel") control_panel = &transform;
        else if (transform.name == "GuardDog") guardDog_1 = &transform;
        else if (transform.name == "GuardDog.001") guardDog_2 = &transform;
        else if (transform.name == "Head") head = &transform;
        else if (transform.name == "Computer") computer = &transform;
        else if (transform.name == "fov1") fov_1 = &transform;
        else if (transform.name == "fov2") fov_2 = &transform;
        else if (transform.name == "laser") laser = &transform;
        else if (transform.name == "laser.001") laser_1 = &transform;
        else if (transform.name == "laser.002") laser_2 = &transform;
        else if (transform.name == "laser.003") laser_3 = &transform;
        else if (transform.name == "laser.004") laser_4 = &transform;
        else if (transform.name == "laser.005") laser_5 = &transform;
        else if (transform.name == "laser.006") laser_6 = &transform;
        else if (transform.name == "laser.007") laser_7 = &transform;
        else if (transform.name == "Collectible") collectible = &transform;
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
    else if (control_panel == nullptr) throw std::runtime_error("ControlPanel not found.");
    else if (guardDog_1 == nullptr) throw std::runtime_error("GuardDog 1 not found.");
    else if (guardDog_2 == nullptr) throw std::runtime_error("GuardDog 2 not found.");
    else if (head == nullptr) throw std::runtime_error("Head not found.");
    else if (laser == nullptr) throw std::runtime_error("laser not found.");
    else if (laser_1 == nullptr) throw std::runtime_error("laser 1 not found.");
    else if (laser_2 == nullptr) throw std::runtime_error("laser 2 not found.");
    else if (laser_3 == nullptr) throw std::runtime_error("laser 3 not found.");
    else if (laser_4 == nullptr) throw std::runtime_error("laser 4 not found.");
    else if (laser_5 == nullptr) throw std::runtime_error("laser 5 not found.");
    else if (laser_6 == nullptr) throw std::runtime_error("laser 6 not found.");
    else if (laser_7 == nullptr) throw std::runtime_error("laser 7 not found.");
    else if (collectible == nullptr) throw std::runtime_error("Collectible not found.");

    player_spawn_point = player_transform->position;
    player_spawn_rotation = player_transform->rotation;
    lasers.clear();

    if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
    camera = &scene.cameras.front();
    camera_spawn_point = camera->transform->position;
    guard_detectables["Wall"] = false;
    guard_detectables["RedPanda"] = false;

    podiums = {diamond_podium, corundum_podium, quartz_podium};

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
    paper_passwd_ptr->description_img_path = "UI/Level3/gem_hint.png";
    paper_passwd_ptr->spawn_point = paper_passwd->position;
    items[paper_passwd_ptr->name] = paper_passwd_ptr;

    auto controll_panel_ptr = std::make_shared<Item>();
    controll_panel_ptr->name = "ControlPanel";
    controll_panel_ptr->interaction_description = "Interact with it";
    controll_panel_ptr->transform = control_panel;
    controll_panel_ptr->spawn_point = control_panel->position;
    items["ControlPanel"] = controll_panel_ptr;

    auto head_ptr = std::make_shared<Item>();
    head_ptr->name = "Head";
    head_ptr->interaction_description = "Collect it.";
    head_ptr->transform = head;
    head_ptr->img_path = "UI/snake.png";
    head_ptr->spawn_point = head->position;
    head_ptr->inventory_description = "This is the Old Summer Palace bronze head of Snake. It was looted by during the Second Opium War and went missing since then.";
    head_ptr->inventory_choices = {};
    items["Head"] = head_ptr;

    auto collectible_ptr = std::make_shared<Item>();
    collectible_ptr->name = "Collectible";
    collectible_ptr->interaction_description = "Collect it.";
    collectible_ptr->transform = collectible;
    collectible_ptr->img_path = "UI/dragon.png";
    collectible_ptr->spawn_point = collectible->position;
    collectible_ptr->inventory_description = "This is the Old Summer Palace bronze head of Dragon. It was looted by during the Second Opium War and went missing since then.";
    collectible_ptr->inventory_choices = {};
    items[collectible_ptr->name] = collectible_ptr;
    
    // initialize guard dogs
    auto guardDog_1_ptr = std::make_shared<GuardDog>();
    guardDog_1_ptr->name = "GuardDog";
    guardDog_1_ptr->transform = guardDog_1;
    guardDog_1_ptr->spawn_point = guardDog_1->position;
    guardDog_1_ptr->guard_fov_meshes = &(*guard_fov_3);
    guardDog_1_ptr->guard_fov_transform = fov_1;
    guard_dogs.push_back(guardDog_1_ptr);

    auto guardDog_2_ptr = std::make_shared<GuardDog>();
    guardDog_2_ptr->name = "GuardDog.001";
    guardDog_2_ptr->transform = guardDog_2;
    guardDog_2_ptr->spawn_point = guardDog_2->position;
    guardDog_2_ptr->guard_fov_meshes = &(*guard_fov_4);
    guardDog_2_ptr->guard_fov_transform = fov_2;
    guard_dogs.push_back(guardDog_2_ptr);

    auto laser_1_ptr = std::make_shared<Laser>();
    laser_1_ptr->name = "laser.001";
    laser_1_ptr->transform = laser_1;
    laser_1_ptr->spawn_point = laser_1->position;
    laser_1_ptr->on = true;
    laser_1_ptr->target_time = 0;
    lasers.push_back(laser_1_ptr);

    auto laser_2_ptr = std::make_shared<Laser>();
    laser_2_ptr->name = "laser.002";
    laser_2_ptr->transform = laser_2;
    laser_2_ptr->spawn_point = laser_2->position;
    laser_2_ptr->on = false;
    laser_2_ptr->target_time = 0;
    lasers.push_back(laser_2_ptr);

    auto laser_3_ptr = std::make_shared<Laser>();
    laser_3_ptr->name = "laser.003";
    laser_3_ptr->transform = laser_3;
    laser_3_ptr->spawn_point = laser_3->position;
    laser_3_ptr->on = true;
    laser_3_ptr->target_time = 0;
    lasers.push_back(laser_3_ptr);

    auto laser_4_ptr = std::make_shared<Laser>();
    laser_4_ptr->name = "laser";
    laser_4_ptr->transform = laser;
    laser_4_ptr->spawn_point = laser->position;
    laser_4_ptr->on = true;
    laser_4_ptr->target_time = 0;
    lasers.push_back(laser_4_ptr);

    auto laser_5_ptr = std::make_shared<Laser>();
    laser_5_ptr->name = "laser.005";
    laser_5_ptr->transform = laser_4;
    laser_5_ptr->spawn_point = laser_4->position;
    laser_5_ptr->on = true;
    laser_5_ptr->target_time = 0;
    lasers.push_back(laser_5_ptr);

    auto laser_6_ptr = std::make_shared<Laser>();
    laser_6_ptr->name = "laser.006";
    laser_6_ptr->transform = laser_5;
    laser_6_ptr->spawn_point = laser_5->position;
    laser_6_ptr->on = true;
    laser_6_ptr->target_time = 0;
    lasers.push_back(laser_6_ptr);

    auto laser_7_ptr = std::make_shared<Laser>();
    laser_7_ptr->name = "laser.007";
    laser_7_ptr->transform = laser_6;
    laser_7_ptr->spawn_point = laser_6->position;
    laser_7_ptr->on = true;
    laser_7_ptr->target_time = 0;
    lasers.push_back(laser_7_ptr);

    auto laser_8_ptr = std::make_shared<Laser>();
    laser_8_ptr->name = "laser.008";
    laser_8_ptr->transform = laser_7;
    laser_8_ptr->spawn_point = laser_7->position;
    laser_8_ptr->on = true;
    laser_8_ptr->target_time = 0;
    lasers.push_back(laser_8_ptr);

    door1 = {laser_1_ptr, laser_2_ptr, laser_3_ptr, laser_4_ptr};
    door2 = {laser_5_ptr, laser_6_ptr, laser_7_ptr, laser_8_ptr};


    // initialize animation drivers
    driver_guardDog1_walk = std::make_shared<Driver>(level3_animations->animations.at("GuardDog-translation"));
    driver_guardDog1_walk->transform = guardDog_1;
    driver_guardDog1_walk->loop = true;
    driver_guardDog1_walk->start();
    drivers.push_back(driver_guardDog1_walk);

    driver_guardDog2_walk = std::make_shared<Driver>(level3_animations->animations.at("GuardDog.001-translation"));
    driver_guardDog2_walk->transform = guardDog_2;
    driver_guardDog2_walk->loop = true;
    driver_guardDog2_walk->start();
    drivers.push_back(driver_guardDog2_walk);

    driver_guardDog1_rotate = std::make_shared<Driver>(level3_animations->animations.at("GuardDog-rotation"));
    driver_guardDog1_rotate->transform = guardDog_1;
    driver_guardDog1_rotate->loop = true;
    driver_guardDog1_rotate->start();
    drivers.push_back(driver_guardDog1_rotate);

    driver_guardDog2_rotate = std::make_shared<Driver>(level3_animations->animations.at("GuardDog.001-rotation"));
    driver_guardDog2_rotate->transform = guardDog_2;
    driver_guardDog2_rotate->loop = true;
    driver_guardDog2_rotate->start();
    drivers.push_back(driver_guardDog2_rotate);

    for(auto& driver: drivers) {
        for(int i=0; i<(int)driver->times.size(); i++){
            driver->times[i] = driver->times[i] * 4.0f;
        }
    }


    driver_rope_descend = std::make_shared<Driver>("Rope-descend", CHANEL_TRANSLATION);
    driver_rope_descend->transform = exit_transform;
    glm::vec3 rope_up_pos = exit_transform->position;
    rope_up_pos.z = 5.0f;
    glm::vec3 rope_down_pos = rope_up_pos;
    rope_down_pos.z = 0.0f;
    driver_rope_descend->add_move_in_straight_line_anim(rope_up_pos, rope_down_pos, rope_move_time, 3);
    driver_rope_descend->loop = false;
    drivers.push_back(driver_rope_descend);

    driver_rope_ascend = std::make_shared<Driver>("Rope-ascend", CHANEL_TRANSLATION);
    driver_rope_ascend->transform = exit_transform;
    driver_rope_ascend->add_move_in_straight_line_anim(rope_down_pos, rope_up_pos, rope_move_time, 3);
    driver_rope_ascend->loop = false;
    drivers.push_back(driver_rope_ascend);

    driver_player_ascend = std::make_shared<Driver>("Player-ascend", CHANEL_TRANSLATION);
    driver_player_ascend->transform = player_transform;
    driver_player_ascend->loop = false;
    drivers.push_back(driver_player_ascend);

    // sound
}

void Level3::handle_enter_key() {
    if(showing_control_panel) {
        // hide control panel
        hide_control_panel();
    }else if(ui->showing_inventory_description) {
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
            level_targets[2] = 1;
            driver_rope_descend->start();
        } else if (curr_item->name == "Collectible") {
            ui->add_inventory_item(curr_item->name, curr_item->img_path);
            curr_item->transform->position.x = -1000.0f;
            Sound::play(*pop_sample, 0.05f, 0.0f);
            level_targets[4] = 1;
        } else if (curr_item->name == "Diamond" || curr_item->name == "Quartz" || curr_item->name == "Corundum") {
            if(gem_to_podium.find(curr_item->name) != gem_to_podium.end()) {
                // remove from podium
                podium_occupied[gem_to_podium[curr_item->name]] = false;
                gem_to_podium.erase(curr_item->name);
            }
            ui->add_inventory_item(curr_item->name, curr_item->img_path);
            curr_item->transform->position.x = -1000.0f;
            Sound::play(*pop_sample, 0.05f, 0.0f);
        } else if (curr_item->name.find("Paper") != std::string::npos) {
            ui->add_inventory_item(curr_item->name, curr_item->img_path, curr_item->description_img_path);
            // uint32_t id = ui->get_inventory_item_id(curr_item->name);
            // ui->show_inventory_description_img(id);
            Sound::play(*pop_sample, 0.05f, 0.0f);
            curr_item->transform->position.x = -1000.0f;
        } else if (curr_item->name == "ControlPanel") {
            ui->hide_all();
            // show control panel
            if(!control_panel_box) {
                control_panel_box = ui->add_box(glm::vec4(200, 100, 1080, 620), glm::u8vec4(0, 0, 0, 200));
            }
            if(control_panel_slots.size()==0) {
                glm::vec2 pivot = glm::vec2(300, 180);
                for (uint32_t i = 0; i < 6; i++) {
                    glm::vec2 pos = pivot + glm::vec2(i*120, 0);
                    control_panel_slots.push_back(ui->add_img("UI/Slot2.png"));
                    control_panel_slots[i]->pos = pos;
                }
            }
            if(!control_panel_text) {
                control_panel_text = ui->add_text("Enter a 6 digits password. Press enter to exit.", glm::vec2(240, 500), ui->font_manual);
            }
            control_panel_box->hide = false;
            for(auto &slot: control_panel_slots) {
                slot->hide = false;
            }
            control_panel_text->text = "Enter a 6 digits password. (Press enter to exit)";
            control_panel_text->hide = false;
            showing_control_panel = true;
            ui->showing_image = true;
            ui->need_update_texture = true;
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
        if (showing_control_panel) {
            driver_guardDog1_walk->stop();
            driver_guardDog1_rotate->stop();
            driver_guardDog2_walk->stop();
            driver_guardDog2_rotate->stop();
        } else {
            driver_guardDog1_walk->start();
            driver_guardDog1_rotate->start();
            driver_guardDog2_walk->start();
            driver_guardDog2_rotate->start();
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
                    if(gem_to_podium["Diamond"] == 0 && gem_to_podium["Quartz"] == 2 && gem_to_podium["Corundum"] == 1) {
                        // TODO: open door
                        for(auto &laser: door1) {
                            laser->on = false;
                            laser->transform->position.z = -10.0f;
                        }
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

    driver_guardDog2_rotate->start();
    driver_guardDog2_walk->start();
    driver_guardDog1_rotate->start();
    driver_guardDog1_walk->start();

    

    for(auto &item: guard_detectables) {
        item.second = false;
    }

    podium_occupied = {false, false, false};
    gem_to_podium.clear();

    control_panel_slots = {};
    control_panel_inputs = {};
    items["ControlPanel"]->interactable = true;
    control_panel_box = nullptr;
    control_panel_text = nullptr;
    showing_control_panel = false;
    player_input = "";
    exit_transform->position.z = 20.0f;
}

void Level3::update() {
    // Field of view collisions
    update_guard_detection();
    if(guard_detectables["RedPanda"] || pause_game) {
        // stop guard animation
        driver_guardDog1_walk->stop();
        driver_guardDog1_rotate->stop();
        driver_guardDog2_walk->stop();
        driver_guardDog2_rotate->stop();
    } else {
        driver_guardDog1_walk->start();
        driver_guardDog1_rotate->start();
        driver_guardDog2_walk->start();
        driver_guardDog2_rotate->start();
    }
    
    
    // animation
}

void Level3::handle_numeric_key(uint32_t key) {
    if(showing_control_panel) {
        if(control_panel_inputs.size() < 6) {
            player_input += std::to_string(key);
            uint32_t i = (uint32_t)control_panel_inputs.size();
            auto img = ui->add_img("UI/Level2/"+std::to_string(key)+".png");
            img->pos = control_panel_slots[i]->pos;
            img->hide = false;
            control_panel_inputs.push_back(img);
        }
        if(control_panel_inputs.size() == 6) {
            // check password
            if(player_input == password) {
                // correct password
                // TODO: disable laser
                // disable_lasers = true;
                for(auto &laser: door2) {
                    laser->on = false;
                    laser->transform->position.z = -10.0f;
                }
                hide_control_panel();
                items["ControlPanel"]->interactable = false;
            } else {
                // wrong password
                control_panel_text->text = "Wrong password. Try again. (Press enter to exit)";
            }
            // clear input
            for(auto &img: control_panel_inputs) {
                ui->imgs.erase(std::remove(ui->imgs.begin(), ui->imgs.end(), img), ui->imgs.end());
            }
            control_panel_inputs.clear();
            player_input = "";
        }
        ui->need_update_texture = true;
    }
}

void Level3::hide_control_panel() {
    control_panel_box->hide = true;
    for(auto &slot: control_panel_slots) {
        slot->hide = true;
    }
    for(auto &img: control_panel_inputs) {
        ui->imgs.erase(std::remove(ui->imgs.begin(), ui->imgs.end(), img), ui->imgs.end());
    }
    control_panel_inputs.clear();
    control_panel_text->hide = true;
    showing_control_panel = false;
    ui->showing_image = false;
    ui->set_menu_button(false);
    ui->set_inventory_button(false);
    ui->set_restart_button(false);
    ui->set_menu_button(false);
}