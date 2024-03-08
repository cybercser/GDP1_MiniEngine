#pragma once

#include <Render/model.h>
#include <Animation/model_bone.h>

#include <string>
#include <vector>
#include <map>

namespace gdp1 {

struct Node {
    Node() = default;
    Node(const std::string& name)
        : name(name)
        , children_count (0) {}

    std::string name;
    glm::mat4 transformation;
    int children_count;
    std::vector<Node> children;
};

class CharacterAnimation {
public:
    // Loads in a model from a file and stores tha information in 'data', 'JSON', and 'file'
    CharacterAnimation(const std::string& file, Model* model, const std::string& name);

    Bone* FindBone(const std::string& name);

    inline const std::string& GetAniName() { return mName; }
    inline float GetTicksPerSecond() { return mTicksPerSecond; }
    inline float GetDuration() { return mDuration; }
    inline const Node& GetRootNode() { return m_RootNode; }
    inline const std::map<std::string, sBoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }

private:
    void ReadBones(const aiAnimation* animation, Model& model);

    void ReadHierarchyData(Node& dest, const aiNode* src);

    std::string mName;
    float mDuration;
    int mTicksPerSecond;

    Node m_RootNode;
    std::vector<Bone> mBones;
    std::map<std::string, sBoneInfo> m_BoneInfoMap;
};

}  // namespace gdp1
