#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"
#include "TextTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "Ray.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <random>

GLuint level1_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > level1_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("level1.pnct"));
	level1_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > level1_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("level1.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = level1_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = level1_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

		// if this doenst work, ask on discord, ping Jim
		drawable.meshBuffer = &(*level1_meshes);
		drawable.mesh = &mesh;

	});
});

WalkMesh const *walkmesh = nullptr;
Load< WalkMeshes > level1_walkmeshes(LoadTagDefault, []() -> WalkMeshes const * {
	WalkMeshes *ret = new WalkMeshes(data_path("level1.w"));
	walkmesh = &ret->lookup("WalkMesh");
	return ret;
});

PlayMode::PlayMode() : scene(*level1_scene) {
	for (auto &transform : scene.transforms) {
		if (transform.name == "RedPanda") player.transform = &transform;
		else if (transform.name == "Bone") bone = &transform;
		else if (transform.name == "GuardDog") guardDog = &transform;
	}

	if (bone == nullptr) throw std::runtime_error("Bone not found.");
	else if (guardDog == nullptr) throw std::runtime_error("GuardDog not found.");

	player.rotation_euler = glm::eulerAngles(player.transform->rotation) / float(M_PI) * 180.0f;
	player.rotation = player.transform->rotation;

	//create a player camera attached to a child of the player transform:
	player.camera = &scene.cameras.front();

	//create a player camera attached to a child of the player transform:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	player.camera = &scene.cameras.front();
	camera_transform = player.camera->transform->position - player.transform->position;

	//start player walking at nearest walk point:
	player.at = walkmesh->nearest_walk_point(player.transform->position);

	ui = std::make_shared<UI>();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LEFT) {
			left_arrow.downs += 1;
			left_arrow.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right_arrow.downs += 1;
			right_arrow.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_i) {
			key_i.downs += 1;
			key_i.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RETURN) {
			enter.downs += 1;
			enter.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_b) {
			key_b.downs += 1;
			key_b.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LEFT) {
			left_arrow.pressed = false;
			ui->arrow_key_callback(true);
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right_arrow.pressed = false;
			ui->arrow_key_callback(false);
			return true;
		} else if (evt.key.keysym.sym == SDLK_i) {
			key_i.pressed = false;
			if(ui->showing_interactable_button) {
				ui->show_description("You found a bone. Do you want to collect it?", "Yes", "No");
			}
			return true;
		} else if (evt.key.keysym.sym == SDLK_RETURN) {
			enter.pressed = false;
			if(ui->showing_description) {
				ui->reset();
				if(!showing_inventory_description) {
					// Interact with item
					if(ui->choice_id ==0) {
						std::cout << "Interact with item: yes" << std::endl;
						ui->add_inventory_item("bone", "UI/bone.png");
						// hide bone
						bone->position.z = -500.0f;
					} else {
						std::cout << "Interact with item: no" << std::endl;
						ui->set_interactable_button(/*hide=*/false);
					}
				} else {
					// Interact with inventory
					if(ui->choice_id == 0) {
						// use item
						std::cout << "Use item: yes" << std::endl;
						ui->remove_inventory_item("bone");
						// TODO: set bone position to somewhere around the dog
					} else {
						std::cout << "Use item: no" << std::endl;
						ui->set_inventory(false);
					}
					showing_inventory_description = false;
				}
			} else if (ui->showing_inventory && ui->inventory_items.size() > 0 && ui->inventory_slot_selected_id == 0) {
				ui->show_description("Do you want to use the bone to distract the guard?", "Yes", "No");
				showing_inventory_description = true;
			}
			
			return true;
		} else if (evt.key.keysym.sym == SDLK_b) {
			key_b.pressed = false;
			ui->set_inventory(ui->showing_inventory);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	//player walking:
	{
		//combine inputs into a move:
		constexpr float playerSpeed = 20.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) {
			move.x =-1.0f;
			player.transform->rotation = player.rotation * glm::angleAxis(glm::radians(90.f),glm::vec3(0.0f, 0.0f, 1.0f));
		}
		if (!left.pressed && right.pressed) {
			move.x =-1.0f;
			player.transform->rotation = player.rotation * glm::angleAxis(glm::radians(-90.f),glm::vec3(0.0f, 0.0f, 1.0f));
		}
		if (down.pressed && !up.pressed) {
			move.x =-1.0f;
			player.transform->rotation = player.rotation * glm::angleAxis(glm::radians(180.f),glm::vec3(0.0f, 0.0f, 1.0f));
		}
		if (!down.pressed && up.pressed) {
			move.x =-1.0f;
			player.transform->rotation = player.rotation * glm::angleAxis(glm::radians(0.f),glm::vec3(0.0f, 0.0f, 1.0f));
		}

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * playerSpeed * elapsed;

		//get move in world coordinate system:
		glm::vec3 remain = player.transform->make_local_to_world() * glm::vec4(move.x, move.y, 0.0f, 0.0f);

		//using a for() instead of a while() here so that if walkpoint gets stuck in
		// some awkward case, code will not infinite loop:
		for (uint32_t iter = 0; iter < 10; ++iter) {
			if (remain == glm::vec3(0.0f)) break;
			WalkPoint end;
			float time;
			walkmesh->walk_in_triangle(player.at, remain, &end, &time);
			player.at = end;
			if (time == 1.0f) {
				//finished within triangle:
				remain = glm::vec3(0.0f);
				break;
			}
			//some step remains:
			remain *= (1.0f - time);
			//try to step over edge:
			glm::quat rotation;
			if (walkmesh->cross_edge(player.at, &end, &rotation)) {
				//stepped to a new triangle:
				player.at = end;
				//rotate step to follow surface:
				remain = rotation * remain;
			} else {
				//ran into a wall, bounce / slide along it:
				glm::vec3 const &a = walkmesh->vertices[player.at.indices.x];
				glm::vec3 const &b = walkmesh->vertices[player.at.indices.y];
				glm::vec3 const &c = walkmesh->vertices[player.at.indices.z];
				glm::vec3 along = glm::normalize(b-a);
				glm::vec3 normal = glm::normalize(glm::cross(b-a, c-a));
				glm::vec3 in = glm::cross(normal, along);

				//check how much 'remain' is pointing out of the triangle:
				float d = glm::dot(remain, in);
				if (d < 0.0f) {
					//bounce off of the wall:
					remain += (-1.25f * d) * in;
				} else {
					//if it's just pointing along the edge, bend slightly away from wall:
					remain += 0.01f * d * in;
				}
			}
		}

		if (remain != glm::vec3(0.0f)) {
			std::cout << "NOTE: code used full iteration budget for walking." << std::endl;
		}

		//update player's position to respect walking:
		player.transform->position = walkmesh->to_world_point(player.at);
		player.camera->transform->position = player.transform->position + camera_transform;

		{ //update player's rotation to respect local (smooth) up-vector:
			
			glm::quat adjust = glm::rotation(
				player.transform->rotation * glm::vec3(0.0f, 0.0f, 1.0f), //current up vector
				walkmesh->to_world_smooth_normal(player.at) //smoothed up vector at walk location
			);
			player.transform->rotation = glm::normalize(adjust * player.transform->rotation);
		}

		/*
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * forward;
		*/
	}

	// Field of view collisions
	{
		constexpr float guardDogVerticalFov = 90.0f;
		constexpr float guardDogHorizontalFov = 120.0f;
		constexpr uint32_t horizontalRays = 30;
		constexpr uint32_t verticalRays = 20;
		float horizontalStep = guardDogHorizontalFov / horizontalRays;
		float verticalStep = guardDogVerticalFov / verticalRays;
		float visionDistance = 2.0f;
		glm::vec3 guardDogPositionWorld = guardDog->make_local_to_world() * glm::vec4(0, 0, 0, 1);
		glm::vec3 guardDogDirectionWorld = glm::normalize(guardDog->make_local_to_world() * glm::vec4(glm::vec3(0.0, -1.0, 0.0) - guardDogPositionWorld, 1));

		for (uint32_t x = 0; x < horizontalRays; x++) {
			float horizontalAngle = - (guardDogHorizontalFov / 2) + (x * horizontalStep);
			glm::vec3 horizontalDirection = glm::angleAxis(glm::radians(horizontalAngle), glm::vec3(0.0f, 0.0f, 1.0f)) * guardDogDirectionWorld;
			for (uint32_t z = 0; z < verticalRays; z++) {
				float verticalAngle = - (guardDogVerticalFov / 2) + (z * verticalStep);
				glm::vec3 direction = glm::angleAxis(glm::radians(verticalAngle), glm::vec3(1.0f, 0.0f, 0.0f)) * horizontalDirection;
				glm::vec3 point = guardDog->position + glm::vec3(0.0f, 0.0f, 1.8f);
				Ray r = Ray(point, direction, glm::vec2(0.0f, 2.0f), (uint32_t)0);
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

							// if the ray intersects the abc plane
							if (t >= 0.00001f && t <= visionDistance) {
								glm::vec3 p = r.at(t);
								glm::vec3 ap = a - p;
								glm::vec3 bp = b - p;
								glm::vec3 cp = c - p;
								if (glm::dot(glm::cross(ac, ab), glm::cross(ac, ap)) > 0 && 
									glm::dot(glm::cross(cb, ca), glm::cross(cb, cp)) > 0 &&
									glm::dot(glm::cross(ba, bc), glm::cross(ba, bp)) > 0) {
										std::cout<<"true"<<std::endl;
									}
							}
						}


					}
				}

			}
		}

	}

	{
		// check for interaction
		if (!ui->showing_interactable_button && glm::distance(player.transform->position, bone->position) < 3.0f) {
			ui->set_interactable_button(/*hide=*/false);
		} else if(ui->showing_interactable_button && glm::distance(player.transform->position, bone->position) > 3.0f) {
			ui->set_interactable_button(/*hide=*/true);
		}
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	left_arrow.downs = 0;
	right_arrow.downs = 0;
	key_i.downs = 0;
	enter.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	player.camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*player.camera);

	/* In case you are wondering if your walkmesh is lining up with your scene, try:
	{
		glDisable(GL_DEPTH_TEST);
		DrawLines lines(player.camera->make_projection() * glm::mat4(player.camera->transform->make_world_to_local()));
		for (auto const &tri : walkmesh->triangles) {
			lines.draw(walkmesh->vertices[tri.x], walkmesh->vertices[tri.y], glm::u8vec4(0x88, 0x00, 0xff, 0xff));
			lines.draw(walkmesh->vertices[tri.y], walkmesh->vertices[tri.z], glm::u8vec4(0x88, 0x00, 0xff, 0xff));
			lines.draw(walkmesh->vertices[tri.z], walkmesh->vertices[tri.x], glm::u8vec4(0x88, 0x00, 0xff, 0xff));
		}
	}
	*/

	// Draw UI
    {	
		ui->update_texture();
		ui->draw();
    }

	{ //use DrawLines to overlay some text:
		// glDisable(GL_DEPTH_TEST);
		// float aspect = float(drawable_size.x) / float(drawable_size.y);
		// DrawLines lines(glm::mat4(
		// 	1.0f / aspect, 0.0f, 0.0f, 0.0f,
		// 	0.0f, 1.0f, 0.0f, 0.0f,
		// 	0.0f, 0.0f, 1.0f, 0.0f,
		// 	0.0f, 0.0f, 0.0f, 1.0f
		// ));

		// constexpr float H = 0.09f;
		// lines.draw_text("Mouse motion looks; WASD moves; escape ungrabs mouse",
		// 	glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
		// 	glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
		// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		// float ofs = 2.0f / drawable_size.y;
		// lines.draw_text("Mouse motion looks; WASD moves; escape ungrabs mouse",
		// 	glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
		// 	glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
		// 	glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();


}
