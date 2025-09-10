#include "PipelineBuilder.h"
#include <cassert>

PipelineBuilder::PipelineBuilder(VkDevice device)
    : device(device){
    // 初始化默认状态
    reset();
}

PipelineBuilder::~PipelineBuilder() {
    
}

PipelineBuilder& PipelineBuilder::setInputAssemblyState(
    VkPrimitiveTopology topology, VkPipelineInputAssemblyStateCreateFlags flags, VkBool32 primitiveRestartEnable) {
    inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(
        topology, flags, primitiveRestartEnable);
    return *this;
}

PipelineBuilder& PipelineBuilder::setRasterizationState(
    VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace, VkPipelineRasterizationStateCreateFlags flags) {
    rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(
        polygonMode, cullMode, frontFace, flags);
    return *this;
}

PipelineBuilder& PipelineBuilder::setColorBlendAttachmentState(
    uint32_t colorWriteMask, VkBool32 blendEnable) {
    blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(
        colorWriteMask, blendEnable);
    colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(
        1, &blendAttachmentState);
    return *this;
}

PipelineBuilder& PipelineBuilder::setDepthStencilState(
    VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp) {
    depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(
        depthTestEnable, depthWriteEnable, depthCompareOp);
    return *this;
}

PipelineBuilder& PipelineBuilder::setViewportState(
    uint32_t viewportCount, uint32_t scissorCount, VkPipelineViewportStateCreateFlags flags) {
    viewportState = vks::initializers::pipelineViewportStateCreateInfo(
        viewportCount, scissorCount, flags);
    return *this;
}

PipelineBuilder& PipelineBuilder::setMultisampleState(
    VkSampleCountFlagBits rasterizationSamples, VkPipelineMultisampleStateCreateFlags flags) {
    multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(
        rasterizationSamples, flags);
    return *this;
}

PipelineBuilder& PipelineBuilder::setDynamicStates(
    const std::vector<VkDynamicState>& dynamicStates, VkPipelineDynamicStateCreateFlags flags) {
    dynamicStateEnables = dynamicStates;
    dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables, flags);
    return *this;
}

PipelineBuilder& PipelineBuilder::setVertexInputState(
    VkPipelineVertexInputStateCreateInfo* state) {
    vertexInputState = state;
    return *this;
}

PipelineBuilder& PipelineBuilder::addShaderStage(const VkPipelineShaderStageCreateInfo stage) {
    shaderStages.push_back(stage);
    return *this;
}

void PipelineBuilder::clearShaderStage() {
    shaderStages.clear();
}

VkResult PipelineBuilder::buildPipeline(VkRenderPass& renderPass, VkPipelineCache& pipelineCache, VkPipelineLayout& pipelineLayout, VkPipeline& outPipeline) {
    // 准备管线创建信息
    VkGraphicsPipelineCreateInfo pipelineCI =
        vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass);

    // 设置所有管线状态
    pipelineCI.pInputAssemblyState = &inputAssemblyState;
    pipelineCI.pRasterizationState = &rasterizationState;
    pipelineCI.pColorBlendState = &colorBlendState;
    pipelineCI.pMultisampleState = &multisampleState;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pDepthStencilState = &depthStencilState;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCI.pStages = shaderStages.data();
    pipelineCI.pVertexInputState = vertexInputState;

    // 创建管线
    return vkCreateGraphicsPipelines(
        device,
        pipelineCache,
        1,
        &pipelineCI,
        nullptr,
        &outPipeline
    );
}

void PipelineBuilder::reset() {
    // 重置所有状态为默认值
    setInputAssemblyState();
    setRasterizationState();
    setColorBlendAttachmentState();
    setDepthStencilState();
    setViewportState();
    setMultisampleState();
    setDynamicStates();
    setVertexInputState();

    // 清除着色器阶段和描述符集布局
    shaderStages.clear();
}
