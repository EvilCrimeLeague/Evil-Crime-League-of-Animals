// Reference: https://github.com/amarantini/Vulkan-Render/blob/main/src/include/scene/driver.h
#pragma once 

#include "Scene.hpp"
#include "read_write_chunk.hpp"

#include <string>
#include <glm/glm.hpp>
#include <fstream>

// animation chanels
const std::string CHANEL_TRANSLATION = "translation"; //3d
const std::string CHANEL_ROTATION = "rotation"; //4d
const std::string CHANEL_SCALE = "scale"; //3d

// intermediate data type for reading animation data from bytes
struct Animation {
    struct AnimationData {
        std::string name;
        std::string channel;
        std::vector<float> times;
        std::vector<glm::vec3> values3d = {}; //for translation and scle
        std::vector<glm::quat> values4d = {}; //for rotation
    };

    std::unordered_map<std::string, AnimationData> animations;

    Animation(const std::string  &filename) {
        // load animation data from file
        std::cout<<filename<<std::endl;
        std::ifstream file(filename, std::ios::binary);

        std::vector<uint32_t> num_animations_v;
	    read_chunk(file, "int0", &num_animations_v);
        uint32_t num_animations = num_animations_v[0];

        for(uint32_t i=0; i<num_animations; ++i) {
            AnimationData anim;

            std::vector< char > names;
            read_chunk(file, "str0", &names);
            std::string name = std::string(names.begin(), names.end());

            auto pos = name.find("-");
            anim.name = name;
            anim.channel = name.substr(pos + 1, name.size() - pos - 1);

            std::vector< float > times;
            read_chunk(file, "time", &times);
            anim.times = times;

            if(anim.channel == CHANEL_TRANSLATION || anim.channel == CHANEL_SCALE) {
                std::vector< glm::vec3 > values3d;
                read_chunk(file, "val3", &values3d);
                anim.values3d = values3d;
            } else if(anim.channel == CHANEL_ROTATION) {
                std::vector< glm::quat > values4d;
                read_chunk(file, "val4", &values4d);
                anim.values4d = values4d;
            } else {
                throw std::runtime_error("Unknown animation channel: "+anim.channel);
            }

            animations[anim.name]=anim;
        }
    }
};

struct Driver {
    std::string name;
    std::string channel;
    std::vector<float> times;
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

    Driver(const Animation::AnimationData& anim) {
        name = anim.name;
        channel = anim.channel;
        times = anim.times;
        values3d = anim.values3d;
        values4d = anim.values4d;
    }

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

    void stop();
};