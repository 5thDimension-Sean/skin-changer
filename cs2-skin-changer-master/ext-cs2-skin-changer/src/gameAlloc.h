#pragma once
#include "../ext/mem.h"

uintptr_t GetMetaData(uintptr_t block)
{
    return (block - 1) & ~0x1FFFFFFULL;
}

uintptr_t GetDescriptor(uintptr_t block)
{
    uintptr_t metaData = GetMetaData(block);

    size_t pageIndex = (block - metaData) >> 16;

    return metaData + 96 * (pageIndex + 3);//96 0x60
}

/*
* Descriptor:
* 0x18 Free List
* 0x20 Allocated Elements Count
* 0x28 Block Size
*/

size_t GetBlockSize(uintptr_t block)
{
    uintptr_t descriptor = GetDescriptor(block);
    return mem.Read<uint32_t>(descriptor + 0x28);
}

uintptr_t Alloc(uintptr_t existingHeapPtr, size_t size)
{
    if (!size)
        return NULL;

    uintptr_t bestDescriptor = NULL;

    uintptr_t metaData = GetMetaData(existingHeapPtr);
    for (size_t i = 0; i < 512; i++)
    {
        uintptr_t descriptor = metaData + ((i + 3) * 0x60);

        uintptr_t freeList = mem.Read<uintptr_t>(descriptor + 0x18);
        if (!freeList)
            continue;

        uint32_t BlockSize = mem.Read<uint32_t>(descriptor + 0x28);
        if (size > BlockSize)
            continue;

        if (!bestDescriptor ||
            BlockSize < mem.Read<uint32_t>(bestDescriptor + 0x28))
        {
            bestDescriptor = descriptor;
        }
    }

    if (bestDescriptor)
    {
        uintptr_t freeList = mem.Read<uintptr_t>(bestDescriptor + 0x18);
        if (!freeList)
            return NULL;

        uint32_t BlockSize = mem.Read<uint32_t>(bestDescriptor + 0x28);

        mem.Write<uintptr_t>(bestDescriptor + 0x18, mem.Read<uintptr_t>(freeList));
        mem.Write<uint32_t>(bestDescriptor + 0x20, mem.Read<uint32_t>(bestDescriptor + 0x20) + 1);//Clear8 just nulls the mem 8 bytes at a time
        mem.Clear8(freeList, BlockSize);
        return freeList;
    }

    return NULL;
}

void Free(uintptr_t block)
{
    if (!block)
        return;

    uintptr_t descriptor = GetDescriptor(block);
    uintptr_t freeList = mem.Read<uintptr_t>(descriptor + 0x18);

    mem.Write<uintptr_t>(block, freeList);
    mem.Write<uintptr_t>(descriptor + 0x18, block);
    mem.Write<uint32_t>(descriptor + 0x20, mem.Read<uint32_t>(descriptor + 0x20) - 1);
}