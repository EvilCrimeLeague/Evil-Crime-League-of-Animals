#include "Level1.hpp"
#include <iostream>

Level1::Level1(Scene const * scene_, std::shared_ptr<UI> ui_): Level(scene_, ui_) {
    for (auto &transform : scene.transforms) {
		if (transform.name == "Bone") bone = &transform;
		else if (transform.name == "GuardDog") guardDog = &transform;
		else if (transform.name == "FOV") fov = &transform;
	}

    if (bone == nullptr) throw std::runtime_error("Bone not found.");
	else if (guardDog == nullptr) throw std::runtime_error("GuardDog not found.");
	else if (fov == nullptr) throw std::runtime_error("FOV not found.");
	std::cout<<guardDog->position.y - fov->position.y<<std::endl;

    // initialize items
    auto bone_ptr = std::make_shared<Item>();
    bone_ptr->name = "Bone";
    bone_ptr->interaction_description = "You found a bone. Do you want to collect it?";
    bone_ptr->interaction_choices = {"Yes", "No"};
    bone_ptr->inventory_description = "Do you want to use the bone to distract the guard?";
    bone_ptr->inventory_choices = {"Yes", "No"};
    bone_ptr->transform = bone;
    bone_ptr->img_path = "UI/bone.png";
    bone_ptr->spawn_point = bone->position;
    items["Bone"] = bone_ptr;

    // initialize guard dogs
    auto guardDog_ptr = std::make_shared<GuardDog>();
    guardDog_ptr->name = "GuardDog";
    guardDog_ptr->transform = guardDog;
    guardDog_ptr->spawn_point = guardDog->position;
    guardDog_ptr->fov = fov;
    guardDog_ptr->fov_spawn_point = fov->position;
    guard_dogs.push_back(guardDog_ptr);
}

void Level1::handle_enter_key() {
    if(ui->showing_inventory_description) {
        // Interact with inventory item
        handle_inventory_choice(ui->choice_id);
    } else if (ui->showing_description) {
        // Interact with item
        handle_description_choice(ui->choice_id);
    } else if (ui->showing_inventory && ui->inventory_items.size() > 0) {
        std::string item_name = ui->get_selected_inventory_item_name();
        if(item_name == "Bone" && glm::distance(player_transform->position, glm::vec3(-4,7.834,0.0))<2.0f) {
            ui->show_description(items[item_name]->inventory_description, items[item_name]->inventory_choices[0], items[item_name]->inventory_choices[1]);
            ui->showing_inventory_description = true;
        }
        
    }	
}

void Level1::handle_interact_key() {
    if(ui->showing_interactable_button) {
        ui->show_description(curr_item->interaction_description, curr_item->interaction_choices[0], curr_item->interaction_choices[1]);
    }
}

void Level1::handle_inventory_choice(uint32_t choice_id) {
    ui->hide_description();
    if(ui->choice_id == 0) {
        // use item
        std::string item_name = ui->get_selected_inventory_item_name();
        ui->remove_inventory_item();
        if(item_name == "Bone") {
            bone->position = glm::vec3(8.0f, 7.0f, 1.0f);
            guardDog->position = glm::vec3(8.0f, 8.5f, 0.424494f);
            fov->position = glm::vec3(8.0f, 6.0f, 0.42449f);
        }
    } else {
        // show inventory again
        ui->set_inventory(false);
    }
    ui->showing_inventory_description = false;
}

void Level1::handle_description_choice(uint32_t choice_id) {
    ui->hide_description();
    ui->set_inventory_button(/*hide=*/false);
    if(ui->choice_id ==0) {
        ui->add_inventory_item(curr_item->name, curr_item->img_path);
        // hide item
        curr_item->transform->position.x = -1000.0f;
    } else {
        // show iteractable button again
        ui->set_interactable_button(/*hide=*/false);
    }
}

void Level1::restart() {
    for(auto &item: items) {
        item.second->transform->position = item.second->spawn_point;
    }

    for(auto &guardDog: guard_dogs) {
        guardDog->transform->position = guardDog->spawn_point;
        guardDog->fov->position = guardDog->fov_spawn_point;
    }
}