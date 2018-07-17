#include "custom_opcode.h"
#include <stddef.h>

#define MAX_NAME_SIZE  256
 char concolic_name[2*MAX_NAME_SIZE];

size_t my_strlen(const char * str)
{
    size_t ret = 0;
    while(*(str+ret))  ++ret;

    return ret;
}

// for memory that needs to be touched.
 inline void __crete_touch_buffer(volatile void *buffer, size_t size)
{
    size_t i;
    volatile char *b = (volatile char *) buffer;
    for (i = 0; i < size; ++i) {
        *b; ++b;
    }
}

//--------------------- For crete-run/preload-run
void crete_send_target_pid(void)
{
    __asm__ __volatile__(
            CRETE_INSTR_SEND_TARGET_PID()
    );
}

void crete_void_target_pid(void)
{
    __asm__ __volatile__(
            CRETE_INSTR_VOID_TARGET_PID()
    );
}

void crete_send_custom_instr_dump(void)
{
    __asm__ __volatile__(
        CRETE_INSTR_DUMP()
    );
}

//------ For program under test
// Function to be captured for replay "crete_make_concolic()":
// 1. Capture values of concolic variable and its name by touching/reading buffer
// 2. Inject a call to helper_crete_make_concolic() so that SVM can inject symbolic values
 void __crete_make_concolic_internal(void *addr, size_t size, const char* name)
{
    // Touch the content of name and force it is being captured
    {
        size_t name_size = my_strlen(name) + 1; // Force touching the terminator '\0'
        volatile char *_addr = (volatile char *) addr;
        volatile char *_name = (volatile char *) name;

        size_t i;
        for (i = 0; (i < size && i < name_size); ++i) {
            *_addr; *_name;
            ++_name; ++_addr;
        }

        if(i < name_size) {
            --_addr;
            for(; i < name_size; ++i)
            {
                *_addr; *_name;
                ++_name;
            }
        } else if (i < size) {
            for(; i < size; ++i)
            {
                *_addr; ++_addr;
            }
        }
    }

    __asm__ __volatile__(
        CRETE_INSTR_MAKE_CONCOLIC_INTERNAL()
        : : "a" (addr), "c" (size), "d" (name)
    );
}

 int crete_send_concolic_name(const char volatile *name) {
    size_t name_size;
    name_size = my_strlen((const char *)name);
    __crete_touch_buffer((void*)name, name_size);

    __asm__ __volatile__(
            CRETE_INSTR_SEND_CONCOLIC_NAME()
        : : "a" (name), "c" (name_size)
    );

    // Convention: when the current concolic variable exceeds the limit,
    // qemu will set the name as all 0s.
    if(name[0] == '\0')
        return 1;
    else
        return 0;
}

// Prepare for "crete_make_concolic()" within VM for tracing:
// Send information from guest to VM, guest_addr, size, name and name size of concolic value, so that:
// 1. Set the correct value of concolic variale from test case
// 2. Enable taint-analysis on this concolic variable
 int crete_pre_make_concolic(void* addr, size_t size, const char* name)
{
    // CRETE_INSTR_SEND_CONCOLIC_NAME must be sent before CRETE_INSTR_PRE_MAKE_CONCOLIC,
    // to satisfy the hanlder's order in qemu/runtime-dump/custom-instruction.cpp
    if(crete_send_concolic_name(name))
        return 1;

    __crete_touch_buffer(addr, size);

    __asm__ __volatile__(
            CRETE_INSTR_PRE_MAKE_CONCOLIC()
        : : "a" (addr), "c" (size)
    );

    return 0;
}

 char crete_check_target_pid(void)
{
    volatile char ret  = 0;

    __asm__ __volatile__(
            CRETE_INSTR_CHECK_TARGET_PID()
            : : "a" (&ret)
    );

    return ret;
}

void crete_make_concolic(void* addr, size_t size, const char* name)
{
    if(!crete_check_target_pid())
        return;

    size_t name_len = my_strlen(name);
    size_t copy_size = (name_len < MAX_NAME_SIZE)? name_len:MAX_NAME_SIZE;
    int i = 0;
    for(; i < copy_size; ++i)
      {
        concolic_name[i] = name[i];
      }
    /* my_strncpy(concolic_name, name, copy_size); */
    concolic_name[copy_size] = '\0';

    if(crete_pre_make_concolic(addr, size, concolic_name))
        return;

    __crete_make_concolic_internal(addr, size, concolic_name);
}

char crete_wait_test_case(void)
{
    volatile char ret  = 0;

    __asm__ __volatile__(
            CRETE_INSTR_WAIT_TEST_CASE()
            : : "a" (&ret)
    );

    return ret;
}

void crete_init(void)
{
    __asm__ __volatile__(
            CRETE_INSTR_PRIME()
    );
}

void crete_iteration_begin(void)
{
  while(!crete_wait_test_case());

  crete_send_target_pid();
}

void crete_iteration_end(void)
{
  crete_void_target_pid();
  crete_send_custom_instr_dump();
}
