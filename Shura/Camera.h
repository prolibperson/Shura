#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Globals.h"

class Camera {
public:
    bool init(const glm::vec3& pos = glm::vec3(0.0f, 2.0f, 5.0f),
        const glm::vec3& up_vec = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw_deg = 0.0f, float pitch_deg = 0.0f, float fov_deg = 60.0f,
        float aspect = 16.0f / 9.0f, float near_plane = 0.1f,
        float far_plane = 100.0f);

    void set_model(const glm::mat4& m);
    void set_camera_pos(const glm::vec3& pos);
    void set_up(const glm::vec3& u);
    void set_front(const glm::vec3& f);
    void set_yaw(float yaw_deg);
    void set_pitch(float pitch_deg);
    void set_fov(float fov_deg);
    void set_aspect_ratio(float aspect);
    void set_near_far(float near_plane, float far_plane);
    void set_projection(const glm::mat4& proj);
    void set_view(const glm::mat4& v);

    void move(const glm::vec3& offset);
    void rotate(float yaw_offset, float pitch_offset);

    inline const glm::mat4& get_model() const { return model; }
    inline const glm::vec3& get_camera_pos() const { return camera_pos; }
    inline const glm::vec3& get_up() const { return up; }
    inline const glm::vec3& get_front() const { return front; }
    inline const glm::vec3& get_right() const { return right; }
    inline float get_yaw() const { return yaw; }
    inline float get_pitch() const { return pitch; }
    inline float get_fov() const { return fov; }
    inline float get_aspect_ratio() const { return aspect_ratio; }
    inline float get_near() const { return near_plane; }
    inline float get_far() const { return far_plane; }
    inline const glm::mat4& get_projection() const { return projection; }
    inline const glm::mat4& get_view() const { return view; }
    inline const glm::mat4& get_mvp() const { return mvp; }

private:
    void update_vectors();
    void update_view();
    void update_projection();
    void update_mvp();

private:
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 mvp = glm::mat4(1.0f);

    glm::vec3 camera_pos = glm::vec3(0.0f, 2.0f, 5.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);

    float yaw = 0.0f;
    float pitch = 0.0f;
    float fov = 60.0f;
    float aspect_ratio = 16.0f / 9.0f;
    float near_plane = 0.1f;
    float far_plane = 100.0f;
};