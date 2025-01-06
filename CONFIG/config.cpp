#include "config.h"

#include "MainDlg.h"
#include "detectdx5.h"

#include <direct.h> // _chdir
#include <mxdirectx/legodxinfo.h>
#include <mxdirectx/mxdirect3d.h>

#include <inifile-cpp/inicpp.h>

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

DECOMP_SIZE_ASSERT(CWinApp, 0xc4)
DECOMP_SIZE_ASSERT(CConfigApp, 0x108)

DECOMP_STATIC_ASSERT(offsetof(CConfigApp, m_display_bit_depth) == 0xd0)

BEGIN_MESSAGE_MAP(CConfigApp, CWinApp)
ON_COMMAND(ID_HELP, OnHelp)
END_MESSAGE_MAP()

// GLOBAL: CONFIG 0x00408e50
CConfigApp g_theApp;

#define MiB (1024 * 1024)

namespace
{
const std::string SettingsIniFile = "Settings.ini";

fs::path GetConfigFilePath()
{
	char FilePath[MAX_PATH];
	if (GetModuleFileNameA(nullptr, FilePath, sizeof(FilePath)))
	{
		return fs::path(FilePath).parent_path() / SettingsIniFile;
	}

	return fs::current_path() / SettingsIniFile;
}
}

// FUNCTION: CONFIG 0x00402dc0
BOOL CConfigApp::InitInstance()
{
	if (!IsLegoNotRunning())
	{
		return FALSE;
	}
	if (!DetectDirectX5())
	{
		AfxMessageBox(
			"\"LEGO\xae Island\" is not detecting DirectX 5 or later.  Please quit all other applications and try "
			"again."
		);
		return FALSE;
	}

	m_device_enumerator = new LegoDeviceEnumerate;
	if (m_device_enumerator->DoEnumerate()) {
		return FALSE;
	}
	m_driver = NULL;
	m_device = NULL;
	m_full_screen = TRUE;
	m_wide_view_angle = TRUE;
	m_use_joystick = FALSE;
	m_music = TRUE;
	m_flip_surfaces = FALSE;
	m_3d_video_ram = FALSE;
	m_joystick_index = -1;
	m_display_bit_depth = 16;
	MEMORYSTATUS memory_status;
	memory_status.dwLength = sizeof(memory_status);
	GlobalMemoryStatus(&memory_status);
	if (memory_status.dwTotalPhys < 12 * MiB) {
		m_3d_sound = FALSE;
		m_model_quality = 0;
		m_texture_quality = 1;
	}
	else if (memory_status.dwTotalPhys < 20 * MiB) {
		m_3d_sound = FALSE;
		m_model_quality = 1;
		m_texture_quality = 1;
	}
	else {
		m_model_quality = 2;
		m_3d_sound = TRUE;
		m_texture_quality = 1;
	}

	if (!fs::exists(GetConfigFilePath()))
	{
		SaveConfig();
	}

	CMainDialog main_dialog(NULL);
	main_dialog.DoModal();
	return FALSE;
}

// FUNCTION: CONFIG 0x00403100
BOOL CConfigApp::IsLegoNotRunning()
{
	HWND hWnd = FindWindowA("Lego Island MainNoM App", "LEGO\xae");
	if (_stricmp(afxCurrentAppName, "config") == 0 || !hWnd) {
		return TRUE;
	}
	if (SetForegroundWindow(hWnd)) {
		ShowWindow(hWnd, SW_RESTORE);
	}
	return FALSE;
}

// FUNCTION: CONFIG 0x004033d0
BOOL CConfigApp::IsDeviceInBasicRGBMode() const
{
	return !GetHardwareDeviceColorModel() && m_device->m_HELDesc.dcmColorModel & D3DCOLOR_RGB;
}

// FUNCTION: CONFIG 0x00403400
D3DCOLORMODEL CConfigApp::GetHardwareDeviceColorModel() const
{
	return m_device->m_HWDesc.dcmColorModel;
}

// FUNCTION: CONFIG 0x00403410
BOOL CConfigApp::IsPrimaryDriver() const
{
	return m_driver && m_driver == &m_device_enumerator->GetDriverList().front();
}

// FUNCTION: CONFIG 0x00403630
BOOL CConfigApp::ValidateSettings()
{
	BOOL is_modified = FALSE;

	if (!IsPrimaryDriver() && !m_full_screen) {
		m_full_screen = TRUE;
		is_modified = TRUE;
	}
	if (IsDeviceInBasicRGBMode()) {
		if (m_3d_video_ram) {
			m_3d_video_ram = FALSE;
			is_modified = TRUE;
		}
		if (m_flip_surfaces) {
			m_flip_surfaces = FALSE;
			is_modified = TRUE;
		}
		if (m_display_bit_depth != 16) {
			m_display_bit_depth = 16;
			is_modified = TRUE;
		}
	}
	if (!GetHardwareDeviceColorModel()) {
		m_draw_cursor = FALSE;
		is_modified = TRUE;
	}
	else {
		if (!m_3d_video_ram) {
			m_3d_video_ram = TRUE;
			is_modified = TRUE;
		}
		if (m_full_screen && !m_flip_surfaces) {
			m_flip_surfaces = TRUE;
			is_modified = TRUE;
		}
	}
	if (m_flip_surfaces) {
		if (!m_3d_video_ram) {
			m_3d_video_ram = TRUE;
			is_modified = TRUE;
		}
		if (!m_full_screen) {
			m_full_screen = TRUE;
			is_modified = TRUE;
		}
	}
	if ((m_display_bit_depth != 8 && m_display_bit_depth != 16) && (m_display_bit_depth != 0 || m_full_screen)) {
		m_display_bit_depth = 8;
		is_modified = TRUE;
	}
	if (m_model_quality < 0 || m_model_quality > 2) {
		m_model_quality = 1;
		is_modified = TRUE;
	}
	if (m_texture_quality < 0 || m_texture_quality > 1) {
		m_texture_quality = 0;
		is_modified = TRUE;
	}
	return is_modified;
}

// FUNCTION: CONFIG 0x004037a0
DWORD CConfigApp::GetConditionalDeviceRenderBitDepth() const
{
	if (IsDeviceInBasicRGBMode()) {
		return 0;
	}
	if (GetHardwareDeviceColorModel()) {
		return 0;
	}
	return m_device->m_HELDesc.dwDeviceRenderBitDepth & 0x800;
}

// FUNCTION: CONFIG 0x004037e0
DWORD CConfigApp::GetDeviceRenderBitStatus() const
{
	if (GetHardwareDeviceColorModel()) {
		return m_device->m_HWDesc.dwDeviceRenderBitDepth & 0x400;
	}
	else {
		return m_device->m_HELDesc.dwDeviceRenderBitDepth & 0x400;
	}
}

// FUNCTION: CONFIG 0x00403810
BOOL CConfigApp::AdjustDisplayBitDepthBasedOnRenderStatus()
{
	if (m_display_bit_depth == 8) {
		if (GetConditionalDeviceRenderBitDepth()) {
			return FALSE;
		}
	}
	if (m_display_bit_depth == 16) {
		if (GetDeviceRenderBitStatus()) {
			return FALSE;
		}
	}
	if (GetConditionalDeviceRenderBitDepth()) {
		m_display_bit_depth = 8;
		return TRUE;
	}
	if (GetDeviceRenderBitStatus()) {
		m_display_bit_depth = 16;
		return TRUE;
	}
	m_display_bit_depth = 8;
	return TRUE;
}

// FUNCTION: CONFIG 0x00403a90
int CConfigApp::ExitInstance()
{
	if (m_device_enumerator) {
		delete m_device_enumerator;
		m_device_enumerator = NULL;
	}
	return CWinApp::ExitInstance();
}

BOOL CConfigApp::LoadConfig()
{
	fs::path IniFilePath = GetConfigFilePath();
	if (!fs::exists(IniFilePath))
	{
		// For some reason the config file was not found. Try to write a new config file.
		SaveConfig();
	}

	ini::IniFile IniFile;
	IniFile.load(IniFilePath.string());

	std::string DeviceId;
	int ParsedDeviceId = -1;
	bool isModified = FALSE;

	auto &GeneralSettings = IniFile["General"];

	auto ReadEntry = [&GeneralSettings]<typename T>(const std::string &EntryName, T* OutValue)
	{
		if (GeneralSettings.contains(EntryName))
		{
			*OutValue = GeneralSettings[EntryName].as<T>();
			return true;
		}

		return false;
	};

	if (ReadEntry("DeviceID", &DeviceId))
	{
		ParsedDeviceId = m_device_enumerator->ParseDeviceName(DeviceId.c_str());
		if (ParsedDeviceId >= 0)
		{
			ParsedDeviceId = m_device_enumerator->GetDevice(ParsedDeviceId, m_driver, m_device);
		}
	}

	if (ParsedDeviceId != 0)
	{
		isModified = TRUE;
		m_device_enumerator->FUN_1009d210();
		ParsedDeviceId = m_device_enumerator->FUN_1009d0d0();
		m_device_enumerator->GetDevice(ParsedDeviceId, m_driver, m_device);
	}

	if (!ReadEntry("DisplayBitDepth", &m_display_bit_depth))
	{
		isModified = TRUE;
	}
	if (!ReadEntry("FlipSurfaces", &m_flip_surfaces))
	{
		isModified = TRUE;
	}
	if (!ReadEntry("FullScreen", &m_full_screen))
	{
		isModified = TRUE;
	}
	if (!ReadEntry("WriteVideoRAM", &m_3d_video_ram))
	{
		isModified = TRUE;
	}
	if (!ReadEntry("WideViewAngle", &m_wide_view_angle))
	{
		isModified = TRUE;
	}
	if (!ReadEntry("Sound3D", &m_3d_sound))
	{
		isModified = TRUE;
	}
	if (!ReadEntry("DrawCursor", &m_draw_cursor))
	{
		isModified = TRUE;
	}
	if (!ReadEntry("ModelQuality", &m_model_quality))
	{
		isModified = TRUE;
	}
	if (!ReadEntry("TextureQuality", &m_texture_quality))
	{
		isModified = TRUE;
	}
	if (!ReadEntry("UseJoystick", &m_use_joystick))
	{
		isModified = TRUE;
	}
	if (!ReadEntry("Music", &m_music))
	{
		isModified = TRUE;
	}
	if (!ReadEntry("JoystickIndex", &m_joystick_index))
	{
		isModified = TRUE;
	}
	return isModified;
}

void CConfigApp::SaveConfig()
{
	ini::IniFile IniFile;

	char buffer[256];

	if (!m_driver || !m_device)
	{
		//No device has been selected yet. Default to the first device
		m_device_enumerator->GetDevice(0, m_driver, m_device);
	}

	m_device_enumerator->FormatDeviceName(buffer, m_driver, m_device);

	auto &GeneralSettings = IniFile["General"];
	GeneralSettings["DeviceID"] = buffer;
	GeneralSettings["DeviceName"] = m_device->m_deviceName;
	GeneralSettings["DisplayBitDepth"] = m_display_bit_depth;
	GeneralSettings["FlipSurfaces"] = m_flip_surfaces;
	GeneralSettings["FullScreen"] = m_full_screen;
	GeneralSettings["WriteVideoRAM"] = m_3d_video_ram;
	GeneralSettings["WideViewAngle"] = m_wide_view_angle;
	GeneralSettings["Sound3D"] = m_3d_sound;
	GeneralSettings["DrawCursor"] = m_draw_cursor;
	GeneralSettings["ModelQuality"] = m_model_quality;
	GeneralSettings["TextureQuality"] = m_texture_quality;
	GeneralSettings["UseJoystick"] = m_use_joystick;
	GeneralSettings["Music"] = m_music;
	GeneralSettings["JoystickIndex"] = m_joystick_index;

	IniFile.save(GetConfigFilePath().string());
}
