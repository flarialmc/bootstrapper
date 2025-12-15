#include <minhook.h>
#include <d3d12.h>
#include <dxgi1_4.h>

ATOM (*_RegisterClassExW)(PVOID) = {};
HRESULT (*_Present)(PVOID, UINT, UINT) = {};
HRESULT (*_ResizeBuffers)(PVOID, UINT, UINT, UINT, DXGI_FORMAT, UINT) = {};
HRESULT (*_CreateSwapChainForHwnd)(PVOID, PVOID, HWND, PVOID, PVOID, PVOID, PVOID) = {};
HRESULT (*_ResizeBuffers1)(PVOID, UINT, UINT, UINT, DXGI_FORMAT, UINT, PVOID, PVOID) = {};

PVOID __wrap_memcpy(PVOID Destination, PVOID Source, SIZE_T Count)
{
    __movsb(Destination, Source, Count);
    return Destination;
}

PVOID __wrap_memset(PVOID Destination, BYTE Data, SIZE_T Count)
{
    __stosb(Destination, Data, Count);
    return Destination;
}

__declspec(dllexport) EXCEPTION_DISPOSITION __CxxFrameHandler4(PVOID pExcept, PVOID pRN, PVOID pContext, PVOID pDC)
{
    static PEXCEPTION_HANDLER __CxxFrameHandler4 = {};

    if (!__CxxFrameHandler4)
    {
        HMODULE hModule = GetModuleHandleW(L"ucrtbase");
        __CxxFrameHandler4 = (PVOID)GetProcAddress(hModule, "__CxxFrameHandler4");
    }

    return __CxxFrameHandler4(pExcept, pRN, pContext, pDC);
}

HRESULT $Present(PVOID pSwapChain, UINT SyncInterval, UINT Flags)
{
    if (!SyncInterval)
        Flags |= DXGI_PRESENT_ALLOW_TEARING;
    return _Present(pSwapChain, SyncInterval, Flags);
}

HRESULT $ResizeBuffers(PVOID pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
                       UINT SwapChainFlags)
{
    SwapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    return _ResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

HRESULT $ResizeBuffers1(PVOID pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT Format,
                        UINT SwapChainFlags, PVOID pCreationNodeMask, PVOID ppPresentQueue)
{
    SwapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    return _ResizeBuffers1(pSwapChain, BufferCount, Width, Height, Format, SwapChainFlags, pCreationNodeMask,
                           ppPresentQueue);
}

HRESULT $CreateSwapChainForHwnd(PVOID pFactory, PVOID pDevice, HWND hWnd, DXGI_SWAP_CHAIN_DESC1 *pDesc,
                                PVOID pFullscreenDesc, PVOID pRestrictToOutput, IDXGISwapChain3 **ppSwapChain)
{
    static BOOL bHooked = FALSE;
    pDesc->Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    HRESULT hResult =
        _CreateSwapChainForHwnd(pFactory, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);

    if (!bHooked && !hResult)
    {
        MH_CreateHook((*ppSwapChain)->lpVtbl->Present, $Present, (PVOID)&_Present);
        MH_CreateHook((*ppSwapChain)->lpVtbl->ResizeBuffers, $ResizeBuffers, (PVOID)&_ResizeBuffers);
        MH_CreateHook((*ppSwapChain)->lpVtbl->ResizeBuffers1, $ResizeBuffers1, (PVOID)&_ResizeBuffers1);
        MH_EnableHook(MH_ALL_HOOKS);
        bHooked = TRUE;
    }

    return hResult;
}

HRESULT $D3D12CreateDevice(IUnknown *pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, PVOID *ppDevice)
{
    return DXGI_ERROR_UNSUPPORTED;
}

ATOM $RegisterClassExW(PWNDCLASSEXW pWndClass)
{
    static BOOL bHooked = FALSE;

    if (!bHooked)
    {
        WCHAR szPath[MAX_PATH] = {};
        ExpandEnvironmentStringsW(L"%LOCALAPPDATA%\\Flarial\\Client\\Config\\Bootstrapper.cfg", szPath, MAX_PATH);

        if (GetPrivateProfileIntW(L"Flarial", L"D3D11", FALSE, szPath) == TRUE)
        {
            HMODULE hModule = LoadLibraryExW(L"D3D12", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
            PROC D3D12CreateDevice = GetProcAddress(hModule, "D3D12CreateDevice");
            MH_CreateHook(D3D12CreateDevice, $D3D12CreateDevice, NULL);
        }

        IDXGIFactory2 *pFactory = NULL;
        CreateDXGIFactory(&IID_IDXGIFactory2, (PVOID)&pFactory);

        PVOID CreateSwapChainForHwnd = pFactory->lpVtbl->CreateSwapChainForHwnd;
        MH_CreateHook(CreateSwapChainForHwnd, $CreateSwapChainForHwnd, (PVOID)&_CreateSwapChainForHwnd);

        MH_EnableHook(MH_ALL_HOOKS);
        pFactory->lpVtbl->Release(pFactory);
        bHooked = TRUE;
    }

    return _RegisterClassExW(pWndClass);
}

BOOL DllMain(HINSTANCE hInstance, DWORD dwReason, PVOID pReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hInstance);
        MH_Initialize();

        MH_CreateHook(RegisterClassExW, &$RegisterClassExW, (PVOID)&_RegisterClassExW);
        MH_EnableHook(MH_ALL_HOOKS);
    }
    return TRUE;
}