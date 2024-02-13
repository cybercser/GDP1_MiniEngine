#pragma once

#include <vector>

// Sets which weights to use when calculating curve segments.
enum class WeightedMode {
    None,  // Exclude both inWeight or outWeight when calculating curve segments.
    In,    // Include inWeight when calculating the previous curve segment.
    Out,   // Include outWeight when calculating the next curve segment.
    Both,  // Include inWeight and outWeight when calculating curve segments.
};

// A single keyframe that can be injected into an animation curve.
struct Keyframe {
    float value;
    float time;  // in seconds

    float inTangent;  // Sets the incoming tangent for this key. The incoming tangent affects the slope of the curve
                      // from the previous key to this key.
    float inWeight;   // Sets the incoming weight for this key. The incoming weight affects the slope of the curve from
                      // the previous key to this key.

    float outTangent;  // Sets the outgoing tangent for this key. The outgoing tangent affects the slope of the curve
                       // from this key to the next key.
    float outWeight;   // Sets the outgoing weight for this key. The outgoing weight affects the slope of the curve from
                       // this key to the next key.

    WeightedMode weightedMode;
};

// Determines how time is treated outside of the keyframed range of an AnimationClip or AnimationCurve.
enum class WrapMode {
    Once,         // Play the animation once.
    Loop,         // Loop the animation.
    PingPong,     // Play the animation forward and backward.
    ClampForever  // Play the animation once and keep the last frame.
};

// Store a collection of Keyframes that can be evaluated over time.
struct AnimationCurve {
    std::vector<Keyframe> keys;
    float length;           // in seconds
    WrapMode postWrapMode;  // The behavior of the animation after the last keyframe.
    WrapMode preWrapMode;   // The behavior of the animation before the first keyframe.

    // ctor
    AnimationCurve(const std::vector<Keyframe>& keys);

    // move ctor
    AnimationCurve(std::vector<Keyframe>&& keys);

    // operator[] const
    const Keyframe& operator[](size_t index) const;

    // operator[] non-const
    Keyframe& operator[](size_t index);

    float Evaluate(float time);

    /**
     * @brief Add a key to the curve
     * @param time TThe time at which to add the key (horizontal axis in the curve graph).
     * @param value The value for the key (vertical axis in the curve graph).
     * @return TThe index of the added key, or -1 if the key could not be added.
     */
    int AddKey(float time, float value);

    /**
     * @brief Add a key to the curve.
     * @param key   The key to add to the curve.
     * @return      TThe index of the added key, or -1 if the key could not be added.
     */
    int AddKey(Keyframe key);

    /**
     * @brief Remove a key from the curve.
     * @param index The index of the key to remove.
     */
    void RemoveKey(int index);

    /**
     * @brief Clear all keys from the curve.
     */
    void ClearKeys();

    /**
     * @brief Creates a constant "curve" starting at timeStart, ending at timeEnd, and set to the value value.
     * @param timeStart The start time for the constant curve.
     * @param timeEnd   The end time for the constant curve.
     * @param value     The value for the constant curve.
     */
    static AnimationCurve Constant(float timeStart, float timeEnd, float value);

    /**
     * @brief Creates a linear curve starting at timeStart, valueStart and ending at timeEnd, valueEnd.
     * @param timeStart The start time for the linear curve.
     * @param timeEnd   The start value for the linear curve.
     * @param valueStart The end time for the linear curve.
     * @param valueEnd   The end value for the linear curve.
     * @return AnimationCurve The linear curve generated from the specified values.
     */
    static AnimationCurve Linear(float timeStart, float timeEnd, float valueStart, float valueEnd);

    /**
     * @brief Creates an ease-in and out curve starting at timeStart, valueStart and ending at timeEnd, valueEnd.
     * @param timeStart The start time for the ease curve.
     * @param timeEnd   The start value for the ease curve.
     * @param valueStart The end time for the ease curve.
     * @param valueEnd   The end value for the ease curve.
     * @return AnimationCurve The ease-in and out curve generated from the specified values.
     */
    static AnimationCurve EaseInOut(float timeStart, float valueStart, float timeEnd, float valueEnd);
};
