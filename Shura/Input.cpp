#include "Input.h"
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

void Input::handle_event(const SDL_Event& event, SDL_Window* window)
{
    switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        bool down = (event.type == SDL_EVENT_KEY_DOWN);
        switch (event.key.scancode) {
        case SDL_SCANCODE_W:
            w = down;
            break;
        case SDL_SCANCODE_S:
            s = down;
            break;
        case SDL_SCANCODE_A:
            a = down;
            break;
        case SDL_SCANCODE_D:
            d = down;
            break;
        case SDL_SCANCODE_LSHIFT:
            lshift = down;
            break;
        case SDL_SCANCODE_LCTRL: {
            if (!down) {
                lctrl = !lctrl;
                // SDL_SetWindowMouseGrab(window, !lctrl);
                SDL_SetWindowRelativeMouseMode(window, !lctrl);
                break;
            }
        }
        }
        break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
        if (lctrl)
            break;

        float xrel = static_cast<float>(event.motion.xrel);
        float yrel = static_cast<float>(event.motion.yrel);

        float yawOffset = xrel * MOUSE_SENSITIVITY;
        float pitchOffset = -yrel * MOUSE_SENSITIVITY;

        camera_inst.rotate(yawOffset, pitchOffset);
        break;
    }
    }
}

void Input::update_camera(float deltaTime)
{
    if (lctrl)
        return;

    glm::vec3 pos = camera_inst.get_camera_pos();
    glm::vec3 front = glm::normalize(camera_inst.get_front());
    glm::vec3 right = glm::normalize(camera_inst.get_right());

    float velocity{};
    if (lshift)
        velocity = MOVE_SPEED * 4 * deltaTime;
    else
        velocity = MOVE_SPEED * deltaTime;

    if (w)
        pos += front * velocity;
    if (s)
        pos -= front * velocity;
    if (a)
        pos -= right * velocity;
    if (d)
        pos += right * velocity;

    camera_inst.set_camera_pos(pos);
}