#pragma once
struct ViewAngles {
    float x, y;
};

struct BonePosition {
    Vector worldPosition;
    Vector screenPosition;
    int playerID;
};

enum bones : int {
    head = 6,
    neck = 5,
    spine = 4,
    spine_1 = 2,
    left_shoulder = 8,
    left_arm = 9,
    left_hand = 11,
    cock = 0,
    right_shoulder = 13,
    right_arm = 14,
    right_hand = 16,
    left_hip = 22,
    left_knee = 23,
    left_feet = 24,
    right_hip = 25,
    right_knee = 26,
    right_feet = 27
};

std::vector<BonePosition> enemyBones;
ViewAngles lastViewAngles = { 0.0f, 0.0f };
bool detectedAimbotSnap = false;
const float snapThreshold = 2.5f;
const float gradualThreshold = 0.2f;
const int frameHistorySize = 8;
std::vector<ViewAngles> angleHistory;

inline void AccurateSleep(int ms) {
    LONGLONG timerResolution;
    LONGLONG wantedTime;
    LONGLONG currentTime;
    QueryPerformanceFrequency((LARGE_INTEGER*)&timerResolution);
    timerResolution /= 1000;
    QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
    wantedTime = currentTime / timerResolution + ms;
    currentTime = 0;
    while (currentTime < wantedTime) {
        QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
        currentTime /= timerResolution;
    }
}

inline bool IsWithinFOV(const Vector& screenPos, float maxFOV)
{
    Vector screenCenter = { (float)GetSystemMetrics(SM_CXSCREEN) / 2, (float)GetSystemMetrics(SM_CYSCREEN) / 2, 0.0f };
    return (screenPos - screenCenter).Length() <= maxFOV + 5.f;
}

inline Vector findClosest(const std::vector<Vector>& playerPositions, const std::vector<Vector>& playersOrigins)
{
    if (playerPositions.empty())
        return { 0.0f, 0.0f, 0.0f };

    Vector center_of_screen = { (float)GetSystemMetrics(SM_CXSCREEN) / 2, (float)GetSystemMetrics(SM_CYSCREEN) / 2, 0.0f };
    float closestScreenDist = FLT_MAX, closestWorldDist = FLT_MAX;
    int bestIndex = -1;

    Vector localpos = mem.Read<Vector>(client + offset::dwLocalPlayerPawn + offset::m_vOldOrigin);

    for (size_t i = 0; i < playerPositions.size(); ++i)
    {
        if (!IsWithinFOV(playerPositions[i], FOV))
            continue;

        float distanceScreen = (playerPositions[i] - center_of_screen).LengthSqr();
        float distance3D = localpos.Distance(playersOrigins[i]);

        if (distanceScreen < closestScreenDist || (distanceScreen == closestScreenDist && distance3D < closestWorldDist))
        {
            closestScreenDist = distanceScreen;
            closestWorldDist = distance3D;
            bestIndex = i;
        }
    }

    return (bestIndex != -1) ? playerPositions[bestIndex] : Vector{ 0.0f, 0.0f, 0.0f };
}


inline void MoveMouseToPlayer(Vector position)
{
    if (position.IsZero())
        return;

    Vector center_of_screen = { (float)GetSystemMetrics(SM_CXSCREEN) / 2, (float)GetSystemMetrics(SM_CYSCREEN) / 2, 0.0f };
    AccurateSleep(1);
    mm(1, static_cast<int>(position.x - center_of_screen.x), static_cast<int>(position.y - center_of_screen.y), 0, 0);
    AccurateSleep(1);
}

void HoldMouse1(bool press) {
    if (press) {
        mm(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        // mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    }
    else {
        mm(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
        //mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    }
}

void UpdateEnemyBones() {
    enemyBones.clear();
    auto entityList = mem.Read<uintptr_t>(client + offset::dwEntityList);
    if (!entityList)
        return;

    auto localPawn = mem.Read<uintptr_t>(client + offset::dwLocalPlayerPawn);
    int localTeam = mem.Read<int>(localPawn + offset::m_iTeamNum);
    view_matrix_t viewMatrix = mem.Read<view_matrix_t>(client + offset::dwViewMatrix);

    for (int i = 0; i <= 12; ++i) {
        uintptr_t list_entry1 = mem.Read<uintptr_t>(entityList + ((8 * (i & 0x7FFF) >> 9) + 16));
        uintptr_t playerController = mem.Read<uintptr_t>(list_entry1 + (120 * (i & 0x1FF)));
        uint32_t playerPawn = mem.Read<uint32_t>(playerController + offset::m_hPlayerPawn);
        uintptr_t list_entry2 = mem.Read<uintptr_t>(entityList + 0x8 * ((playerPawn & 0x7FFF) >> 9) + 16);
        uintptr_t pCSPlayerPawnPtr = mem.Read<uintptr_t>(list_entry2 + (120 * (playerPawn & 0x1FF)));

        if (mem.Read<int>(pCSPlayerPawnPtr + offset::m_iHealth) <= 0 ||
            mem.Read<int>(pCSPlayerPawnPtr + offset::m_iTeamNum) == localTeam)
            continue;

        uintptr_t gamescene = mem.Read<uintptr_t>(pCSPlayerPawnPtr + offset::m_pGameSceneNode);
        uintptr_t bonearray = mem.Read<uintptr_t>(gamescene + offset::m_modelState + 0x80);

        for (int boneIndex : { head, neck, spine, spine_1, left_shoulder, left_arm, left_hand, right_shoulder, right_arm, right_hand, left_hip, left_knee, left_feet, right_hip, right_knee, right_feet }) {
            Vector bonePos = mem.Read<Vector>(bonearray + (boneIndex * 32));
            Vector screenPos;
            if (Vector::world_to_screen(viewMatrix, bonePos, screenPos)) {
                enemyBones.push_back({ bonePos, screenPos, i });
            }
        }
    }
}

bool IsAimbotLocking() {
    auto localPawn = mem.Read<uintptr_t>(client + offset::dwLocalPlayerPawn);
    ViewAngles currentViewAngles = mem.Read<ViewAngles>(client + offset::dwViewAngles);

    if (angleHistory.size() >= frameHistorySize) {
        angleHistory.erase(angleHistory.begin());
    }
    angleHistory.push_back(currentViewAngles);

    int screenCenterX = GetSystemMetrics(SM_CXSCREEN) / 2;
    int screenCenterY = GetSystemMetrics(SM_CYSCREEN) / 2;

    for (const auto& bone : enemyBones) {
        if ((abs(bone.screenPosition.x - screenCenterX) <= 5) &&
            (abs(bone.screenPosition.y - screenCenterY) <= 5)) {
            detectedAimbotSnap = true;
            return true;
        }
    }
    detectedAimbotSnap = false;
    return false;
}
