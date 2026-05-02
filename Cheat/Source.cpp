bool exitLoop = false;
bool isMenuVisible = true;
bool sizeChanged = false;
int maxPlayerCount = 500;
int loopDelay = 10;

#include <mjLib/mjLib.hpp>
#include <DirectX/D3D11.h>
#include <DirectX/D3DX11.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_impl_dx11.h>
#include <ImGui/imgui_internal.h>
#include "DrawImGui.hpp"

#include "Game.hpp"
#include "Memory.hpp"
#include "Menu.hpp"
#include "SimpleMath.h"
#include "Cheat.hpp"
#include "Overlay.hpp"
#include <windows.h>
#include <chrono>
#include <iostream>
#include <string>

void AllocateConsole() {
    AllocConsole();
    FILE* pCout;
    FILE* pCin;
    FILE* pCerr;
    freopen_s(&pCout, "CONOUT$", "w", stdout);
    freopen_s(&pCin, "CONIN$", "r", stdin);
    freopen_s(&pCerr, "CONOUT$", "w", stderr);
    SetConsoleTitleA("Mynx Loader");
    HWND hwnd = GetConsoleWindow();
    if (hwnd != NULL) {
        ShowWindow(hwnd, SW_SHOW);
    }
}

bool AuthenticateUser() {
    AllocateConsole();

    std::cout << "========================================\n";
    std::cout << "     Mynx Loader\n";
    std::cout << "========================================\n\n";
    std::cout << "Type 'admin' to start the application\n\n";

    std::string input;

    while (true) {
        std::cout << "Enter key: ";
        std::getline(std::cin, input);

        if (input == "admin") {
            std::cout << "Starting application...\n\n";
            Sleep(1000);
            HWND hwnd = GetConsoleWindow();
            if (hwnd != NULL) {
                ShowWindow(hwnd, SW_HIDE);
            }
            return true;
        }
        else {
            std::cout << "Invalid key! Type 'admin' to continue.\n\n";
        }
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    if (!AuthenticateUser()) {
        return -1;
    }

    try {
        std::thread([&]() {
            while (!Game.pID) {
                Game.pID = FindGame();
                Sleep(1000);
            }

            Sleep(5000);
            Game.hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Game.pID);
            if (!Game.hProcess) {
                return;
            }

            while (!Game.hWnd) {
                Game.hWnd = FindWindow(Game.lpClassName, NULL);
                Offsets.GameBase = GetBaseAddress();
                Offsets.CitizenPlayernamesBase = GetBaseAddress(XorString("citizen-playernames-five.dll"));
                if (Offsets.CitizenPlayernamesBase == 0) {
                    Offsets.CitizenPlayernamesBase = GetBaseAddress(XorString("citizen-resources.dll"));
                    if (Offsets.CitizenPlayernamesBase == 0) {
                        Offsets.CitizenPlayernamesBase = GetBaseAddress(XorString("citizen-server.dll"));
                    }
                }
                Sleep(500);
            }

            ReadOffsets();
            std::thread([&]() { UpdateOverlay(); }).detach();
            std::thread([&]() { UpdatePeds(); }).detach();
            std::thread([=]() { SetAim(); }).detach();
            std::thread([=]() { AddPlayerList(); }).detach();
            std::thread([=]() { AddVehicleList(); }).detach();
            std::thread([=]() { Exploits(); }).detach();
            std::thread([=]() { StrafeMacro(); }).detach();
            }).detach();

        while (!exitLoop) {
            if (CreateOverlay()) {
                OverlayMain();
            }
            Sleep(100);
        }
    }
    catch (const std::exception& e) {
        return -1;
    }
    catch (...) {
        return -1;
    }
}