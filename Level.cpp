#include "Level.hpp"

Level::Level(std::shared_ptr<UI> ui_): ui(ui_) {};

std::shared_ptr<Level::Item> Level::get_closest_item(glm::vec3 player_position) {
    std::shared_ptr<Item> closest_item = nullptr;
    float closest_distance = interactable_distance;
    for(auto itr = items.begin(); itr != items.end(); itr++) {
        float distance = glm::distance(itr->second->transform->position, player_position);
        if(distance < closest_distance) {
            closest_distance = distance;
            closest_item = itr->second;
        }
    }
    // std::cout<<"closet distance: "<<closest_distance<<"\n";
    curr_item = closest_item;
    return closest_item;
}

void Level::update_guard_detection() {
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
		// std::vector< float > const &vertices;

		// for (Scene::Drawable &d : scene.drawables) {
		// 	auto item - guard_detectables.find(d.transform->name);
		// 	if (item != guard_detectables.end()) {
		// 		GLuint start = d.mesh->start;
		// 		GLuint count = d.mesh->count;
		// 		glm::mat4x3 transform = d.transform->make_local_to_world();
		// 		for (GLuint i = start; i < start _ count; i++) {
		// 			glm::vec3 a = transform * glm::vec4(d.meshBuffer->data[i].Position, 1);
		// 			glm::vec3 b = transform * glm::vec4(d.meshBuffer->data[i + 1].Position, 1);
		// 			glm::vec3 c = transform * glm::vec4(d.meshBuffer->data[i + 2].Position, 1);
		// 			if (a.y > point.y && b.y > point.y && c.y > point.y) {
		// 				vertices.push_back(a);
		// 				vertices.push_back(b);
		// 				vertices.push_back(c);
		// 			}
		// 		}
		// 	}
		// }
		// glm::vec3 guardDogPositionWorld = guardDog->transform->make_local_to_world() * glm::vec4(0, 0, 0, 1);
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
								//std::cout<<"ray intersects plane of "<<d.transform->name<<std::endl;
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
				if (closest_item == "Bone") {

				}
				guard_detectables[closest_item] = true;
			}
		}
    }
}

// bool Level::update_bone_dist_infront() {
// 	// level1 = std::make_shared<Level1>;
// 	auto barycentric_weights = [&](glm::vec3 const &a, glm::vec3 const &b, glm::vec3 const &c, glm::vec3 const &pt) {
// 		// Reference: https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
// 		glm::vec3 v0 = b - a, v1 = c - a, v2 = pt - a;
// 		float d00 = glm::dot(v0, v0);
// 		float d01 = glm::dot(v0, v1);
// 		float d11 = glm::dot(v1, v1);
// 		float d20 = glm::dot(v2, v0);
// 		float d21 = glm::dot(v2, v1);
// 		float denom = d00 * d11 - d01 * d01;
// 		float v = (d11 * d20 - d01 * d21) / denom;
// 		float w = (d00 * d21 - d01 * d20) / denom;
// 		float u = 1.0f - v - w;
// 		return glm::vec3(u,v,w);
// 	};
// 	glm::vec3 position = level1.bone->local_to_world() * glm::vec4(0.0, 0.0, 0.0, 1.0f);
// 	float radius = 0.25;
// 	glm::vec3 dir = glm::normalize(player_transform->make_local_to_world() * glm::vec4(-1.0, 0.0, 0.0, 0.0));
// 	for (Scene::Drawable &d: scene.drawables) {
// 		if (d.transform->name != "Floor" && d.transform->name != "WalkMesh" && d.transform->name != "RedPanda") {
// 			GLuint start = d.mesh.start;
// 			GLuint count = d.mesh->count;
// 			glm::mat4x3 transform = d.transform->make_local_to_world();
// 			for (GLuint i = start; i < start + count; i += 3) {
// 				glm::vec3 const &a = transform * glm::vec4(d.meshBuffer->data[i].Position, 1);
// 				glm::vec3 const &b = transform * glm::vec4(d.meshBuffer->data[i + 1].Position, 1);
// 				glm::vec3 const &c = transform * glm::vec4(d.meshBuffer->data[i + 2].Position, 1);
// 				glm::vec3 coords = barycentric_weights(a, b, c, position);
// 				glm::vec3 worldCoords = coords.x * a + coords.y * b + coords.z * c;

// 				if (coords.x >= 0 && coords.y >= 0 && coords.z >= 0) {
// 					float distance = glm::length2(position - worldCoords);
// 					if (distance < glm::length2(radius)) {
// 						return true;
// 					}
// 				}
// 			}
// 		}
// 	}
// 	return false;
	// {
	// 	constexpr float playerVerticalFov = 10.0f;
	// 	constexpr float playerHorizontalFov = 10.0f;
	// 	constexpr uint32_t horizontalRays = 2;
	// 	constexpr uint32_t verticalRays = 2;
	// 	float horizontalStep = playerHorizontalFov / horizontalRays;
	// 	float verticalStep = playerVerticalFov / verticalRays;
	// 	closest_dist_infront = 5.0;
	// 	glm::vec3 playerDirectionWorld = glm::normalize(player_transform->make_local_to_world() * glm::vec4(-1.0, 0.0, 0.0, 0.0));
	// 	for (uint32_t x = 0; x < horizontalRays; x++) {
	// 		float horizontalAngle = - (playerHorizontalFov / 2) + (x * horizontalStep);
	// 		glm::vec3 horizontalDirection = glm::angleAxis(glm::radians(horizontalAngle), glm::vec3(0.0f, 0.0f, 1.0f)) * playerDirectionWorld;

	// 		for (uint32_t z = 0; z < verticalRays; z++) {
	// 			float verticalAngle = - (playerVerticalFov / 2) + (z * verticalStep);
	// 			glm::vec3 direction = glm::angleAxis(glm::radians(verticalAngle), glm::vec3(1.0f, 0.0f, 0.0f)) * horizontalDirection;

	// 			glm::vec3 point = player_transform->position;
	// 			Ray r = Ray(point, direction, glm::vec2(0.0f, 5.0f), (uint32_t)0);
	// 			if (x == 0 && z == 0) first = r.at(5.0);
	// 			else if (x == 0 && z == 1) second = r.at(5.0);
	// 			else if (x == 1 && z == 0) third = r.at(5);
	// 			else fourth = r.at(5);
	// 			// loop through primitives 
	// 			for (Scene::Drawable &d : scene.drawables) {
	// 				if (d.transform->name != "Floor" && d.transform->name != "RedPanda" && d.transform->name != "WalkMesh") {

	// 				GLuint start = d.mesh->start;
	// 				GLuint count = d.mesh->count;
	// 				glm::mat4x3 transform = d.transform->make_local_to_world();
	// 				for (GLuint i = start; i < start + count; i+=3) {
	// 					glm::vec3 a = transform * glm::vec4(d.meshBuffer->data[i].Position, 1);
	// 					glm::vec3 b = transform * glm::vec4(d.meshBuffer->data[i + 1].Position, 1);
	// 					glm::vec3 c = transform * glm::vec4(d.meshBuffer->data[i + 2].Position, 1);
	// 					glm::vec3 ba = a - b;
	// 					glm::vec3 ca = a - c;
	// 					glm::vec3 bc = c - b;
	// 					glm::vec3 ac = c - a;
	// 					glm::vec3 ab = b - a;
	// 					glm::vec3 cb = b - c;
	// 					glm::vec3 normal = glm::normalize(glm::cross(ba, bc));
	// 					float denominator = glm::dot(normal, r.dir);
	// 					if (glm::abs(denominator) > 0.00001f) {
	// 						float t = glm::dot(a - r.point, normal) / denominator;
	// 						// if the ray intersects the abc plane
	// 						if (t >= 0.00001f) {
	// 							glm::vec3 p = r.at(t);
	// 							glm::vec3 ap = p - a;
	// 							glm::vec3 bp = p - b;
	// 							glm::vec3 cp = p - c;
	// 							if (glm::dot(glm::cross(ac, ab), glm::cross(ac, ap)) > 0 && 
	// 								glm::dot(glm::cross(cb, ca), glm::cross(cb, cp)) > 0 &&
	// 								glm::dot(glm::cross(ba, bc), glm::cross(ba, bp)) > 0) {
 	// 									closest_dist_infront = std::min(t, closest_dist_infront);
	// 									std::cout<<closest_dist_infront<<std::endl;
	// 							}
	// 						}
	// 					}
	// 				}
	// 				}
	// 			}
	// 		}
	// 	}
	// }
    //}

void Level::update_animation(const float deltaTime) {
    for(auto& driver: drivers){
        driver->animate(deltaTime);
    }
}