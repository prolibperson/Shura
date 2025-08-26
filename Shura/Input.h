#pragma once
#include "Camera.h"
#include "Globals.h"
#include <glm/glm.hpp>

class Input {
public:
	void handle_event(const SDL_Event& event, SDL_Window* window);
	void update_camera(float deltaTime);

	Camera camera_inst;
	bool lctrl = false;

	float MOVE_SPEED = 200.0f;
	float MOUSE_SENSITIVITY = 0.13f;
	float PITCH_LIMIT = 89.0f;
	float PITCH_LIMIT_RAD = glm::radians(PITCH_LIMIT);

private:
	bool w = false;
	bool s = false;
	bool a = false;
	bool d = false;
	bool lshift = false;
};