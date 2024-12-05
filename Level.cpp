#include "Level.hpp"
#include  "Mesh.hpp"
#include "Level1.hpp"

#include <algorithm>
#include <glm/gtx/norm.hpp>

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
	for (auto guardDog: guard_dogs) guardDog->guard_fov_data.clear();

    for (auto guardDog: guard_dogs) {
		// if (guardDog->name == "GuardDog1") {
        constexpr float guardDogHorizontalFov = 100.0f;
		constexpr float guardDogVerticalFov = 90.0f;
		constexpr uint32_t verticalRays = 15;
		constexpr uint32_t horizontalRays = 17;
		float verticalStep = guardDogVerticalFov / verticalRays;
		float horizontalStep = guardDogHorizontalFov / horizontalRays;
		float visionDistance = 5.2f;
		glm::vec3 point = guardDog->transform->position + glm::vec3(0.0, 0.0, 0.7);
		Vertex point_vertex;
		point_vertex.Position = guardDog->guard_fov_transform->make_world_to_local() * glm::vec4(point, 1);
		point_vertex.Color = glm::u8vec4(0xff, 0x00, 0x00, 0x7);
		glm::vec3 guardDogDirectionWorld = glm::normalize(guardDog->transform->make_local_to_world() * glm::vec4(-1.0, 0.0, 0.0, 0.0));
		for (uint32_t x = 0; x < verticalRays; x++) {
			float verticalAngle = (guardDogVerticalFov / 2) + (x * verticalStep);
			glm::vec3 verticalDirection = glm::angleAxis(glm::radians(verticalAngle), glm::vec3(1.0f, 0.0f, 0.0f)) * guardDogDirectionWorld;
			for (uint32_t z = 0; z < horizontalRays; z++) {
				float horizontalAngle = - (guardDogHorizontalFov / 2) + (z * horizontalStep);
				glm::vec3 direction = glm::angleAxis(glm::radians(horizontalAngle), glm::vec3(0.0f, 0.0f, 1.0f)) * verticalDirection;
				Ray r = Ray(point, direction, glm::vec2(0.0f, 5.2f), (uint32_t)0);
				float closest_t = 5.2f;
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
				ray_vertex.Position = guardDog->guard_fov_transform->make_world_to_local() * glm::vec4(r.at(closest_t), 1);
				ray_vertex.Color = glm::u8vec4(0xff, 0x00, 0x00, 0x20);
				// add vertices
				if (x == 0 && z == 0) {
					guardDog->guard_fov_data.push_back(ray_vertex);
					guardDog->guard_fov_data.push_back(point_vertex);
				} else if (x == verticalRays - 1 && z == horizontalRays - 1) {
					guardDog->guard_fov_data.push_back(ray_vertex);
				} else {
					guardDog->guard_fov_data.push_back(ray_vertex);
					guardDog->guard_fov_data.push_back(ray_vertex);
					guardDog->guard_fov_data.push_back(point_vertex);
				}
			}
		}
		std::reverse(guardDog->guard_fov_data.begin(), guardDog->guard_fov_data.end());
		guardDog->guard_fov_meshes->ChangeBuffer(guardDog->guard_fov_data);
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
	driver_player_ascend->add_move_in_straight_line_anim(player_transform->position+glm::vec3(0,0,0.5f), player_transform->position+glm::vec3(0,0,5.0f), rope_move_time, 3);
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

bool Level::check_laser_hits() {
	auto barycentric_weights = [&](glm::vec3 const &a, glm::vec3 const &b, glm::vec3 const &c, glm::vec3 const &pt) {
		// Reference: https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
		glm::vec3 v0 = b - a, v1 = c - a, v2 = pt - a;
		float d00 = glm::dot(v0, v0);
		float d01 = glm::dot(v0, v1);
		float d11 = glm::dot(v1, v1);
		float d20 = glm::dot(v2, v0);
		float d21 = glm::dot(v2, v1);
		float denom = d00 * d11 - d01 * d01;
		float v = (d11 * d20 - d01 * d21) / denom;
		float w = (d00 * d21 - d01 * d20) / denom;
		float u = 1.0f - v - w;
		return glm::vec3(u,v,w);
	};
	for (Scene::Drawable &player: scene.drawables) {
		if (player.transform->name == "RedPanda") {
			GLuint player_start = player.mesh->start;
			GLuint player_count = player.mesh->count;
			glm::mat4x3 new_player_transform = player.transform->make_local_to_world();
			for (GLuint j = player_start; j < player_start + player_count; j+= 12) {
				glm::vec3 position = new_player_transform * glm::vec4(player.meshBuffer->data[j].Position, 1);
				for (Scene::Drawable &d: scene.drawables) {
					if (d.transform->name.find("Laser") != std::string::npos || d.transform->name.find("laser") != std::string::npos) {
						GLuint start = d.mesh->start;
						GLuint count = d.mesh->count;
						glm::mat4x3 transform = d.transform->make_local_to_world();
						// glm::vec3 target_pos = position + glm::vec3(dir.x * distance, dir.y * distance, dir.z * distance);
						for (GLuint i = start; i < start + count; i += 3) {
							glm::vec3 const &a = transform * glm::vec4(d.meshBuffer->data[i].Position, 1);
							glm::vec3 const &b = transform * glm::vec4(d.meshBuffer->data[i + 1].Position, 1);
							glm::vec3 const &c = transform * glm::vec4(d.meshBuffer->data[i + 2].Position, 1);
							glm::vec3 coords = barycentric_weights(a, b, c, position);
							glm::vec3 worldCoords = coords.x * a + coords.y * b + coords.z * c;
							if (coords.x >= 0 && coords.y >= 0 && coords.z >= 0) {
								float dist = glm::length2(position - worldCoords);
								if (dist < 0.005) {
									for (auto l : lasers) {
										if (l->name == d.transform->name) return true;
									}
								}
								
							}
						}
					}
				}
			}
		}
	}
	return false;
}

void Level::move_lasers() {
	for (auto laser : lasers) {
		if (laser->on) {
			laser->transform->position = laser->spawn_point;
		} else {
			laser->transform->position = glm::vec3(100, 100, 100);
		}
	}
}

void Level::reset_items() {
    for(auto &item: items) {
        item.second->transform->position = item.second->spawn_point;
        item.second->added = false;
		item.second->interactable = true;
    }
}