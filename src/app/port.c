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


#if defined(_M_IX86)
typedef uint32_t stack_word;
#define PORT_PC Eip
#define PORT_SP Esp
#else
typedef uint64_t stack_word;
#define PORT_PC Rip
#define PORT_SP Rsp
#endif

static void log_registers(const CONTEXT *context)
{
#if defined(_M_IX86)
    platform_log("port:   eax %08X ebx %08X ecx %08X edx %08X esi %08X edi %08X ebp %08X esp %08X", context->Eax, context->Ebx, context->Ecx, context->Edx,
        context->Esi, context->Edi, context->Ebp, context->Esp);
#else
    platform_log("port:   rax %016llX rbx %016llX rcx %016llX rdx %016llX", context->Rax, context->Rbx, context->Rcx, context->Rdx);
    platform_log("port:   rsi %016llX rdi %016llX rbp %016llX rsp %016llX", context->Rsi, context->Rdi, context->Rbp, context->Rsp);
#endif
}

#if defined(_M_X64)
static void log_unwound_frames(const CONTEXT *fault, uintptr_t start, uintptr_t end)
{
    CONTEXT context = *fault;
    for (int32_t depth = 0; depth < 48 && context.Rip != 0; depth++) {
        uintptr_t pc = (uintptr_t) context.Rip;
        if (pc > start && pc < end) {
            platform_log("port:   frame %d rva 0x%X", depth, (uint32_t) (pc - start));
        } else {
            platform_log("port:   frame %d 0x%llX", depth, (unsigned long long) pc);
        }
        DWORD64 image_base = 0;
        PRUNTIME_FUNCTION function = RtlLookupFunctionEntry(context.Rip, &image_base, NULL);
        if (!function) {
            context.Rip = *(const DWORD64 *) (uintptr_t) context.Rsp;
            context.Rsp += 8;
            continue;
        }
        void *handler_data = NULL;
        DWORD64 establisher = 0;
        RtlVirtualUnwind(UNW_FLAG_NHANDLER, image_base, context.Rip, function, &context, &handler_data, &establisher, NULL);
    }
}
#else
static void log_unwound_frames(const CONTEXT *fault, uintptr_t start, uintptr_t end)
{
    (void) fault;
    (void) start;
    (void) end;
}
#endif

static LONG handle_exception(EXCEPTION_POINTERS *info, int32_t resolve)
{
    (void) resolve;
    DWORD code = info->ExceptionRecord->ExceptionCode;
    if (code != EXCEPTION_BREAKPOINT && code != EXCEPTION_ACCESS_VIOLATION && code != EXCEPTION_ILLEGAL_INSTRUCTION && code != EXCEPTION_PRIV_INSTRUCTION &&
        code != EXCEPTION_STACK_OVERFLOW) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    CONTEXT *context = info->ContextRecord;
    unsigned long long pc = (unsigned long long) context->PORT_PC;
    const stack_word *stack = (const stack_word *) (uintptr_t) context->PORT_SP;
    unsigned long long caller = 0;
    if (!IsBadReadPtr(stack, sizeof(stack_word))) {
        caller = (unsigned long long) stack[0];
    }
    unsigned long long target = code == EXCEPTION_ACCESS_VIOLATION ? (unsigned long long) info->ExceptionRecord->ExceptionInformation[1] : 0;
    platform_log("port: %s at 0x%llX (address 0x%llX, [sp] 0x%llX)", exception_name(code), pc, target, caller);
    fprintf(stderr, "%s at 0x%llX (address 0x%llX, [sp] 0x%llX)\n", exception_name(code), pc, target, caller);
    log_registers(context);
    HMODULE self = GetModuleHandleA(0);
    const IMAGE_NT_HEADERS *nt = (const IMAGE_NT_HEADERS *) ((const uint8_t *) self + ((const IMAGE_DOS_HEADER *) self)->e_lfanew);
    uintptr_t start = (uintptr_t) self;
    uintptr_t end = start + nt->OptionalHeader.SizeOfImage;
    platform_log("port:   our image at 0x%llX (map addresses are relative to the preferred base 0x%llX)", (unsigned long long) start,
        (unsigned long long) nt->OptionalHeader.ImageBase);
    log_unwound_frames(context, start, end);
    for (int32_t i = 0, found = 0; i < 8192 && found < 40 && !IsBadReadPtr(stack + i, sizeof(stack_word)); i++) {
        if (stack[i] > start && stack[i] < end) {
            platform_log("port:   [sp+0x%X] 0x%llX (rva 0x%X)", i * (int32_t) sizeof(stack_word), (unsigned long long) stack[i], (uint32_t) (stack[i] - start));
            found++;
        }
    }
    ExitProcess(3);
}


static LONG WINAPI unhandled_exception_filter(EXCEPTION_POINTERS *info)
{
    return handle_exception(info, 0);
}

#if defined(_M_IX86)
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
#endif

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
