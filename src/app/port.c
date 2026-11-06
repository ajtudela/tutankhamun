// tutankhamun's run time: the C runtime reports to stderr, the game's static initializers run, and a crash is logged
// with the registers and the return addresses on the stack (a stack cookie found overwritten with its function).

#include "app/image.h"
#include "core/static_init.h"
#include "platform/log.h"

#include <windows.h>
#include <crtdbg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *exception_name(DWORD code)
{
    switch (code) {
        case EXCEPTION_BREAKPOINT:
            return "int3";
        case EXCEPTION_ACCESS_VIOLATION:
            return "access violation";
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            return "illegal instruction";
        default:
            return "exception";
    }
}


static LONG handle_exception(EXCEPTION_POINTERS *info, int32_t resolve)
{
    (void) resolve;
    DWORD code = info->ExceptionRecord->ExceptionCode;
    if (code != EXCEPTION_BREAKPOINT && code != EXCEPTION_ACCESS_VIOLATION && code != EXCEPTION_ILLEGAL_INSTRUCTION && code != EXCEPTION_PRIV_INSTRUCTION &&
        code != EXCEPTION_STACK_OVERFLOW) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    CONTEXT *context = info->ContextRecord;
    uint32_t eip = context->Eip;
    uint32_t caller = 0;
    if (!IsBadReadPtr((void *) context->Esp, 4)) {
        caller = *(uint32_t *) context->Esp;
    }
    uint32_t target = code == EXCEPTION_ACCESS_VIOLATION ? (uint32_t) info->ExceptionRecord->ExceptionInformation[1] : 0;
    int32_t named = 0;
    if (!named) {
        platform_log("port: %s at 0x%08X (address 0x%08X, [esp] 0x%08X)", exception_name(code), eip, target, caller);
        fprintf(stderr, "%s at 0x%08X (address 0x%08X, [esp] 0x%08X)\n", exception_name(code), eip, target, caller);
    }
    platform_log("port:   eax %08X ebx %08X ecx %08X edx %08X esi %08X edi %08X ebp %08X esp %08X", context->Eax, context->Ebx, context->Ecx, context->Edx,
        context->Esi, context->Edi, context->Ebp, context->Esp);
    HMODULE self = GetModuleHandleA(0);
    const IMAGE_NT_HEADERS32 *nt = (const IMAGE_NT_HEADERS32 *) ((const uint8_t *) self + ((const IMAGE_DOS_HEADER *) self)->e_lfanew);
    uint32_t start = (uint32_t) (uintptr_t) self, end = start + nt->OptionalHeader.SizeOfImage;
    const uint32_t *stack = (const uint32_t *) context->Esp;
    platform_log("port:   our image at 0x%08X (map addresses are relative to the preferred base 0x%08X)", start, nt->OptionalHeader.ImageBase);
    for (int32_t i = 0, found = 0; i < 8192 && found < 40 && !IsBadReadPtr(stack + i, 4); i++) {
        if (stack[i] > start && stack[i] < end) {
            platform_log("port:   [esp+0x%X] 0x%08X (rva 0x%X)", i * 4, stack[i], stack[i] - start);
            found++;
        }
    }
    ExitProcess(3);
}


static LONG WINAPI unhandled_exception_filter(EXCEPTION_POINTERS *info)
{
    return handle_exception(info, 0);
}

extern uintptr_t __security_cookie;

static __declspec(noreturn) void __stdcall cookie_failure(uint32_t from, const uint32_t *stack)
{
    uint32_t start = (uint32_t) (uintptr_t) GetModuleHandleA(0);
    platform_log("port: stack cookie overwritten in the function at rva 0x%X", from - start);
    fprintf(stderr, "stack cookie overwritten at rva 0x%X\n", from - start);
    for (int32_t i = 0, found = 0; i < 4096 && found < 20 && !IsBadReadPtr(stack + i, 4); i++) {
        if (stack[i] > start && stack[i] - start < 0x400000) {
            platform_log("port:   [esp+0x%X] rva 0x%X", i * 4, stack[i] - start);
            found++;
        }
    }
    __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE);
}

__declspec(naked) void __fastcall __security_check_cookie(uintptr_t cookie)
{
    (void) cookie;
    __asm {
        cmp ecx, __security_cookie
        jne failed
        ret
    failed:
        lea eax, [esp + 4]
        push eax
        push dword ptr [esp + 4]
        call cookie_failure
    }
}


static void invalid_parameter(const wchar_t *expression, const wchar_t *function, const wchar_t *file, unsigned int line, uintptr_t reserved)
{
    (void) reserved;
    platform_log("crt: invalid parameter %ls in %ls (%ls:%u)", expression ? expression : L"?", function ? function : L"?", file ? file : L"?", line);
}

static void crt_init(void)
{
    _set_invalid_parameter_handler(invalid_parameter);
    _CrtSetReportMode(_CRT_ASSERT, 0);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
}

void image_run_static_initializers(void)
{
    game_static_init();
}

int32_t image_init(void)
{
    crt_init();
    SetUnhandledExceptionFilter(unhandled_exception_filter);
    return 1;
}
