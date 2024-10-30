// Reference: https://github.com/amarantini/Vulkan-Render/blob/main/src/include/scene/driver.h
#pragma once 

#include "Scene.hpp"
#include <string>
#include <glm/glm.hpp>

// animation chanels
const std::string CHANEL_TRANSLATION = "translation"; //3d
const std::string CHANEL_ROTATION = "rotation"; //4d
const std::string CHANEL_SCALE = "scale"; //3d

struct Driver {
    std::string name;
    std::string channel;
    std::vector<double> times;
    std::vector<glm::vec3> values3d = {}; //for translation and scle
    std::vector<glm::quat> values4d = {}; //for rotation
    Scene::Transform* transform;
    size_t frame_idx = 0;
    bool loop = true;
    bool finished = false;
    float frame_time = 0.0f;
    bool playing = false;

    Driver(const std::string& _name, const std::string& _channel)
        : name(_name), channel(_channel) {}

    void restart();

    bool isFinished(const float t);

    void animate(const float deltaTime);

    void setPlaybackTime(float time);

    void start();

    void add_walk_in_straight_line_anim(const glm::vec3& start, const glm::vec3& end, const float duration, int num_frames) {
        for(int i=0; i<=num_frames; i++) {
            float t = i/(float)num_frames;
            times.push_back(t*duration);
            values3d.push_back(start + t*(end-start));
        }
    }
};