#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <thread>
#include <Windows.h>
#include <string>
#include <sstream>
#include <chrono>
#include "mem/memify.h"

float screenWidth = 1440;
float screenHeight = 1080;

#include "math.h"
#include "includes.h"
#include "utils.h"

bool autorevolverRun = true;
bool jumpRun = true;
bool mainRun = true;

BOOL IsProcessRunning(DWORD pid)
{
    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
    DWORD ret = WaitForSingleObject(process, 0);
    CloseHandle(process);
    return ret == WAIT_TIMEOUT;
}

class PlayerPawn
{
public:
    char pad_0000[9192]; //0x0000
    bool IsScoped; //0x23E8
    char pad_23E9[1111]; //0x23E9
};

int main()
{
    SetConsoleTitleA("aimbot");
    std::cout << "Welcome back, " << std::getenv("USERNAME") << "!\n";

    if (dll != NULL)
        mm = (MouseMove)GetProcAddress(dll, "mouse_event");

    hwnd = FindWindowA(0, "Counter-Strike 2");

    while (!client) { std::this_thread::sleep_for(std::chrono::milliseconds(15)); client = mem.GetBase("client.dll"); }
    while (!engine) { std::this_thread::sleep_for(std::chrono::milliseconds(15)); engine = mem.GetBase("engine2.dll"); }

    int PID = 0;
    GetWindowThreadProcessId(hwnd, (LPDWORD)&PID);

    screenWidth = mem.Read<int>(engine + offset::dwWindowWidth);
    screenHeight = mem.Read<int>(engine + offset::dwWindowHeight);
    std::cout << "Pid: " << PID << std::endl;
    std::cout << "Your game resolution " << screenWidth << "x" << screenHeight << std::endl;

    while (mainRun)
    {
        AccurateSleep(1);

        auto localPawn = mem.Read<uintptr_t>(client + offset::dwLocalPlayerPawn);
        PlayerPawn ent;
        mem.ReadRaw(localPawn, &ent, sizeof(PlayerPawn));

        auto view_matrix = mem.Read<view_matrix_t>(client + offset::dwViewMatrix);
        if (!localPawn) continue;

        std::vector<Vector> playerPositions, playerOrigins;
        playerPositions.reserve(32);
        playerOrigins.reserve(32);
        std::vector<int> validTargets;

        int localTeam = mem.Read<int>(localPawn + offset::m_iTeamNum);

        for (int i = 0; i <= 12; ++i)
        {
            uintptr_t list_entry1 = mem.Read<uintptr_t>(entityList + ((8 * (i & 0x7FFF) >> 9) + 16));
            uintptr_t playerController = mem.Read<uintptr_t>(list_entry1 + (120 * (i & 0x1FF)));
            uint32_t playerPawn = mem.Read<uint32_t>(playerController + offset::m_hPlayerPawn);
            uintptr_t list_entry2 = mem.Read<uintptr_t>(entityList + 0x8 * ((playerPawn & 0x7FFF) >> 9) + 16);
            uintptr_t pCSPlayerPawnPtr = mem.Read<uintptr_t>(list_entry2 + (120 * (playerPawn & 0x1FF)));

            int health = mem.Read<int>(pCSPlayerPawnPtr + offset::m_iHealth);
            if (health <= 0)
            {
				lockedTargetIndex = -1;
                continue;
            }

            if (mem.Read<int>(pCSPlayerPawnPtr + offset::m_iTeamNum) == localTeam)
                continue;

            Vector playerPosition = mem.Read<Vector>(pCSPlayerPawnPtr + offset::m_vOldOrigin);
            uintptr_t gamescene = mem.Read<uintptr_t>(pCSPlayerPawnPtr + offset::m_pGameSceneNode);
            uintptr_t bonearray = mem.Read<uintptr_t>(gamescene + offset::m_modelState + 0x80);
            Vector headPos = mem.Read<Vector>(bonearray + (6 * 32));

            Vector f, h;
            if (Vector::world_to_screen(view_matrix, playerPosition, f) && Vector::world_to_screen(view_matrix, headPos, h))
            {
                if (IsWithinFOV(h, FOV))
                {
                    playerOrigins.push_back(playerPosition);
                    playerPositions.push_back(h);
                    validTargets.push_back(i);
                }
            }
        }

        bool keyHeld = GetAsyncKeyState(VK_MENU);

        if (!keyHeld) {
            lockedTargetIndex = -1;
            previousTargetPosition = { 0.0f, 0.0f, 0.0f };
            wasKeyHeldLastFrame = false;
        }

        if (keyHeld)
        {
            if (lockedTargetIndex == -1 && !validTargets.empty())
            {
                float closestDist = FLT_MAX;
                int bestIndex = -1;
                Vector screenCenter{ screenWidth / 2, screenHeight / 2, 0.0f };

                for (int i = 0; i < validTargets.size(); ++i)
                {
                    float screenDist = (playerPositions[i] - screenCenter).LengthSqr();
                    if (screenDist < closestDist)
                    {
                        closestDist = screenDist;
                        bestIndex = validTargets[i];
                    }
                }

                if (bestIndex != -1)
                    lockedTargetIndex = bestIndex;
            }

            if (lockedTargetIndex != -1)
            {
                auto it = std::find(validTargets.begin(), validTargets.end(), lockedTargetIndex);
                if (it != validTargets.end())
                {
                    int index = std::distance(validTargets.begin(), it);
                    Vector targetScreenPos = playerPositions[index];

                    if (!previousTargetPosition.IsZero())
                    {
                        float positionChange = (targetScreenPos - previousTargetPosition).Length();
                        if (positionChange > maxAllowedFlickDistance)
                        {
                            continue;
                        }
                    }

                    if (IsWithinFOV(targetScreenPos, FOV))
                    {
                        MoveMouseToPlayer(targetScreenPos);
                        previousTargetPosition = targetScreenPos;
                    }
                }
            }
        }

        if (GetAsyncKeyState(VK_END) || !IsProcessRunning(PID))
            mainRun = false;

    }

    std::cout << "Goodbye...\n";
	Sleep(1000);

    return 0;
}
