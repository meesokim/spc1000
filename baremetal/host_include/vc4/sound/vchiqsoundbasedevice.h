#ifndef _vc4_sound_vchiqsoundbasedevice_h
#define _vc4_sound_vchiqsoundbasedevice_h

#include <circle/types.h>
#include <vc4/vchiq/vchiqdevice.h>

enum {
    VCHIQSoundDestinationAuto
};

class CVCHIQSoundBaseDevice {
public:
    CVCHIQSoundBaseDevice(CVCHIQDevice *pVCHIQ, unsigned nRate, unsigned nChunkSize, int nDest)
        : m_nRate(nRate), m_nChunkSize(nChunkSize) {}
    virtual ~CVCHIQSoundBaseDevice() {}
    virtual unsigned GetChunk(s16 *pBuffer, unsigned nChunkSize) = 0;
    boolean Start();
    void Stop();

private:
    unsigned m_nRate;
    unsigned m_nChunkSize;
};

#endif
