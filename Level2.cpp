#include "Level2.hpp"

#include <iostream>


GLuint level2_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > level2_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("level1.pnct"));
	level2_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > level2_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("level1.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
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
	WalkMeshes *ret = new WalkMeshes(data_path("level1.w"));
	WalkMesh const *walkmesh = &ret->lookup("WalkMesh");
	return walkmesh;
});

Load< Animation > level2_animations(LoadTagDefault, []() -> Animation const * {
	Animation *anim = new Animation(data_path("level1.anim"));
	return anim;
});

Level2::Level2(std::shared_ptr<UI> ui_): Level(ui_) {
    scene = *level2_scene;
    walkmesh = level2_walkmesh;

    for (auto &transform : scene.transforms) {
        if (transform.name == "RedPanda") player_transform = &transform;
        else if (transform.name == "Jewel") target_transform = &transform;
		else if (transform.name == "GuardDog") guardDog = &transform;
		else if (transform.name == "FOV") fov = &transform;
        else if (transform.name == "Vase.001") vase = &transform;
        else if (transform.name == "Painting.002") painting_1 = &transform;
        else if (transform.name == "Painting.003") painting_2 = &transform;
	}

    if (target_transform == nullptr) throw std::runtime_error("Target not found.");
    else if (player_transform == nullptr) throw std::runtime_error("Player not found.");
	else if (guardDog == nullptr) throw std::runtime_error("GuardDog not found.");
	else if (fov == nullptr) throw std::runtime_error("FOV not found.");

    // fov->parent = guardDog;

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
    painting_1_ptr->img_path = "UI/Level2/painting.png";
    painting_1_ptr->description_img_path = "UI/Level2/256test.png";
    painting_1_ptr->spawn_point = painting_1->position;
    items["Painting1"] = painting_1_ptr;
    
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

    driver_fov_move = std::make_shared<Driver>("FOV-move", CHANEL_TRANSLATION);
    driver_fov_move->transform = fov;
    driver_fov_move->loop = false;
    drivers.push_back(driver_fov_move);

    // sound
}

void Level2::handle_enter_key() {
    if(ui->showing_inventory_description) {
        // Interact with inventory item
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
    else if (ui->showing_inventory && ui->inventory_items.size() > 0) {
        Sound::play(*pop_sample, 0.1f, 0.0f);
        if(ui->inventory_slot_selected_id < ui->inventory_items.size()) {
            ui->show_inventory_description_img(ui->inventory_slot_selected_id);
        }
        
        
    }
}

void Level2::handle_interact_key() {
    if(ui->showing_interactable_button) {
        // ui->show_description(curr_item->interaction_description, curr_item->interaction_choices[0], curr_item->interaction_choices[1]);
        
        if(curr_item->name == "Paper") {
            if (!curr_item->added) {
                curr_item->added = true;
                ui->add_inventory_item(curr_item->name, curr_item->img_path, curr_item->description_img_path);
            }
            uint32_t id = ui->get_inventory_item_id(curr_item->name);
            ui->show_inventory_description_img(id);
            Sound::play(*pop_sample, 0.05f, 0.0f);
        } else if (curr_item->name.find("Painting") != std::string::npos) {
            if (!curr_item->added) {
                curr_item->img = ui->add_img(curr_item->description_img_path);
                curr_item->added = true;
            }
            ui->show_img(curr_item->img);
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
        if(curr_item->name == "Paper") {
            // hide item
            curr_item->transform->position.x = -1000.0f;
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

    fov->position.x = guardDog->position.x;
    fov->position.y = guardDog->position.y;

}

void Level2::update() {
    // Field of view collisions
    update_guard_detection();
    
    
    // animation
}