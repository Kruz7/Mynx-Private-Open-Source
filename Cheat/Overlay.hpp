struct OverlayStruct {
    HWND hWnd;
    WNDCLASSEX wndClassEx;
    LPCSTR lpClassName = "Steam";
    LPCSTR lpWindowName = "Steam";
    ID3D11Device* pDevice;
    ID3D11DeviceContext* pDeviceContext;
    IDXGISwapChain* pSwapChain;
    ID3D11RenderTargetView* pRenderTargetView;
    POINT WindowSize;
} Overlay;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void DestroyOverlay();
void RenderWatermark(); 

void OverlayMain() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    InitializeMenu(Overlay.pDevice);
    ImGui_ImplWin32_Init(Overlay.hWnd);
    ImGui_ImplDX11_Init(Overlay.pDevice, Overlay.pDeviceContext);

    while (!exitLoop) {
        if (sizeChanged) {
            sizeChanged = false;
            break;
        }

        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (GetAsyncKeyState(Cheats::MenuUtils::MenuKey) & 1) {
            isMenuVisible = !isMenuVisible;
            ImGui::GetIO().MouseDrawCursor = isMenuVisible;
            SetWindowLong(Overlay.hWnd, GWL_EXSTYLE, isMenuVisible ? WS_EX_TOOLWINDOW : (WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW));
            UpdateWindow(Overlay.hWnd);
        }

        if (GetAsyncKeyState(Cheats::NoClip::Key) & 1) {
            Cheats::NoClip::Enabled = !Cheats::NoClip::Enabled;
        }

        if (isMenuVisible) {
            if (Menu.MenuAlpha < 1.0f) {
                Menu.MenuAlpha += Menu.AlphaSpeed;
                if (Menu.MenuAlpha > 1.0f) {
                    Menu.MenuAlpha = 1.0f;
                }
            }
        }
        else {
            if (Menu.MenuAlpha > 0.0f) {
                Menu.MenuAlpha -= Menu.AlphaSpeed;
                if (Menu.MenuAlpha < 0.0f) {
                    Menu.MenuAlpha = 0.0f;
                }
            }
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
            Draw::Always();
            Draw::Esp();
            Draw::Vehicle();
            RenderKeybindOverlay();
            RenderDamageLog();
            RenderNotifications();
            RenderWatermark();
            if (isMenuVisible) {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, Menu.MenuAlpha);
                DrawMenu();
                ImGui::PopStyleVar();
            }
        }
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.f, 0.f, 0.f, 0.f };
        Overlay.pDeviceContext->OMSetRenderTargets(1, &Overlay.pRenderTargetView, NULL);
        Overlay.pDeviceContext->ClearRenderTargetView(Overlay.pRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        Overlay.pSwapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    DestroyOverlay();
}

void UpdateOverlay() {
    while (!exitLoop) {
        Sleep(3000);
        if (!mjLib::Process::Check(Game.pID)) {
            exit(0);
            break;
        }

        Game.hWnd = FindWindow(Game.lpClassName, NULL);
        if (!Game.hWnd) {
            exit(0);
            break;
        }

        if (!Overlay.hWnd || sizeChanged) {
            continue;
        }

        RECT currentRect{};
        POINT currentPoint{};
        GetClientRect(Game.hWnd, &currentRect);
        ClientToScreen(Game.hWnd, &currentPoint);

        if (Overlay.WindowSize.x != Game.lpRect.right || Overlay.WindowSize.y != Game.lpRect.bottom) {
            Overlay.WindowSize.x = Game.lpRect.right;
            Overlay.WindowSize.y = Game.lpRect.bottom;
            sizeChanged = true;
        }

        if (currentRect.left != Game.lpRect.left || currentRect.right != Game.lpRect.right || currentRect.top != Game.lpRect.top || currentRect.bottom != Game.lpRect.bottom || currentPoint.x != Game.lpPoint.x || currentPoint.y != Game.lpPoint.y) {
            Game.lpRect = currentRect;
            Game.lpPoint = currentPoint;
            SetWindowPos(Overlay.hWnd, nullptr, currentPoint.x, currentPoint.y, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, SWP_NOREDRAW);
            SetWindowDisplayAffinity(Overlay.hWnd, WDA_EXCLUDEFROMCAPTURE);
        }

        Sleep(loopDelay);
    }
}

bool CreateOverlay() {
    WNDCLASSEX wndClassEx;
    ZeroMemory(&wndClassEx, sizeof(wndClassEx));
    wndClassEx.cbSize = sizeof(WNDCLASSEX);
    wndClassEx.lpfnWndProc = WndProc;
    wndClassEx.hInstance = GetModuleHandle(NULL);
    wndClassEx.lpszClassName = Overlay.lpClassName;
    wndClassEx.lpszMenuName = Overlay.lpWindowName;
    wndClassEx.hCursor = NULL;
    wndClassEx.hbrBackground = NULL;
    wndClassEx.hIcon = NULL;
    wndClassEx.hIconSm = NULL;
    wndClassEx.cbClsExtra = 0;
    wndClassEx.cbWndExtra = 0;
    wndClassEx.style = 0;
    if (!RegisterClassExA(&wndClassEx)) {
        mjLib::Logger::WriteLog(XorString("Window class registration failed!"), mjLib::Logger::LogLevel::LOG_ERROR);
        return false;
    }

    HWND targetHwnd = Game.hWnd ? Game.hWnd : GetDesktopWindow();
    GetClientRect(targetHwnd, &Game.lpRect);
    ClientToScreen(targetHwnd, &Game.lpPoint);
    Overlay.WindowSize.x = Game.lpRect.right;
    Overlay.WindowSize.y = Game.lpRect.bottom;
    Overlay.hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST, Overlay.lpClassName, Overlay.lpWindowName, WS_POPUP | WS_VISIBLE, Game.lpPoint.x, Game.lpPoint.y, Game.lpRect.right - Game.lpRect.left, Game.lpRect.bottom - Game.lpRect.top, NULL, NULL, wndClassEx.hInstance, NULL);
    if (!Overlay.hWnd) {
        mjLib::Logger::WriteLog(XorString("Window creation failed!"), mjLib::Logger::LogLevel::LOG_ERROR);
        return false;
    }

    if (!SetLayeredWindowAttributes(Overlay.hWnd, RGB(0, 0, 0), 255, LWA_ALPHA)) {
        mjLib::Logger::WriteLog(XorString("Layered window attributes setup failed!"), mjLib::Logger::LogLevel::LOG_ERROR);
        return false;
    }

    MARGINS margin = { -1 };
    if (DwmExtendFrameIntoClientArea(Overlay.hWnd, &margin) != S_OK) {
        mjLib::Logger::WriteLog(XorString("Failed to extend frame into client area!"), mjLib::Logger::LogLevel::LOG_ERROR);
        return false;
    }

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = 0;
    swapChainDesc.BufferDesc.Height = 0;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = Overlay.hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hResult = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevelArray, 2, D3D11_SDK_VERSION, &swapChainDesc, &Overlay.pSwapChain, &Overlay.pDevice, &featureLevel, &Overlay.pDeviceContext);
    if (FAILED(hResult)) {
        mjLib::Logger::WriteLog(XorString("Direct3D device and swap chain creation failed!"), mjLib::Logger::LogLevel::LOG_ERROR);
        return false;
    }

    ID3D11Texture2D* pBackBuffer;
    hResult = Overlay.pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (FAILED(hResult)) {
        mjLib::Logger::WriteLog(XorString("Failed to retrieve back buffer!"), mjLib::Logger::LogLevel::LOG_ERROR);
        return false;
    }

    hResult = Overlay.pDevice->CreateRenderTargetView(pBackBuffer, NULL, &Overlay.pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hResult)) {
        mjLib::Logger::WriteLog(XorString("Render target view creation failed!"), mjLib::Logger::LogLevel::LOG_ERROR);
        return false;
    }

    SetWindowLong(Overlay.hWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST);
    SetWindowDisplayAffinity(Overlay.hWnd, WDA_EXCLUDEFROMCAPTURE);
    ShowWindow(Overlay.hWnd, SW_SHOWDEFAULT);
    UpdateWindow(Overlay.hWnd);
    return true;
}

void DestroyOverlay() {
    if (Overlay.pRenderTargetView) {
        Overlay.pRenderTargetView->Release();
        Overlay.pRenderTargetView = nullptr;
    }

    if (Overlay.pSwapChain) {
        Overlay.pSwapChain->Release();
        Overlay.pSwapChain = nullptr;
    }

    if (Overlay.pDeviceContext) {
        Overlay.pDeviceContext->Release();
        Overlay.pDeviceContext = nullptr;
    }

    if (Overlay.pDevice) {
        Overlay.pDevice->Release();
        Overlay.pDevice = nullptr;
    }

    if (Overlay.hWnd) {
        DestroyWindow(Overlay.hWnd);
        Overlay.hWnd = nullptr;
    }

    if (Overlay.lpClassName) {
        UnregisterClass(Overlay.lpClassName, GetModuleHandle(NULL));
    }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
        return true;
    }

    switch (uMsg) {
    case WM_CLOSE:
    case WM_DESTROY:
        exit(0);
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
