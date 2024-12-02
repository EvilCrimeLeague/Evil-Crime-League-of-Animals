#include "Level0.hpp"
#include "Sound.hpp"
#include "PlayMode.hpp"
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
        else if (transform.name == "Achievement") achievement_paintings = &transform;
        else if (transform.name == "Achievement.001") achievement_die = &transform;
        else if (transform.name == "Achievement.002") achievement_no_die = &transform;
        else if (transform.name == "Achievement.003") achievement_collectibles = &transform;
        else if (transform.name == "CuratorDog") guard_dog = &transform;
	}

    if (player_transform == nullptr) throw std::runtime_error("Player not found.");
    else if (head_sheep == nullptr) throw std::runtime_error("Head-Sheep not found.");
    else if (head_chicken == nullptr) throw std::runtime_error("Head-Chicken not found.");
    else if (head_snake == nullptr) throw std::runtime_error("Head-Snake not found.");
    else if (head_dog == nullptr) throw std::runtime_error("Head-Dog not found.");
    else if (head_dragon == nullptr) throw std::runtime_error("Head-Dragon not found.");
    else if (achievement_paintings == nullptr) throw std::runtime_error("Achievement not found.");
    else if (achievement_die == nullptr) throw std::runtime_error("Achievement.001 not found.");
    else if (achievement_no_die == nullptr) throw std::runtime_error("Achievement.002 not found.");
    else if (achievement_collectibles == nullptr) throw std::runtime_error("Achievement.003 not found.");
    else if (guard_dog == nullptr) throw std::runtime_error("Guard dog not found.");

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
    head_dog_ptr->interaction_description = "This is the Old Summer Palace bronze head of the Dog. In 1860 during the Second Opium War, the Old Summer Palace was ransacked and destroyed by British and French forces. To this day, the head's whereabouts remain unknown.";
    head_dog_ptr->transform = head_dog;
    head_dog_ptr->show_description_box = true;
    head_dog_ptr->spawn_point = head_dog->position;
    items["Head-Dog"] = head_dog_ptr;

    auto head_dragon_ptr = std::make_shared<Item>();
    head_dragon_ptr->name = "Head-Dragon";
    head_dragon_ptr->interaction_description = "This is the Old Summer Palace bronze head of the Dragon. In 1860 during the Second Opium War, the Old Summer Palace was ransacked and destroyed by British and French forces. To this day, the head's whereabouts remain unknown.";
    head_dragon_ptr->transform = head_dragon;
    head_dragon_ptr->show_description_box = true;
    head_dragon_ptr->spawn_point = head_dragon->position;
    items["Head-Dragon"] = head_dragon_ptr;

    auto head_chicken_ptr = std::make_shared<Item>();
    head_chicken_ptr->name = "Head-Chicken";
    head_chicken_ptr->interaction_description = "This is the Old Summer Palace bronze head of the Rooster. In 1860 during the Second Opium War, the Old Summer Palace was ransacked and destroyed by British and French forces. To this day, the head's whereabouts remain unknown.";
    head_chicken_ptr->transform = head_chicken;
    head_chicken_ptr->show_description_box = true;
    head_chicken_ptr->spawn_point = head_chicken->position;
    items["Head-Chicken"] = head_chicken_ptr;

    auto head_snake_ptr = std::make_shared<Item>();
    head_snake_ptr->name = "Head-Snake";
    head_snake_ptr->interaction_description = "This is the Old Summer Palace bronze head of the Snake. In 1860 during the Second Opium War, the Old Summer Palace was ransacked and destroyed by British and French forces. To this day, the head's whereabouts remain unknown.";
    head_snake_ptr->transform = head_snake;
    head_snake_ptr->show_description_box = true;
    head_snake_ptr->spawn_point = head_snake->position;
    items["Head-Snake"] = head_snake_ptr;

    auto head_sheep_ptr = std::make_shared<Item>();
    head_sheep_ptr->name = "Head-Sheep";
    head_sheep_ptr->interaction_description = "This is the Old Summer Palace bronze head of the Goat. In 1860 during the Second Opium War, the Old Summer Palace was ransacked and destroyed by British and French forces. To this day, the head's whereabouts remain unknown.";
    head_sheep_ptr->transform = head_sheep;
    head_sheep_ptr->show_description_box = true;
    head_sheep_ptr->spawn_point = head_sheep->position;
    items["Head-Sheep"] = head_sheep_ptr;

    auto guard_dog_ptr = std::make_shared<GuardDog>();
    guard_dog_ptr->name = "GuardDog";
    guard_dog_ptr->transform = guard_dog;
    guard_dog_ptr->spawn_point = guard_dog->position;
    
    // initialize guard dogs

    // initialize drivers

    // sound

    // initialize achievement
    auto achievement_paintings_ptr = std::make_shared<Item>();
    achievement_paintings_ptr->name = "Achievement-Paintings";
    achievement_paintings_ptr->interaction_description = "???";
    achievement_paintings_ptr->transform = achievement_paintings;
    achievement_paintings_ptr->show_description_box = true;
    achievement_paintings_ptr->spawn_point = achievement_paintings->position;
    items[achievement_paintings_ptr->name] = achievement_paintings_ptr;

    auto achievement_die_ptr = std::make_shared<Item>();
    achievement_die_ptr->name = "Achievement-Die";
    achievement_die_ptr->interaction_description = "???";
    achievement_die_ptr->transform = achievement_die;
    achievement_die_ptr->show_description_box = true;
    achievement_die_ptr->spawn_point = achievement_die->position;
    items[achievement_die_ptr->name] = achievement_die_ptr;

    auto achievement_no_die_ptr = std::make_shared<Item>();
    achievement_no_die_ptr->name = "Achievement-No-Die";
    achievement_no_die_ptr->interaction_description = "???";
    achievement_no_die_ptr->transform = achievement_no_die;
    achievement_no_die_ptr->show_description_box = true;
    achievement_no_die_ptr->spawn_point = achievement_no_die->position;
    items[achievement_no_die_ptr->name] = achievement_no_die_ptr;

    auto achievement_collectibles_ptr = std::make_shared<Item>();
    achievement_collectibles_ptr->name = "Achievement-Collectibles";
    achievement_collectibles_ptr->interaction_description = "???";
    achievement_collectibles_ptr->transform = achievement_collectibles;
    achievement_collectibles_ptr->show_description_box = true;
    achievement_collectibles_ptr->spawn_point = achievement_collectibles->position;
    items[achievement_collectibles_ptr->name] = achievement_collectibles_ptr;
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

    if(info->achievements[3] == 0) {
        if(std::all_of(info->targets_obtained.begin(), info->targets_obtained.end(), [](int i){return i==1;})) {
            info->achievements[3] = 1;
            info->update_game_info();
        }
    }

    if(info->achievements[0] == 1) {
        items["Achievement-Paintings"]->interaction_description = "Viewed all paintings in Level 2."; // show the achievement
    } 
    if(info->achievements[3] == 1) {
        items["Achievement-Collectibles"]->interaction_description = "Collected all heads in all levels."; // show the achievement
    }
    if(info->achievements[1] == 1) {
        items["Achievement-Die"]->interaction_description = "Died more than 10 times in a level."; // show the achievement
    }
    if(info->achievements[2] == 1) {
        items["Achievement-No-Die"]->interaction_description = "Made through a level with no death."; // show the achievement
    }
}

void Level0::update() {
    if ((player_transform->position.y >= guard_dog->position.y) && (info->achievements[3] == 0)) {
        stop_player = true;
        player_transform->position.y = guard_dog->position.y;
    } else if ((player_transform->position.y < guard_dog->position.y)) stop_player = false;
}