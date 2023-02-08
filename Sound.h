#ifndef Sound_H
#define Sound_H

#ifdef __cplusplus
extern "C" void __fastcall Sound(int Frequency);
extern "C" void __fastcall NoSound(void);
#else // __cplusplus
void __fastcall Sound(int Frequency);
void __fastcall NoSound(void);
#endif // __cplusplus

#endif // Sound_H