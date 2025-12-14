#include "gpch.h"
#include "SoundSource.h"
#include <iostream>

SoundSource::SoundSource()
{
	alGenSources(1, &_Source);
	alSourcef(_Source, AL_PITCH, _Pitch);
	alSourcef(_Source, AL_GAIN, _Gain);
	alSource3f(_Source, AL_POSITION, _Position[0], _Position[1], _Position[2]);
	alSource3f(_Source, AL_VELOCITY, _Velocity[0], _Velocity[1], _Velocity[2]);
	alSourcei(_Source, AL_LOOPING, _LoopSound);
	alSourcei(_Source, AL_BUFFER, _Buffer);
}

SoundSource::~SoundSource()
{
	alDeleteSources(1, &_Source);
}

void SoundSource::Play()
{
	if (IsPlaying())
		Stop();

	alSourcei(_Source, AL_BUFFER, (ALint)_Buffer);
	alSourcePlay(_Source);
}

void SoundSource::Stop() {
	alSourceStop(_Source);
}

bool SoundSource::IsPlaying() const {
	ALint state;
	alGetSourcei(_Source, AL_SOURCE_STATE, &state);
	return state == AL_PLAYING;
}