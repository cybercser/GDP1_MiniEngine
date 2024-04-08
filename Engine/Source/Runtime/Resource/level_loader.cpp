#include "level_loader.h"

#include <fstream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#pragma region Json Arbitrary Type Conversion

// for glm::vec3, glm::vec4 and glm::quat
namespace glm {
void from_json(const json& j, vec3& vec) {
    j.at("x").get_to(vec.x);
    j.at("y").get_to(vec.y);
    j.at("z").get_to(vec.z);
}

void to_json(json& j, const vec3& vec) {
    j = json{{"x", vec.x}, {"y", vec.y}, {"z", vec.z}};
}

void from_json(const json& j, vec4& vec) {
    j.at("r").get_to(vec.r);
    j.at("g").get_to(vec.g);
    j.at("b").get_to(vec.b);
    j.at("a").get_to(vec.a);
}

void to_json(json& j, const vec4& vec) {
    j = json{{"r", vec.r}, {"g", vec.g}, {"b", vec.b}, {"a", vec.a}};
}

void from_json(const json& j, quat& q) {
    q.w = j.at("w").get<float>();
    q.x = j.at("x").get<float>();
    q.y = j.at("y").get<float>();
    q.z = j.at("z").get<float>();
}

void to_json(json& j, const quat& q) {
    j = json{{"w", q.w}, {"x", q.x}, {"y", q.y}, {"z", q.z}};
}

}  // namespace glm

namespace gdp1 {

// for TransformDesc
void from_json(const json& j, TransformDesc& xform) {
    xform.localPosition = j.value("localPosition", glm::vec3(0.0f));
    xform.localEulerAngles = j.value("localEulerAngles", glm::vec3(0.0f));
    xform.localScale = j.value("localScale", glm::vec3(1.0f));
}

void to_json(json& j, const TransformDesc& xform) {
    j = {{"localPosition", xform.localPosition},
         {"localEulerAngles", xform.localEulerAngles},
         {"localScale", xform.localScale}};
}

// for RigidbodyDesc
void from_json(const json& j, RigidbodyDesc& rbDesc) {
    j.at("objectName").get_to(rbDesc.objectName);
    j.at("active").get_to(rbDesc.active);
    j.at("collider").get_to(rbDesc.collider);
    j.at("invMass").get_to(rbDesc.invMass);
    j.at("position").get_to(rbDesc.position);
    j.at("orientation").get_to(rbDesc.orientation);
    j.at("velocity").get_to(rbDesc.velocity);
    j.at("applyGravity").get_to(rbDesc.applyGravity);
}

void to_json(json& j, const RigidbodyDesc& rbDesc) {
    j = json{{"objectName", rbDesc.objectName}, {"active", rbDesc.active},     {"collider", rbDesc.collider},
             {"invMass", rbDesc.invMass},       {"position", rbDesc.position}, {"orientation", rbDesc.orientation},
             {"velocity", rbDesc.velocity},     {"applyGravity", rbDesc.applyGravity}};
}

// for SoftbodyDesc
void from_json(const json& j, SoftbodyDesc& sbDesc) {
    j.at("objectName").get_to(sbDesc.objectName);
    j.at("mass").get_to(sbDesc.mass);
    j.at("iterations").get_to(sbDesc.iterations);
    j.at("springStrength").get_to(sbDesc.springStrength);
}

void to_json(json& j, const SoftbodyDesc& sbDesc) {
    j = json{{"objectName", sbDesc.objectName}, {"mass", sbDesc.mass},
             {"iterations", sbDesc.iterations}, {"springStrength", sbDesc.springStrength}};
}

// for TextureDesc
void from_json(const json& j, TexturesDesc& textureDesc) {
    textureDesc.name = j.value("name", "DefaultName");
    textureDesc.type = j.value("type", "DefaultType");
    textureDesc.hasFBO = j.value("hasFBO", false);
}

void to_json(json& j, const TexturesDesc& textureDesc) {
    j = {{"name", textureDesc.name},
         {"type", textureDesc.type},
         {"hasFBO", textureDesc.hasFBO}};
}

// for ModelDesc
void from_json(const json& j, ModelDesc& modelDesc) {
    j.at("name").get_to(modelDesc.name);
    j.at("filepath").get_to(modelDesc.filepath);
    j.at("shader").get_to(modelDesc.shader);
    j.at("textures").get_to(modelDesc.textures);
}

void to_json(json& j, const ModelDesc& modelDesc) {
    j = {{"name", modelDesc.name}, {"filepath", modelDesc.filepath}, {"shader", modelDesc.shader}, {"textures", modelDesc.textures}};
}

// for AnimationDesc
void from_json(const json& j, AnimationRefDesc& animDesc) {
    j.at("name").get_to(animDesc.name);
    j.at("path").get_to(animDesc.path);
}

void to_json(json& j, const AnimationRefDesc& animDesc) {
    j = {{"name", animDesc.name}, {"path", animDesc.path}};
}

// for AudioSourceDesc
void from_json(const json& j, AudioSourceDesc& asDesc) {
    j.at("name").get_to(asDesc.name);
    j.at("filepath").get_to(asDesc.filepath);
    j.at("volume").get_to(asDesc.volume);
    j.at("pitch").get_to(asDesc.pitch);
    j.at("pan").get_to(asDesc.pan);
    j.at("streamed").get_to(asDesc.streamed);
    j.at("loop").get_to(asDesc.loop);
    j.at("playOnAwake").get_to(asDesc.playOnAwake);
    j.at("spatial").get_to(asDesc.spatial);
    j.at("position").get_to(asDesc.position);
    j.at("minDistance").get_to(asDesc.minDistance);
    j.at("maxDistance").get_to(asDesc.maxDistance);
}

void to_json(json& j, const AudioSourceDesc& asDesc) {
    j = {{"name", asDesc.name},
         {"filepath", asDesc.filepath},
         {"volume", asDesc.volume},
         {"pitch", asDesc.pitch},
         {"pan", asDesc.pan},
         {"streamed", asDesc.streamed},
         {"loop", asDesc.loop},
         {"playOnAwake", asDesc.playOnAwake},
         {"spatial", asDesc.spatial},
         {"position", asDesc.position},
         {"minDistance", asDesc.minDistance},
         {"maxDistance", asDesc.maxDistance}};
}

// for CameraDesc
void from_json(const json& j, CameraDesc& camDesc) {
    j.at("name").get_to(camDesc.name);
    j.at("position").get_to(camDesc.position);
    j.at("up").get_to(camDesc.up);
    j.at("yaw").get_to(camDesc.yaw);
    j.at("pitch").get_to(camDesc.pitch);
    j.at("fov").get_to(camDesc.fov);
    j.at("nearZ").get_to(camDesc.nearZ);
    j.at("farZ").get_to(camDesc.farZ);
}

void to_json(json& j, const CameraDesc& camDesc) {
    j = {{"name", camDesc.name},   {"position", camDesc.position}, {"up", camDesc.up},       {"yaw", camDesc.yaw},
         {"pitch", camDesc.pitch}, {"fov", camDesc.fov},           {"nearZ", camDesc.nearZ}, {"farZ", camDesc.farZ}};
}

// for DirectionalLight
void from_json(const json& j, DirectionalLight& dl) {
    j.at("name").get_to(dl.name);
    j.at("direction").get_to(dl.direction);
    j.at("color").get_to(dl.color);
    j.at("intensity").get_to(dl.intensity);
}

void to_json(json& j, const DirectionalLight& dl) {
    j = {{"name", dl.name}, {"direction", dl.direction}, {"color", dl.color}, {"intensity", dl.intensity}};
}

// for PointLight
void from_json(const json& j, PointLight& pl) {
    j.at("name").get_to(pl.name);
    j.at("position").get_to(pl.position);
    j.at("color").get_to(pl.color);
    j.at("intensity").get_to(pl.intensity);
    j.at("constant").get_to(pl.constant);
    j.at("linear").get_to(pl.linear);
    j.at("quadratic").get_to(pl.quadratic);
}

void to_json(json& j, const PointLight& pl) {
    j = {{"name", pl.name},         {"position", pl.position}, {"color", pl.color},        {"intensity", pl.intensity},
         {"constant", pl.constant}, {"linear", pl.linear},     {"quadratic", pl.quadratic}};
}

// for SpotLight
void from_json(const json& j, SpotLight& sl) {
    j.at("name").get_to(sl.name);
    j.at("position").get_to(sl.position);
    j.at("direction").get_to(sl.direction);
    j.at("ambient").get_to(sl.ambient);
    j.at("diffuse").get_to(sl.diffuse);
    j.at("specular").get_to(sl.specular);
    j.at("cutoff").get_to(sl.cutoff);
    j.at("outerCutoff").get_to(sl.outerCutoff);
    j.at("constant").get_to(sl.constant);
    j.at("linear").get_to(sl.linear);
    j.at("quadratic").get_to(sl.quadratic);
    j.at("cookie").get_to(sl.cookie);
}

void to_json(json& j, const SpotLight& sl) {
    j = {{"name", sl.name},       {"position", sl.position},       {"direction", sl.direction},
         {"ambient", sl.ambient}, {"diffuse", sl.diffuse},         {"specular", sl.specular},
         {"cutoff", sl.cutoff},   {"outerCutoff", sl.outerCutoff}, {"constant", sl.constant},
         {"linear", sl.linear},   {"quadratic", sl.quadratic},     {"cookie", sl.cookie}};
}

// for skybox description
void from_json(const json& j, SkyboxDesc& sbDesc) {
    j.at("name").get_to(sbDesc.name);
    j.at("size").get_to(sbDesc.size);
    j.at("faces").get_to(sbDesc.faces);
}

void to_json(json& j, const SkyboxDesc& sbDesc) {
    j = {{"name", sbDesc.name}, {"size", sbDesc.size}, {"faces", sbDesc.faces}};
}

// for GameObjectDesc
void from_json(const json& j, GameObjectDesc& goDesc) {
    goDesc.name = j.value("name", "");
    goDesc.modelName = j.value("model", "");
    goDesc.visible = j.value("visible", true);
    goDesc.transform = j.value("transform", TransformDesc{});  // Initialize with default TransformDesc
    goDesc.children = j.value("children", std::vector<std::string>{});
    goDesc.parentName = j.value("parent", "");
    goDesc.hasFBO = j.value("hasFBO", false);
    goDesc.setLit = j.value("setLit", false);
    goDesc.isStatic = j.value("isStatic", true);
}

void to_json(json& j, const GameObjectDesc& goDesc) {
    j = {{"name", goDesc.name},
         {"model", goDesc.modelName},
         {"visible", goDesc.visible},
         {"transform", goDesc.transform},
         {"children", goDesc.children},
         {"parent", goDesc.parentName},
         {"setLit", goDesc.setLit},       
         {"isStatic", goDesc.isStatic},
         {"hasFBO", goDesc.hasFBO}};
}

// for CharacterAnimation Desc
void from_json(const json& j, CharacterAnimationRefDesc& animDesc) {
    j.at("name").get_to(animDesc.name);
    j.at("path").get_to(animDesc.path);
    j.at("model").get_to(animDesc.model);
}

void to_json(json& j, const CharacterAnimationRefDesc& animDesc) {
    j = {{"name", animDesc.name}, {"path", animDesc.path}, {"model", animDesc.model}};
}

// for LevelDesc
void from_json(const json& j, LevelDesc& lvlDesc) {
    j.at("name").get_to(lvlDesc.name);
    j.at("comment").get_to(lvlDesc.comment);
    j.at("version").get_to(lvlDesc.version);
    j.at("cameras").get_to(lvlDesc.cameraDescs);
    j.at("directionalLights").get_to(lvlDesc.directionalLights);
    j.at("pointLights").get_to(lvlDesc.pointLights);
    j.at("spotLights").get_to(lvlDesc.spotLights);
    j.at("models").get_to(lvlDesc.modelDescs);
    j.at("gameObjects").get_to(lvlDesc.gameObjectDescs);
    j.at("rigidbodies").get_to(lvlDesc.rigidbodyDescs);
    j.at("softbodies").get_to(lvlDesc.softbodyDescs);
    j.at("animation").get_to(lvlDesc.animationRefDesc);
    j.at("character_animations").get_to(lvlDesc.characterAnimationRefDescs);
    j.at("audioSources").get_to(lvlDesc.audioSourceDescs);
    j.at("skybox").get_to(lvlDesc.skyboxDesc);
}

void to_json(json& j, const LevelDesc& lvlDesc) {
    j = {{"name", lvlDesc.name},
         {"comment", lvlDesc.comment},
         {"version", lvlDesc.version},
         {"cameras", lvlDesc.cameraDescs},
         {"directionalLights", lvlDesc.directionalLights},
         {"pointLights", lvlDesc.pointLights},
         {"spotLights", lvlDesc.spotLights},
         {"models", lvlDesc.modelDescs},
         {"gameObjects", lvlDesc.gameObjectDescs},
         {"rigidbodies", lvlDesc.rigidbodyDescs},
         {"animation", lvlDesc.animationRefDesc},
         {"character_animations", lvlDesc.characterAnimationRefDescs},
         {"audioSources", lvlDesc.audioSourceDescs},
         {"skybox", lvlDesc.skyboxDesc}};
}

// for PositionKeyframe
void from_json(const json& j, PositionKeyframe& posKey) {
    j.at("time").get_to(posKey.time);
    j.at("value").get_to(posKey.value);
    j.at("easingType").get_to(posKey.easingType);
    j.at("objectName").get_to(posKey.objectName);
}

void to_json(json& j, const PositionKeyframe& posKey) {
    j = {{"time", posKey.time},
         {"value", posKey.value},
         {"easingType", posKey.easingType},
         {"objectName", posKey.objectName}};
}

// for RotationKeyframe
void from_json(const json& j, RotationKeyframe& rotKey) {
    j.at("time").get_to(rotKey.time);
    j.at("value").get_to(rotKey.value);
    j.at("easingType").get_to(rotKey.easingType);
    j.at("objectName").get_to(rotKey.objectName);
}

void to_json(json& j, const RotationKeyframe& rotKey) {
    j = {{"time", rotKey.time},
         {"value", rotKey.value},
         {"easingType", rotKey.easingType},
         {"objectName", rotKey.objectName}};
}

// for ScaleKeyframe
void from_json(const json& j, ScaleKeyframe& scaleKey) {
    j.at("time").get_to(scaleKey.time);
    j.at("value").get_to(scaleKey.value);
    j.at("easingType").get_to(scaleKey.easingType);
    j.at("objectName").get_to(scaleKey.objectName);
}

void to_json(json& j, const ScaleKeyframe& scaleKey) {
    j = {{"time", scaleKey.time},
         {"value", scaleKey.value},
         {"easingType", scaleKey.easingType},
         {"objectName", scaleKey.objectName}};
}

// for AnimationClip
void from_json(const json& j, AnimationClip& animClip) {
    j.at("name").get_to(animClip.name);
    j.at("length").get_to(animClip.length);
    j.at("positionKeys").get_to(animClip.positionKeys);
    j.at("rotationKeys").get_to(animClip.rotationKeys);
    j.at("scaleKeys").get_to(animClip.scaleKeys);
}

void to_json(json& j, const AnimationClip& animClipDesc) {
    j = {{"name", animClipDesc.name},
         {"length", animClipDesc.length},
         {"positionKeys", animClipDesc.positionKeys},
         {"rotationKeys", animClipDesc.rotationKeys},
         {"scaleKeys", animClipDesc.scaleKeys}};
}

// for Animation
void from_json(const json& j, Animation& animDesc) {
    j.at("name").get_to(animDesc.name);
    j.at("clips").get_to(animDesc.clips);
}

void to_json(json& j, const Animation& animDesc) {
    j = {{"name", animDesc.name}, {"clips", animDesc.clips}};
}

#pragma endregion

bool LevelLoader::LoadLevel(const std::string& levelFilePath) {
    std::ifstream lvlFile(levelFilePath);
    if (!lvlFile.is_open()) {
        return false;
    }

    json j = json::parse(lvlFile);
    desc_ = std::move(j.get<LevelDesc>());

    return true;
}

const LevelDesc& LevelLoader::GetLevelDesc() const {
    return desc_;
}

bool AnimationLoader::LoadAnimation(const std::string& animationFilePath, Animation& anim) {
    std::ifstream animFile(animationFilePath);
    if (!animFile.is_open()) {
        return false;
    }

    json j = json::parse(animFile);
    anim = std::move(j.get<Animation>());

    return true;
}

}  // namespace gdp1
