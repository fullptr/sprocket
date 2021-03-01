#pragma once
#include <string>
#include <string_view>
#include <memory>

#include "Maths.h"

namespace Sprocket {
namespace Audio {

struct SoundImpl;
struct SourceImpl;

class Sound
{
    std::unique_ptr<SoundImpl> d_impl;

public:
    Sound();
    bool Load(std::string_view filename);
    friend class Source;
};


class Source
{
    std::unique_ptr<SourceImpl> d_impl;

public:
    Source();

    Source(Source&&) = delete;
    Source& operator=(const Source&) = delete;
        // Non-copyable

    void SetSound(const Sound& sound);

    void SetPosition(float x, float y, float z);
    void SetPosition(const glm::vec3& position);

    void Play() const;
    void Pause() const;
    void Stop() const;
};

}
}