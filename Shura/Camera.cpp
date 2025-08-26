#include "Camera.h"
#include <algorithm>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

bool Camera::init(const glm::vec3& pos, const glm::vec3& up_vec, float yaw_deg,
    float pitch_deg, float fov_deg, float aspect, float near_plane_val,
    float far_plane_val)
{
    camera_pos = pos;
    world_up = up_vec;
    yaw = glm::radians(yaw_deg);
    pitch = glm::radians(pitch_deg);
    fov = fov_deg;
    aspect_ratio = aspect;
    near_plane = near_plane_val;
    far_plane = far_plane_val;
    model = glm::mat4(1.0f);

    update_vectors();
    update_view();
    update_projection();
    update_mvp();

    return true;
}

void Camera::set_model(const glm::mat4& m)
{
    model = m;
    update_mvp();
}

void Camera::set_camera_pos(const glm::vec3& pos)
{
    camera_pos = pos;
    update_view();
}

void Camera::set_up(const glm::vec3& u)
{
    world_up = u;
    update_vectors();
}

void Camera::set_front(const glm::vec3& f)
{
    front = glm::normalize(f);
    update_view();
}

void Camera::set_yaw(float yaw_deg)
{
    yaw = glm::radians(yaw_deg);
    update_vectors();
}

void Camera::set_pitch(float pitch_deg)
{
    pitch = glm::radians(glm::clamp(pitch_deg, -89.0f, 89.0f));
    update_vectors();
}

void Camera::set_fov(float fov_deg)
{
    fov = glm::clamp(fov_deg, 1.0f, 120.0f);
    update_projection();
}

void Camera::set_aspect_ratio(float aspect)
{
    aspect_ratio = aspect;
    update_projection();
}

void Camera::set_near_far(float near_plane_val, float far_plane_val)
{
    near_plane = near_plane_val;
    far_plane = far_plane_val;
    update_projection();
}

void Camera::set_projection(const glm::mat4& proj)
{
    projection = proj;
    update_mvp();
}

void Camera::set_view(const glm::mat4& v)
{
    view = v;
    update_mvp();
}

void Camera::move(const glm::vec3& offset)
{
    camera_pos += offset;
    update_view();
}

void Camera::rotate(float yaw_offset, float pitch_offset)
{
    yaw += glm::radians(yaw_offset);
    pitch += glm::radians(pitch_offset);

    pitch = glm::clamp(pitch, glm::radians(-89.0f), glm::radians(89.0f));
    update_vectors();
}

void Camera::update_vectors()
{
    front.x = cos(yaw) * cos(pitch);
    front.y = sin(pitch);
    front.z = sin(yaw) * cos(pitch);
    front = glm::normalize(front);

    right = glm::normalize(glm::cross(front, world_up));
    up = glm::normalize(glm::cross(right, front));
    update_view();
}

void Camera::update_view()
{
    view = glm::lookAt(camera_pos, camera_pos + front, up);
    update_mvp();
}

void Camera::update_projection()
{
    projection = glm::perspective(
        glm::radians(90.0f), aspect_ratio, near_plane, far_plane);
    update_mvp();
}

void Camera::update_mvp() { mvp = projection * view * model; }