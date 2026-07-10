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
        uint32_t subclassId; // MurmurHash2 of weapon class name with seed 0x31415926
        const char* name;
        const char* modelPath;
    };

    struct GloveConfig {
        uint16_t defIndex;
        uint32_t paint;
        const char* name;
    };

    // Butterfly Knife subclass: MurmurHash2("weapon_knife_butterfly", 0x31415926) = 0xF4ACCFD1
    // CT: Butterfly Knife | Black Pearl Doppler + Sport Gloves | Pandora's Box
    inline constexpr KnifeConfig CtKnifeConfig = { 515, 417, 0xF4ACCFD1, "Butterfly Black Pearl",
        "phase2/weapons/models/knife/knife_butterfly/weapon_knife_butterfly_ag2.vmdl" };
    inline constexpr GloveConfig CtGloveConfig = { 5030, 10037, "Pandora's Box" };

    // T: Butterfly Knife | Gamma Doppler Emerald + Sport Gloves | Hedge Maze
    inline constexpr KnifeConfig TKnifeConfig = { 515, 568, 0xF4ACCFD1, "Butterfly Gamma Emerald",
        "phase2/weapons/models/knife/knife_butterfly/weapon_knife_butterfly_ag2.vmdl" };
    inline constexpr GloveConfig TGloveConfig = { 5030, 10038, "Hedge Maze" };

    // --- PISTOLS ---
    constexpr WeaponSkinConfig PistolConfigs[] = {
        { WeaponsEnum::Deagle, 962, "Desert Eagle", "Printstream", true },
        { WeaponsEnum::Elite, 196, "Dual Berettas", "Elite Skin", false },
        { WeaponsEnum::FiveSeven, 274, "Five-Seven", "Five-Seven Skin", false },
        { WeaponsEnum::Glock, 1119, "Glock-18", "Gamma Doppler Emerald", true },
        { WeaponsEnum::Tec9, 459, "Tec-9", "Bamboo Forest", true },
        { WeaponsEnum::P200, 32, "P2000", "Silver", false },
        { WeaponsEnum::p250, 102, "P250", "Whiteout", false },
        { WeaponsEnum::UspS, 1065, "USP-S", "Whiteout", false },
        { WeaponsEnum::Cz65A, 32, "CZ75-Auto", "Silver", false },
        { WeaponsEnum::Revolver, 522, "R8 Revolver", "R8 Revolver Skin", false },
    };

    // --- SMGS ---
    constexpr WeaponSkinConfig SMGConfigs[] = {
        { WeaponsEnum::Mac10, 32, "MAC-10", "Silver", false },
        { WeaponsEnum::Mp7, 102, "MP7", "Whiteout", false },
        { WeaponsEnum::Mp9, 609, "MP9", "Airlock", true },
        { WeaponsEnum::P90, 359, "P90", "Asiimov", true },
        { WeaponsEnum::Bizon, 542, "PP-Bizon", "PP-Bizon Skin", false },
        { WeaponsEnum::Ump45, 441, "UMP-45", "Labyrinth", true },
        { WeaponsEnum::Mp5SD, 888, "MP5-SD", "MP5-SD Skin", false },
    };

    // --- RIFLES ---
    constexpr WeaponSkinConfig RifleConfigs[] = {
        { WeaponsEnum::Ak47, 801, "AK-47", "Asiimov", true },
        { WeaponsEnum::Famas, 626, "FAMAS", "Mecha Industries", true },
        { WeaponsEnum::Galil, 297, "Galil AR", "Tuxedo", false },
        { WeaponsEnum::Aug, 305, "AUG", "Torque", true },
        { WeaponsEnum::Sg556, 61, "SG 553", "Hypnotic", false },
        { WeaponsEnum::M4A4, 309, "M4A4", "Howl", true },
        { WeaponsEnum::M4A1Silencer, 587, "M4A1-S", "Mecha Industries", true },
    };

    // --- SNIPERS ---
    constexpr WeaponSkinConfig SniperConfigs[] = {
        { WeaponsEnum::Awp, 344, "AWP", "Dragon Lore", true },
        { WeaponsEnum::Ssg08, 253, "SSG 08", "Acid Fade", false },
        { WeaponsEnum::Scar20, 597, "SCAR-20", "Bloodsport", true },
        { WeaponsEnum::G3Sg1, 493, "G3SG1", "G3SG1 Skin", false },
    };

    // --- SHOTGUNS ---
    constexpr WeaponSkinConfig ShotgunConfigs[] = {
        { WeaponsEnum::Nova, 62, "Nova", "Nova Skin", false },
        { WeaponsEnum::Xm1014, 393, "XM1014", "XM1014 Skin", false },
        { WeaponsEnum::Mag7, 32, "MAG-7", "Silver", false },
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
