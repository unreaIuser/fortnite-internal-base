#include <misc/includes.hpp>
#include <misc/util.h>
#include <misc/xor.h>
#include <misc/lazy.h>
#include <misc/math.h>
#include <misc/memory.h>
#include <misc/offsets.h>
#include <settings.h>
#include <format>
#include <string>
#include <string_view>
#include <vectors/vector_2d.h>
#include "spoofcall.h"

#pragma warning(disable: 4996)

int Screen_X, Screen_Y;
int X, Y;

#define BYTEn(x, n)   (*((_BYTE*)&(x)+n))
#define BYTE1(x)   BYTEn(x,  1)         

#define LOG(LogLevel, Msg, ...) \
Console::Log(LogLevel, std::format(Msg, __VA_ARGS__));

namespace vectors
{
    struct FVector2D
    {
        float X;                                                        // 0x0000(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)
        float Y;                                                        // 0x0004(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)

        inline FVector2D()
            : X(0), Y(0)
        { }

        inline FVector2D(float x, float y)
            : X(x),
            Y(y)
        { }


        FVector2D operator+(const FVector2D& v) {
            return FVector2D{ X + v.X, Y + v.Y };
        }
    };
}

enum LogLevel
{
    Log,
    Loading,
    Warning,
    Error
};

namespace math
{
    inline float custom_sqrtf(float _X)
    {
        return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(_X)));
    }

    inline float custom_sinf(float _X)
    {
        return _mm_cvtss_f32(_mm_sin_ps(_mm_set_ss(_X)));
    }

    inline float custom_cosf(float _X)
    {
        return _mm_cvtss_f32(_mm_cos_ps(_mm_set_ss(_X)));
    }

    inline float custom_acosf(float _X)
    {
        return _mm_cvtss_f32(_mm_acos_ps(_mm_set_ss(_X)));
    }

    inline float custom_tanf(float _X)
    {
        return _mm_cvtss_f32(_mm_tan_ps(_mm_set_ss(_X)));
    }

    inline float custom_atan2f(float _X, float _Y)
    {
        return _mm_cvtss_f32(_mm_atan2_ps(_mm_set_ss(_X), _mm_set_ss(_Y)));
    }

    inline int custom_compare(const char* X, const char* Y)
    {
        while (*X && *Y) {
            if (*X != *Y) {
                return 0;
            }
            X++;
            Y++;
        }

        return (*Y == '\0');
    }

    inline int custom_wcompare(const wchar_t* X, const wchar_t* Y)
    {
        while (*X && *Y) {
            if (*X != *Y) {
                return 0;
            }
            X++;
            Y++;
        }

        return (*Y == L'\0');
    }

    inline const wchar_t* custom_wcsstr(const wchar_t* X, const wchar_t* Y)
    {
        while (*X != L'\0') {
            if ((*X == *Y) && custom_wcompare(X, Y)) {
                return X;
            }
            X++;
        }
        return NULL;
    }

    inline const char* custom_strstr(const char* X, const char* Y)
    {
        while (*X != '\0') {
            if ((*X == *Y) && custom_compare(X, Y)) {
                return X;
            }
            X++;
        }
        return NULL;
    }

    inline int custom_strlen(const char* string)
    {
        int cnt = 0;
        if (string)
        {
            for (; *string != 0; ++string) ++cnt;
        }
        return cnt;
    }

    inline int custom_wcslen(const wchar_t* string)
    {
        int cnt = 0;
        if (string)
        {
            for (; *string != 0; ++string) ++cnt;
        }
        return cnt;
    }
}

class UClass {
public:
    BYTE _padding_0[0x40];
    UClass* SuperClass;
};

class UObject {
public:
    PVOID VTableObject;
    DWORD ObjectFlags;
    DWORD InternalIndex;
    UClass* Class;
    BYTE _padding_0[0x8];
    UObject* Outer;

    inline BOOLEAN IsA(PVOID parentClass) {
        for (auto super = this->Class; super; super = super->SuperClass) {
            if (super == parentClass) {
                return TRUE;
            }
        }

        return FALSE;
    }
};

class FUObjectItem {
public:
    UObject* Object;
    DWORD Flags;
    DWORD ClusterIndex;
    DWORD SerialNumber;
    DWORD SerialNumber2;
};

class TUObjectArray {
public:
    FUObjectItem* Objects[9];
};

class GObjects {
public:
    TUObjectArray* ObjectArray;
    BYTE _padding_0[0xC];
    DWORD ObjectCount;
};

struct FLinearColor
{
    float R;
    float G;
    float B;
    float A;

    FLinearColor()
    {
        R = G = B = A = 0;
    }

    FLinearColor(float R, float G, float B, float A)
    {
        this->R = R;
        this->G = G;
        this->B = B;
        this->A = A;
    }
};

BOOL valid_pointer(DWORD64 address)
{
    if (!IsBadWritePtr((LPVOID)address, (UINT_PTR)8)) return TRUE;
    else return FALSE;
}

static BOOL ProcessEvent(uintptr_t address, void* fnobject, void* parms)
{
    if (!valid_pointer(address))
        return FALSE;

    auto index = *reinterpret_cast<void***>(address);
    if (!index) return FALSE;

    auto fProcessEvent = static_cast<void(*)(void* address, void* fnobject, void* parms)>(index[0x4C]);
    if (!fProcessEvent) return FALSE;

    SpoofCall(fProcessEvent, (void*)address, (void*)fnobject, (void*)parms);
    return TRUE;
}

namespace fort
{
    //static GObjects* objects = nullptr;
    static GObjects* objects = nullptr;

    static void FreeFN(__int64 address)
    {
        auto func = reinterpret_cast<__int64(__fastcall*)(__int64 a1)>(signatures::FreeFN);

        SpoofCall(func, address);
    }

    static bool GetBoneLocation(uintptr_t CurrentActor, int id, Vector3* out)
    {
        uintptr_t mesh = read<uintptr_t>(CurrentActor + offset_actor_mesh);
        if (!mesh) return false;

        auto fGetBoneMatrix = ((Structs::FMatrix * (__fastcall*)(uintptr_t, Structs::FMatrix*, int))(signatures::GetBoneMatrix));
        SpoofCall(fGetBoneMatrix, mesh, Structs::myMatrix, id);

        out->x = Structs::myMatrix->M[3][0];
        out->y = Structs::myMatrix->M[3][1];
        out->z = Structs::myMatrix->M[3][2];

        return true;
    }

    static bool WorldToScreen(Vector3 WorldLocation, Vector3* out)
    {
        auto WorldToScreen = reinterpret_cast<bool(__fastcall*)(uintptr_t pPlayerController, Vector3 vWorldPos, Vector3 * vScreenPosOut, char)>(signatures::WorldToScreen);
        SpoofCall(WorldToScreen, (uintptr_t)offset_player_controller, WorldLocation, out, (char)0);

        return true;
    }

    static const char* GetObjectName(uintptr_t Object)
    {
        if (Object == NULL)
            return ("");

        auto fGetObjName = reinterpret_cast<Structs::FString * (__fastcall*)(int* index, Structs::FString * res)>(signatures::GetNameByIndex);

        int index = *(int*)(Object + 0x18);

        Structs::FString result;
        SpoofCall(fGetObjName, &index, &result);

        if (result.c_str() == NULL)
            return ("");

        auto result_str = result.ToString();

        if (result.c_str() != NULL)
            FreeFN((__int64)result.c_str());

        return result_str.c_str();
    }


    static const char* GetUObjectNameLoop(Structs::UObject* Object)
    {
        std::string name("");

        for (auto i = 0; Object; Object = Object->Outer, ++i) {

            auto fGetObjName = reinterpret_cast<Structs::FString * (__fastcall*)(int* index, Structs::FString * res)>(signatures::GetNameByIndex);

            int index = *(int*)(reinterpret_cast<uint64_t>(Object) + 0x18);

            Structs::FString internalName;
            SpoofCall(fGetObjName, &index, &internalName);

            if (internalName.c_str() == NULL) {
                break;
            }

            auto objectName = internalName.ToString();


            name = objectName.c_str() + std::string(i > 0 ? xorstr(".") : xorstr("")) + name;
            FreeFN((__int64)internalName.c_str());
        }

        return name.c_str();
    }

    static PVOID FindObjectV2(const char* name)
    {
        for (auto array : Structs::objects->ObjectArray->Objects) {
            auto fuObject = array;
            std::cout << "";
            for (auto i = 0; i < 0x10000 && fuObject->Object; ++i, ++fuObject)
            {
                std::cout << "";
                auto object = fuObject->Object;

                if (object->ObjectFlags != 0x41) {
                    continue;
                }
                std::cout << "";

                if (strstr(GetUObjectNameLoop(object), name))
                {
                    return object;
                }
            }
        }

        return 0;
    }
};

namespace Console
{
    static void Log(LogLevel level, std::string Msg)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        switch (level)
        {
        case LogLevel::Warning:
            std::cout << _("[!] ") << Msg << "\n";
            break;
        case LogLevel::Loading:
            std::cout << _("[~] ") << Msg << "\n";
            break;
        case LogLevel::Error:
            std::cout << _("[-] ") << Msg << "\n";
            break;
        case LogLevel::Log:
            std::cout << _("[+] ") << Msg << "\n";
            break;
        default:
            break;
        }


    }
}

typedef HRESULT(__stdcall* Present_t) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(__stdcall* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef BYTE _BYTE;
typedef DWORD _DWORD;
typedef unsigned __int64 _QWORD;

ID3D11RenderTargetView* RenderTarget;
ID3D11DeviceContext* Context;
ID3D11Device* Device;
Present_t OPresent;
WNDPROC oWndProc;
HWND Window = 0;

typedef __int64(__fastcall* thookFunction)(void* address, __int64 fnc, _QWORD* original, int a);
thookFunction hookFunction = nullptr;

int Width;
int Height;

namespace Globals
{
    static bool Open = true;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Listening for INSERT key (will put the variable "Open" as 1/0 (so true / false), so if the variable is valid (having a value equals / superior to 1, our menu will be opened, and if it's equals to 0 or negative, then it will be closed).
    if (uMsg == WM_KEYDOWN && LOWORD(wParam) == VK_INSERT)
        Globals::Open ^= 1;

    // ImGuiIO (Used to define our components position using MousePos from the game window).
    ImGuiIO& io = ImGui::GetIO();
    POINT position;

    GetCursorPos(&position);
    ScreenToClient(Window, &position);
    io.MousePos.x = (float)position.x;
    io.MousePos.y = (float)position.y;

    if (Globals::Open)
    {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
        return true;
    }

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

auto ActorLoop()->void
{
    if (Settings::ActorLoop)
    {
        if (Settings::FovCircle)
        {
            ImGui::GetOverlayDrawList()->AddCircle(ImVec2(X / 2, Y / 2), Settings::fov_value, ImColor(255, 255, 255, 255), 20, 1.0f);
        }

        if (Settings::Crosshair)
        {
            ImGui::GetOverlayDrawList()->AddLine(ImVec2(X / 2 - 7, Y / 2), ImVec2(X / 2 + 1, Y / 2), ImColor(255, 0, 0, 255), 1.0);
            ImGui::GetOverlayDrawList()->AddLine(ImVec2(X / 2 + 8, Y / 2), ImVec2(X / 2 + 1, Y / 2), ImColor(255, 0, 0, 255), 1.0);

            ImGui::GetOverlayDrawList()->AddLine(ImVec2(X / 2, Y / 2 - 7), ImVec2(X / 2 + 1, Y / 2), ImColor(255, 0, 0, 255), 1.0);
            ImGui::GetOverlayDrawList()->AddLine(ImVec2(X / 2, Y / 2 + 8), ImVec2(X / 2 + 1, Y / 2), ImColor(255, 0, 0, 255), 1.0);
        }

        auto UWorld = read<uintptr_t>(signatures::UWorld);
        if (!UWorld) return;

        auto OwningGameInstance = read<uintptr_t>(UWorld + offset_game_instance);
        if (!OwningGameInstance) return;

        auto LocalPlayers_Array = read<uintptr_t>(OwningGameInstance + offset_local_players_array);
        if (!LocalPlayers_Array) return;

        auto PlayerController = read<uintptr_t>(LocalPlayers_Array + offset_player_controller);
        if (!PlayerController) return;

        auto LocalPawn = read<uintptr_t>(PlayerController + offset_apawn);
        if (!LocalPawn) return;

        auto RootComponent = read<uintptr_t>(LocalPawn + offset_root_component);
        if (!RootComponent) return;

        auto PlayerState = read<uintptr_t>(UWorld + offset_persistent_level);
        if (!PlayerState) return;

        auto PlayerCameraManager = read<uintptr_t>(PlayerController + offset_camera_manager);
        if (!PlayerCameraManager) return;

        auto PersistentLevel = read<uintptr_t>(UWorld + offset_persistent_level);
        if (!PersistentLevel) return;

        auto AActors = read<uintptr_t>(PersistentLevel + offset_actor_array);
        if (!AActors) return;

        auto ActorsCount = read<int>(PersistentLevel + offset_actor_count);
        if (!ActorsCount) return;

        for (int i = 0; i < ActorsCount; i++)
        {
            auto CurrentActor = read<uintptr_t>(AActors + i * sizeof(uintptr_t));
            if (!CurrentActor) return;

            auto uinstigator = read<uintptr_t>(CurrentActor + offsets_instigator_actor);
            if (!uinstigator) return;

            auto root = read<uintptr_t>(uinstigator + offset_root_component);
            if (!root) return;

            vec2 screen;

            if (valid_pointer(LocalPawn))
            {
                uintptr_t MyState = read<uintptr_t>(LocalPawn + offset_player_state);
                if (!MyState) continue;

                uintptr_t MyTeamIndex = read<uintptr_t>(MyState + offset_teamindex);
                if (!MyTeamIndex) continue;

                uintptr_t EnemyState = read<uintptr_t>(CurrentActor + offset_player_state);
                if (!EnemyState) continue;

                uintptr_t EnemyTeamIndex = read<uintptr_t>(EnemyState + offset_teamindex);
                if (!EnemyTeamIndex) continue;

                if (CurrentActor == LocalPawn) continue;
            }
        }
    }
}

HRESULT __fastcall hkPresentScene(IDXGISwapChain* pSwapChain, unsigned int SyncInterval, unsigned int Flags)
{

    // Definition of "first use" variable.
    static bool first = false;

    // If it's not the first time we are using / CALLING hkPresentScene then:
    if (!first)
    {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&Device)))
        {
            Device->GetImmediateContext(&Context);
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            Window = sd.OutputWindow;

            ID3D11Texture2D* pBackBuffer;
            D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };

            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            Device->CreateRenderTargetView(pBackBuffer, NULL, &RenderTarget);
            pBackBuffer->Release();
            pBackBuffer->GetDesc(&backBufferDesc);

            Screen_X = backBufferDesc.Width;
            Screen_Y = backBufferDesc.Height;

            oWndProc = (WNDPROC)SetWindowLongPtr(Window, GWLP_WNDPROC, (LONG_PTR)WndProc);

            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();

            // Font can be changed to any existing font on your pc.
            io.Fonts->AddFontFromFileTTF(_("C:\\Windows\\Fonts\\Tahoma.ttf"), 13.0f);

            X = (float)backBufferDesc.Width;
            Y = (float)backBufferDesc.Height;

            ImGui_ImplWin32_Init(Window);
            ImGui_ImplDX11_Init(Device, Context);

            first = true;
        }

        // If !SUCCEEDED then we return to OPresent using our parameters.
        else { return OPresent(pSwapChain, SyncInterval, Flags); }
    }

    // Device context
    if (Device || Context)
    {
        ID3D11Texture2D* renderTargetTexture = nullptr;
        if (!RenderTarget)
        {
            // Basic D3D11 Rendering
            if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&renderTargetTexture))))
            {
                // If SUCCEEDED, creating render target view.
                Device->CreateRenderTargetView(renderTargetTexture, nullptr, &RenderTarget);
                renderTargetTexture->Release();
            }
        }
    }

    // Creating the ImGui Frames
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Initializing our actor_loop.
    ActorLoop();

    // If variable open is true then: (if our menu is open)
    if (Globals::Open)
    {
        // Over-drawing mouse cursor.
        ImGui::GetIO().MouseDrawCursor = 1;
        ImGui::SetWindowSize(ImVec2(230, 300));

        ImGui::Begin(_("fortnite internal"));
        ImGui::Checkbox(_("actorloop"), &Settings::ActorLoop);
        ImGui::Checkbox(_("Fov circle"), &Settings::FovCircle);
        ImGui::Checkbox(_("Snaplins"), &Settings::snaplines);
        ImGui::Checkbox(_("Crosshair"), &Settings::Crosshair);
        ImGui::Checkbox(_("FOV CHANGER"), &Settings::FovChanger);

        ImGui::End();
    }
    else
    {
        ImGui::GetIO().MouseDrawCursor = 0;
    }

    ImGui::EndFrame();
    Context->OMSetRenderTargets(1, &RenderTarget, NULL);
    ImGui::Render();

    if (RenderTarget)
    {
        RenderTarget->Release();
        RenderTarget = nullptr;
    }

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return OPresent(pSwapChain, SyncInterval, Flags);
}

void hook(__int64 addr, __int64 func, __int64* orig)
{
    // Definition of our Hook_Adress variable.
    static uintptr_t Hook_Adress;

    // If Hook_Adress is invalid, then we sigscan
    if (!Hook_Adress) { Hook_Adress = sig_scan(_("GameOverlayRenderer64.dll"), _("48 ? ? ? ? 57 48 83 EC 30 33 C0")); }

    // Hook definition
    auto hook = ((__int64(__fastcall*)(__int64 addr, __int64 func, __int64* orig, __int64 smthng))(Hook_Adress));
    hook((__int64)addr, (__int64)func, orig, (__int64)1);
}

auto init()->void
{
    // Allocating a console.
    SAFE_CALL(AllocConsole)();

    // Opening stdout (for output).
    freopen("CONOUT$", "w", stdout);

    // Calling GameOverlayRenderer64 to check if it's loaded or not.
    if (!SAFE_CALL(GetModuleHandleA)(_("GameOverlayRenderer64.dll")))
    {
        LOG(Error, "GameOverlayRenderer64.dll not loaded.");
        return;
    }

    // Loading Fortnite signatures.
    Load_Signatures();
    LOG(Log, "Signatures loaded");

    // Loading Objects
    LOG(Loading, "Loading objects...");

    functions::FOV = SpoofCall(fort::FindObjectV2, (const char*)xorstr("FOV"));
    if (!functions::FOV) { LOG(Error, "FOV not found."); for (;;); }
    std::cout << "[+] FOV: " << functions::FOV << std::endl;

    functions::K2_DrawLine = (Structs::UObject*)find::FindObject(_(L"Engine.Canvas.K2_DrawLine"));
    if (!functions::K2_DrawLine) { LOG(Error, "K2_DrawLine not found."); for (;;); }
    std::cout << "[+] K2_DrawLine: " << (Structs::UObject*)functions::K2_DrawLine << std::endl;

    functions::K2_DrawBox = (Structs::UObject*)find::FindObject(_(L"Engine.Canvas.K2_DrawBox"));
    if (!functions::K2_DrawBox) { LOG(Error, "K2_DrawBox not found."); for (;;); }
    std::cout << "[+] K2_DrawBox: " << (Structs::UObject*)functions::K2_DrawBox << std::endl;

    functions::K2_DrawText = (Structs::UObject*)find::FindObject(_(L"Engine.Canvas.K2_DrawText"));
    if (!functions::K2_DrawText) { LOG(Error, "K2_DrawText not found."); for (;;); }
    std::cout << "[+] K2_DrawText: " << (Structs::UObject*)functions::K2_DrawText << std::endl;

    functions::K2_DrawTextSize = (Structs::UObject*)find::FindObject(_(L"Engine.Canvas.K2_TextSize"));
    if (!functions::K2_DrawTextSize) { LOG(Error, "K2_DrawTextSize not found."); for (;;); }
    std::cout << "[+] K2_DrawTextSize: " << (Structs::UObject*)functions::K2_DrawTextSize << std::endl;

    functions::Font = (Structs::UObject*)find::FindObject(_(L"/Engine/EngineFonts/Roboto.Roboto"));
    if (!functions::Font) { LOG(Error, "Roboto not found."); for (;;); }
    std::cout << "[+] Roboto: " << (Structs::UObject*)functions::Font << std::endl;

    // Sigscanning Steam DXGI PResentScene.
    uintptr_t Steam_DXGI_PresentScene = sig_scan(_("GameOverlayRenderer64.dll"), _("48 89 6C 24 18 48 89 74 24 20 41 56 48 83 EC 20 41 8B E8"));

    // If Steam_DXGI_PresentScene is valid, then we can hook our hkPresentScene.
    if (Steam_DXGI_PresentScene)
    {
        hook(Steam_DXGI_PresentScene, (__int64)hkPresentScene, (__int64*)&OPresent);
        LOG(Log, "Hooked hkPresentScene");
        //printf("[+] hkPresentScene: %p", hkPresentScene);
    }

    // Else if sigscan failed then we display an error message.
    else { LOG(Error, "Steam_DXGI_PresentScene is invalid."); }
}

bool __stdcall DllMain(HMODULE dll_module, DWORD conclusion, LPVOID reserved)
{
    // Basic non-used parameters but needed for dll calling.
    UNREFERENCED_PARAMETER(dll_module);
    UNREFERENCED_PARAMETER(reserved);

    if (conclusion == 1)
    {
        // Calling our init function.
        init();
    }

    return TRUE;
}