#include "windows.h"
#include "cocos2d.h"
#include "MinHook.h"
#include "hackpro_ext.h"

// original function pointers

void (__thiscall *fpMainLoop)(cocos2d::CCDirector *self);
void (__thiscall *fpPollEvents)(cocos2d::CCEGLView *self);

// hook functions

void __thiscall hkMainLoop(cocos2d::CCDirector *self) {
    fpPollEvents(self->getOpenGLView());
    fpMainLoop(self);
}

void __thiscall hkPollEvents(void *self) {};

// mega hack callbacks

void __stdcall enableHooks(void *) {
    MH_EnableHook(MH_ALL_HOOKS);
}

void __stdcall disableHooks(void *) {
    MH_DisableHook(MH_ALL_HOOKS);
}

DWORD WINAPI thread(LPVOID lpParameter) {
    MH_Initialize();
    const HINSTANCE cocos = GetModuleHandleA("libcocos2d.dll");
    MH_CreateHook(reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(cocos) + 0xffb10), hkMainLoop, reinterpret_cast<LPVOID *>(&fpMainLoop));
    MH_CreateHook(GetProcAddress(cocos, "?pollEvents@CCEGLView@cocos2d@@QAEXXZ"), hkPollEvents, reinterpret_cast<LPVOID *>(&fpPollEvents));

    if (InitialiseHackpro()) {
        if (HackproIsReady()) {
            void *extension = HackproInitialiseExt("Input Lag");
            void *checkbox = HackproAddCheckbox(extension, "-1 frame", enableHooks, disableHooks);
            HackproSetCheckbox(checkbox, true);
            HackproCommitExt(extension);
        }
    } else {
        MH_EnableHook(MH_ALL_HOOKS);
    }
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		CreateThread(0, 0, thread, 0, 0, 0);
	return TRUE;
}