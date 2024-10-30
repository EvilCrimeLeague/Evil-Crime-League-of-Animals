#include "Level.hpp"

Level::Level(Scene const * scene_, std::shared_ptr<UI> ui_): scene(*scene_), ui(ui_) {
    for (auto &transform : scene.transforms) {
        if (transform.name == "RedPanda") player_transform = &transform;
        else if (transform.name == "Jewel") target_transform = &transform;
    }

    if (target_transform == nullptr) throw std::runtime_error("Target not found.");
    else if (player_transform == nullptr) throw std::runtime_error("Player not found.");

    if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
    camera = &scene.cameras.front();
};

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

bool Level::update_guard() {
    bool seen_by_guard = false;
    for (auto guardDog: guard_dogs) {
        constexpr float guardDogVerticalFov = 90.0f;
		constexpr float guardDogHorizontalFov = 120.0f;
		constexpr uint32_t horizontalRays = 20;
		constexpr uint32_t verticalRays = 10;
		float horizontalStep = guardDogHorizontalFov / horizontalRays;
		float verticalStep = guardDogVerticalFov / verticalRays;
		float visionDistance = 5.0f;
		glm::vec3 guardDogPositionWorld = guardDog->transform->make_local_to_world() * glm::vec4(0, 0, 0, 1);
		glm::vec3 guardDogDirectionWorld = glm::normalize(guardDog->transform->make_local_to_world() * glm::vec4(glm::vec3(0.0, -1.0, 0.0) - guardDogPositionWorld, 1));

		for (uint32_t x = 0; x < horizontalRays; x++) {
			float horizontalAngle = - (guardDogHorizontalFov / 2) + (x * horizontalStep);
			glm::vec3 horizontalDirection = glm::angleAxis(glm::radians(horizontalAngle), glm::vec3(0.0f, 0.0f, 1.0f)) * guardDogDirectionWorld;
			for (uint32_t z = 0; z < verticalRays; z++) {
				float closest_t = 10000000;
				float verticalAngle = - (guardDogVerticalFov / 2) + (z * verticalStep);
				glm::vec3 direction = glm::angleAxis(glm::radians(verticalAngle), glm::vec3(1.0f, 0.0f, 0.0f)) * horizontalDirection;
				glm::vec3 point = guardDog->transform->position + glm::vec3(0.0f, 0.0f, 1.8f);
				direction.y = -direction.y;
				Ray r = Ray(point, direction, glm::vec2(0.0f, 2.0f), (uint32_t)0);
				// std::cout<<"point: "<<point.x<<" "<<point.y<<" "<<point.z<<std::endl;
				// std::cout<<"dir: "<<direction.x<<" "<<direction.y<<" "<<direction.z<<std::endl;
				// loop through primitives 
				for (Scene::Drawable &d : scene.drawables) {
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
										if ((t <= closest_t) && (d.transform->name == "RedPanda")) {
											// player seen by guard
											seen_by_guard = true;
										}
										
									}
							}
						}


					}
				}

			}
		}
    }


    return seen_by_guard;
}