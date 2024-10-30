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

PlayMode::PlayMode() {
	ui = std::make_shared<UI>();

	auto level1 = std::make_shared<Level1>(ui);
	levels.push_back(level1);
	level = level1;

	player.transform = level->player_transform;
	player.rotation_euler = glm::eulerAngles(player.transform->rotation) / float(M_PI) * 180.0f;
	player.rotation = player.transform->rotation;

	//create a player camera attached to a child of the player transform:
	camera = level->camera;
	player.camera = camera;

	camera_transform = player.camera->transform->position - player.transform->position;

	//start player walking at nearest walk point:
	player.at = level->walkmesh->nearest_walk_point(player.transform->position);
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
		} else if (evt.key.keysym.sym == SDLK_f) {
			key_f.downs += 1;
			key_f.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RETURN) {
			enter.downs += 1;
			enter.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_e) {
			key_e.downs += 1;
			key_e.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_r) {
			key_r.downs += 1;
			key_r.pressed = true;
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
		} else if (evt.key.keysym.sym == SDLK_f) {
			key_f.pressed = false;
			level->handle_interact_key();
			return true;
		} else if (evt.key.keysym.sym == SDLK_RETURN) {
			enter.pressed = false;
			level->handle_enter_key();
			return true;
		} else if (evt.key.keysym.sym == SDLK_e) {
			key_e.pressed = false;
			ui->set_inventory(ui->showing_inventory);
			return true;
		} else if (evt.key.keysym.sym == SDLK_r) {
			key_r.pressed = false;
			restart();
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	//player walking:
	paused = ui->showing_inventory_description || ui->showing_description;
	if(game_over || paused) 
		return;

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
			level->walkmesh->walk_in_triangle(player.at, remain, &end, &time);
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
			if (level->walkmesh->cross_edge(player.at, &end, &rotation)) {
				//stepped to a new triangle:
				player.at = end;
				//rotate step to follow surface:
				remain = rotation * remain;
			} else {
				//ran into a wall, bounce / slide along it:
				glm::vec3 const &a = level->walkmesh->vertices[player.at.indices.x];
				glm::vec3 const &b = level->walkmesh->vertices[player.at.indices.y];
				glm::vec3 const &c = level->walkmesh->vertices[player.at.indices.z];
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
		player.transform->position = level->walkmesh->to_world_point(player.at);
		player.camera->transform->position = player.transform->position + camera_transform;

		{ //update player's rotation to respect local (smooth) up-vector:
			
			glm::quat adjust = glm::rotation(
				player.transform->rotation * glm::vec3(0.0f, 0.0f, 1.0f), //current up vector
				level->walkmesh->to_world_smooth_normal(player.at) //smoothed up vector at walk location
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

	{
		// update animation
		level->update_animation(elapsed);
	}

	// Field of view collisions
	{
		seen_by_guard = level->update_guard();
	}

	{
		if(seen_by_guard_timer > 1.0f) {
			// player caught, restart game
			restart();
		}
		if(seen_by_guard) {
			if(!ui->showing_alarm) {
				ui->set_alarm(/*hide=*/false);
			}
			seen_by_guard_timer += elapsed;
		} else {
			ui->set_alarm(/*hide=*/true);
			seen_by_guard_timer = 0.0f;
		}
	}

	{
		// player and item interaction
		curr_item = level->get_closest_item(player.transform->position);
		if(!ui->showing_interactable_button && curr_item!=nullptr) {
			ui->set_interactable_button(/*hide=*/false);
		} else if (ui->showing_interactable_button && curr_item==nullptr) {
			ui->set_interactable_button(/*hide=*/true);
		}

		if(get_distance(player.transform->position, level->target_transform->position) < 2.0f) {
			game_over = true;
			ui->show_game_over(true);
		}
	}

	

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	left_arrow.downs = 0;
	right_arrow.downs = 0;
	key_f.downs = 0;
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

	level->scene.draw(*player.camera);

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

void PlayMode::restart(){
	ui->reset();
	seen_by_guard_timer = 0.0f;
	seen_by_guard = false;
	game_over = false;
	paused = false;
	player.transform->position = level->player_spawn_point;
	player.at = level->walkmesh->nearest_walk_point(player.transform->position);
	level->restart();
}
