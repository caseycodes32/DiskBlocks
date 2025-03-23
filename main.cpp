#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <GL/GL.h>
#include <tchar.h>

#include "disk_crawler.h"
#include "ui_helper.h"

// Data stored per platform window
struct WGL_WindowData { HDC hDC; };

// Constants
static const POINT      k_WindowSize = {854, 480};

// Data
static HGLRC            g_hRC;
static WGL_WindowData   g_MainWindow;
static int              g_Width;
static int              g_Height;
static bool             g_MouseDownOnHeader;
static POINT            g_LastMousePos;

// Data
bool                    b_ScanComplete;
char                    c_InputPath[260];
DiskElement             de_FileTree;
DiskElement             de_SelectionTree;
std::vector<std::string> v_Drives;
int                     i_DriveIdx;
std::string             s_SelectedPath;
bool                    b_SelectionError;

// Forward declarations of helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data);
void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data);
void ResetDeviceWGL();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_OWNDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"DiskBlocks", nullptr };
    ::RegisterClassExW(&wc);

    RECT RectWindow;
	GetWindowRect(GetDesktopWindow(), &RectWindow);
	int WindowStartX = RectWindow.right / 2 - (k_WindowSize.x / 2);
	int WindowStartY = RectWindow.bottom / 2 - (k_WindowSize.y / 2);

    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"DiskBlocks", WS_POPUP, WindowStartX, WindowStartY, k_WindowSize.x, k_WindowSize.y, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize OpenGL
    if (!CreateDeviceWGL(hwnd, &g_MainWindow))
    {
        CleanupDeviceWGL(hwnd, &g_MainWindow);
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    wglMakeCurrent(g_MainWindow.hDC, g_hRC);

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
    io.IniFilename = NULL;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Modify ImGui Style Defaults
    ImGuiStyle& style = ImGui::GetStyle();
    style.TabRounding = 8.f;
    style.FrameRounding = 8.f;
    style.GrabRounding = 8.f;
    //style.WindowRounding = 8.f;
    style.PopupRounding = 8.f;
    style.ChildRounding = 8.f;

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_InitForOpenGL(hwnd);
    ImGui_ImplOpenGL3_Init();

    LONG lStyle = GetWindowLong( hwnd, GWL_STYLE );
    SetWindowLong( hwnd, GWL_STYLE, lStyle & 
        (~(WS_CAPTION|WS_THICKFRAME|WS_MINIMIZEBOX|WS_SYSMENU ) ) );

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;


    // Initialization code
    std::thread tTree = InitializePopulateTreeThread(de_FileTree, "C:", b_ScanComplete);
    v_Drives = ListDrives();
    i_DriveIdx = -1;

    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
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
        if (::IsIconic(hwnd))
        {
            ::Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        DWORD dwFlag = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
        static bool open = true;

		if (!open)
			ExitProcess(0);


        ImGui::SetNextWindowSize(ImVec2(k_WindowSize.x, k_WindowSize.y));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("DiskBlocks", &open, dwFlag); 
        
        ImGui::BeginChild("Selection Pane", ImVec2(k_WindowSize.x / 2 - 12, k_WindowSize.y - 36), ImGuiChildFlags_Borders);
            ImGui::Text("Enter a path or choose a drive:");
            ImGui::PushItemWidth(200);
            ImGui::InputTextWithHint("##Path", "C:/Folder/xyz", c_InputPath, 260);
            ImGui::PopItemWidth();
            if (strlen(c_InputPath) != 0)
            {
                ImGui::SameLine();
                if (ImGui::Button("Select"))
                {
                    if (PathIsDirectory(std::string(c_InputPath)))
                    {
                        b_SelectionError = false;
                        s_SelectedPath = std::string(c_InputPath);
                    }
                    else
                        b_SelectionError = true;
                }
                if (b_SelectionError)
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: Invalid Directory");
            }
            for (int i = 0; i < v_Drives.size(); i++)
            {
                if (ImGui::Button(v_Drives.at(i).c_str(), ImVec2(30, 30)))
                {
                    i_DriveIdx = i;
                    de_SelectionTree = DiskElement();
                    de_SelectionTree.name = v_Drives.at(i) + ":";
                }
                if (i+1 != v_Drives.size()) ImGui::SameLine();
            }
            if (i_DriveIdx != -1)
            {
                ImGui::BeginChild("Directory Selection Tree", ImVec2(-1, 140));
                    UIDirectoryTree(de_SelectionTree, s_SelectedPath);
                ImGui::EndChild();
            }
            ImGui::Separator();
            ImGui::Text("Selected dir: %s", s_SelectedPath.c_str());
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("Visualization Pane", ImVec2(k_WindowSize.x / 2 - 12, k_WindowSize.y - 36), ImGuiChildFlags_Borders);
        ImGui::EndChild();

        ImGui::End();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, g_Width, g_Height);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Present
        ::SwapBuffers(g_MainWindow.hDC);
    }

    //join thread
    tTree.join();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceWGL(hwnd, &g_MainWindow);
    wglDeleteContext(g_hRC);
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    HDC hDc = ::GetDC(hWnd);
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    const int pf = ::ChoosePixelFormat(hDc, &pfd);
    if (pf == 0)
        return false;
    if (::SetPixelFormat(hDc, pf, &pfd) == FALSE)
        return false;
    ::ReleaseDC(hWnd, hDc);

    data->hDC = ::GetDC(hWnd);
    if (!g_hRC)
        g_hRC = wglCreateContext(data->hDC);
    return true;
}

void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    wglMakeCurrent(nullptr, nullptr);
    ::ReleaseDC(hWnd, data->hDC);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool mousedown = false;
POINT lastLocation;
POINT currentpos;

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
        if (wParam != SIZE_MINIMIZED)
        {
            g_Width = LOWORD(lParam);
            g_Height = HIWORD(lParam);
        }
        return 0;
    case WM_LBUTTONDOWN:
    {
        GetCursorPos(&g_LastMousePos);
        RECT RectWindow;
        GetWindowRect(hWnd, &RectWindow);
        if (g_LastMousePos.y - RectWindow.top < 16)
        {
            g_MouseDownOnHeader = true;
        }
        break;
    }
    case WM_LBUTTONUP:
    {
        g_MouseDownOnHeader = false;
        break;
    }
    case WM_MOUSEMOVE:
    {
        if (g_MouseDownOnHeader)
        {
            POINT CurrentMousePoint;
            GetCursorPos(&CurrentMousePoint);
            int xDiff = CurrentMousePoint.x - g_LastMousePos.x;
            int yDiff = CurrentMousePoint.y - g_LastMousePos.y;
            
            RECT RectWindow;
            GetWindowRect(hWnd, &RectWindow);

            SetWindowPos(hWnd, nullptr, RectWindow.left + xDiff, RectWindow.top + yDiff, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

            g_LastMousePos = CurrentMousePoint;
        }
        break;
    }
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
