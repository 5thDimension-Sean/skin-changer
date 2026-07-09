#pragma once

#include "SDK/weapon/C_EconEntity.h"

// Base configuration for weapon skins with weapon definitions and skin IDs
namespace BaseConfig {

    struct WeaponSkinConfig {
        WeaponsEnum weapon;
        uint32_t skinId;
        const char* weaponName;
        const char* skinName;
    };

    // --- PISTOLS ---
    constexpr WeaponSkinConfig PistolConfigs[] = {
        { WeaponsEnum::Deagle, 37, "Deagle", "Deagle Skin" },
        { WeaponsEnum::Elite, 196, "Elite", "Elite Skin" },
        { WeaponsEnum::FiveSeven, 274, "Five-Seven", "Five-Seven Skin" },
        { WeaponsEnum::Glock, 1119, "Glock-18", "Glock-18 Skin" },
        { WeaponsEnum::Tec9, 889, "Tec-9", "Tec-9 Skin" },
        { WeaponsEnum::P200, 389, "P250", "P250 Skin" },
        { WeaponsEnum::p250, 749, "P250", "P250 Skin" },
        { WeaponsEnum::UspS, 1065, "USP-S", "USP-S Skin" },
        { WeaponsEnum::Cz65A, 325, "CZ75-Auto", "CZ75-Auto Skin" },
        { WeaponsEnum::Revolver, 522, "R8 Revolver", "R8 Revolver Skin" },
    };

    // --- SMGS ---
    constexpr WeaponSkinConfig SMGConfigs[] = {
        { WeaponsEnum::Mac10, 433, "MAC-10", "MAC-10 Skin" },
        { WeaponsEnum::Mp7, 696, "MP7", "MP7 Skin" },
        { WeaponsEnum::Mp9, 33, "MP9", "MP9 Skin" },
        { WeaponsEnum::P90, 359, "P90", "P90 Skin" },
        { WeaponsEnum::Bizon, 542, "PP-Bizon", "PP-Bizon Skin" },
        { WeaponsEnum::Ump45, 879, "UMP-45", "UMP-45 Skin" },
        { WeaponsEnum::Mp5SD, 888, "MP5-SD", "MP5-SD Skin" },
    };

    // --- RIFLES ---
    constexpr WeaponSkinConfig RifleConfigs[] = {
        { WeaponsEnum::Ak47, 921, "AK-47", "AK-47 Skin" },
        { WeaponsEnum::Famas, 604, "FAMAS", "FAMAS Skin" },
        { WeaponsEnum::Galil, 428, "Galil AR", "Galil AR Skin" },
        { WeaponsEnum::Aug, 33, "AUG", "AUG Skin" },
        { WeaponsEnum::Sg556, 487, "SG 553", "SG 553 Skin" },
        { WeaponsEnum::M4A4, 309, "M4A4", "M4A4 Skin" },
        { WeaponsEnum::M4A1Silencer, 984, "M4A1-S", "M4A1-S Skin" },
    };

    // --- SNIPERS ---
    constexpr WeaponSkinConfig SniperConfigs[] = {
        { WeaponsEnum::Awp, 344, "AWP Dragon Lore", "AWP Dragon Lore Skin" },
        { WeaponsEnum::Ssg08, 624, "SSG 08", "SSG 08 Skin" },
        { WeaponsEnum::Scar20, 165, "SCAR-20", "SCAR-20 Skin" },
        { WeaponsEnum::G3Sg1, 493, "G3SG1", "G3SG1 Skin" },
    };

    // --- SHOTGUNS ---
    constexpr WeaponSkinConfig ShotgunConfigs[] = {
        { WeaponsEnum::Nova, 62, "Nova", "Nova Skin" },
        { WeaponsEnum::Xm1014, 393, "XM1014", "XM1014 Skin" },
        { WeaponsEnum::Mag7, 948, "MAG-7", "MAG-7 Skin" },
        { WeaponsEnum::Sawedoof, 256, "Sawed-Off", "Sawed-Off Skin" },
    };

    // --- MACHINEGUNS ---
    constexpr WeaponSkinConfig MachinegunConfigs[] = {
        { WeaponsEnum::M249, 1042, "M249", "M249 Skin" },
        { WeaponsEnum::Negev, 240, "Negev", "Negev Skin" },
    };

    // Helper function to get skin ID for a weapon
    inline uint32_t GetSkinIdForWeapon(WeaponsEnum weapon) {
        // PISTOLS
        for (const auto& config : PistolConfigs) {
            if (config.weapon == weapon) return config.skinId;
        }

        // SMGS
        for (const auto& config : SMGConfigs) {
            if (config.weapon == weapon) return config.skinId;
        }

        // RIFLES
        for (const auto& config : RifleConfigs) {
            if (config.weapon == weapon) return config.skinId;
        }

        // SNIPERS
        for (const auto& config : SniperConfigs) {
            if (config.weapon == weapon) return config.skinId;
        }

        // SHOTGUNS
        for (const auto& config : ShotgunConfigs) {
            if (config.weapon == weapon) return config.skinId;
        }

        // MACHINEGUNS
        for (const auto& config : MachinegunConfigs) {
            if (config.weapon == weapon) return config.skinId;
        }

        return 0; // Default: no skin
    }

    // Helper function to determine weapon type
    inline CSWeaponType GetWeaponType(WeaponsEnum weapon) {
        // PISTOLS
        for (const auto& config : PistolConfigs) {
            if (config.weapon == weapon) return CSWeaponType::WEAPONTYPE_PISTOL;
        }

        // SMGS
        for (const auto& config : SMGConfigs) {
            if (config.weapon == weapon) return CSWeaponType::WEAPONTYPE_SUBMACHINEGUN;
        }

        // RIFLES
        for (const auto& config : RifleConfigs) {
            if (config.weapon == weapon) return CSWeaponType::WEAPONTYPE_RIFLE;
        }

        // SNIPERS
        for (const auto& config : SniperConfigs) {
            if (config.weapon == weapon) return CSWeaponType::WEAPONTYPE_SNIPER_RIFLE;
        }

        // SHOTGUNS
        for (const auto& config : ShotgunConfigs) {
            if (config.weapon == weapon) return CSWeaponType::WEAPONTYPE_SHOTGUN;
        }

        // MACHINEGUNS
        for (const auto& config : MachinegunConfigs) {
            if (config.weapon == weapon) return CSWeaponType::WEAPONTYPE_MACHINEGUN;
        }

        return CSWeaponType::WEAPONTYPE_UNKNOWN;
    }
}
