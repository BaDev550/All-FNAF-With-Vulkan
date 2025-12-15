#pragma once
#include <AL\al.h>

class SoundSource
{
public:
	SoundSource();
	~SoundSource();
	void SetBuffer(const ALuint& buffer) {
		_Buffer = buffer;
	}
	void SetLoopSound(bool loop) { 
		_LoopSound = loop; 
		alSourcei(_Source, AL_LOOPING, _LoopSound);
	}

	void Play();
	void PlayOnce();
	void Stop();
	bool IsPlaying() const;
private:
	ALuint _Source;
	float _Pitch = 1.f;
	float _Gain = 1.f;
	float _Position[3] = { 0,0,0 };
	float _Velocity[3] = { 0,0,0 };
	bool _LoopSound = false;
	ALuint _Buffer = 0;
};