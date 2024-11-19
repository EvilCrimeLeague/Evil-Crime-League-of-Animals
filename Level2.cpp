#include "Level2.hpp"

#include <iostream>


GLuint level2_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > level2_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("level2.pnct"));
	level2_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > level2_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("level2.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = level2_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = level2_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

		// if this doenst work, ask on discord, ping Jim
		drawable.meshBuffer = &(*level2_meshes);
		drawable.mesh = &mesh;
	});
});

Load< WalkMesh > level2_walkmesh(LoadTagDefault, []() -> WalkMesh const * {
	WalkMeshes *ret = new WalkMeshes(data_path("level2.w"));
	WalkMesh const *walkmesh = &ret->lookup("WalkMesh");
	return walkmesh;
});

Load< Animation > level2_animations(LoadTagDefault, []() -> Animation const * {
	Animation *anim = new Animation(data_path("level2.anim"));
	return anim;
});

Level2::Level2(std::shared_ptr<UI> ui_, std::shared_ptr<GameInfo> info_): Level(ui_, info_) {
    scene = *level2_scene;
    walkmesh = level2_walkmesh;

    for (auto &transform : scene.transforms) {
        if (transform.name == "RedPanda") player_transform = &transform;
        else if (transform.name == "Head") head = &transform;
		else if (transform.name == "GuardDog") guardDog_1 = &transform;
        else if (transform.name == "GuardDog.001") guardDog_2 = &transform;
        else if (transform.name == "Painting") painting_1 = &transform;
        else if (transform.name == "Painting.001") painting_2 = &transform;
        else if (transform.name == "ControlPanel") control_panel = &transform;
        else if (transform.name == "Paper") paper_1 = &transform;
        else if (transform.name == "Paper.001") paper_2 = &transform;
        else if (transform.name == "Rope") exit_transform = &transform;
	}

    if (head == nullptr) throw std::runtime_error("Target not found.");
    else if (player_transform == nullptr) throw std::runtime_error("Player not found.");
	else if (guardDog_1 == nullptr) throw std::runtime_error("GuardDog 1 not found.");
    else if (guardDog_2 == nullptr) throw std::runtime_error("GuardDog 2 not found.");
    else if (painting_1 == nullptr) throw std::runtime_error("Painting 1 not found.");
    else if (painting_2 == nullptr) throw std::runtime_error("Painting 2 not found.");
    else if (control_panel == nullptr) throw std::runtime_error("ControlPanel not found.");
    else if (paper_1 == nullptr) throw std::runtime_error("Paper 1 not found.");
    else if (paper_2 == nullptr) throw std::runtime_error("Paper 2 not found.");
    else if (exit_transform == nullptr) throw std::runtime_error("Exit not found.");
    else if (head == nullptr) throw std::runtime_error("Head not found.");

    for (auto &drawable : scene.drawables) {
        if (drawable.transform->name == "Painting") {
            drawable.texture = gen_texture_from_img("UI/Level2/256test.png");
        } else if (drawable.transform->name == "Painting.001") {
            drawable.texture = gen_texture_from_img("UI/Level2/256test.png");
        }
	}

    player_spawn_point = player_transform->position;
    player_spawn_rotation = player_transform->rotation;

    if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
    camera = &scene.cameras.front();
    camera_spawn_point = camera->transform->position;
    guard_detectables["Wall"] = false;

    // initialize items
    auto painting_1_ptr = std::make_shared<Item>();
    painting_1_ptr->name = "Painting1";
    painting_1_ptr->interaction_description = "Look at it";
    painting_1_ptr->transform = painting_1;
    painting_1_ptr->description_img_path = "UI/Level2/256test.png";
    painting_1_ptr->spawn_point = painting_1->position;
    items["Painting1"] = painting_1_ptr;

    auto painting_2_ptr = std::make_shared<Item>();
    painting_2_ptr->name = "Painting2";
    painting_2_ptr->interaction_description = "Look at it";
    painting_2_ptr->transform = painting_2;
    painting_2_ptr->description_img_path = "UI/Level2/256test.png";
    painting_2_ptr->spawn_point = painting_2->position;
    items["Painting2"] = painting_2_ptr;

    auto controll_panel_ptr = std::make_shared<Item>();
    controll_panel_ptr->name = "ControlPanel";
    controll_panel_ptr->interaction_description = "Interact with it";
    controll_panel_ptr->transform = control_panel;
    controll_panel_ptr->spawn_point = control_panel->position;
    items["ControlPanel"] = controll_panel_ptr;

    auto paper_ptr = std::make_shared<Item>();
    paper_ptr->name = "Paper";
    paper_ptr->interaction_description = "Pick it up";
    paper_ptr->transform = paper_1;
    paper_ptr->img_path = "UI/Level2/paper.png";
    paper_ptr->inventory_description = "The paper contains a note: \"The secret is hidden in the paintings.\"";
    paper_ptr->inventory_choices = {};
    paper_ptr->spawn_point = paper_1->position;
    items["Paper"] = paper_ptr;

    auto paper_2_ptr = std::make_shared<Item>();
    paper_2_ptr->name = "Paper_2";
    paper_2_ptr->interaction_description = "Pick it up";
    paper_2_ptr->transform = paper_2;
    paper_2_ptr->img_path = "UI/Level2/paper.png";
    paper_2_ptr->inventory_description = "The paper contains a note: \"The order of the secret is also hidden in the painting.\"";
    paper_2_ptr->inventory_choices = {};
    paper_2_ptr->spawn_point = paper_2->position;
    items["Paper_2"] = paper_2_ptr;

    auto head_ptr = std::make_shared<Item>();
    head_ptr->name = "Head";
    head_ptr->interaction_description = "Collect it.";
    head_ptr->transform = head;
    head_ptr->img_path = "UI/dragon.png";
    head_ptr->spawn_point = head->position;
    head_ptr->inventory_description = "This is the Old Summer Palace bronze head of Dragon. It was looted by during the Second Opium War and went missing since then.";
    head_ptr->inventory_choices = {};
    items["Head"] = head_ptr;
    
    // initialize guard dogs
    auto guardDog_1_ptr = std::make_shared<GuardDog>();
    guardDog_1_ptr->name = "GuardDog1";
    guardDog_1_ptr->transform = guardDog_1;
    guardDog_1_ptr->spawn_point = guardDog_1->position;
    guard_dogs.push_back(guardDog_1_ptr);

    auto guardDog_2_ptr = std::make_shared<GuardDog>();
    guardDog_2_ptr->name = "GuardDog2";
    guardDog_2_ptr->transform = guardDog_2;
    guardDog_2_ptr->spawn_point = guardDog_2->position;
    guard_dogs.push_back(guardDog_2_ptr);

    // initialize animation drivers
    driver_guardDog1_walk = std::make_shared<Driver>(level2_animations->animations.at("GuardDog.001-translation"));
    driver_guardDog1_walk->transform = guardDog_1;
    driver_guardDog1_walk->loop = true;
    driver_guardDog1_walk->start();
    drivers.push_back(driver_guardDog1_walk);

    driver_guardDog2_walk = std::make_shared<Driver>(level2_animations->animations.at("GuardDog.001-translation"));
    driver_guardDog2_walk->transform = guardDog_2;
    driver_guardDog2_walk->loop = true;
    driver_guardDog2_walk->start();
    drivers.push_back(driver_guardDog2_walk);

    driver_guardDog1_rotate = std::make_shared<Driver>(level2_animations->animations.at("GuardDog.001-rotation"));
    driver_guardDog1_rotate->transform = guardDog_1;
    driver_guardDog1_rotate->loop = true;
    driver_guardDog1_rotate->start();
    drivers.push_back(driver_guardDog1_rotate);

    driver_guardDog2_rotate = std::make_shared<Driver>(level2_animations->animations.at("GuardDog.001-rotation"));
    driver_guardDog2_rotate->transform = guardDog_2;
    driver_guardDog2_rotate->loop = true;
    driver_guardDog2_rotate->start();
    drivers.push_back(driver_guardDog2_rotate);

    // slow done guard movement
    for(auto& driver: drivers) {
        for(int i=0; i<driver->times.size(); i++){
            driver->times[i] = driver->times[i] * 2.0f;
        }
    }

    driver_guardDog1_rotate->setPlaybackTime(driver_guardDog1_rotate->times[static_cast<int>(driver_guardDog1_rotate->times.size()/2)]);
    driver_guardDog1_walk->setPlaybackTime(driver_guardDog1_walk->times[static_cast<int>(driver_guardDog1_walk->times.size()/2)]);

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

    // driver_fov_move = std::make_shared<Driver>("FOV-move", CHANEL_TRANSLATION);
    // driver_fov_move->transform = fov;
    // driver_fov_move->loop = false;
    // drivers.push_back(driver_fov_move);

    // sound
}

void Level2::handle_enter_key() {
    if(showing_control_panel) {
        // hide control panel
        hide_control_panel();
    } else if(ui->showing_inventory_description) {
        ui->hide_inventory_description_img();
        ui->hide_description();
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

void Level2::handle_interact_key() {
    if(ui->showing_interactable_button) {
        // ui->show_description(curr_item->interaction_description, curr_item->interaction_choices[0], curr_item->interaction_choices[1]);
        
        if(curr_item->name.find("Paper") != std::string::npos) {
            if (!curr_item->added) {
                curr_item->added = true;
                ui->add_inventory_item(curr_item->name, curr_item->img_path, curr_item->description_img_path);
            }
            // uint32_t id = ui->get_inventory_item_id(curr_item->name);
            // ui->show_inventory_description_img(id);
            Sound::play(*pop_sample, 0.05f, 0.0f);
            curr_item->transform->position.x = -1000.0f;
        } else if (curr_item->name.find("Painting") != std::string::npos) {
            if (!curr_item->added) {
                curr_item->img = ui->add_img(curr_item->description_img_path);
                curr_item->added = true;
            }
            ui->show_img(curr_item->img);
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
        } else if (curr_item->name == "Head") {
            ui->add_inventory_item(curr_item->name, curr_item->img_path);
            curr_item->transform->position.x = -1000.0f;
            Sound::play(*pop_sample, 0.05f, 0.0f);
            level_targets[1] = 1;
            driver_rope_descend->start();
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

void Level2::handle_inventory_choice(uint32_t choice_id) {
    ui->hide_inventory_description_img();
}

void Level2::handle_description_choice(uint32_t choice_id) {
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

void Level2::restart() {
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
    driver_guardDog1_rotate->setPlaybackTime(driver_guardDog1_rotate->times[static_cast<int>(driver_guardDog1_rotate->times.size()/2)]);
    driver_guardDog1_walk->setPlaybackTime(driver_guardDog1_walk->times[static_cast<int>(driver_guardDog1_walk->times.size()/2)]);
    driver_guardDog2_rotate->start();
    driver_guardDog2_walk->start();
    driver_guardDog1_rotate->start();
    driver_guardDog1_walk->start();

    driver_player_ascend->clear();

    for(auto &item: guard_detectables) {
        item.second = false;
    }

    // driver_guardDog_walk->clear();
    // driver_fov_move->clear();

    // fov->position.x = guardDog->position.x;
    // fov->position.y = guardDog->position.y;

    control_panel_slots = {};
    control_panel_inputs = {};
    items["ControlPanel"]->interactable = true;
    control_panel_box = nullptr;
    control_panel_text = nullptr;
    showing_control_panel = false;
    player_input = "";

}

void Level2::update() {
    // Field of view collisions
    update_guard_detection();
    
    
    // animation
}

void Level2::handle_numeric_key(uint32_t key) {
    if(showing_control_panel) {
        if(control_panel_inputs.size() < 6) {
            player_input += std::to_string(key);
            uint32_t i = control_panel_inputs.size();
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

void Level2::hide_control_panel() {
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