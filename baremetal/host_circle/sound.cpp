#include <vc4/sound/vchiqsoundbasedevice.h>
#include <SDL2/SDL.h>

static void SDLCALL HostAudioCallback(void *userdata, Uint8 *stream, int len) {
    CVCHIQSoundBaseDevice *pSound = (CVCHIQSoundBaseDevice*)userdata;
    int samples = len / 2;
    SDL_memset(stream, 0, len);
    pSound->GetChunk((s16*)stream, samples);
}

boolean CVCHIQSoundBaseDevice::Start() {
    SDL_AudioSpec wanted;
    SDL_memset(&wanted, 0, sizeof(wanted));
    wanted.freq = m_nRate;
    wanted.format = AUDIO_S16SYS;
    wanted.channels = 1;
    wanted.samples = 512;
    wanted.callback = HostAudioCallback;
    wanted.userdata = this;

    if (SDL_OpenAudio(&wanted, NULL) < 0) {
        fprintf(stderr, "SDL_OpenAudio failed: %s\n", SDL_GetError());
        return FALSE;
    }
    SDL_PauseAudio(0);
    return TRUE;
}

void CVCHIQSoundBaseDevice::Stop() {
    SDL_CloseAudio();
}
