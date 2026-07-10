#include "menu.h"
#include "baseConfig.h"
#include "SDK/entity/CWeaponServices.h"
#include "SDK/weapon/C_EconEntity.h"
#include "SDK/CEconItemAttributeManager.h"

#include <array>

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

    // Sig scan for SetModel — used for knife model injection
    // CBaseModelEntity::SetModel typically starts with a distinctive prologue
    uintptr_t setModelFn = mem.SigScan(L"client.dll", "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC 20 48 8B E9 48 8B FA");
    std::cout << "[Init] SetModel sig scan: 0x" << std::hex << setModelFn << std::dec << std::endl;
    if (!setModelFn)
    {
        // Try alternate sig pattern
        setModelFn = mem.SigScan(L"client.dll", "40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B C8 48 8B D3");
        std::cout << "[Init] SetModel alt sig: 0x" << std::hex << setModelFn << std::dec << std::endl;
    }

    // Pre-allocate model path string in game memory (persists for the session)
    uintptr_t modelPathAlloc = 0;

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
        static bool knifeModelInjected = false;

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
                              << " Def=" << weaponDefRaw << "->" << knifeConfig.defIndex
                              << " Paint=" << knifeConfig.paint
                              << " Entity=0x" << std::hex << weapon << std::dec << std::endl;

                    // Write item definition + fallback values
                    mem.Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, knifeConfig.defIndex);
                    mem.Write<uint32_t>(item + Offsets::m_iItemIDHigh, -1);
                    mem.Write<uint32_t>(weapon + Offsets::m_nFallbackPaintKit, knifeConfig.paint);
                    mem.Write<uint32_t>(weapon + Offsets::m_nFallbackSeed, 0);
                    mem.Write<float>(weapon + Offsets::m_flFallbackWear, 0.0001f);
                    mem.Write<int32_t>(item + Offsets::m_iEntityQuality, 3);
                    mem.Write<uint32_t>(item + Offsets::m_iAccountID, 1);
                    mem.Write<uint32_t>(weapon + Offsets::m_OriginalOwnerXuidLow, 1);

                    std::cout << "[Knife] Wrote item data OK" << std::endl;

                    econItemAttributeManager.Create(item, SkinInfo_t{ static_cast<int>(knifeConfig.paint), false, knifeConfig.name, WeaponsEnum::CtKnife });

                    // --- MODEL INJECTION via SetModel (CreateRemoteThread) ---
                    if (setModelFn && !knifeModelInjected)
                    {
                        std::cout << "[ModelInject] Attempting SetModel injection..." << std::endl;
                        std::cout << "[ModelInject] SetModel func: 0x" << std::hex << setModelFn << std::dec << std::endl;
                        std::cout << "[ModelInject] Model path: " << knifeConfig.modelPath << std::endl;

                        // Allocate model path string in game memory (once)
                        if (!modelPathAlloc)
                        {
                            modelPathAlloc = mem.Allocate(0, 512);
                            std::cout << "[ModelInject] Allocated path buffer: 0x" << std::hex << modelPathAlloc << std::dec << std::endl;
                        }

                        if (modelPathAlloc)
                        {
                            // Write model path string + null terminator
                            std::string modelStr(knifeConfig.modelPath);
                            mem.WriteString(modelPathAlloc, modelStr);
                            mem.Write<char>(modelPathAlloc + modelStr.size(), '\0');

                            // Build x64 shellcode to call SetModel(this=weapon, modelPath)
                            // Windows x64 calling convention: rcx=this, rdx=arg1
                            std::vector<uint8_t> shellcode;

                            // sub rsp, 0x28 (shadow space + alignment)
                            shellcode.push_back(0x48); shellcode.push_back(0x83);
                            shellcode.push_back(0xEC); shellcode.push_back(0x28);

                            // mov rcx, <weapon address> (10 bytes: 48 B9 + imm64)
                            shellcode.push_back(0x48); shellcode.push_back(0xB9);
                            for (int i = 0; i < 8; i++)
                                shellcode.push_back((uint8_t)((weapon >> (i * 8)) & 0xFF));

                            // mov rdx, <modelPathAlloc> (10 bytes: 48 BA + imm64)
                            shellcode.push_back(0x48); shellcode.push_back(0xBA);
                            for (int i = 0; i < 8; i++)
                                shellcode.push_back((uint8_t)((modelPathAlloc >> (i * 8)) & 0xFF));

                            // mov rax, <setModelFn> (10 bytes: 48 B8 + imm64)
                            shellcode.push_back(0x48); shellcode.push_back(0xB8);
                            for (int i = 0; i < 8; i++)
                                shellcode.push_back((uint8_t)((setModelFn >> (i * 8)) & 0xFF));

                            // call rax (2 bytes: FF D0)
                            shellcode.push_back(0xFF); shellcode.push_back(0xD0);

                            // add rsp, 0x28
                            shellcode.push_back(0x48); shellcode.push_back(0x83);
                            shellcode.push_back(0xC4); shellcode.push_back(0x28);

                            // xor eax, eax (return 0)
                            shellcode.push_back(0x31); shellcode.push_back(0xC0);

                            // ret
                            shellcode.push_back(0xC3);

                            // Allocate and write shellcode near SetModel for cache locality
                            uintptr_t codeAddr = mem.MakeFunction(shellcode, setModelFn);
                            std::cout << "[ModelInject] Shellcode at: 0x" << std::hex << codeAddr << std::dec
                                      << " (" << shellcode.size() << " bytes)" << std::endl;

                            if (codeAddr)
                            {
                                std::cout << "[ModelInject] Executing via CreateRemoteThread..." << std::endl;
                                mem.CallThread(codeAddr);
                                std::cout << "[ModelInject] SetModel call completed!" << std::endl;
                                knifeModelInjected = true;
                                mem.Free(codeAddr);
                            }
                            else
                            {
                                std::cout << "[ModelInject] ERROR: Failed to allocate shellcode" << std::endl;
                            }
                        }
                    }
                    else if (!setModelFn)
                    {
                        std::cout << "[ModelInject] SKIP: SetModel sig not found (model won't change)" << std::endl;
                    }

                    ShouldUpdate = true;
                }
                else
                {
                    // Def already matches — model should be applied
                    if (knifeModelInjected)
                    {
                        std::cout << "[Knife] Model injection active, def stable at " << knifeConfig.defIndex << std::endl;
                        knifeModelInjected = false; // don't spam this message
                    }
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
                          << " Paint=" << gloveConfig.paint
                          << " Pawn=0x" << std::hex << localPlayer << std::dec << std::endl;

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
        {
            std::cout << "[Update] Calling RegenerateWeaponSkins via CreateRemoteThread..." << std::endl;
            UpdateWeapons(weapons);
            std::cout << "[Update] RegenerateWeaponSkins completed" << std::endl;
        }

        ForceUpdate = false;
    }
    
    return 0;
}