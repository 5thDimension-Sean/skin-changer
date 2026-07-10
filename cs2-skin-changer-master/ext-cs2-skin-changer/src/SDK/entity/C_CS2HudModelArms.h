#include "dwEntityListManager.h"
#include "../../../ext/offsets.h"

#pragma once

uintptr_t GetHudArms()
{
    const uintptr_t localPlayer = GetLocalPlayer();
    if (!localPlayer)
        return 0;

    return GetEntityByHandle(mem.Read<uint32_t>(localPlayer + Offsets::m_hHudModelArms));
}

uintptr_t GetHudWeapon(const uintptr_t weapon)
{
    const auto& armsBase = GetHudArms();
    if (!armsBase)
        return 0;

    const auto& armsNode = mem.Read<uintptr_t>(armsBase + Offsets::m_pGameSceneNode);
    if (!armsNode)
        return 0;

    for (uintptr_t viewModel = mem.Read<uintptr_t>(armsNode + Offsets::m_pChild); viewModel; viewModel = mem.Read<uintptr_t>(viewModel + Offsets::m_pNextSibling))
    {
        const uintptr_t owner = mem.Read<uintptr_t>(viewModel + Offsets::m_pOwner);
        if (!owner)
            continue;

        if (GetEntityByHandle(mem.Read<uint32_t>(owner + Offsets::m_hOwnerEntity)) != weapon)
            continue;

        return owner;
    }

    return 0;
}

//uintptr_t GetHudWeapon(const uintptr_t& weapon)
//{
//	uintptr_t hudWeapon = NULL;
//
//	while (!hudWeapon) { hudWeapon = GetEntityByHandle(mem.Read<uint16_t>(weapon + Offsets::m_hViewModelWeapon)); }
//
//	return hudWeapon;
//}

//std::array<uintptr_t, 8> GetAWeapons(uintptr_t LocalPlayer)
//{
//    std::array<uintptr_t, 8> weapons{};
//    int idx = 0;
//
//    const uintptr_t LocalNode = mem->Read<uintptr_t>(LocalPlayer + Offsets::m_pGameSceneNode);
//
//    uintptr_t weapon = mem->Read<uintptr_t>(LocalNode + Offsets::m_pChild);
//    while (weapon && idx < weapons.size())
//    {
//        uintptr_t weaponEntity = mem->Read<uintptr_t>(weapon + Offsets::m_pOwner);
//
//        if (weaponEntity != 0)
//            weapons[idx++] = weaponEntity;
//
//        weapon = mem->Read<uintptr_t>(weapon + Offsets::m_pNextSibling);
//    }
//
//    return weapons;
//}
