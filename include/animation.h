#ifndef INCLUDE_ANIMATION_H_
#define INCLUDE_ANIMATION_H_

class AnimationDefinition
{
    friend class AnimationCycle;

private:
    int size = 0;
    int endframe = 0;
    bool loop = false;
    int seq_start = 0, seq_end = 0;
    float default_animation_speed = 1.0f / 30.0f; // 30 FPS by default

public:
    AnimationDefinition() = default;
    AnimationDefinition(int start, int end, bool loop, int endframe, float speed = 1.0f / 30.0f);
};

class AnimationCycle : private AnimationDefinition
{
private:
    int index = 0;
    bool finished = false;
    float current_animation_speed = 1.0f / 30.0f; // 30 FPS by default
    double next_animation = 0.0f;

    bool inIntro() const;

public:
    AnimationCycle() = default;
    void setStaticFrame(int nr);
    void start(const AnimationDefinition& animation);
    void startRandom(const AnimationDefinition& animation);
    void startSequence(int start, int end, bool loop, int endframe);
    void startRandomSequence(int start, int end, bool loop, int endframe);
    bool update(double time);
    int getFrame() const;
    bool isFinished() const;
    void setSpeed(float seconds_per_frame);
    void setDefaultSpeed(float seconds_per_frame);
    void resetSpeed();
    float speed() const;
};

#endif // INCLUDE_ANIMATION_H_
