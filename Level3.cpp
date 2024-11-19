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
	}

    if (player_transform == nullptr) throw std::runtime_error("Player not found.");
    else if (exit_transform == nullptr) throw std::runtime_error("Exit not found.");

    player_spawn_point = player_transform->position;
    player_spawn_rotation = player_transform->rotation;

    if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
    camera = &scene.cameras.front();
    camera_spawn_point = camera->transform->position;
    guard_detectables["Wall"] = false;

    // initialize items
    
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

void Level3::handle_interact_key() {
    if(ui->showing_interactable_button) {
        if (curr_item->name == "Head") {
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

void Level3::handle_inventory_choice(uint32_t choice_id) {
    ui->hide_inventory_description_img();
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

}

void Level3::update() {
    // Field of view collisions
    update_guard_detection();
    
    
    // animation
}

void Level3::handle_numeric_key(uint32_t key) {
}