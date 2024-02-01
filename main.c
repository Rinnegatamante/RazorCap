#include <vitasdk.h>
#include <taihen.h>
#include <stdio.h>

#ifndef HAVE_DEVKIT
#define DEFAULT_RAZOR_CAPTURE_PATH "ur0:data/librazorcapture_es4.suprx"
#endif

static SceUID mod_id;
static SceUID setFrameBuf_hook, ioOpen_hook;
static tai_hook_ref_t setFrameBuf_ref, ioOpen_ref;

static char titleid[10];

int sceDisplaySetFrameBuf_patch(const SceDisplayFrameBuf *pParam, SceDisplaySetBufSync sync) {
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

	return TAI_CONTINUE(int, setFrameBuf_ref, pParam, sync);
}

// Fixes faulty `mode` requested by RazorCap with some applications
SceUID sceIoOpen_patch(const char * file, int flags, SceMode mode) {
	return TAI_CONTINUE(SceUID, ioOpen_ref, file, flags, 0606);
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args) {
	sceAppMgrAppParamGetString(0, 12, titleid, sizeof(titleid));
	titleid[sizeof(titleid)-1] = '\0';

#ifdef HAVE_DEVKIT
	sceSysmoduleLoadModule(SCE_SYSMODULE_RAZOR_CAPTURE);
#else
	mod_id = sceKernelLoadStartModule(DEFAULT_RAZOR_CAPTURE_PATH, 0, NULL, 0, NULL, NULL);
	ioOpen_hook = taiHookFunctionImport(&ioOpen_ref, "SceRazorCapture", TAI_ANY_LIBRARY, 0x6C60AC61, sceIoOpen_patch);
#endif

	setFrameBuf_hook = taiHookFunctionImport(&setFrameBuf_ref, TAI_MAIN_MODULE, TAI_ANY_LIBRARY, 0x7A410B64, sceDisplaySetFrameBuf_patch);

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
#ifdef HAVE_DEVKIT
	sceSysmoduleUnloadModule(SCE_SYSMODULE_RAZOR_CAPTURE);
#else
	taiHookRelease(ioOpen_hook, ioOpen_ref);
	sceKernelStopUnloadModule(mod_id, 0, NULL, 0, NULL, NULL);
#endif
	taiHookRelease(setFrameBuf_hook, setFrameBuf_ref);
	return SCE_KERNEL_STOP_SUCCESS;
}
