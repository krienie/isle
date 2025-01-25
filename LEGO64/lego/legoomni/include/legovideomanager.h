#pragma once

#include "legophonemelist.h"
#include "mxvideomanager.h"
#include "mxvulkan/mxvulkan.h"

#include <memory>

class LegoVideoManager : public MxVideoManager
{
public:
	LegoVideoManager() = default;
	~LegoVideoManager() override;

	MxResult Tickle() override;
	void Destroy() override;
	MxResult Create(MxVideoParam& p_videoParam, MxU32 p_frequencyMS, MxBool p_createThread) override;

private:
	std::unique_ptr<MxVulkan> mVulkanRHI;
};

/*#include "decomp.h"
#include "legophonemelist.h"
#include "mxvideomanager.h"

//#include <d3drm.h>
//#include <ddraw.h>

class Lego3DManager;
class LegoROI;
class MxDirect3D;
class MxStopWatch;
struct ViewportAppData;

namespace Tgl
{
class Renderer;
}

// VTABLE: LEGO1 0x100d9c88
// SIZE 0x590
class LegoVideoManager : public MxVideoManager {
public:
	LegoVideoManager();
	~LegoVideoManager() override;

	int EnableRMDevice();
	int DisableRMDevice();
	void EnableFullScreenMovie(MxBool p_enable);
	void EnableFullScreenMovie(MxBool p_enable, MxBool p_scale);
	void MoveCursor(MxS32 p_cursorX, MxS32 p_cursorY);
	void ToggleFPS(MxBool p_visible);

	MxResult Tickle() override;                                                                       // vtable+0x08
	void Destroy() override;                                                                          // vtable+0x18
	MxResult Create(MxVideoParam& p_videoParam, MxU32 p_frequencyMS, MxBool p_createThread) override; // vtable+0x2c
	MxResult RealizePalette(MxPalette*) override;                                                     // vtable+0x30
	void UpdateView(MxU32 p_x, MxU32 p_y, MxU32 p_width, MxU32 p_height) override;                    // vtable+0x34
	virtual MxPresenter* GetPresenterAt(MxS32 p_x, MxS32 p_y);                                        // vtable+0x38

	// FUNCTION: LEGO1 0x1007ab10
	virtual LegoPhonemeList* GetPhonemeList() { return m_phonemeRefList; } // vtable+0x3c

	void SetSkyColor(float p_red, float p_green, float p_blue);
	void OverrideSkyColor(MxBool p_shouldOverride);
	MxResult ResetPalette(MxBool p_ignoreSkyColor);
	MxPresenter* GetPresenterByActionObjectName(const char* p_char);

	void FUN_1007c520();

	Tgl::Renderer* GetRenderer() { return m_renderer; }

	// FUNCTION: BETA10 0x100117e0
	Lego3DManager* Get3DManager() { return m_3dManager; }

	// FUNCTION: BETA10 0x1003a380
	LegoROI* GetViewROI() { return m_viewROI; }

	MxDirect3D* GetDirect3D() { return m_direct3d; }
	MxBool GetRender3D() { return m_render3d; }
	double GetElapsedSeconds() { return m_elapsedSeconds; }

	void SetRender3D(MxBool p_render3d) { m_render3d = p_render3d; }
	void SetUnk0x554(MxBool p_unk0x554) { m_unk0x554 = p_unk0x554; }

private:
	MxResult CreateDirect3D();
	MxResult ConfigureD3DRM();
	void DrawFPS();

	inline void DrawCursor();

	Tgl::Renderer* m_renderer;
	Lego3DManager* m_3dManager;
	LegoROI* m_viewROI;
	undefined4 m_unk0x70;
	MxDirect3D* m_direct3d;
	undefined4 m_unk0x78[27];
	MxBool m_render3d;
	MxBool m_unk0xe5;
	MxBool m_unk0xe6;
	PALETTEENTRY m_paletteEntries[256];
	LegoPhonemeList* m_phonemeRefList;
	MxBool m_isFullscreenMovie;
	MxPalette* m_palette;
	MxStopWatch* m_stopWatch;
	double m_elapsedSeconds;
	MxBool m_fullScreenMovie;
	MxBool m_drawCursor;
	MxS32 m_cursorXCopy;
	MxS32 m_cursorYCopy;
	MxS32 m_cursorX;
	MxS32 m_cursorY;
	LPDIRECTDRAWSURFACE m_cursorSurface;
	RECT m_cursorRect;
	LPDIRECTDRAWSURFACE m_unk0x528;
	MxBool m_drawFPS;
	RECT m_fpsRect;
	HFONT m_arialFont;
	SIZE m_fpsSize;
	MxFloat m_unk0x54c;
	MxFloat m_unk0x550;
	MxBool m_unk0x554;
	MxBool m_paused;
	D3DVALUE m_back;
	D3DVALUE m_front;
	float m_cameraWidth;
	float m_cameraHeight;
	D3DVALUE m_fov;
	IDirect3DRMFrame* m_camera;
	D3DRMPROJECTIONTYPE m_projection;
	ViewportAppData* m_appdata;
	D3DRMRENDERQUALITY m_quality;
	DWORD m_shades;
	D3DRMTEXTUREQUALITY m_textureQuality;
	DWORD m_rendermode;
	BOOL m_dither;
	DWORD m_bufferCount;
};
*/
