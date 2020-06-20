// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "common/assert.h"
#include "core/core.h"
#include "core/hle/kernel/errors.h"
#include "core/hle/kernel/kernel.h"
#include "core/hle/kernel/memory/page_table.h"
#include "core/hle/kernel/shared_memory.h"

namespace Kernel {

SharedMemory::SharedMemory(KernelCore& kernel, Core::DeviceMemory& device_memory)
    : Object{kernel}, device_memory{device_memory} {}

SharedMemory::~SharedMemory() = default;

std::shared_ptr<SharedMemory> SharedMemory::Create(
    KernelCore& kernel, Core::DeviceMemory& device_memory, Process* owner_process,
    Memory::PageLinkedList&& page_list, Memory::MemoryPermission owner_permission,
    Memory::MemoryPermission user_permission, PAddr physical_address, std::size_t size,
    std::string name) {

    std::shared_ptr<SharedMemory> shared_memory{
        std::make_shared<SharedMemory>(kernel, device_memory)};

    shared_memory->owner_process = owner_process;
    shared_memory->page_list = std::move(page_list);
    shared_memory->owner_permission = owner_permission;
    shared_memory->user_permission = user_permission;
    shared_memory->physical_address = physical_address;
    shared_memory->size = size;
    shared_memory->name = name;

    return shared_memory;
}

ResultCode SharedMemory::Map(Process& target_process, VAddr address, std::size_t size,
                             Memory::MemoryPermission permissions) {
    const u64 page_count{(size + Memory::PageSize - 1) / Memory::PageSize};

    if (page_list.GetNumPages() != page_count) {
        UNIMPLEMENTED_MSG("Page count does not match");
    }

    const Memory::MemoryPermission expected =
        &target_process == owner_process ? owner_permission : user_permission;

    if (permissions != expected) {
        UNIMPLEMENTED_MSG("Permission does not match");
    }

    return target_process.PageTable().MapPages(address, page_list, Memory::MemoryState::Shared,
                                               permissions);
}

ResultCode SharedMemory::Unmap(Process& target_process, VAddr address, u64 unmap_size) {
    if (unmap_size != size) {
        LOG_ERROR(Kernel,
                  "Invalid size passed to Unmap. Size must be equal to the size of the "
                  "memory managed. Shared memory size=0x{:016X}, Unmap size=0x{:016X}",
                  size, unmap_size);
        return ERR_INVALID_SIZE;
    }

    return target_process.PageTable().UnmapMemory(address, size);
}

} // namespace Kernel
