#pragma once

#include "SDK/weapon/C_EconEntity.h"
#include "../ext/skindb.h"

// Base configuration for weapon skins with weapon definitions and skin IDs
namespace BaseConfig {

    struct WeaponSkinConfig {
        WeaponsEnum weapon;
        uint32_t skinId;
        const char* weaponName;
        const char* skinName;
        bool useOldModel;
    };

    // --- KNIFE / GLOVE CONFIG (per-team) ---
    struct KnifeConfig {
        uint16_t defIndex;
        uint32_t paint;
        const char* name;
    };

    struct GloveConfig {
        uint16_t defIndex;
        uint32_t paint;
        const char* name;
    };

    // CT: Butterfly Knife | Black Pearl Doppler + Sport Gloves | Pandora's Box
    inline constexpr KnifeConfig CtKnifeConfig = { 515, 417, "Butterfly Black Pearl" };
    inline constexpr GloveConfig CtGloveConfig = { 5030, 10037, "Pandora's Box" };

    // T: Butterfly Knife | Gamma Doppler Emerald + Sport Gloves | Hedge Maze
    inline constexpr KnifeConfig TKnifeConfig = { 515, 568, "Butterfly Gamma Emerald" };
    inline constexpr GloveConfig TGloveConfig = { 5030, 10038, "Hedge Maze" };

    // --- PISTOLS ---
    constexpr WeaponSkinConfig PistolConfigs[] = {
        { WeaponsEnum::Deagle, 37, "Deagle", "Deagle Skin", false },
        { WeaponsEnum::Elite, 196, "Elite", "Elite Skin", false },
        { WeaponsEnum::FiveSeven, 274, "Five-Seven", "Five-Seven Skin", false },
        { WeaponsEnum::Glock, 1119, "Glock-18", "Glock-18 Skin", false },
        { WeaponsEnum::Tec9, 889, "Tec-9", "Tec-9 Skin", false },
        { WeaponsEnum::P200, 389, "P250", "P250 Skin", false },
        { WeaponsEnum::p250, 749, "P250", "P250 Skin", false },
        { WeaponsEnum::UspS, 1065, "USP-S", "USP-S Skin", false },
        { WeaponsEnum::Cz65A, 325, "CZ75-Auto", "CZ75-Auto Skin", false },
        { WeaponsEnum::Revolver, 522, "R8 Revolver", "R8 Revolver Skin", false },
    };

    // --- SMGS ---
    constexpr WeaponSkinConfig SMGConfigs[] = {
        { WeaponsEnum::Mac10, 433, "MAC-10", "MAC-10 Skin", false },
        { WeaponsEnum::Mp7, 696, "MP7", "MP7 Skin", false },
        { WeaponsEnum::Mp9, 33, "MP9", "MP9 Skin", false },
        { WeaponsEnum::P90, 359, "P90", "P90 Skin", false },
        { WeaponsEnum::Bizon, 542, "PP-Bizon", "PP-Bizon Skin", false },
        { WeaponsEnum::Ump45, 879, "UMP-45", "UMP-45 Skin", false },
        { WeaponsEnum::Mp5SD, 888, "MP5-SD", "MP5-SD Skin", false },
    };

    // --- RIFLES ---
    constexpr WeaponSkinConfig RifleConfigs[] = {
        { WeaponsEnum::Ak47, 921, "AK-47", "AK-47 Skin", false },
        { WeaponsEnum::Famas, 604, "FAMAS", "FAMAS Skin", false },
        { WeaponsEnum::Galil, 428, "Galil AR", "Galil AR Skin", false },
        { WeaponsEnum::Aug, 33, "AUG", "AUG Skin", false },
        { WeaponsEnum::Sg556, 487, "SG 553", "SG 553 Skin", false },
        { WeaponsEnum::M4A4, 309, "M4A4", "M4A4 Skin", false },
        { WeaponsEnum::M4A1Silencer, 984, "M4A1-S", "M4A1-S Skin", false },
    };

    // --- SNIPERS ---
    constexpr WeaponSkinConfig SniperConfigs[] = {
        { WeaponsEnum::Awp, 344, "AWP", "Dragon Lore", true },
        { WeaponsEnum::Ssg08, 624, "SSG 08", "SSG 08 Skin", false },
        { WeaponsEnum::Scar20, 165, "SCAR-20", "SCAR-20 Skin", false },
        { WeaponsEnum::G3Sg1, 493, "G3SG1", "G3SG1 Skin", false },
    };

    // --- SHOTGUNS ---
    constexpr WeaponSkinConfig ShotgunConfigs[] = {
        { WeaponsEnum::Nova, 62, "Nova", "Nova Skin", false },
        { WeaponsEnum::Xm1014, 393, "XM1014", "XM1014 Skin", false },
        { WeaponsEnum::Mag7, 948, "MAG-7", "MAG-7 Skin", false },
        { WeaponsEnum::Sawedoof, 256, "Sawed-Off", "Sawed-Off Skin", false },
    };

    // --- MACHINEGUNS ---
    constexpr WeaponSkinConfig MachinegunConfigs[] = {
        { WeaponsEnum::M249, 1042, "M249", "M249 Skin", false },
        { WeaponsEnum::Negev, 240, "Negev", "Negev Skin", false },
    };

    // Helper function to get skin ID for a weapon
    inline uint32_t GetSkinIdForWeapon(WeaponsEnum weapon) {
        for (const auto& config : PistolConfigs)
            if (config.weapon == weapon) return config.skinId;
        for (const auto& config : SMGConfigs)
            if (config.weapon == weapon) return config.skinId;
        for (const auto& config : RifleConfigs)
            if (config.weapon == weapon) return config.skinId;
        for (const auto& config : SniperConfigs)
            if (config.weapon == weapon) return config.skinId;
        for (const auto& config : ShotgunConfigs)
            if (config.weapon == weapon) return config.skinId;
        for (const auto& config : MachinegunConfigs)
            if (config.weapon == weapon) return config.skinId;
        return 0;
    }

    // Helper function to determine weapon type
    inline CSWeaponType GetWeaponType(WeaponsEnum weapon) {
        for (const auto& config : PistolConfigs)
            if (config.weapon == weapon) return CSWeaponType::WEAPONTYPE_PISTOL;
        for (const auto& config : SMGConfigs)
            if (config.weapon == weapon) return CSWeaponType::WEAPONTYPE_SUBMACHINEGUN;
        for (const auto& config : RifleConfigs)
            if (config.weapon == weapon) return CSWeaponType::WEAPONTYPE_RIFLE;
        for (const auto& config : SniperConfigs)
            if (config.weapon == weapon) return CSWeaponType::WEAPONTYPE_SNIPER_RIFLE;
        for (const auto& config : ShotgunConfigs)
            if (config.weapon == weapon) return CSWeaponType::WEAPONTYPE_SHOTGUN;
        for (const auto& config : MachinegunConfigs)
            if (config.weapon == weapon) return CSWeaponType::WEAPONTYPE_MACHINEGUN;
        return CSWeaponType::WEAPONTYPE_UNKNOWN;
    }

    // Push every configured weapon skin into the skin manager
    template <size_t N>
    inline void ApplyConfigArray(const WeaponSkinConfig (&configs)[N]) {
        for (const auto& config : configs) {
            skinManager->AddSkin(SkinInfo_t{
                static_cast<int>(config.skinId),
                config.useOldModel,
                config.skinName,
                config.weapon
            });
        }
    }

    inline void ApplyBaseConfig() {
        ApplyConfigArray(PistolConfigs);
        ApplyConfigArray(SMGConfigs);
        ApplyConfigArray(RifleConfigs);
        ApplyConfigArray(SniperConfigs);
        ApplyConfigArray(ShotgunConfigs);
        ApplyConfigArray(MachinegunConfigs);
    }

    // Check if a weapon definition index is a knife (standard or custom)
    inline bool IsKnife(uint16_t defIndex) {
        if (defIndex == CtKnife || defIndex == Tknife)
            return true;
        return KnifeModels.count(defIndex) > 0;
    }
}
