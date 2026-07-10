#include "menu.h"
#include "baseConfig.h"
#include "SDK/entity/CWeaponServices.h"
#include "SDK/weapon/C_EconEntity.h"
#include "SDK/CEconItemAttributeManager.h"

#include <array>
#include <iomanip>

#define MEM_BLOCK_SIZE 0x58
#define ITEM_DESCRIPTION_OFFSET 0x200
uintptr_t StealMemBlockFromWeapon(uintptr_t pWeapon, bool cleanBlock = true)
{
	const uintptr_t item = pWeapon + Offsets::m_AttributeManager + Offsets::m_Item;
    const uintptr_t itemDescription = item + ITEM_DESCRIPTION_OFFSET;

    while (true)
    {
		Sleep(5);

        uintptr_t block = mem.Read<uintptr_t>(itemDescription);
        if (!block) continue;

        mem.Write<uintptr_t>(itemDescription, NULL);

		if (!cleanBlock) return block;

        mem.Write<std::array<char, MEM_BLOCK_SIZE>>(block, {});

        return block;
    }
}

struct PostUpdateData_t
{
    uintptr_t Identity;
    uint32_t flags;
    uint32_t flags2;
};
static_assert(sizeof(PostUpdateData_t) == 0x10);

PostUpdateData_t GetClientEntityUpdateData(uintptr_t Target)
{
    const uintptr_t Identity = mem.Read<uintptr_t>(Target + Offsets::m_pEntity);
    const size_t maxEntities = mem.Read<uint32_t>(entitylist + cs2_dumper::offsets::client_dll::dwGameEntitySystem_highestEntityIndex);
    const uintptr_t updateList = mem.Read<uintptr_t>(entitylist + 0xC00);
    if (!updateList || !maxEntities || !Identity) return {};

    for (size_t i = 0; i < maxEntities; i++)
    {
        PostUpdateData_t data = mem.Read<PostUpdateData_t>(updateList + (sizeof(PostUpdateData_t) + i));
        if (data.Identity == Identity)
        {
            return data;
        }
    }return {};
}

bool SetClientEntityUpdateDataFlags(uintptr_t Target, uint32_t flags)
{
	const uintptr_t Identity = mem.Read<uintptr_t>(Target + Offsets::m_pEntity);
	const size_t maxEntities = mem.Read<uint32_t>(entitylist + cs2_dumper::offsets::client_dll::dwGameEntitySystem_highestEntityIndex);
    const uintptr_t updateList = mem.Read<uintptr_t>(entitylist + 0xC00);
    if (!updateList || !maxEntities || !Identity) return false;

    for (size_t i = 0; i < maxEntities; i++)
    {
        PostUpdateData_t data = mem.Read<PostUpdateData_t>(updateList + (sizeof(PostUpdateData_t) + i));
        if (data.Identity == Identity)
        {
            data.flags = flags;
            mem.Write<PostUpdateData_t>(updateList + (sizeof(PostUpdateData_t) + i), data);

            return true;
        }
	}return false;
}

// Created with ReClass.NET 1.2 by KN4CK3R

class UpdateContainer_t
{
public:
    uintptr_t pEnt; //0x0000
    uintptr_t pOther; //0x0008
    uint32_t uFlags; //0x0010
    char pad_0014[12]; //0x0014
}; //Size: 0x0020
static_assert(sizeof(UpdateContainer_t) == 0x20);

bool UpdateData(uintptr_t Target, uint32_t flags)
{
    const size_t maxEntities = mem.Read<uint32_t>(entitylist + cs2_dumper::offsets::client_dll::dwGameEntitySystem_highestEntityIndex);
    const uintptr_t updateList = mem.Read<uintptr_t>(client + 0x2323E70);//C:\\buildworker\\csgo_rel_win64\\build\\src\\game\\client\\cdll_client_int.cpp
    if (!updateList || !maxEntities || !Target) return false;

    for (size_t i = 0; i < maxEntities; i++)
    {
        UpdateContainer_t data = mem.Read<UpdateContainer_t>(updateList + (sizeof(UpdateContainer_t) + i));
        if (data.pEnt == Target)
        {
            data.uFlags = flags;
            mem.Write<UpdateContainer_t>(updateList + (sizeof(UpdateContainer_t) + i), data);

            return true;
        }
    }return false;
}

UpdateContainer_t ReadData(uintptr_t Target)
{
    const size_t maxEntities = mem.Read<uint32_t>(entitylist + cs2_dumper::offsets::client_dll::dwGameEntitySystem_highestEntityIndex);
    const uintptr_t updateList = mem.Read<uintptr_t>(client + 0x2323E70);
    if (!updateList || !maxEntities || !Target) return {};

    for (size_t i = 0; i < maxEntities; i++)
    {
        UpdateContainer_t data = mem.Read<UpdateContainer_t>(updateList + (sizeof(UpdateContainer_t) + i));
        if (data.pEnt == Target)
        {
            return data;
        }
    }return {};
}

enum IdentityFlags : uint32_t
{
    INVALID_EHANDLE = 1 << 0,
    SPAWN_IN_PROGRESS = 1 << 1,
    STAGING_LIST = 1 << 2,
    POST_DATA_UPDATE = 1 << 3,
    DELETE_IN_PROGRESS = 1 << 4,
    STASIS = 1 << 5,
    ISOLATED_ALLOCATION_NETWORKABLE = 1 << 6,
    DORMANT = 1 << 7,
    PRE_SPAWN = 1 << 8,
    MARKED_FOR_DELETE = 1 << 9,
    CONSTRUCTION_IN_PROGRESS = 1 << 10,
    ISOLATED_ALLOCATION = 1 << 11,
    HAS_BEEN_UNSERIALIZED = 1 << 12,
    SUSPENDED = 1 << 13,
    ANONYMOUS_ALLOCATION = 1 << 14,
    SUSPEND_OUTSIDE_PVS = 1 << 15,
};

int main()
{
    //uintptr_t econDesc = mem.Read<uintptr_t>(GetActiveWeapon(GetLocalPlayer()) + 0x13D0);
    //
    //uintptr_t block = Alloc(GetLocalPlayer(), 32);
    //std::cout << std::hex << block << std::endl;
    //std::cout << std::dec << GetBlockSize(block) << std::endl;
    //Free(block);
    //

    mem.Write<uint16_t>(Sigs::RegenerateWeaponSkins + 0x52, Offsets::m_AttributeManager + Offsets::m_Item + Offsets::m_AttributeList + Offsets::m_Attributes);

    // Pre-allocate model path string in game memory (persists for session)
    uintptr_t modelPathAlloc = 0;
    uintptr_t setModelFn = 0;
    bool setModelScanned = false;

    skindb->Dump();

    BaseConfig::ApplyBaseConfig();

    InitMenu();
    
    std::cout << "SkinChanger Started\n";

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

            // --- KNIFE HANDLING (processed every frame, not gated by iItemIDHigh) ---
            if (BaseConfig::IsKnife(weaponDefRaw))
            {
                const auto& knifeConfig = (team == 3) ? BaseConfig::CtKnifeConfig : BaseConfig::TKnifeConfig;

                if (weaponDefRaw != knifeConfig.defIndex || ForceUpdate)
                {
                    std::cout << "[Knife] Team=" << (int)team
                              << " OrigDef=" << weaponDefRaw
                              << " -> TargetDef=" << knifeConfig.defIndex
                              << " Paint=" << knifeConfig.paint << std::endl;

                    mem.Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, knifeConfig.defIndex);
                    mem.Write<uint32_t>(item + Offsets::m_iItemIDHigh, -1);
                    mem.Write<uint32_t>(weapon + Offsets::m_nFallbackPaintKit, knifeConfig.paint);
                    mem.Write<uint32_t>(weapon + Offsets::m_nFallbackSeed, 0);
                    mem.Write<float>(weapon + Offsets::m_flFallbackWear, 0.0001f);
                    mem.Write<int32_t>(item + Offsets::m_iEntityQuality, 3);
                    mem.Write<uint32_t>(item + Offsets::m_iAccountID, 1);

                    econItemAttributeManager.Create(item, SkinInfo_t{ static_cast<int>(knifeConfig.paint), false, knifeConfig.name, WeaponsEnum::CtKnife });

                    // --- FIND SetModel VIA VTABLE (once) ---
                    if (!setModelScanned)
                    {
                        setModelScanned = true;
                        const uintptr_t vtable = mem.Read<uintptr_t>(weapon);
                        std::cout << "[VtableScan] Weapon vtable: 0x" << std::hex << vtable << std::dec << std::endl;

                        if (vtable)
                        {
                            const uintptr_t clientBase = mem.GetModuleBase(L"client.dll");
                            std::cout << "[VtableScan] client.dll base: 0x" << std::hex << clientBase << std::dec << std::endl;

                            // Scan vtable indices 0-60, print first 8 bytes of each function
                            // SetModel typically has prologue: 48 89 5C 24 XX ... (push rbx pattern)
                            // or: 40 53 48 83 EC XX (push rbx, sub rsp)
                            // and takes 2 args (this + const char* modelName)
                            std::cout << "[VtableScan] Dumping vtable indices 0-60:" << std::endl;
                            for (int idx = 0; idx <= 60; idx++)
                            {
                                uintptr_t fn = mem.GetVtableFunc(vtable, idx);
                                if (!fn || fn < clientBase) continue;

                                // Read first 8 bytes of function
                                uint8_t bytes[8] = {};
                                for (int b = 0; b < 8; b++)
                                    bytes[b] = mem.Read<uint8_t>(fn + b);

                                // Print index and bytes
                                std::cout << "[VtableScan] [" << idx << "] 0x" << std::hex << fn << " = ";
                                for (int b = 0; b < 8; b++)
                                    std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)bytes[b] << " ";
                                std::cout << std::dec << std::endl;

                                // Heuristic: SetModel has signature like:
                                // 48 89 5C 24 XX 48 89 74 (save regs, 2+ args)
                                // 48 89 5C 24 XX 57 48 83 (save regs, sub rsp)
                                // 48 89 5C 24 XX 48 89 6C (save regs)
                                // 40 53 48 83 EC XX 48 8B (push rbx, allocate, move this)
                                if ((bytes[0] == 0x48 && bytes[1] == 0x89 && bytes[2] == 0x5C && bytes[3] == 0x24) ||
                                    (bytes[0] == 0x40 && bytes[1] == 0x53 && bytes[2] == 0x48 && bytes[3] == 0x83))
                                {
                                    // Candidate — check if it references model-related code
                                    // For now mark the FIRST match around index 24-30 as likely SetModel
                                    if (idx >= 20 && idx <= 35 && !setModelFn)
                                    {
                                        setModelFn = fn;
                                        std::cout << "[VtableScan] >>> CANDIDATE SetModel at index " << idx
                                                  << " = 0x" << std::hex << fn << std::dec << std::endl;
                                    }
                                }
                            }

                            if (!setModelFn)
                                std::cout << "[VtableScan] No SetModel candidate found in indices 20-35" << std::endl;
                        }
                    }

                    // --- MODEL INJECTION ---
                    if (setModelFn)
                    {
                        std::cout << "[ModelInject] Using SetModel at 0x" << std::hex << setModelFn << std::dec << std::endl;
                        std::cout << "[ModelInject] Model: " << knifeConfig.modelPath << std::endl;

                        // Allocate model path in game memory (once)
                        if (!modelPathAlloc)
                        {
                            modelPathAlloc = mem.Allocate(0, 512);
                            std::cout << "[ModelInject] Path buffer: 0x" << std::hex << modelPathAlloc << std::dec << std::endl;
                        }

                        if (modelPathAlloc)
                        {
                            std::string modelStr(knifeConfig.modelPath);
                            mem.WriteString(modelPathAlloc, modelStr);
                            mem.Write<char>(modelPathAlloc + modelStr.size(), '\0');

                            // x64 shellcode: SetModel(this=weapon, path=modelPathAlloc)
                            std::vector<uint8_t> sc;
                            // sub rsp, 0x28
                            sc.push_back(0x48); sc.push_back(0x83); sc.push_back(0xEC); sc.push_back(0x28);
                            // mov rcx, weapon
                            sc.push_back(0x48); sc.push_back(0xB9);
                            for (int i = 0; i < 8; i++) sc.push_back((uint8_t)((weapon >> (i*8)) & 0xFF));
                            // mov rdx, modelPathAlloc
                            sc.push_back(0x48); sc.push_back(0xBA);
                            for (int i = 0; i < 8; i++) sc.push_back((uint8_t)((modelPathAlloc >> (i*8)) & 0xFF));
                            // mov rax, setModelFn
                            sc.push_back(0x48); sc.push_back(0xB8);
                            for (int i = 0; i < 8; i++) sc.push_back((uint8_t)((setModelFn >> (i*8)) & 0xFF));
                            // call rax
                            sc.push_back(0xFF); sc.push_back(0xD0);
                            // add rsp, 0x28
                            sc.push_back(0x48); sc.push_back(0x83); sc.push_back(0xC4); sc.push_back(0x28);
                            // xor eax, eax; ret
                            sc.push_back(0x31); sc.push_back(0xC0); sc.push_back(0xC3);

                            uintptr_t code = mem.MakeFunction(sc, setModelFn);
                            std::cout << "[ModelInject] Shellcode at 0x" << std::hex << code << std::dec << std::endl;

                            if (code)
                            {
                                std::cout << "[ModelInject] Calling via CreateRemoteThread..." << std::endl;
                                mem.CallThread(code);
                                std::cout << "[ModelInject] Done!" << std::endl;
                                mem.Free(code);
                            }
                        }
                    }
                    else if (setModelScanned)
                    {
                        std::cout << "[ModelInject] SKIP: no SetModel found (check vtable dump above)" << std::endl;
                    }

                    ShouldUpdate = true;
                }
                continue; // skip normal skin flow for knives
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

        // --- GLOVE APPLICATION (check def index, not iItemIDHigh) ---
        {
            const auto& gloveConfig = (team == 3) ? BaseConfig::CtGloveConfig : BaseConfig::TGloveConfig;

            const uintptr_t glovesItem = localPlayer + Offsets::m_EconGloves;
            const uint16_t currentGloveDef = mem.Read<uint16_t>(glovesItem + Offsets::m_iItemDefinitionIndex);

            if (currentGloveDef != gloveConfig.defIndex || ForceUpdate)
            {
                std::cout << "[Gloves] Team=" << (int)team
                          << " CurrentDef=" << currentGloveDef
                          << " -> TargetDef=" << gloveConfig.defIndex
                          << " Paint=" << gloveConfig.paint << std::endl;

                mem.Write<uint16_t>(glovesItem + Offsets::m_iItemDefinitionIndex, gloveConfig.defIndex);
                mem.Write<uint32_t>(glovesItem + Offsets::m_iItemIDHigh, -1);
                mem.Write<bool>(glovesItem + Offsets::m_bInitialized, true);
                mem.Write<int32_t>(glovesItem + Offsets::m_iEntityQuality, 4);
                mem.Write<uint32_t>(glovesItem + Offsets::m_iAccountID, 1);

                econItemAttributeManager.Create(glovesItem, SkinInfo_t{ static_cast<int>(gloveConfig.paint), false, gloveConfig.name, WeaponsEnum::none });

                mem.Write<bool>(localPlayer + Offsets::m_bNeedToReApplyGloves, true);
                std::cout << "[Gloves] Wrote def=" << gloveConfig.defIndex
                          << " paint=" << gloveConfig.paint
                          << " NeedToReApply=true" << std::endl;
                ShouldUpdate = true;
            }
        }

        if (ShouldUpdate || ForceUpdate)
            UpdateWeapons(weapons);

        ForceUpdate = false;
    }
    
    return 0;
}