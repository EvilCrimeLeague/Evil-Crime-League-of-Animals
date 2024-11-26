#include "Level0.hpp"
#include "Sound.hpp"
#include <glm/gtx/norm.hpp>

#include <iostream>

GLuint level0_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > level0_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("level0.pnct"));
	level0_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > level0_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("level0.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = level0_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = level0_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

		// if this doenst work, ask on discord, ping Jim
		drawable.meshBuffer = &(*level0_meshes);
		drawable.mesh = &mesh;

	});
});

Load< WalkMesh > level0_walkmesh(LoadTagDefault, []() -> WalkMesh const * {
	WalkMeshes *ret = new WalkMeshes(data_path("level0.w"));
	WalkMesh const *walkmesh = &ret->lookup("WalkMesh");
	return walkmesh;
});

// Load< Animation > level0_animations(LoadTagDefault, []() -> Animation const * {
// 	Animation *anim = new Animation(data_path("level0.anim"));
// 	return anim;
// });

Level0::Level0(std::shared_ptr<UI> ui_, std::shared_ptr<GameInfo> info_): Level(ui_, info_) {
    scene = *level0_scene;
    walkmesh = level0_walkmesh;

    for (auto &transform : scene.transforms) {
        if (transform.name == "RedPanda") player_transform = &transform;
        else if (transform.name == "Head") head_sheep = &transform;
        else if (transform.name == "Head.001") head_chicken = &transform;
        else if (transform.name == "Head.002") head_snake = &transform;
        else if (transform.name == "Head.003") head_dog = &transform;
        else if (transform.name == "Head.004") head_dragon = &transform;
	}

    if (player_transform == nullptr) throw std::runtime_error("Player not found.");

    heads = {head_sheep, head_chicken, head_snake, head_dog, head_dragon};

    for(auto head: heads){
        if (head == nullptr) throw std::runtime_error("Head not found.");
        head->position.z += 100.0f; // hide the heads
    }

    player_spawn_point = player_transform->position;
    player_spawn_rotation = player_transform->rotation;

    if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
    camera = &scene.cameras.front();
    camera_spawn_point = camera->transform->position;

    // initialize items
    auto head_dog_ptr = std::make_shared<Item>();
    head_dog_ptr->name = "Head-Dog";
    head_dog_ptr->interaction_description = "This is the Old Summer Palace bronze head of Dog. It was looted by during the Second Opium War and went missing since then.";
    head_dog_ptr->transform = head_dog;
    head_dog_ptr->show_description_box = true;
    head_dog_ptr->spawn_point = head_dog->position;
    items["Head-Dog"] = head_dog_ptr;

    auto head_dragon_ptr = std::make_shared<Item>();
    head_dragon_ptr->name = "Head-Dragon";
    head_dragon_ptr->interaction_description = "This is the Old Summer Palace bronze head of Dragon. It was looted by during the Second Opium War and went missing since then.";
    head_dragon_ptr->transform = head_dragon;
    head_dragon_ptr->show_description_box = true;
    head_dragon_ptr->spawn_point = head_dragon->position;
    items["Head-Dragon"] = head_dragon_ptr;

    auto head_chicken_ptr = std::make_shared<Item>();
    head_chicken_ptr->name = "Head-Chicken";
    head_chicken_ptr->interaction_description = "This is the Old Summer Palace bronze head of Rooster. It was looted by during the Second Opium War and went missing since then.";
    head_chicken_ptr->transform = head_chicken;
    head_chicken_ptr->show_description_box = true;
    head_chicken_ptr->spawn_point = head_chicken->position;
    items["Head-Chicken"] = head_chicken_ptr;

    auto head_snake_ptr = std::make_shared<Item>();
    head_snake_ptr->name = "Head-Snake";
    head_snake_ptr->interaction_description = "This is the Old Summer Palace bronze head of Snake. It was looted by during the Second Opium War and went missing since then.";
    head_snake_ptr->transform = head_snake;
    head_snake_ptr->show_description_box = true;
    head_snake_ptr->spawn_point = head_snake->position;
    items["Head-Snake"] = head_snake_ptr;

    auto head_sheep_ptr = std::make_shared<Item>();
    head_sheep_ptr->name = "Head-Sheep";
    head_sheep_ptr->interaction_description = "This is the Old Summer Palace bronze head of Sheep. It was looted by during the Second Opium War and went missing since then.";
    head_sheep_ptr->transform = head_sheep;
    head_sheep_ptr->show_description_box = true;
    head_sheep_ptr->spawn_point = head_sheep->position;
    items["Head-Sheep"] = head_sheep_ptr;
    
    // initialize guard dogs

    // initialize drivers

    // sound
}

void Level0::handle_enter_key() {
    if (ui->showing_description) {
        // Interact with item
        ui->hide_description();
    } 
}

void Level0::handle_interact_key() {
    if(ui->showing_interactable_button) {
        ui->show_description(curr_item->interaction_description);
    }
}

void Level0::restart() {
    for(auto &item: items) {
        item.second->transform->position = item.second->spawn_point;
    }
    
    for(int i=0; i<(int)heads.size(); i++){
        if(info->targets_obtained[i] == 1) {
            heads[i]->position.z = 1.3f; // show the heads
        } else {
            heads[i]->position.z = 100.0f; // hide the heads
        }
    }
}

void Level0::update() {
    
}