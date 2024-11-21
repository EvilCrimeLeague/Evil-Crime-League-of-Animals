#include "Level.hpp"
#include  "Mesh.hpp"
#include "Level1.hpp"

Load< Sound::Sample > collect_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("collect.wav"));
});

Load< Sound::Sample > pop_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("pop.wav"));
});

Load< Sound::Sample > rolling_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("rolling.wav"));
});

Level::Level(std::shared_ptr<UI> ui_, std::shared_ptr<GameInfo> info_): ui(ui_), info(info_) {};

std::shared_ptr<Level::Item> Level::get_closest_item(glm::vec3 player_position) {
    std::shared_ptr<Item> closest_item = nullptr;
    float closest_distance = interactable_distance;
    for(auto itr = items.begin(); itr != items.end(); itr++) {
		if(!itr->second->interactable) continue;
        float distance = glm::distance(itr->second->transform->position, player_position);
        if(distance < closest_distance) {
            closest_distance = distance;
            closest_item = itr->second;
        }
    }

    curr_item = closest_item;
    return closest_item;
}

void Level::update_guard_detection() {
	// MeshBuffer *guard_fov_meshbuffer = guard_fov_meshes;
	for (auto &item: guard_detectables) {
		item.second = false;
	}
    for (auto guardDog: guard_dogs) {
        constexpr float guardDogVerticalFov = 120.0f;
		constexpr float guardDogHorizontalFov = 90.0f;
		constexpr uint32_t horizontalRays = 5;
		constexpr uint32_t verticalRays = 20;
		float horizontalStep = guardDogHorizontalFov / horizontalRays;
		float verticalStep = guardDogVerticalFov / verticalRays;
		float visionDistance = 5.2f;
		glm::vec3 point = guardDog->transform->position;
		Vertex point_vertex;
		point_vertex.Position = point;
		point_vertex.Color = glm::u8vec4(0x88, 0x00, 0xff, 0xff);
		glm::vec3 guardDogDirectionWorld = glm::normalize(guardDog->transform->make_local_to_world() * glm::vec4(-1.0, 0.0, 0.0, 0.0));
		//std::cout<<guardDogDirectionWorld.x<<" "<<guardDogDirectionWorld.y<<" "<<guardDogDirectionWorld.z<<std::endl;
		for (uint32_t x = 0; x < horizontalRays; x++) {
			float horizontalAngle = - (guardDogHorizontalFov / 2) + (x * horizontalStep);
			glm::vec3 horizontalDirection = glm::angleAxis(glm::radians(horizontalAngle), glm::vec3(0.0f, 0.0f, 1.0f)) * guardDogDirectionWorld;

			for (uint32_t z = 0; z < verticalRays; z++) {
				float verticalAngle = - (guardDogVerticalFov / 2) + (z * verticalStep);
				glm::vec3 direction = glm::angleAxis(glm::radians(verticalAngle), glm::vec3(1.0f, 0.0f, 0.0f)) * horizontalDirection;
				Ray r = Ray(point, direction, glm::vec2(0.0f, 2.0f), (uint32_t)0);
				float closest_t = 10000000;
				std::string closest_item = "Wall";
				// loop through primitives 
				for (Scene::Drawable &d : scene.drawables) {
					
					auto item_seen = guard_detectables.find(d.transform->name);
					if ( item_seen != guard_detectables.end()) {
						GLuint start = d.mesh->start;
						GLuint count = d.mesh->count;
						glm::mat4x3 transform = d.transform->make_local_to_world();
						for (GLuint i = start; i < start + count; i+=3) {
							glm::vec3 a = transform * glm::vec4(d.meshBuffer->data[i].Position, 1);
							glm::vec3 b = transform * glm::vec4(d.meshBuffer->data[i + 1].Position, 1);
							glm::vec3 c = transform * glm::vec4(d.meshBuffer->data[i + 2].Position, 1);
							glm::vec3 ab = a - b;
							glm::vec3 ac = a - c;
							glm::vec3 cb = c - b;
							glm::vec3 ca = c - a;
							glm::vec3 ba = b - a;
							glm::vec3 bc = b - c;
							glm::vec3 normal = glm::cross(ba, cb);
							float denominator = glm::dot(normal, r.dir);

							if (glm::abs(denominator) > 0.00001f) {
								float t = glm::dot(a - r.point, normal) / denominator;
								// if the ray intersects the abc plane
								if (t >= 0.00001f && t <= visionDistance) {
									glm::vec3 p = r.at(t);
									glm::vec3 ap = a - p;
									glm::vec3 bp = b - p;
									glm::vec3 cp = c - p;
									if (glm::dot(glm::cross(ac, ab), glm::cross(ac, ap)) > 0 && 
										glm::dot(glm::cross(cb, ca), glm::cross(cb, cp)) > 0 &&
										glm::dot(glm::cross(ba, bc), glm::cross(ba, bp)) > 0) {
											closest_t = std::min(t, closest_t);
											if (t <= closest_t) {
												// guard sees something
												closest_item = d.transform->name;
											}
										}
								}
							}
						}
					}
				}
				guard_detectables[closest_item] = true;
				Vertex ray_vertex;
				ray_vertex.Position = r.at(closest_t);
				ray_vertex.Color = glm::u8vec4(0x88, 0x00, 0xff, 0xff);
				// add vertices
				if (x == 0 && z == 0) {
					guard_fov_data.push_back(ray_vertex);
					guard_fov_data.push_back(point_vertex);
				} else if (x == horizontalRays - 1 && z == horizontalRays - 1) {
					guard_fov_data.push_back(ray_vertex);
				} else {
					guard_fov_data.push_back(ray_vertex);
					guard_fov_data.push_back(ray_vertex);
					guard_fov_data.push_back(point_vertex);
				}
			}
		}
		// guard_fov_meshes->ChangeBuffer(guard_fov_data);
		// for (Scene::Drawable &d : scene.drawables) {
		// 	if (d.transform->name == "FOV") {
		// 		d.meshBuffer = guard_fov_meshes;
		// 	}
		// }

    }
}

void Level::update_animation(const float deltaTime) {
    for(auto& driver: drivers){
        driver->animate(deltaTime);
    }
}

void Level::exit() {
	driver_rope_ascend->start();
	player_transform->position.x = exit_transform->position.x+0.5f;
	player_transform->position.y = exit_transform->position.y;
	player_transform->rotation = glm::angleAxis(glm::radians(90.f),glm::vec3(0.0f, 0.0f, 1.0f));
	driver_player_ascend->add_move_in_straight_line_anim(player_transform->position+glm::vec3(0,0,0.5f), player_transform->position+glm::vec3(0,0,5.0f), 5.0f, 3);
	driver_player_ascend->start();
}

bool Level::is_exit_finished() {
	return driver_rope_ascend->finished && driver_player_ascend->finished;
}

bool Level::is_target_obtained() {
	return std::any_of(level_targets.begin(), level_targets.end(), [](uint32_t v) { return v>0; });
}

GLint gen_texture_from_img(const std::string img_path) {
	std::vector< glm::u8vec4 > data;
	glm::uvec2 size;
	load_png(data_path(img_path), &size, &data, UpperLeftOrigin);

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size[0], size[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	return tex;
}