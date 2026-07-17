#ifndef __MC6847_H__
#define __MC6847_H__

void InitMC6847(unsigned char *emul, unsigned char* in_VRAM, int w, int h);
void Update6847(unsigned char gmode);

#endif // __MC6847_H__
