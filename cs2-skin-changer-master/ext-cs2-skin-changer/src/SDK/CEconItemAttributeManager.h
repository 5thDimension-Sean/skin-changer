#include <iostream>
#include <unordered_map>
#include <vector>

#include "../../ext/skindb.h"

#pragma once

class CPtrGameVector
{
public:
	uint64_t size;
	uintptr_t ptr;
};

class CEconItemAttribute
{
public:
	uintptr_t vtable; //0x0000
	uintptr_t owner; //0x0008
	char pad_0010[32]; //0x0010
	uint16_t defIndex; //0x0030
	char pad_0032[2]; //0x0032
	float value; //0x0034
	float initValue; //0x0038
	int32_t refundableCurrency; //0x003C
	bool setBonus; //0x0040
	char pad_0041[7]; //0x0041
}; //Size: 0x0048

enum ItemAttributeDefinitions
{
	Default = 0,
	Paint = 6,
	Pattern = 7,
	Wear = 8,
};

#define MaxEconItemAttributeManager 10
class CEconItemAttributeManager
{
private:
	std::unordered_map<uintptr_t, uintptr_t> allocations;

	template <typename T>
	CEconItemAttribute Make(const uint16_t def, const T value)
	{
		float fValue = (float)value;

		CEconItemAttribute attribute{};
		attribute.defIndex = def;
		attribute.initValue = fValue;
		attribute.value = fValue;

		return attribute;
	}

public:
	uint8_t GetSize(const uintptr_t AttributeList)
	{
		return mem.Read<uint64_t>(AttributeList + Offsets::m_Attributes);
	}

	void Create(const uintptr_t item, const SkinInfo_t skin)
	{
		std::vector<CEconItemAttribute> attributes;

		if (skin.Paint)
		{
			attributes.push_back(Make(ItemAttributeDefinitions::Paint, skin.Paint));
			attributes.push_back(Make(ItemAttributeDefinitions::Pattern, 0));
			attributes.push_back(Make(ItemAttributeDefinitions::Wear, 0.01f));
		}
			
		//if (skin.pattern)
		//	attributes.push_back(Make(ItemAttributeDefinitions::Pattern, skin.pattern));
		//
		//if (skin.wear)
		//	attributes.push_back(Make(ItemAttributeDefinitions::Wear, skin.wear));

		const CPtrGameVector preAttributes = mem.Read<CPtrGameVector>(item + Offsets::m_AttributeList + Offsets::m_Attributes);

		if (attributes.empty() || preAttributes.size || preAttributes.ptr)
			return;// dont want to get a mem block for nothing
		
		const uintptr_t memBlock = mem.Allocate();
		if (!memBlock)
			return;

		for (uint8_t i = 0; i < attributes.size(); i++)
		{
			mem.Write<CEconItemAttribute>(memBlock + (i * sizeof(CEconItemAttribute)), attributes[i]);
		}

		CPtrGameVector Attributes;
		Attributes.size = attributes.size();
		Attributes.ptr = memBlock;

		mem.Write<CPtrGameVector>(item + Offsets::m_AttributeList + Offsets::m_Attributes, Attributes);
		allocations[item] = memBlock;

		std::cout << "[Attributes] created item=0x" << std::hex << item
				  << " block=0x" << memBlock << std::dec
				  << " count=" << attributes.size()
				  << " paint=" << skin.Paint << std::endl;
	}

	void Replace(const uintptr_t item, const SkinInfo_t skin)
	{
		std::vector<CEconItemAttribute> attributes;
		if (skin.Paint)
		{
			attributes.push_back(Make(ItemAttributeDefinitions::Paint, skin.Paint));
			attributes.push_back(Make(ItemAttributeDefinitions::Pattern, 0));
			attributes.push_back(Make(ItemAttributeDefinitions::Wear, 0.01f));
		}

		if (attributes.empty())
			return;

		const uintptr_t memBlock = mem.Allocate();
		if (!memBlock)
		{
			std::cout << "[Attributes] replace allocation failed item=0x"
					  << std::hex << item << std::dec << std::endl;
			return;
		}

		for (uint8_t i = 0; i < attributes.size(); i++)
			mem.Write<CEconItemAttribute>(memBlock + (i * sizeof(CEconItemAttribute)), attributes[i]);

		const uintptr_t attributeAddress = item + Offsets::m_AttributeList + Offsets::m_Attributes;
		const CPtrGameVector previous = mem.Read<CPtrGameVector>(attributeAddress);
		mem.Write<CPtrGameVector>(attributeAddress, CPtrGameVector{ attributes.size(), memBlock });

		const auto allocation = allocations.find(item);
		bool freedPrevious = false;
		if (allocation != allocations.end() && allocation->second == previous.ptr)
			freedPrevious = mem.Free(previous.ptr);

		allocations[item] = memBlock;
		std::cout << "[Attributes] replaced item=0x" << std::hex << item
				  << " oldBlock=0x" << previous.ptr
				  << " newBlock=0x" << memBlock << std::dec
				  << " oldCount=" << previous.size
				  << " newCount=" << attributes.size()
				  << " paint=" << skin.Paint
				  << " freedOwnedOld=" << freedPrevious << std::endl;
	}

	int GetPaint(const uintptr_t item)
	{
		const CPtrGameVector attributes =
			mem.Read<CPtrGameVector>(item + Offsets::m_AttributeList + Offsets::m_Attributes);
		for (uint64_t i = 0; i < attributes.size && i < MaxEconItemAttributeManager; ++i)
		{
			const CEconItemAttribute attribute =
				mem.Read<CEconItemAttribute>(attributes.ptr + (i * sizeof(CEconItemAttribute)));
			if (attribute.defIndex == ItemAttributeDefinitions::Paint)
				return static_cast<int>(attribute.value);
		}

		return 0;
	}

	void Remove(const uintptr_t item)
	{
		const uintptr_t AttributeList = item + Offsets::m_AttributeList + Offsets::m_Attributes;
		const CPtrGameVector attributes = mem.Read<CPtrGameVector>(AttributeList);
		if (!attributes.size) return;

		const auto allocation = allocations.find(item);
		if (allocation == allocations.end() || allocation->second != attributes.ptr)
			return;

		mem.Write<CPtrGameVector>(AttributeList, CPtrGameVector());

		mem.Free(attributes.ptr);
		allocations.erase(allocation);
	}
};
CEconItemAttributeManager econItemAttributeManager;