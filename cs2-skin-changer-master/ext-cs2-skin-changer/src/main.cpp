#include "menu.h"
#include "baseConfig.h"
#include "SDK/entity/CWeaponServices.h"
#include "SDK/weapon/C_EconEntity.h"
#include "SDK/CEconItemAttributeManager.h"

#include <array>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <initializer_list>
#include <unordered_map>

struct RefreshFunctions
{
    uintptr_t updateSubclass = 0;
    uintptr_t setModel = 0;
    uintptr_t updateSkin = 0;
    uintptr_t updateCompositeMaterialSet = 0;
};

static uintptr_t ScanFunction(const char* name, std::initializer_list<const char*> patterns)
{
    for (const char* pattern : patterns)
    {
        const uintptr_t address = mem.SigScan(L"client.dll", pattern);
        std::cout << "[Signature] " << name << " pattern=\"" << pattern
                  << "\" result=0x" << std::hex << address << std::dec << std::endl;
        if (address)
            return address;
    }

    std::cout << "[Signature] " << name << " NOT FOUND" << std::endl;
    return 0;
}

static const RefreshFunctions& GetRefreshFunctions()
{
    static const RefreshFunctions functions = {
        ScanFunction("UpdateSubclass", {
            "4C 8B DC 53 48 81 EC ? ? ? ? 48 8B 41",
            "40 53 48 83 EC 30 48 8B 41 10 48 8B D9 8B 50 30"
        }),
        ScanFunction("SetModel", {
            "40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24"
        }),
        ScanFunction("UpdateSkin", {
            "48 89 5C 24 08 57 48 83 EC 20 8B DA 48 8B F9 E8 ? ? ? ? F6 C3 01"
        }),
        ScanFunction("UpdateCompositeMaterialSet", {
            "40 55 53 41 57 48 8D AC 24 00 FE ? ?"
        })
    };

    return functions;
}

static bool CallOneArgument(const char* name, uintptr_t function, uintptr_t argument)
{
    if (!function)
    {
        std::cout << "[RemoteCall] " << name << " skipped: signature missing" << std::endl;
        return false;
    }

    std::cout << "[RemoteCall] " << name << " function=0x" << std::hex << function
              << " rcx=0x" << argument << std::dec << std::endl;
    return mem.CallThread(function, reinterpret_cast<LPVOID>(argument));
}

static bool CallTwoArguments(
    const char* name,
    uintptr_t function,
    uintptr_t firstArgument,
    uintptr_t secondArgument)
{
    if (!function)
    {
        std::cout << "[RemoteCall] " << name << " skipped: signature missing" << std::endl;
        return false;
    }

    std::vector<uint8_t> shellcode = {
        0x48, 0xB9, 0, 0, 0, 0, 0, 0, 0, 0,
        0x48, 0xBA, 0, 0, 0, 0, 0, 0, 0, 0,
        0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,
        0x48, 0x83, 0xEC, 0x28,
        0xFF, 0xD0,
        0x48, 0x83, 0xC4, 0x28,
        0x33, 0xC0,
        0xC3
    };

    std::memcpy(shellcode.data() + 2, &firstArgument, sizeof(firstArgument));
    std::memcpy(shellcode.data() + 12, &secondArgument, sizeof(secondArgument));
    std::memcpy(shellcode.data() + 22, &function, sizeof(function));

    const uintptr_t shellcodeAddress = mem.Allocate();
    if (!shellcodeAddress)
    {
        std::cout << "[RemoteCall] " << name << " failed: shellcode allocation failed" << std::endl;
        return false;
    }

    mem.WriteBytes(shellcodeAddress, shellcode);
    std::cout << "[RemoteCall] " << name << " function=0x" << std::hex << function
              << " rcx=0x" << firstArgument
              << " rdx=0x" << secondArgument
              << " shellcode=0x" << shellcodeAddress << std::dec << std::endl;

    const bool called = mem.CallThread(shellcodeAddress);
    const bool freed = mem.Free(shellcodeAddress);
    std::cout << "[RemoteCall] " << name << " result=" << called
              << " shellcodeFreed=" << freed << std::endl;
    return called;
}

static std::string ReadModelName(uintptr_t entity)
{
    if (!entity)
        return {};

    const uintptr_t node = mem.Read<uintptr_t>(entity + Offsets::m_pGameSceneNode);
    if (!node)
        return {};

    const uintptr_t modelState = node + Offsets::m_modelState;
    const uintptr_t modelName = mem.Read<uintptr_t>(modelState + Offsets::m_ModelName);
    if (!modelName)
        return {};

    std::string result;
    for (size_t i = 0; i < 256; ++i)
    {
        const char character = mem.Read<char>(modelName + i);
        if (!character)
            break;
        result.push_back(character);
    }
    return result;
}

static void DumpItemState(const char* label, uintptr_t entity)
{
    if (!entity)
    {
        std::cout << label << " entity=null" << std::endl;
        return;
    }

    const uintptr_t item = entity + Offsets::m_AttributeManager + Offsets::m_Item;
    const uintptr_t node = mem.Read<uintptr_t>(entity + Offsets::m_pGameSceneNode);
    const uintptr_t modelState = node ? node + Offsets::m_modelState : 0;

    std::cout << label << " entity=0x" << std::hex << entity
              << " node=0x" << node
              << " hModel=0x" << (modelState ? mem.Read<uintptr_t>(modelState + Offsets::m_hModel) : 0)
              << " subclass=0x" << mem.Read<uint32_t>(entity + Offsets::m_nSubclassID)
              << " itemIDHigh=0x" << mem.Read<uint32_t>(item + Offsets::m_iItemIDHigh)
              << " itemIDLow=0x" << mem.Read<uint32_t>(item + Offsets::m_iItemIDLow)
              << std::dec
              << " def=" << mem.Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex)
              << " paint=" << mem.Read<uint32_t>(entity + Offsets::m_nFallbackPaintKit)
              << " wear=" << mem.Read<float>(entity + Offsets::m_flFallbackWear)
              << " initialized=" << mem.Read<bool>(item + Offsets::m_bInitialized)
              << " disallowSOC=" << mem.Read<bool>(item + Offsets::m_bDisallowSOC)
              << " restoreMaterial=" << mem.Read<bool>(item + Offsets::m_bRestoreCustomMaterialAfterPrecache)
              << " model=\"" << ReadModelName(entity) << "\"" << std::endl;
}

static void DumpModelState(const char* label, uintptr_t entity)
{
    if (!entity)
    {
        std::cout << label << " entity=null" << std::endl;
        return;
    }

    const uintptr_t node = mem.Read<uintptr_t>(entity + Offsets::m_pGameSceneNode);
    const uintptr_t modelState = node ? node + Offsets::m_modelState : 0;
    std::cout << label << " entity=0x" << std::hex << entity
              << " node=0x" << node
              << " hModel=0x" << (modelState ? mem.Read<uintptr_t>(modelState + Offsets::m_hModel) : 0)
              << " mesh=0x" << (modelState ? mem.Read<uint64_t>(modelState + Offsets::m_MeshGroupMask) : 0)
              << std::dec
              << " model=\"" << ReadModelName(entity) << "\"" << std::endl;
}

static uintptr_t GetRemoteModelPath(const char* modelPath)
{
    static std::unordered_map<std::string, uintptr_t> paths;
    const auto existing = paths.find(modelPath);
    if (existing != paths.end())
        return existing->second;

    const size_t pathLength = std::strlen(modelPath);
    const uintptr_t remotePath = mem.Allocate(0, pathLength + 1, PAGE_READWRITE);
    if (!remotePath)
        return 0;

    mem.WriteString(remotePath, modelPath);
    mem.Write<char>(remotePath + pathLength, '\0');
    paths.emplace(modelPath, remotePath);
    std::cout << "[Model] persistent path=\"" << modelPath
              << "\" remotePath=0x" << std::hex << remotePath << std::dec << std::endl;
    return remotePath;
}

static bool SetEntityModel(const char* label, uintptr_t entity, const char* modelPath)
{
    if (!entity)
    {
        std::cout << "[Model] " << label << " skipped: entity missing" << std::endl;
        return false;
    }

    const uintptr_t remotePath = GetRemoteModelPath(modelPath);
    if (!remotePath)
    {
        std::cout << "[Model] " << label << " failed: path allocation failed" << std::endl;
        return false;
    }

    std::cout << "[Model] " << label << " path=\"" << modelPath
              << "\" remotePath=0x" << std::hex << remotePath << std::dec << std::endl;

    const bool called = CallTwoArguments(
        label,
        GetRefreshFunctions().setModel,
        entity,
        remotePath);
    const std::string readback = ReadModelName(entity);

    std::cout << "[Model] " << label << " called=" << called
              << " readback=\"" << readback << "\""
              << (readback == modelPath ? " OK" : " MISMATCH") << std::endl;
    return called && readback == modelPath;
}

static bool KnifeNeedsRefresh(
    uintptr_t weapon,
    uintptr_t hudWeapon,
    const BaseConfig::KnifeConfig& config)
{
    const uintptr_t item = weapon + Offsets::m_AttributeManager + Offsets::m_Item;
    const bool worldModelMatches = ReadModelName(weapon) == config.modelPath;
    const bool hudModelMatches = !hudWeapon || ReadModelName(hudWeapon) == config.modelPath;
    const bool needsRefresh =
        mem.Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex) != config.defIndex ||
        mem.Read<uint32_t>(item + Offsets::m_iItemIDHigh) != UINT32_MAX ||
        mem.Read<uint32_t>(item + Offsets::m_iItemIDLow) != UINT32_MAX ||
        mem.Read<uint32_t>(weapon + Offsets::m_nSubclassID) != config.subclassId ||
        mem.Read<uint32_t>(weapon + Offsets::m_nFallbackPaintKit) != config.paint ||
        !worldModelMatches ||
        !hudModelMatches ||
        ForceUpdate;

    if (!needsRefresh)
        return false;

    static std::unordered_map<uintptr_t, std::chrono::steady_clock::time_point> nextAttempts;
    const auto now = std::chrono::steady_clock::now();
    if (now < nextAttempts[weapon])
        return false;
    nextAttempts[weapon] = now + std::chrono::milliseconds(500);

    std::cout << "[Knife][Check] weapon=0x" << std::hex << weapon
              << " hud=0x" << hudWeapon << std::dec
              << " worldModelMatches=" << worldModelMatches
              << " hudModelMatches=" << hudModelMatches
              << " force=" << ForceUpdate
              << " refresh=true" << std::endl;
    return true;
}

static void ApplyKnife(
    uintptr_t weapon,
    uintptr_t hudWeapon,
    const BaseConfig::KnifeConfig& config)
{
    const uintptr_t item = weapon + Offsets::m_AttributeManager + Offsets::m_Item;
    const RefreshFunctions& functions = GetRefreshFunctions();

    DumpItemState("[Knife][Before]", weapon);
    if (hudWeapon)
        DumpModelState("[Knife][HudBefore]", hudWeapon);

    mem.Write<bool>(item + Offsets::m_bDisallowSOC, false);
    mem.Write<bool>(item + Offsets::m_bRestoreCustomMaterialAfterPrecache, true);
    mem.Write<bool>(item + Offsets::m_bInitialized, true);
    mem.Write<uint32_t>(item + Offsets::m_iItemIDHigh, UINT32_MAX);
    mem.Write<uint32_t>(item + Offsets::m_iItemIDLow, UINT32_MAX);
    mem.Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, config.defIndex);
    mem.Write<int32_t>(item + Offsets::m_iEntityQuality, 3);
    mem.Write<uint32_t>(
        item + Offsets::m_iAccountID,
        mem.Read<uint32_t>(weapon + Offsets::m_OriginalOwnerXuidLow));
    mem.Write<uint32_t>(weapon + Offsets::m_nSubclassID, config.subclassId);
    mem.Write<uint32_t>(weapon + Offsets::m_nFallbackPaintKit, config.paint);
    mem.Write<float>(weapon + Offsets::m_flFallbackWear, 0.01f);
    mem.Write<uint32_t>(weapon + Offsets::m_nFallbackSeed, 0);

    std::cout << "[Knife][Write] def=" << config.defIndex
              << " paint=" << config.paint
              << " subclass=0x" << std::hex << config.subclassId << std::dec
              << " itemIDs=0xFFFFFFFF disallowSOC=false initialized=true"
              << " restoreMaterial=true" << std::endl;

    CallOneArgument("UpdateSubclass", functions.updateSubclass, weapon);

    mem.Write<bool>(item + Offsets::m_bDisallowSOC, false);
    mem.Write<bool>(item + Offsets::m_bRestoreCustomMaterialAfterPrecache, true);
    mem.Write<bool>(item + Offsets::m_bInitialized, true);
    mem.Write<uint32_t>(item + Offsets::m_iItemIDHigh, UINT32_MAX);
    mem.Write<uint32_t>(item + Offsets::m_iItemIDLow, UINT32_MAX);
    mem.Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, config.defIndex);
    mem.Write<uint32_t>(weapon + Offsets::m_nSubclassID, config.subclassId);
    mem.Write<uint32_t>(weapon + Offsets::m_nFallbackPaintKit, config.paint);
    std::cout << "[Knife][PostSubclass] reasserted definition, item IDs, SOC,"
              << " subclass, and paint" << std::endl;

    SetEntityModel("SetModel(world)", weapon, config.modelPath);
    if (hudWeapon)
        SetEntityModel("SetModel(hud)", hudWeapon, config.modelPath);

    SetMeshMask(weapon, 2);
    if (hudWeapon)
        SetMeshMask(hudWeapon, 2);

    SkinInfo_t knifeSkin{ static_cast<int>(config.paint), true, config.name, WeaponsEnum::CtKnife };
    econItemAttributeManager.Replace(item, knifeSkin);
    std::cout << "[Knife][Attributes] count="
              << static_cast<int>(econItemAttributeManager.GetSize(item + Offsets::m_AttributeList))
              << std::endl;

    CallOneArgument("UpdateCompositeMaterialSet", functions.updateCompositeMaterialSet, weapon);
    CallOneArgument("UpdateSkin", functions.updateSkin, weapon);
    std::cout << "[Knife][RegenerateWeaponSkins] function=0x" << std::hex
              << Sigs::RegenerateWeaponSkins << std::dec << std::endl;
    mem.CallThread(Sigs::RegenerateWeaponSkins);

    DumpItemState("[Knife][After]", weapon);
    if (hudWeapon)
        DumpModelState("[Knife][HudAfter]", hudWeapon);
}

static void DumpGloveState(const char* label, uintptr_t localPlayer)
{
    const uintptr_t item = localPlayer + Offsets::m_EconGloves;
    std::cout << label
              << " pawn=0x" << std::hex << localPlayer
              << " itemIDHigh=0x" << mem.Read<uint32_t>(item + Offsets::m_iItemIDHigh)
              << " itemIDLow=0x" << mem.Read<uint32_t>(item + Offsets::m_iItemIDLow)
              << std::dec
              << " def=" << mem.Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex)
              << " paint=" << econItemAttributeManager.GetPaint(item)
              << " attributeCount="
              << static_cast<int>(econItemAttributeManager.GetSize(item + Offsets::m_AttributeList))
              << " initialized=" << mem.Read<bool>(item + Offsets::m_bInitialized)
              << " disallowSOC=" << mem.Read<bool>(item + Offsets::m_bDisallowSOC)
              << " restoreMaterial=" << mem.Read<bool>(item + Offsets::m_bRestoreCustomMaterialAfterPrecache)
              << " needReapply=" << mem.Read<bool>(localPlayer + Offsets::m_bNeedToReApplyGloves)
              << " changed=" << static_cast<int>(
                    mem.Read<uint8_t>(localPlayer + Offsets::m_nEconGlovesChanged))
              << std::endl;
}

static bool GloveNeedsRefresh(
    uintptr_t localPlayer,
    const BaseConfig::GloveConfig& config)
{
    const uintptr_t item = localPlayer + Offsets::m_EconGloves;
    return
        mem.Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex) != config.defIndex ||
        mem.Read<uint32_t>(item + Offsets::m_iItemIDHigh) != UINT32_MAX ||
        mem.Read<uint32_t>(item + Offsets::m_iItemIDLow) != UINT32_MAX ||
        econItemAttributeManager.GetPaint(item) != static_cast<int>(config.paint) ||
        !mem.Read<bool>(item + Offsets::m_bInitialized) ||
        ForceUpdate;
}

static void ApplyGloves(
    uintptr_t localPlayer,
    const BaseConfig::GloveConfig& config)
{
    const uintptr_t item = localPlayer + Offsets::m_EconGloves;
    DumpGloveState("[Gloves][Before]", localPlayer);

    mem.Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, config.defIndex);
    mem.Write<uint32_t>(item + Offsets::m_iItemIDHigh, UINT32_MAX);
    mem.Write<uint32_t>(item + Offsets::m_iItemIDLow, UINT32_MAX);
    mem.Write<uint32_t>(item + Offsets::m_iAccountID, 1);
    mem.Write<int32_t>(item + Offsets::m_iEntityQuality, 4);
    mem.Write<bool>(item + Offsets::m_bInitialized, true);
    mem.Write<bool>(item + Offsets::m_bDisallowSOC, false);
    mem.Write<bool>(item + Offsets::m_bRestoreCustomMaterialAfterPrecache, true);

    SkinInfo_t gloveSkin{
        static_cast<int>(config.paint),
        false,
        config.name,
        WeaponsEnum::none
    };
    econItemAttributeManager.Replace(item, gloveSkin);

    std::cout << "[Gloves][Write] def=" << config.defIndex
              << " paint=" << config.paint
              << " itemIDs=0xFFFFFFFF disallowSOC=false initialized=true"
              << " restoreMaterial=true" << std::endl;
}

static void PulseGloveRefresh(
    uintptr_t localPlayer,
    uint8_t pulse,
    uint8_t pulseCount)
{
    const uintptr_t item = localPlayer + Offsets::m_EconGloves;
    const uint8_t changed =
        mem.Read<uint8_t>(localPlayer + Offsets::m_nEconGlovesChanged);
    mem.Write<uint8_t>(
        localPlayer + Offsets::m_nEconGlovesChanged,
        static_cast<uint8_t>(changed + 1));
    mem.Write<bool>(item + Offsets::m_bInitialized, true);
    mem.Write<bool>(localPlayer + Offsets::m_bNeedToReApplyGloves, true);

    std::cout << "[Gloves][Frame " << static_cast<int>(pulse)
              << "/" << static_cast<int>(pulseCount) << "]"
              << " needReapply="
              << mem.Read<bool>(localPlayer + Offsets::m_bNeedToReApplyGloves)
              << " changed=" << static_cast<int>(
                    mem.Read<uint8_t>(localPlayer + Offsets::m_nEconGlovesChanged))
              << " initialized=" << mem.Read<bool>(item + Offsets::m_bInitialized)
              << std::endl;
}

int main()
{
    skindb->Dump();

    BaseConfig::ApplyBaseConfig();

    InitMenu();
    
    std::cout << "SkinChanger Started\n";

    constexpr uint8_t gloveRefreshFrameCount = 3;
    uint8_t gloveRefreshFramesRemaining = 0;
    auto nextGloveAttempt = std::chrono::steady_clock::time_point{};

    while (!GetAsyncKeyState(VK_END))
    {
        Sleep(5);

        const uintptr_t localController = GetLocalController();
        const uintptr_t inventoryServices = mem.Read<uintptr_t>(localController + Offsets::m_pInventoryServices);
        const uintptr_t localPlayer = GetLocalPlayer();

        mem.Write<uint16_t>(inventoryServices + Offsets::m_unMusicID, skinManager->MusicKit.id);

        CurrentWeaponDef = mem.Read<WeaponsEnum>(GetActiveWeapon(localPlayer) + Offsets::m_AttributeManager + Offsets::m_Item + Offsets::m_iItemDefinitionIndex);

        UpdateActiveMenuDef(localPlayer);
        OnFrame();

        bool ShouldUpdate = false;

        const uint8_t team = mem.Read<uint8_t>(localPlayer + Offsets::m_iTeamNum);

        const std::vector<uintptr_t> weapons = GetWeapons(localPlayer);
        for (const uintptr_t weapon : weapons)
        {
            const uintptr_t item = weapon + Offsets::m_AttributeManager + Offsets::m_Item;

            // Read def index BEFORE "already applied" check to always detect knives
            const uint16_t weaponDefRaw = mem.Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);

            if (BaseConfig::IsKnife(weaponDefRaw))
            {
                const auto& knifeConfig = (team == 3) ? BaseConfig::CtKnifeConfig : BaseConfig::TKnifeConfig;
                const uintptr_t hudWeapon = GetHudWeapon(weapon);

                if (KnifeNeedsRefresh(weapon, hudWeapon, knifeConfig))
                {
                    std::cout << "[Knife] Team=" << (int)team
                              << " OrigDef=" << weaponDefRaw
                              << " -> TargetDef=" << knifeConfig.defIndex
                              << " Paint=" << knifeConfig.paint << std::endl;
                    ApplyKnife(weapon, hudWeapon, knifeConfig);
                    ShouldUpdate = true;
                }
                continue;
            }

            // --- NORMAL WEAPON SKIN (gated by iItemIDHigh) ---
            if(ForceUpdate)
                mem.Write<uint32_t>(item + Offsets::m_iItemIDHigh, NULL);

            if (mem.Read<uint32_t>(item + Offsets::m_iItemIDHigh) == -1)//already applied
                continue;

            mem.Write<uint32_t>(item + Offsets::m_iItemIDHigh, -1);

            SkinInfo_t skin = GetSkin(item);
            if (!skin.Paint)
                continue;

            mem.Write<uint32_t>(weapon + Offsets::m_nFallbackPaintKit, skin.Paint);

            const uint64_t mask = skin.bUsesOldModel + 1;

            const uintptr_t hudWeapon = GetHudWeapon(weapon);
            SetMeshMask(weapon, mask);
            SetMeshMask(hudWeapon, mask);

            econItemAttributeManager.Create(item, skin);

            ShouldUpdate = true;
        }

        {
            const auto& gloveConfig = (team == 3) ? BaseConfig::CtGloveConfig : BaseConfig::TGloveConfig;
            const auto now = std::chrono::steady_clock::now();
            if (!gloveRefreshFramesRemaining &&
                now >= nextGloveAttempt &&
                GloveNeedsRefresh(localPlayer, gloveConfig))
            {
                const uintptr_t glovesItem = localPlayer + Offsets::m_EconGloves;
                const uint16_t currentGloveDef =
                    mem.Read<uint16_t>(glovesItem + Offsets::m_iItemDefinitionIndex);
                std::cout << "[Gloves] Team=" << (int)team
                          << " CurrentDef=" << currentGloveDef
                          << " -> TargetDef=" << gloveConfig.defIndex
                          << " Paint=" << gloveConfig.paint << std::endl;

                ApplyGloves(localPlayer, gloveConfig);
                gloveRefreshFramesRemaining = gloveRefreshFrameCount;
                nextGloveAttempt = now + std::chrono::milliseconds(500);
                ShouldUpdate = true;
            }

            if (gloveRefreshFramesRemaining)
            {
                const uint8_t pulse =
                    gloveRefreshFrameCount - gloveRefreshFramesRemaining + 1;
                PulseGloveRefresh(
                    localPlayer,
                    pulse,
                    gloveRefreshFrameCount);
                --gloveRefreshFramesRemaining;
                ShouldUpdate = true;

                if (!gloveRefreshFramesRemaining)
                    DumpGloveState("[Gloves][After]", localPlayer);
            }
        }

        if (ShouldUpdate || ForceUpdate)
            UpdateWeapons(weapons);

        ForceUpdate = false;
    }
    
    return 0;
}