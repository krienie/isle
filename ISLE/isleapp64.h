
#pragma once

#include "mxtypes.h"
#include "mxvideoparam.h"

#include <string>
#include <SDL2/SDL.h>
#include <windows.h>

// SIZE 0x8c
class IsleApp {
public:
	IsleApp();
	~IsleApp();

	void Close();

	BOOL SetupLegoOmni();
	void SetupVideoFlags(
		BOOL fullScreen,
		BOOL flipSurfaces,
		BOOL backBuffers,
		BOOL using8bit,
		BOOL using16bit,
		BOOL param_6,
		BOOL param_7,
		BOOL wideViewAngle,
		const std::string &deviceId
	);
	MxResult SetupWindow(HINSTANCE hInstance, LPSTR lpCmdLine);

	BOOL ReadReg(LPCSTR name, LPSTR outValue, DWORD outSize);
	BOOL ReadRegBool(LPCSTR name, BOOL* out);
	BOOL ReadRegInt(LPCSTR name, int* out);

	void LoadConfig();
	void Tick(BOOL sleepIfNotNextFrame);

	SDL_Window* GetWindowHandle() const { return m_windowHandle; }
	MxLong GetFrameDelta() const { return m_frameDelta; }
	BOOL GetFullScreen() const { return m_fullScreen; }
	HCURSOR GetCursorCurrent() const { return m_cursorCurrent; }
	HCURSOR GetCursorBusy() const { return m_cursorBusy; }
	HCURSOR GetCursorNo() const { return m_cursorNo; }
	BOOL GetDrawCursor() const { return m_drawCursor; }

	void SetWindowActive(BOOL p_windowActive) { m_windowActive = p_windowActive; }

private:
	std::string m_hdPath;
	std::string m_cdPath;
	std::string m_deviceId;
	std::string m_savePath;
	BOOL m_fullScreen;         // 0x10
	BOOL m_flipSurfaces;       // 0x14
	BOOL m_backBuffersInVram;  // 0x18
	BOOL m_using8bit;          // 0x1c
	BOOL m_using16bit;         // 0x20
	int m_unk0x24;             // 0x24
	BOOL m_use3dSound;         // 0x28
	BOOL m_useMusic;           // 0x2c
	BOOL m_useJoystick;        // 0x30
	int m_joystickIndex;       // 0x34
	BOOL m_wideViewAngle;      // 0x38
	int m_islandQuality;       // 0x3c
	int m_islandTexture;       // 0x40
	BOOL m_gameStarted;        // 0x44
	MxLong m_frameDelta;       // 0x48
	MxVideoParam m_videoParam; // 0x4c
	BOOL m_windowActive;       // 0x70
	SDL_Window* m_windowHandle = nullptr;
	BOOL m_drawCursor;         // 0x78
	HCURSOR m_cursorArrow;     // 0x7c
	HCURSOR m_cursorBusy;      // 0x80
	HCURSOR m_cursorNo;        // 0x84
	HCURSOR m_cursorCurrent;   // 0x88
};
