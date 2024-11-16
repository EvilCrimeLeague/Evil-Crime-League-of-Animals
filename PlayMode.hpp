#include "Mode.hpp"

#include "Scene.hpp"
#include "WalkMesh.hpp"
#include "UI.hpp"
#include "Level.hpp"
#include "Level1.hpp"
#include "Level2.hpp"

#include <glm/glm.hpp>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, left_arrow, right_arrow, key_f/*interact*/, enter/*make choice*/, key_e/*show inventory*/, key_r/*restart*/, key_q/*show menu*/, key_1, key_2, key_3, key_4, key_5, key_6, key_7, key_8, key_9, key_0;

	//camera:
	glm::vec3 camera_transform;

	//player info:
	struct Player {
		WalkPoint at;
		//transform is at player's feet and will be yawed by mouse left/right motion:
		Scene::Transform *transform = nullptr;
		//camera is at player's head and will be pitched by mouse up/down motion:
		Scene::Camera *camera = nullptr;
		glm::vec3 rotation_euler;
		glm::quat rotation;
	} player;

	// UI:
    std::shared_ptr<UI> ui;

	// levels:
	std::vector<std::shared_ptr<Level> > levels;
	std::shared_ptr<Level> level;
	int level_id = 0;

	// game status:
	std::shared_ptr<Level::Item> curr_item;

	bool game_over = false;
	bool paused = false;

	float seen_by_guard_timer = 0.0f;

	// helper functions
	float get_distance(glm::vec3 a, glm::vec3 b) {
		// get distance between two points disregarding z
		a.z = 0.0f;
		b.z = 0.0f;
		return glm::distance(a, b);
	}

	void restart(bool new_level=false);
	float walk_timer = 0;
	float speed_percent = 0;
	float playerSpeed = 0;

	GameInfo game_info;
};
