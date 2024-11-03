#include "Driver.hpp"

void Driver::restart(){
    finished = false;
    frame_time = 0.0f;
    frame_idx = 0;
}

bool Driver::isFinished(const float t){
    frame_time += t;
    if(frame_time > times.back()){
        if(!loop) {
            finished = true;
            return true;
        } 
        else {
            frame_time -= times.back();
            frame_idx = 0;
        }
    }
    while(times[frame_idx+1]<frame_time){
        frame_idx++;
    }
    return false;
}

void Driver::animate(const float deltaTime){
    if(finished || !playing)
        return;
    if(isFinished(deltaTime)){
        // std::cout<<name<<" animation finished\n";
        return;
    } else {
        float t = (frame_time-times[frame_idx])/(times[frame_idx+1]-times[frame_idx]);
        if(channel==CHANEL_SCALE){
            transform->scale = glm::mix(values3d[frame_idx], values3d[frame_idx+1], t);
        } else if (channel==CHANEL_TRANSLATION){
            transform->position = glm::mix(values3d[frame_idx], values3d[frame_idx+1], t);
        } else if(channel == CHANEL_ROTATION) {
            transform->rotation = glm::slerp(values4d[frame_idx], values4d[frame_idx+1], t);
        }
    }
}

void Driver::setPlaybackTime(float time) {
    frame_time = time;
    frame_idx = 0;
    while(times[frame_idx+1]<frame_time){
        frame_idx++;
    }
}

void Driver::start(){
    playing = true;
}

void Driver::stop(){
    playing = false;
}