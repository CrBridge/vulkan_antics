#include "va_buffer.hpp"

#include <cassert>
#include <cstring>

namespace va {

    VkDeviceSize VaBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
        if (minOffsetAlignment > 0) {
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        }
        return instanceSize;
    }

    VaBuffer::VaBuffer(
        VaDevice& device,
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize minOffsetAlignment)
        : lveDevice{ device },
        instanceSize{ instanceSize },
        instanceCount{ instanceCount },
        usageFlags{ usageFlags },
        memoryPropertyFlags{ memoryPropertyFlags } {
        alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
        bufferSize = alignmentSize * instanceCount;
        device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
    }

    VaBuffer::~VaBuffer() {
        unmap();
        vkDestroyBuffer(lveDevice.device(), buffer, nullptr);
        vkFreeMemory(lveDevice.device(), memory, nullptr);
    }

    VkResult VaBuffer::map(VkDeviceSize size, VkDeviceSize offset) {
        assert(buffer && memory && "Called map on buffer before create");
        return vkMapMemory(lveDevice.device(), memory, offset, size, 0, &mapped);
    }

    void VaBuffer::unmap() {
        if (mapped) {
            vkUnmapMemory(lveDevice.device(), memory);
            mapped = nullptr;
        }
    }

    void VaBuffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset) {
        assert(mapped && "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE) {
            memcpy(mapped, data, bufferSize);
        }
        else {
            char* memOffset = (char*)mapped;
            memOffset += offset;
            memcpy(memOffset, data, size);
        }
    }

    VkResult VaBuffer::flush(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(lveDevice.device(), 1, &mappedRange);
    }

    VkResult VaBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(lveDevice.device(), 1, &mappedRange);
    }

    VkDescriptorBufferInfo VaBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        return VkDescriptorBufferInfo{
            buffer,
            offset,
            size,
        };
    }

    void VaBuffer::writeToIndex(void* data, int index) {
        writeToBuffer(data, instanceSize, index * alignmentSize);
    }

    VkResult VaBuffer::flushIndex(int index) { return flush(alignmentSize, index * alignmentSize); }

    VkDescriptorBufferInfo VaBuffer::descriptorInfoForIndex(int index) {
        return descriptorInfo(alignmentSize, index * alignmentSize);
    }

    VkResult VaBuffer::invalidateIndex(int index) {
        return invalidate(alignmentSize, index * alignmentSize);
    }

}