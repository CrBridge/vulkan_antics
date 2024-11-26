#pragma once

#include "va_device.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace va {

    class VaDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(VaDevice& vaDevice) : vaDevice{ vaDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<VaDescriptorSetLayout> build() const;

        private:
            VaDevice& vaDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        VaDescriptorSetLayout(
            VaDevice& vaDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~VaDescriptorSetLayout();
        VaDescriptorSetLayout(const VaDescriptorSetLayout&) = delete;
        VaDescriptorSetLayout& operator=(const VaDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        VaDevice& vaDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class VaDescriptorWriter;
    };

    class VaDescriptorPool {
    public:
        class Builder {
        public:
            Builder(VaDevice& vaDevice) : vaDevice{ vaDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<VaDescriptorPool> build() const;

        private:
            VaDevice& vaDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        VaDescriptorPool(
            VaDevice& vaDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~VaDescriptorPool();
        VaDescriptorPool(const VaDescriptorPool&) = delete;
        VaDescriptorPool& operator=(const VaDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

        VkDescriptorPool getDescriptorPool() const { return descriptorPool; }

    private:
        VaDevice& vaDevice;
        VkDescriptorPool descriptorPool;

        friend class VaDescriptorWriter;
    };

    class VaDescriptorWriter {
    public:
        VaDescriptorWriter(VaDescriptorSetLayout& setLayout, VaDescriptorPool& pool);

        VaDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        VaDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        VaDescriptorSetLayout& setLayout;
        VaDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };
}