#pragma once
#include <AL/alc.h>

class SoundDevice {
public:
	SoundDevice();
	~SoundDevice();
private:
	ALCdevice* _Device;
	ALCcontext* _Context;
};