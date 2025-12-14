#include "gpch.h"
#include "SoundDevice.h"
#include <AL/al.h>

SoundDevice::SoundDevice()
{
	_Device = alcOpenDevice(nullptr);
	ASSERT(_Device, "Failed to get audio device");
	_Context = alcCreateContext(_Device, nullptr);
	ASSERT(_Context, "Failed to create audio context");
	ASSERT(alcMakeContextCurrent(_Context), "Failed to make audio context as default");

	const ALCchar* name = nullptr;
	if (alcIsExtensionPresent(_Device, "ALC_ENUMERATE_ALL_EXT"))
		name = alcGetString(_Device, ALC_ALL_DEVICES_SPECIFIER);
	if (!name || alcGetError(_Device) != AL_NO_ERROR)
		name = alcGetString(_Device, ALC_DEVICE_SPECIFIER);
	LOG_INFO("Audio Device: {}", name);
}

SoundDevice::~SoundDevice()
{
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(_Context);
	alcCloseDevice(_Device);
}
