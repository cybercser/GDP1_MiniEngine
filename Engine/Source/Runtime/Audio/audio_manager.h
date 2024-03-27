#pragma once

#include <fmod.hpp>
#include <glm/glm.hpp>

#include <vector>
#include <map>
#include <string>

namespace gdp1 {

void CheckError(FMOD_RESULT result, const char* file, int line);
#define FMODCheckError(result) CheckError(result, __FILE__, __LINE__)

struct Audio {
    const char* name;
    FMOD::Sound* Audio;
};

struct Channel {
    const char* name;
    FMOD::Channel* fmodChannel;
    float volume;
    float pitch;
    float pan;
    bool playing;
};

struct DriverInfo {
    int id;
    char name[256];
    FMOD_GUID guid;
    int systemrate;
    FMOD_SPEAKERMODE speakermode;
    int speakermodechannels;
};

// We can use defines to have a consistent lookup index for
// the specific channel group you want.
#define CHANNELGROUP_MASTER_INDEX 0
#define CHANNELGROUP_MUSIC_INDEX 1
#define CHANNELGROUP_SFX_INDEX 2
#define CHANNELGROUP_FAR_INDEX 3

// This can be a singleton, but we are not focusing on
// design patterns at the moment.
class AudioManager {
public:
    // Constructor
    AudioManager();

    // Destructor (Gets called on delete)
    ~AudioManager();

    // Lifecycle
    void Initialize();
    void Update();
    void Destroy();

    // System stuff
    std::vector<DriverInfo> GetDeviceDrivers();
    void SetDeviceDriver(int id);

    // Assets
    void LoadAudioStream(const char* source);
    void LoadAudio(const char* file);
    int PlayAudio(const char* AudioName, int ChannelGroupIndex, glm::vec3 soundPos = glm::vec3(0.f, 1.f, 0.f),
                  glm::vec3 soundVel = glm::vec3(0.f, 0.f, 0.f));

    // Channel Controls
    void SetChannelVolume(int id, float value);
    void SetChannelPitch(int id, float value);
    void SetChannelPan(int id, float value);

    void SetChannelGroupVolume(int id, float value);
    void SetChannelGroupPitch(int id, float value);
    void SetChannelGroupPan(int id, float value);

    bool GetChannelPlaying(int id);
    void GetPlaybackPosition(int id, unsigned int& value);

    // DSP Effects
    void AddReverbFilterOnChannel(int channelId);
    void AddLowPassFilterOnChannel(int channelId);
    void AddHighPassFilterOnChannel(int channelId);
    void AddDistortionFilterOnChannel(int channelId);
    void AddChorusPassOnChannel(int channelId);
    void AddEchoPassOnChannel(int channelId);

    void SetReverbValuesOnChannel(int channelId, float decay, float density, float diffusion);
    void SetLowPassFilterValuesOnChannel(int channelId, float limit);
    void SetHighPassFilterValuesOnChannel(int channelId, float limit);
    void SetDistortionLevelFilterValuesOnChannel(int channelId, float level);
    void SetChorusPassValuesOnChannel(int channelId, float mix, float rate, float depth);

    void SetChannel3DAttributes(int channelId, glm::vec3& position, glm::vec3& velocity);

    // 3D Stuff
    void Load3DAudio(const char* file, float minDistance = 0.5f, float maxDistance = 10.f);
    void SetListenerAttributes(const glm::vec3& position, const glm::vec3& velocity, const glm::vec3& forward,
                               const glm::vec3& up);

    int AddPolygon(float direct, float reverb, bool doublesided, const std::vector<glm::vec3>& vertices,
                   const glm::vec3& position, const glm::vec3& scale);

    // AudioManager.h ADD THIS FUNCTION
    void GetListenerAttributes(glm::vec3& position, glm::vec3& velocity, glm::vec3& forward, glm::vec3& up);

    // Don't worry about this for now
    unsigned int GetData(const char* sound, void* data);

    // Debug
    void PrintInfo() const;

private:
    inline FMOD::ChannelGroup* GetMasterChannelGroup() { return m_ChannelGroupList[CHANNELGROUP_MASTER_INDEX]; }
    inline FMOD::ChannelGroup* GetSFXChannelGroup() { return m_ChannelGroupList[CHANNELGROUP_SFX_INDEX]; }
    inline FMOD::ChannelGroup* GetMusicChannelGroup() { return m_ChannelGroupList[CHANNELGROUP_MUSIC_INDEX]; }

    bool m_Initialized = false;

    std::map<const char*, Audio*> m_AudioMap;
    std::vector<Channel*> m_ChannelList;
    Audio* m_CurrentAudio;

    int m_NextChannelId = 0;

    FMOD::System* m_System = nullptr;

    // 3D
    FMOD::Geometry* m_Geometry;

    // Implemented
    FMOD::DSP* m_ReverbDSP;
    FMOD::DSP* m_HighPassDSP;
    FMOD::DSP* m_LowPassDSP;
    FMOD::DSP* m_DistortionDSP;
    FMOD::DSP* m_ChorusPassDSP;
    FMOD::DSP* m_EchoPassDSP;

    // Not implemented
    FMOD::DSP* m_DelayPassDSP;

    // Channel Groups
    // FMOD::ChannelGroup* m_MasterChannelGroup;
    // FMOD::ChannelGroup* m_SFXChannelGroup;
    // FMOD::ChannelGroup* m_MusicChannelGroup;

    std::vector<FMOD::ChannelGroup*> m_ChannelGroupList;
};
}  // namespace audio