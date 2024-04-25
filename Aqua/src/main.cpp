// Dear ImGui: standalone example application for DirectX 9

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>
#include <D3dx9tex.h>
#include "settings.h"
#include "imgui_internal.h"
#include "blur.hpp"
#include "gui.h"
#include <thread>
#pragma comment(lib, "D3dx9")
#pragma comment(lib,"d3dx9.lib")


//auto window = GetCurrentWindow();
//auto draw = window->DrawList;
//auto pos = window->Pos;
//auto size = window->Size;
//auto style = GetStyle();

//GetBackgroundDrawList()->AddImage(bg, ImVec2(0, 0), io.DisplaySize);

//const auto& pWindowDrawList = ImGui::GetWindowDrawList();
//DrawBackgroundBlur(pWindowDrawList, g_pd3dDevice);

IDirect3DTexture9* pfp{ };

// Data
static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using namespace ImGui;
IDirect3DTexture9* bg{ };

static bool isDownloaded = false;

DWORD no_bg = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground;

void downloadIMG()
{

    ImGuiIO io;

    float deltaTime = 100.0f;

    io.DeltaTime = deltaTime;

    if (!isDownloaded) {

        system("curl -o C:\\logo.png https://files.catbox.moe/w8aizs.png");
        isDownloaded = !isDownloaded;
    }
}

bool LoadTextureFromFile(const char* filename, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height);

void style() 
{

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg] = ImVec4(23.0f / 255, 23.0f / 255, 23.0f / 255, 0.87);
    colors[ImGuiCol_ChildBg] = ImVec4(23.0f / 255, 23.0f / 255, 23.0f / 255, 1.0f);
    colors[ImGuiCol_BorderShadow] = ImVec4( 0.0f, .0f, 0.0f, 1.0f);
    style.WindowRounding = 12.0f;
}

int main(int, char**)
{
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX9 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;

    int my_image_width = 0;
    int my_image_height = 0;
    PDIRECT3DTEXTURE9 my_texture = NULL;
    bool ret = LoadTextureFromFile("Aqua.png", &my_texture, &my_image_width, &my_image_height);
    IM_ASSERT(ret);

    io.Fonts->AddFontFromMemoryTTF(satoshi_font, sizeof(satoshi_font), 15.0f);
    io.Fonts->AddFontFromMemoryTTF(Satoshi_BOLD, sizeof(Satoshi_BOLD), 15.0f);

    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            g_d3dpp.BackBufferWidth = g_ResizeWidth;
            g_d3dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice();
        }

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();


        ImGui::NewFrame();




        style();

        ////////////////////////////////////////////////////////////////
        /**/ ImGui::Begin("blur", NULL, no_bg);                       //  <- Only way to setup blur correctly i found, it bugs if you aply it to
        /**/ ImGuiStyle& style = ImGui::GetStyle();                   //     your main window, the borders are bugged, idk why
        /**/ const auto& pWindowDrawList = ImGui::GetWindowDrawList();//
        /**/ DrawBackgroundBlur(pWindowDrawList, g_pd3dDevice);       //
        /**/ ImGui::End();                                            // 
        ////////////////////////////////////////////////////////////////

        c_gui::gui* gui = new c_gui::gui();

        if (!bg)

            D3DXCreateTextureFromFileExA(g_pd3dDevice, "C:\\wallpaper\\wallpaper.png", 1920, 1080, D3DX_DEFAULT, 0,
                D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &bg);
        if (!pfp)
            D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice, &pfp_raw, sizeof pfp_raw, 30, 30, D3DX_DEFAULT, 0,
                D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pfp);

       GetBackgroundDrawList()->AddImage(bg, ImVec2(0, 0), io.DisplaySize);

        gui->applyStyle();
        gui->begin("test", 350, 400);
        {
            gui->psFont();
            DrawBackgroundBlur(pWindowDrawList, g_pd3dDevice);

            if (!logged) 
            {
                ImVec2 windowSize = ImGui::GetWindowSize();
                auto window = GetCurrentWindow();
                auto size = window->Size;

                gui->particles();
                gui->setpos(80, 10);
                Image((void*)my_texture, ImVec2(200, 200));

                gui->hintInput("", "User", user, 65, 180);
                gui->hintInputPass("", "Pass", pass, 65, 230);

                if (gui->button("Login", 225, 30, 65, windowSize.y - 85))
                {
                    logged = !logged;
                }
            }

            if (logged)
            {
                gui->particles();

                ImVec2 windowSize = ImGui::GetWindowSize();
                auto window = GetCurrentWindow();
                auto size = window->Size;
                auto pos = window->Pos;
                auto draw = window->DrawList;

                gui->sws(500, 350);

                gui->child("userChild", 150, 50, windowSize.x - 200, windowSize.y - 320);
                {
                    ImVec2 childSize = ImGui::GetWindowSize();

                    draw->AddImageRounded(pfp, ImVec2(pos.x + 415, pos.y + 40), ImVec2(pos.x + 415 + img_size.x, pos.y + 40 + img_size.y), ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE, rounding); // <- pfp

                    gui->text("Baikzz # 0", 10, 5); // <- Discord id or whatever
                    gui->text("3!N1G6Aw524!", 10, 25); //<- Token or something, idk
                }
                gui->endChild();

                gui->child("infoChild", 200, 50, windowSize.x - 450, windowSize.y - 320);
                {
                    ImVec2 childSize = ImGui::GetWindowSize();

                    gui->bText("Time left: ", 10, 5);
                    gui->bText("Version: ", 10, 25);

                    gui->text("29d", childSize.x - 40, 5);
                    gui->text("2.3", childSize.x - 40, 25);
                }
                gui->endChild();

                gui->child("mainChild", 400, 200, 50, windowSize.y - 230);
                {
                    ImVec2 childSize = ImGui::GetWindowSize();

                    if (gui->button("SPOOF", 300, 30, 50, childSize.y - 75))
                    {

                    }

                    if (gui->button("CHECK SERIALS", 145, 30, 50, childSize.y - 37.5)) 
                    {

                    } 

                    if (gui->button("TEMP SPOOF", 145, 30, 205, childSize.y - 37.5)) 
                    {

                    }

                    gui->bText("Status", childSize.x - 270, 10);
                    gui->text("UD (Ultra Detected)", childSize.x - 220, 10);
                }
                gui->endChild();
            }
        }
        gui->stpFont();
        gui->end();

        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}


// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void particles() {

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    static const int numParticles = 15;
    static ImVec2 particlePositions[numParticles];
    static ImVec2 particleDistance;
    static ImVec2 particleVelocities[numParticles];

    static bool initialized = false;
    if (!initialized)
    {
        for (int i = 0; i < numParticles; ++i)
        {
            particlePositions[i] = ImVec2(
                ImGui::GetWindowPos().x + ImGui::GetWindowSize().x * static_cast<float>(rand()) / RAND_MAX,
                ImGui::GetWindowPos().y + ImGui::GetWindowSize().y * static_cast<float>(rand()) / RAND_MAX
            );

            particleVelocities[i] = ImVec2(
                static_cast<float>((rand() % 11) - 5),
                static_cast<float>((rand() % 11) - 5)
            );

        }

        initialized = true;
    }

    ImVec2 cursorPos = ImGui::GetIO().MousePos;
    for (int i = 0; i < numParticles; ++i)
    {
        //draw lines to particles
        for (int j = i + 1; j < numParticles; ++j)
        {
            float distance = std::hypotf(particlePositions[j].x - particlePositions[i].x, particlePositions[j].y - particlePositions[i].y);
            float opacity = 1.0f - (distance / 4.0f);  // opacity cahnge

            if (opacity > 0.0f)
            {
                ImU32 lineColor = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, opacity));
                drawList->AddLine(particlePositions[i], particlePositions[j], lineColor);
            }
        }

        //draw lines to cursor
        float distanceToCursor = std::hypotf(cursorPos.x - particlePositions[i].x, cursorPos.y - particlePositions[i].y);
        float opacityToCursor = 1.0f - (distanceToCursor / 37.0f);  // Adjust the divisor to control the opacity change

        if (opacityToCursor > 0.0f)
        {
            ImU32 lineColorToCursor = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, opacityToCursor));
            drawList->AddLine(cursorPos, particlePositions[i], lineColorToCursor);
        }
    }

    //update and render particles
    float deltaTime = ImGui::GetIO().DeltaTime;
    for (int i = 0; i < numParticles; ++i)
    {
        particlePositions[i].x += particleVelocities[i].x * deltaTime;
        particlePositions[i].y += particleVelocities[i].y * deltaTime;

        // Stay in window
        if (particlePositions[i].x < ImGui::GetWindowPos().x)
            particlePositions[i].x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x;
        else if (particlePositions[i].x > ImGui::GetWindowPos().x + ImGui::GetWindowSize().x)
            particlePositions[i].x = ImGui::GetWindowPos().x;

        if (particlePositions[i].y < ImGui::GetWindowPos().y)
            particlePositions[i].y = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
        else if (particlePositions[i].y > ImGui::GetWindowPos().y + ImGui::GetWindowSize().y)
            particlePositions[i].y = ImGui::GetWindowPos().y;

        ImU32 particleColour = ImGui::ColorConvertFloat4ToU32(settings::particleColour);

        //render particles behind components
        drawList->AddCircleFilled(particlePositions[i], 1.5f, particleColour);
    }
}


void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


bool InputWHint(const char* label, const char* hint, char input[20], size_t bufferSize) {
    return ImGui::InputTextWithHint(label, hint, input, bufferSize);
}

bool LoadTextureFromFile(const char* filename, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height)
{
    // Load texture from disk
    PDIRECT3DTEXTURE9 texture;
    HRESULT hr = D3DXCreateTextureFromFileA(g_pd3dDevice, filename, &texture);
    if (hr != S_OK)
        return false;

    // Retrieve description of the texture surface so we can access its size
    D3DSURFACE_DESC my_image_desc;
    texture->GetLevelDesc(0, &my_image_desc);
    *out_texture = texture;
    *out_width = (int)my_image_desc.Width;
    *out_height = (int)my_image_desc.Height;
    return true;
}

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
