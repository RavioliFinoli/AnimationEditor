
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

Camera* pCamera = nullptr;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance); //1. Skapa f�nster

	AnimationEditorApplication app;
	app.Init(wndHandle);
	app.LoadStaticMeshFilesInDirectory("C:\\Repos\\AnimationEditor\\Assets");
	pCamera = app.GetCamera();
	//init mouse
	{
		RAWINPUTDEVICE Rid[1];
		Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		Rid[0].dwFlags = RIDEV_INPUTSINK;
		Rid[0].hwndTarget = wndHandle;
		RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
	}

	if (wndHandle)
	{
		
		ShowWindow(wndHandle, nCmdShow);

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				app.Render(); //8. Rendera
				app.Present(); //9. V�xla front- och back-buffer
			}
		}

		DestroyWindow(wndHandle);
	}

	return (int)msg.wParam;
}

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
