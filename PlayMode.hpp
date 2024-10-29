#include "Mode.hpp"

#include "Scene.hpp"
#include "WalkMesh.hpp"
#include "UI.hpp"

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
	} left, right, down, up, left_arrow, right_arrow, key_f/*interact*/, enter/*make choice*/, key_e/*show inventory*/;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//camera:
	Scene::Camera *camera = nullptr;

	Scene::Transform *guardDog = nullptr;
	Scene::Transform *bone = nullptr;
	Scene::Transform *jewel = nullptr;
	Scene::Transform *fov = nullptr;

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
		glm::vec3 spawn_point;
	} player;

    std::shared_ptr<UI> ui;

	bool showing_inventory_description = false;

	bool game_over = false;
	bool paused = false;

	float get_distance(glm::vec3 a, glm::vec3 b) {
		// get distance between two points disregarding z
		a.z = 0.0f;
		b.z = 0.0f;
		return glm::distance(a, b);
	}

	const float iteractable_distance = 2.0f;
};
