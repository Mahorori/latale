#pragma once

namespace dinput8
{
	typedef HRESULT(WINAPI * pfnDirectInput8Create)(HINSTANCE hinst,
		DWORD dwVersion,
		REFIID riidltf,
		LPVOID *ppvOut,
		LPUNKNOWN punkOuter);

	typedef HRESULT(WINAPI * pfnDllCanUnloadNow)();

	typedef HRESULT(WINAPI * pfnDllGetClassObject)(
		const CLSID & rclsid,
		const IID & riid,
		void ** ppv);

	typedef HRESULT(WINAPI * pfnDllRegisterServer)();

	typedef HRESULT(WINAPI * pfnDllUnregisterServer)();

	BOOL Init();
}