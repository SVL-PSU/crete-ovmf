#ifndef CRETE_CUSTOM_INSTR_H
#define CRETE_CUSTOM_INSTR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// For crete-run/preload-run
void crete_ovmf_init(void)
void crete_ovmf_finish(void)

void crete_make_concolic(void* addr, size_t size, const char* name);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // CRETE_CUSTOM_INSTR_H
