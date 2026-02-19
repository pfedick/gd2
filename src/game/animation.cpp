#include <stdio.h>
#include <ppl7.h>
#include "animation.h"

AnimationDefinition::AnimationDefinition(int start, int end, bool loop, int endframe, float speed)
{
    this->seq_start = start;
    this->seq_end = end;
    this->loop = loop;
    this->endframe = endframe;
    this->default_animation_speed = speed;
    if (seq_end > seq_start)
        size = seq_end - seq_start + 1;
    else
        size = -(seq_start - seq_end + 1);
}

void AnimationCycle::start(const AnimationDefinition& animation)
{
    index = 0;
    current_animation_speed = animation.default_animation_speed;
    default_animation_speed = animation.default_animation_speed;
    this->size = animation.size;
    this->endframe = animation.endframe;
    this->seq_start = animation.seq_start;
    this->seq_end = animation.seq_end;
    this->loop = animation.loop;
    finished = false;
}

void AnimationCycle::startRandom(const AnimationDefinition& animation)
{
    start(animation);
    index = ppl7::rand(0, size);

    if (seq_end > seq_start) {
        index = ppl7::rand(0, size);
        // if (index >= size) index = 0;
    } else {
        index = -ppl7::rand(0, -size);
    }
}

void AnimationCycle::setStaticFrame(int nr)
{
    seq_start = 0;
    seq_end = 0;
    index = 0;
    loop = false;
    finished = true;
    endframe = nr;
}

void AnimationCycle::startSequence(int start, int end, bool loop, int endframe)
{
    current_animation_speed = default_animation_speed;
    seq_start = start;
    seq_end = end;
    index = 0;
    if (seq_end > seq_start)
        size = seq_end - seq_start + 1;
    else
        size = -(seq_start - seq_end + 1);
    this->loop = loop;
    finished = false;
    this->endframe = endframe;
}

void AnimationCycle::startRandomSequence(int start, int end, bool loop, int endframe)
{
    current_animation_speed = default_animation_speed;
    seq_start = start;
    seq_end = end;
    if (seq_end > seq_start) {
        size = seq_end - seq_start + 1;
        index = ppl7::rand(0, size);
    } else {
        size = -(seq_start - seq_end + 1);
        index = -ppl7::rand(0, -size);
    }

    this->loop = loop;
    finished = false;
    this->endframe = endframe;
}

bool AnimationCycle::update(double time)
{
    if (time < next_animation) return false;
    next_animation = time + current_animation_speed;
    if (finished) return false;

    if (size >= 0) {
        index++;
        if (index >= size) {
            if (loop == true) {
                index = 0;
            } else {
                finished = true;
            }
        }
    } else {
        index--;
        if (index <= size) {
            if (loop == true) {
                index = 0;
            } else {
                finished = true;
            }
        }
    }
    return true;
}

int AnimationCycle::getFrame() const
{
    if (finished) return endframe;
    return seq_start + index;
}

bool AnimationCycle::isFinished() const
{
    return finished;
}

void AnimationCycle::setSpeed(float seconds_per_frame)
{
    current_animation_speed = seconds_per_frame;
}

void AnimationCycle::setDefaultSpeed(float seconds_per_frame)
{
    default_animation_speed = seconds_per_frame;
}

void AnimationCycle::resetSpeed()
{
    current_animation_speed = default_animation_speed;
}

float AnimationCycle::speed() const
{
    return current_animation_speed;
}
