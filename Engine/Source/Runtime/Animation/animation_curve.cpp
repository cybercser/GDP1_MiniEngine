#include "animation_curve.h"

AnimationCurve::AnimationCurve(std::vector<Keyframe>&& keys)
    : keys(std::move(keys)) {
}

AnimationCurve::AnimationCurve(const std::vector<Keyframe>& keys)
    : keys(keys) {
}

Keyframe& AnimationCurve::operator[](size_t index) {
    return keys.at(index);
}

const Keyframe& AnimationCurve::operator[](size_t index) const {
    return keys.at(index);
}

float AnimationCurve::Evaluate(float time) {
    if (keys.size() == 0) {
        return 0.f;
    }
    if (keys.size() == 1) {
        return keys[0].value;
    }
    if (time <= keys.front().time) {
        return keys.front().value;
    }
    if (time >= keys.back().time) {
        return keys.back().value;
    }
    // find the keyframes to interpolate between
    size_t keyIndex = 0;
    for (size_t i = 0; i < keys.size(); ++i) {
        if (keys[i].time > time) {
            keyIndex = i;
            break;
        }
    }
    // interpolate
    // Cubic Spline Interpolation
    // ref: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#interpolation-cubic
    // ref: https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Interpolation_on_the_unit_interval
    // ref: https://github.khronos.org/glTF-Tutorials/gltfTutorial/gltfTutorial_007_Animations.html
    float t0 = keys[keyIndex - 1].time;
    float v0 = keys[keyIndex - 1].value;

    float t1 = keys[keyIndex].time;
    float v1 = keys[keyIndex].value;

    float outTan = keys[keyIndex - 1].outTangent;
    float inTan = keys[keyIndex].inTangent;

    float t = (time - t0) / (t1 - t0);
    float t2 = t * t;
    float t3 = t2 * t;

    float h00 = 2 * t3 - 3 * t2 + 1;
    float h10 = t3 - 2 * t2 + t;
    float h01 = -2 * t3 + 3 * t2;
    float h11 = t3 - t2;

    // return v0 + (v1 - v0) * t;
    return h00 * v0 + h10 * (t1 - t0) * outTan + h01 * v1 + h11 * (t1 - t0) * inTan;
}

int AnimationCurve::AddKey(Keyframe key) {
    return AddKey(key.time, key.value);
}

int AnimationCurve::AddKey(float time, float value) {
    // check if the key exists
    for (size_t i = 0; i < keys.size(); ++i) {
        if (keys[i].time == time) {
            return -1;
        }
    }
    // add the key
    keys.push_back({value, time});
    return keys.size() - 1;
}

void AnimationCurve::RemoveKey(int index) {
    keys.erase(keys.begin() + index);
}

void AnimationCurve::ClearKeys() {
    keys.clear();
}

AnimationCurve AnimationCurve::Constant(float timeStart, float timeEnd, float value) {
    Keyframe startKey = {value, timeStart};
    Keyframe endKey = {value, timeEnd};
    return AnimationCurve{{{startKey, endKey}}};
}

AnimationCurve AnimationCurve::Linear(float timeStart, float timeEnd, float valueStart, float valueEnd) {
    Keyframe startKey = {valueStart, timeStart};
    Keyframe endKey = {valueEnd, timeEnd};
    return AnimationCurve{{{startKey, endKey}}};
}

AnimationCurve AnimationCurve::EaseInOut(float timeStart, float valueStart, float timeEnd, float valueEnd) {
    Keyframe startKey = {valueStart, timeStart};
    Keyframe endKey = {valueEnd, timeEnd};
    return AnimationCurve{{{startKey, endKey}}};
}
