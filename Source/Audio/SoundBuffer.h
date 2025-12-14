#pragma once
#include <AL/al.h>
#include <vector>

class SoundBuffer {
public:
	SoundBuffer();
	~SoundBuffer();

	ALuint AddSoundEffect(const char* path);
	bool RemoveSoundEffect(const ALuint& buffer);
private:
	std::vector<ALuint> _SoundEffectBuffers;
};