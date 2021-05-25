#include <vitasdk.h>
#include <taihen.h>

#define HOOKS_NUM      1

#ifndef HAVE_DEVKIT
#define DEFAULT_RAZOR_CAPTURE_PATH "ur0:data/librazorcapture_es4.suprx"
#endif

// Hooks related variables
static SceUID g_hooks[HOOKS_NUM], mod_id;
static tai_hook_ref_t ref[HOOKS_NUM];
static uint8_t cur_hook = 0;
static char titleid[16];

// Generic hooking function
void hookFunction(uint32_t nid, const void* func) {
	g_hooks[cur_hook] = taiHookFunctionImport(&ref[cur_hook], TAI_MAIN_MODULE, TAI_ANY_LIBRARY, nid, func);
	cur_hook++;
}

int sceDisplaySetFrameBuf_patched(const SceDisplayFrameBuf *pParam, int sync) {
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(0, &pad, 1);
	
	if ((pad.buttons & SCE_CTRL_START) &&
		(pad.buttons & SCE_CTRL_SELECT) &&
		(pad.buttons & SCE_CTRL_LEFT)){
		SceDateTime date;
		sceRtcGetCurrentClockLocalTime(&date);
		char fname[256];
		sprintf(fname, "ux0:data/cap_%s-%02d_%02d_%04d-%02d_%02d_%02d.sgx", titleid, date.day, date.month, date.year, date.hour, date.minute, date.second);
		sceRazorGpuCaptureSetTriggerNextFrame(fname);
	}
	
	return TAI_CONTINUE(int, ref[0], pParam, sync);
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args) {
	sceAppMgrAppParamGetString(0, 12, titleid , 256);
	
#ifdef HAVE_DEVKIT
	sceSysmoduleLoadModule(SCE_SYSMODULE_RAZOR_CAPTURE);
#else
	mod_id = sceKernelLoadStartModule(DEFAULT_RAZOR_CAPTURE_PATH, 0, NULL, 0, NULL, NULL);
#endif
	hookFunction(0x7A410B64, sceDisplaySetFrameBuf_patched);
	
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
#ifdef HAVE_DEVKIT
	sceSysmoduleUnloadModule(SCE_SYSMODULE_RAZOR_CAPTURE);
#else
	sceKernelStopUnloadModule(mod_id, 0, NULL, 0, NULL, NULL);
#endif
	taiHookRelease(g_hooks[0], ref[0]);
	
	return SCE_KERNEL_STOP_SUCCESS;
	
}