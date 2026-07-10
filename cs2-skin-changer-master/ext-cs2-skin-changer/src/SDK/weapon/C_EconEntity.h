#include "../../../ext/offsets.h"
#include "../ext/mem.h"

#include "../../../ext/sigs.h"

#include "../CEconItemAttributeManager.h"
#include "../entity/C_CS2HudModelArms.h"

#include "weapon.h"

#pragma once

//SkinInfo_t GetSkin(const uintptr_t item)
//{
//    const WeaponsEnum weaponType = mem.Read<WeaponsEnum>(item + Offsets::m_iItemDefinitionIndex);
//
//    switch (weaponType)
//    {
//    case WeaponsEnum::UspS:
//        return SkinInfo_t(653, true);
//    case WeaponsEnum::Awp:
//        return SkinInfo_t(344, true);
//    case WeaponsEnum::Scar20:
//        return SkinInfo_t(312, true);
//    }
//
//    return SkinInfo_t{ 0, false, std::string(), WeaponsEnum::none };
//}

SkinInfo_t GetSkin(const uintptr_t item)
{
    const WeaponsEnum weaponType = mem.Read<WeaponsEnum>(item + Offsets::m_iItemDefinitionIndex);

    return skinManager->GetSkin(weaponType);
    
    //switch (weaponType)
    //{
    //    // Pistols
    //case WeaponsEnum::Glock:
    //    return SkinInfo_t(38, false);   // Fade
    //
    //case WeaponsEnum::UspS:
    //    return SkinInfo_t(653, true);   // Neo-Noir
    //
    //case WeaponsEnum::P200:
    //    return SkinInfo_t(389, false);  // Fire Elemental
    //
    //case WeaponsEnum::Deagle:
    //    return SkinInfo_t(351, false);  // Conspiracy
    //
    //    // Rifles
    //case WeaponsEnum::Ak47:
    //    return SkinInfo_t(675, false);  // Empress
    //
    //case WeaponsEnum::M4A1Silencer:
    //    return SkinInfo_t(681, true);   // Nightmare
    //
    //case WeaponsEnum::M4A4:
    //    return SkinInfo_t(309, true);  // Howl
    //
    //case WeaponsEnum::Awp:
    //    return SkinInfo_t(344, true);   // Dragon Lore
    //
    //case WeaponsEnum::Famas:
    //    return SkinInfo_t(919, false);  // Commemoration
    //
    //case WeaponsEnum::Galil:
    //    return SkinInfo_t(428, false);  // Eco
    //
    //    // SMGs
    //case WeaponsEnum::Mp9:
    //    return SkinInfo_t(734, false);  // Starlight Protector
    //
    //case WeaponsEnum::Mac10:
    //    return SkinInfo_t(433, false);  // Neon Rider
    //
    //case WeaponsEnum::Mp7:
    //    return SkinInfo_t(1020, false); // Bloodsport
    //
    //    // Heavy
    //case WeaponsEnum::Nova:
    //    return SkinInfo_t(537, false);  // Hyper Beast
    //
    //case WeaponsEnum::Xm1014:
    //    return SkinInfo_t(850, false);  // Incinegator
    //
    //case WeaponsEnum::Scar20:
    //    return SkinInfo_t(312, true);   // Cyrex
    //
    //case WeaponsEnum::G3Sg1:
    //    return SkinInfo_t(511, false);  // The Executioner
    //
    //    // Snipers
    //case WeaponsEnum::Ssg08:
    //    return SkinInfo_t(624, true);  // Dragonfire
    //
    //    // Default
    //default:
    //    return SkinInfo_t{ 0, false, std::string(), WeaponsEnum::none };
    //}
}


void SetMeshMask(const uintptr_t ent, const uint64_t mask)
{
    const auto& node = mem.Read<uintptr_t>(ent + Offsets::m_pGameSceneNode);
    if (!node)
    {
        std::cout << "[Mesh] entity=0x" << std::hex << ent << std::dec
                  << " has no scene node" << std::endl;
        return;
    }

    const auto model = node + Offsets::m_modelState;
    const uint64_t before = mem.Read<uint64_t>(model + Offsets::m_MeshGroupMask);
    mem.Write<uint64_t>(model + Offsets::m_MeshGroupMask, mask);
    const uint64_t after = mem.Read<uint64_t>(model + Offsets::m_MeshGroupMask);

    std::cout << "[Mesh] entity=0x" << std::hex << ent
              << " node=0x" << node
              << " mask=0x" << before << " -> 0x" << after
              << std::dec << (after == mask ? " OK" : " FAILED") << std::endl;
}

void UpdateHud(const uintptr_t weapon, const uint32_t delay = 200)
{
    const uintptr_t Identity = mem.Read<uintptr_t>(weapon + Offsets::m_pEntity);

    mem.Write<uint32_t>(Identity + Offsets::m_flags, HideHudFlag);

    Sleep(delay);

    mem.Write<uint32_t>(Identity + Offsets::m_flags, NULL);
}

void UpdateWeapons(const std::vector<uintptr_t> weapons)
{
    mem.CallThread(Sigs::RegenerateWeaponSkins);

    //clean up & hud update

    for (const uintptr_t& weapon : weapons)
    {
        const uintptr_t item = weapon + Offsets::m_AttributeManager + Offsets::m_Item;
        const uint16_t definition =
            mem.Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);
        const bool isKnife =
            definition == WeaponsEnum::CtKnife ||
            definition == WeaponsEnum::Tknife ||
            (definition >= 500 && definition <= 526);

        if (isKnife)
        {
            std::cout << "[UpdateWeapons] preserving knife def=" << definition
                      << " paint="
                      << mem.Read<uint32_t>(weapon + Offsets::m_nFallbackPaintKit)
                      << " attributes="
                      << static_cast<int>(econItemAttributeManager.GetSize(item + Offsets::m_AttributeList))
                      << std::endl;
            continue;
        }
        
        if (mem.Read<uint32_t>(weapon + Offsets::m_nFallbackPaintKit) == -1)
            continue;

        mem.Write<uint32_t>(weapon + Offsets::m_nFallbackPaintKit, -1);

        //UpdateHud(weapon);

        econItemAttributeManager.Remove(item);
        continue;
    }
}

//inline uintptr_t GetVdata(const uintptr_t& pWeapon)
//{
//    return mem.Read<uintptr_t>(pWeapon + Offsets::m_pVdata);
//}
//
//inline bool IsMeleeWeapon(const uintptr_t& pWeapon)
//{
//    return mem.Read<bool>(GetVdata(pWeapon) + Offsets::m_bMeleeWeapon);
//}

//inline CSWeaponType GetWeaponType(const uintptr_t& pWeapon)
//inline uint32_t GetWeaponType(const uintptr_t& pWeapon)
//{
//    return mem.Read<uint32_t>(GetVdata(pWeapon) + Offsets::m_WeaponType);
//}
//
//inline void SetWeaponType(const uintptr_t& pWeapon, const CSWeaponType type)
//{
//    mem.Write<CSWeaponType>(GetVdata(pWeapon) + Offsets::m_WeaponType, type);
//}