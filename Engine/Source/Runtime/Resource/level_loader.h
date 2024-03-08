#pragma once

#include "level_object_description.h"
#include "Animation/animation_data.h"
#include "Animation/character_animation.h"

namespace gdp1 {

class AnimationLoader {
public:
    AnimationLoader() = default;
    ~AnimationLoader() = default;

    bool LoadAnimation(const std::string& animationFilePath, Animation& anim);
};

class LevelLoader {
public:
    LevelLoader() = default;
    ~LevelLoader() = default;

    bool LoadLevel(const std::string& levelFilePath);
    const LevelDesc& GetLevelDesc() const;

private:
    LevelDesc desc_;
};

}  // namespace gdp1
