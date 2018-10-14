
#include "importer/FormatHeader.h"
#include "importer/FormatImporter.h"
#include "AnimationEditorApplication.h"

#ifndef UNICODE
#define UNICODE
#endif 

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

#include <windows.h>
#include <d3dcompiler.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

Camera* pCamera;

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = L"BTH_D3D_DEMO";
	if (!RegisterClassEx(&wcex))
		return false;

	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND handle = CreateWindow(
		L"BTH_D3D_DEMO",
		L"BTH Direct3D Demo",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	return handle;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int i = 0;
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_INPUT:
	{
		UINT rawInputSize;

		GetRawInputData((HRAWINPUT)(lParam), RID_INPUT, nullptr, &rawInputSize, sizeof(RAWINPUTHEADER));

		LPBYTE inputBuffer = new BYTE[rawInputSize];

		UINT poo = GetRawInputData((HRAWINPUT)(lParam), RID_INPUT, inputBuffer, &rawInputSize, sizeof(RAWINPUTHEADER));

		RAWINPUT* inp = (RAWINPUT*)inputBuffer;   // valid

		if (inp->header.dwType == RIM_TYPEMOUSE)
		{
			int xPosRelative = inp->data.mouse.lLastX;
			int yPosRelative = inp->data.mouse.lLastY;
			float pitch = static_cast<float>(yPosRelative) / 100.f;
			float yaw = static_cast<float>(xPosRelative) / 100.f;
			pCamera->IncreasePitchYaw(pitch, yaw);
		}
		break;
	}
	}


	return DefWindowProc(hWnd, message, wParam, lParam);
}
