#ifndef INCLUDE_ANIMATION_H_
#define INCLUDE_ANIMATION_H_

class AnimationDefinition
{
    friend class AnimationCycle;

private:
    int* cycle = nullptr;
    int size = 0;
    int endframe = 0;
    bool loop = false;
    int seq_start = 0, seq_end = 0;
    float default_animation_speed = 0.056f;

public:
    AnimationDefinition() = default;
    AnimationDefinition(int start, int end, bool loop, int endframe, float speed = 0.056f);
};

class AnimationCycle : private AnimationDefinition
{
private:
    int index = 0;
    bool finished = false;
    float current_animation_speed = 0.056f;
    double next_animation = 0.0f;

public:
    AnimationCycle() = default;
    void setStaticFrame(int nr);
    void start(const AnimationCycle& other);
    void start(int* cycle_array, int size, bool loop, int endframe);
    void startRandom(int* cycle_array, int size, bool loop, int endframe);

    void start(const AnimationDefinition& animation);
    void startRandom(const AnimationDefinition& animation);

    void startSequence(int start, int end, bool loop, int endframe);
    void startRandomSequence(int start, int end, bool loop, int endframe);
    bool update(double time);
    int getFrame() const;
    bool isFinished() const;
    int getIndex() const;
    void setSpeed(float seconds_per_frame);
    void setDefaultSpeed(float seconds_per_frame);
    void resetSpeed();
    float speed() const;
};

#endif // INCLUDE_ANIMATION_H_
