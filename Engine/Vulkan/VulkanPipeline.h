namespace Engine
{
	class VulkanPipeline
	{
	private:
        void createRenderPass()
        {
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = swapChainImageFormat;
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentDescription depthAttachment{};
            depthAttachment.format = findDepthFormat();
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depthAttachmentRef{};
            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;
            subpass.pDepthStencilAttachment = &depthAttachmentRef;

            VkSubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
            VkRenderPassCreateInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            renderPassInfo.pAttachments = attachments.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            renderPassInfo.dependencyCount = 1;
            renderPassInfo.pDependencies = &dependency;

            if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create render pass!");
            }
        }

        void createDescriptorSetLayout()
        {
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding = 1;
            samplerLayoutBinding.descriptorCount = 1;
            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers = nullptr;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

            VkDescriptorSetLayoutBinding samplerLayoutBinding2{};
            samplerLayoutBinding2.binding = 2;
            samplerLayoutBinding2.descriptorCount = 1;
            samplerLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            samplerLayoutBinding2.pImmutableSamplers = nullptr;
            samplerLayoutBinding2.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

            std::array<VkDescriptorSetLayoutBinding, 3> bindings = { uboLayoutBinding, samplerLayoutBinding, samplerLayoutBinding2 };
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            layoutInfo.pBindings = bindings.data();

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }

        void createGraphicsPipeline()
        {
            auto vertShaderCode = readFile("Engine/Shaders/vert.spv");
            auto fragShaderCode = readFile("Engine/Shaders/frag.spv");

            VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
            VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = vertShaderModule;
            vertShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = fragShaderModule;
            fragShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();

            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.scissorCount = 1;

            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineDepthStencilStateCreateInfo depthStencil{};
            depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencil.depthTestEnable = VK_TRUE;
            depthStencil.depthWriteEnable = VK_TRUE;
            depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
            depthStencil.depthBoundsTestEnable = VK_FALSE;
            depthStencil.stencilTestEnable = VK_FALSE;

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo colorBlending{};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.logicOp = VK_LOGIC_OP_COPY;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment;
            colorBlending.blendConstants[0] = 0.0f;
            colorBlending.blendConstants[1] = 0.0f;
            colorBlending.blendConstants[2] = 0.0f;
            colorBlending.blendConstants[3] = 0.0f;

            std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };
            VkPipelineDynamicStateCreateInfo dynamicState{};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
            dynamicState.pDynamicStates = dynamicStates.data();

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

            if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create pipeline layout!");
            }

            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssembly;
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pMultisampleState = &multisampling;
            pipelineInfo.pDepthStencilState = &depthStencil;
            pipelineInfo.pColorBlendState = &colorBlending;
            pipelineInfo.pDynamicState = &dynamicState;
            pipelineInfo.layout = pipelineLayout;
            pipelineInfo.renderPass = renderPass;
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create graphics pipeline!");
            }

            vkDestroyShaderModule(device, fragShaderModule, nullptr);
            vkDestroyShaderModule(device, vertShaderModule, nullptr);
        }

        #pragma region Compute raytracing
        void createRayTracingImageBuffer()
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = swapChainExtent.width;
            imageInfo.extent.height = swapChainExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT; // or VK_FORMAT_R8G8B8A8_UNORM
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.flags = 0; // Optional, but safe

            auto result = vkCreateImage(device, &imageInfo, nullptr, &raytracingImage);
            if (result != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create image!");
            }

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(device, raytracingImage, &memRequirements);

            // 2. Allocate memory
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(
                memRequirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );

            if (vkAllocateMemory(device, &allocInfo, nullptr, &raytracingImageMemory) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate ray tracing image memory!");
            }

            // 3. Bind memory to the image
            vkBindImageMemory(device, raytracingImage, raytracingImageMemory, 0);
        }

        void createRayTracingImageView()
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = raytracingImage;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT; // Same format you used for the image
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &viewInfo, nullptr, &raytracingImageView) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create ray tracing image view!");
            }
        }

        void createComputeRayTracingDescriptorSetLayout()
        {
            for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                std::array<VkDescriptorSetLayoutBinding, 6> bindings{};

                // Binding 0: Storage Image
                bindings[0].binding = 0;
                bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                bindings[0].descriptorCount = 1;
                bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[0].pImmutableSamplers = nullptr;

                // Binding 1: Uniform Buffer (Camera)
                bindings[1].binding = 1;
                bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                bindings[1].descriptorCount = 1;
                bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[1].pImmutableSamplers = nullptr;

                // Binding 2: BVH buffer
                bindings[2].binding = 2;
                bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                bindings[2].descriptorCount = 1;
                bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[2].pImmutableSamplers = nullptr;

                // Binding 3: Triangle buffer
                bindings[3].binding = 3;
                bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                bindings[3].descriptorCount = 1;
                bindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[3].pImmutableSamplers = nullptr;

                // Binding 4: Instance buffer
                bindings[4].binding = 4;
                bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                bindings[4].descriptorCount = 1;
                bindings[4].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[4].pImmutableSamplers = nullptr;

                // Binding 5: Light buffer
                bindings[5].binding = 5;
                bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                bindings[5].descriptorCount = 1;
                bindings[5].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[5].pImmutableSamplers = nullptr;

                // Create layout
                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
                layoutInfo.pBindings = bindings.data();

                if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &rayTracingDescriptorSetLayout[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create descriptor set layout!");
                }
            }
        }

        void createComputeRayTracingPipelineLayout()
        {
            VkPushConstantRange range = {};
            range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            range.offset = 0;
            range.size = sizeof(PushConstants);

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 2;
            pipelineLayoutInfo.pSetLayouts = rayTracingDescriptorSetLayout;
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = &range;

            if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &rayTracingPipelineLayout) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create ray tracing pipeline layout!");
            }
        }

        void createComputeRayTracingPipeline()
        {
            auto raytracingComputeShaderCode = readFile("Engine/Shaders/compute.spv");
            VkShaderModule raytracingComputeShaderModule = createShaderModule(raytracingComputeShaderCode);

            VkPipelineShaderStageCreateInfo shaderStageInfo{};
            shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageInfo.module = raytracingComputeShaderModule;
            shaderStageInfo.pName = "main";

            VkComputePipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            pipelineInfo.stage = shaderStageInfo;
            pipelineInfo.layout = rayTracingPipelineLayout;

            auto result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &rayTracingPipeline);

            if (result != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create ray tracing compute pipeline!");
            }
        }

        void allocateComputeRayTracingPipelineBuffers()
        {
            // Create descriptor sets
            VkDescriptorSet descriptorSet;
            {
                std::vector<VkDescriptorSetLayoutBinding> bindings(6);

                // Binding 0: Storage Image
                bindings[0].binding = 0;
                bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                bindings[0].descriptorCount = 1;
                bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[0].pImmutableSamplers = nullptr;

                // Binding 1: Uniform Buffer
                bindings[1].binding = 1;
                bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                bindings[1].descriptorCount = 1;
                bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[1].pImmutableSamplers = nullptr;

                // Binding 2: BVH Nodes (Storage Buffer)
                bindings[2].binding = 2;
                bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                bindings[2].descriptorCount = 1;
                bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[2].pImmutableSamplers = nullptr;

                // Binding 3: Triangles (Storage Buffer)
                bindings[3].binding = 3;
                bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                bindings[3].descriptorCount = 1;
                bindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[3].pImmutableSamplers = nullptr;

                // Binding 4: Instances (Storage Buffer)
                bindings[4].binding = 4;
                bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                bindings[4].descriptorCount = 1;
                bindings[4].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[4].pImmutableSamplers = nullptr;

                // Binding 5: Light instances (Storage Buffer)
                bindings[5].binding = 5;
                bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                bindings[5].descriptorCount = 1;
                bindings[5].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[5].pImmutableSamplers = nullptr;

                VkDescriptorSetLayoutCreateInfo layoutInfo = {};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
                layoutInfo.pBindings = bindings.data();

                VkDescriptorSetLayout descriptorSetLayout;
                vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);

                std::array<VkDescriptorPoolSize, 3> poolSizes = { {
                    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4 },
                } };

                VkDescriptorPoolCreateInfo poolInfo = {};
                poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                poolInfo.pPoolSizes = poolSizes.data();
                poolInfo.maxSets = 1;

                VkDescriptorPool descriptorPool;
                vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);

                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = descriptorPool;
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = &descriptorSetLayout;

                vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
            }

            // Create staging buffers
            {
                createBuffer(
                    largeBufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    bvhStagingBuffer,
                    bvhStagingBufferMemory
                );

                createBuffer(
                    largeBufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    triangleStagingBuffer,
                    triangleStagingBufferMemory
                );

                createBuffer(
                    normalBufferSize,  // actualLightCount * sizeof(LightInstance)
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    lightStagingBuffer,
                    lightStagingBufferMemory
                );

                createBuffer(
                    normalBufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    instanceStagingBuffer,
                    instanceStagingBufferMemory
                );
            }

            // Write to descriptor sets
            std::array<VkWriteDescriptorSet, 6> descriptorWrites = {};
            {
                // Storage Image
                {
                    VkDescriptorImageInfo imageInfo{};
                    imageInfo.imageView = raytracingImageView;
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

                    descriptorWrites[0] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                    descriptorWrites[0].dstSet = descriptorSet;
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    descriptorWrites[0].descriptorCount = 1;
                    descriptorWrites[0].pImageInfo = &imageInfo;
                }

                // Uniform Buffer (Camera)
                {
                createBuffer(
                    sizeof(CameraUBO),
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    cameraBuffer,
                    cameraBufferMemory
                );

                if (cameraBuffer == VK_NULL_HANDLE) {
                    throw std::runtime_error("cameraBuffer is null!");
                }

                VkDescriptorBufferInfo cameraBufferInfo{};
                cameraBufferInfo.buffer = cameraBuffer;
                cameraBufferInfo.offset = 0;
                cameraBufferInfo.range = sizeof(CameraUBO);

                descriptorWrites[1] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                descriptorWrites[1].dstSet = descriptorSet;
                descriptorWrites[1].dstBinding = 1;
                descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrites[1].descriptorCount = 1;
                descriptorWrites[1].pBufferInfo = &cameraBufferInfo;
                }

                // BVH Nodes
                {
                    createBuffer(
                        largeBufferSize,  // size of your BVH data
                        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        bvhBuffer,
                        bvhBufferMemory
                    );

                    VkDescriptorBufferInfo bvhBufferInfo = {};
                    bvhBufferInfo.buffer = bvhBuffer;
                    bvhBufferInfo.offset = 0;
                    bvhBufferInfo.range = VK_WHOLE_SIZE;

                    descriptorWrites[2] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                    descriptorWrites[2].dstSet = descriptorSet;
                    descriptorWrites[2].dstBinding = 2;
                    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrites[2].descriptorCount = 1;
                    descriptorWrites[2].pBufferInfo = &bvhBufferInfo;
                }

                // Triangles
                {
                    createBuffer(
                        largeBufferSize,  // size of triangle data
                        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        triangleBuffer,
                        triangleBufferMemory
                    );

                    VkDescriptorBufferInfo triangleBufferInfo = {};
                    triangleBufferInfo.buffer = triangleBuffer;
                    triangleBufferInfo.offset = 0;
                    triangleBufferInfo.range = VK_WHOLE_SIZE;

                    descriptorWrites[3] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                    descriptorWrites[3].dstSet = descriptorSet;
                    descriptorWrites[3].dstBinding = 3;
                    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrites[3].descriptorCount = 1;
                    descriptorWrites[3].pBufferInfo = &triangleBufferInfo;
                }

                // Instances
                {
                    createBuffer(
                        normalBufferSize,  // size of instance data
                        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        instanceBuffer,
                        instanceBufferMemory
                    );

                    VkDescriptorBufferInfo instanceBufferInfo = {};
                    instanceBufferInfo.buffer = instanceBuffer;
                    instanceBufferInfo.offset = 0;
                    instanceBufferInfo.range = VK_WHOLE_SIZE;

                    descriptorWrites[4] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                    descriptorWrites[4].dstSet = descriptorSet;
                    descriptorWrites[4].dstBinding = 4;
                    descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrites[4].descriptorCount = 1;
                    descriptorWrites[4].pBufferInfo = &instanceBufferInfo;
                }

                // Lights
                {
                    createBuffer(
                        normalBufferSize,  // size of instance data
                        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        lightBuffer,
                        lightBufferMemory
                    );

                    VkDescriptorBufferInfo lightBufferInfo = {};
                    lightBufferInfo.buffer = lightBuffer;
                    lightBufferInfo.offset = 0;
                    lightBufferInfo.range = VK_WHOLE_SIZE;

                    descriptorWrites[5] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                    descriptorWrites[5].dstSet = descriptorSet;
                    descriptorWrites[5].dstBinding = 5;
                    descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrites[5].descriptorCount = 1;
                    descriptorWrites[5].pBufferInfo = &lightBufferInfo;
                }
            }

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
        #pragma endregion

        #pragma region Compositing
        void createCompositingDescriptorSetLayout()
        {
            for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                std::array<VkDescriptorSetLayoutBinding, 2> bindings{};

                // Binding 0: Ray traced storage Image
                bindings[0].binding = 0;
                bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                bindings[0].descriptorCount = 1;
                bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[0].pImmutableSamplers = nullptr;

                // Binding 1: Rasterized storage image
                bindings[1].binding = 1;
                bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                bindings[1].descriptorCount = 1;
                bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[1].pImmutableSamplers = nullptr;

                /*
                // Binding 2: Result storage image
                bindings[2].binding = 2;
                bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                bindings[2].descriptorCount = 1;
                bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[2].pImmutableSamplers = nullptr;
                */

                // Create layout
                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
                layoutInfo.pBindings = bindings.data();

                if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &compositingDescriptorSetLayout[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to create descriptor set layout!");
                }
            }
        }

        void createCompositingPipelineLayout()
        {
            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 2;
            pipelineLayoutInfo.pSetLayouts = compositingDescriptorSetLayout;

            if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &compositingPipelineLayout) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create ray tracing pipeline layout!");
            }
        }

        void createCompositingPipeline()
        {
            auto compositingShaderCode = readFile("Engine/Shaders/compositing.spv");
            VkShaderModule compositingShaderModule = createShaderModule(compositingShaderCode);

            VkPipelineShaderStageCreateInfo shaderStageInfo{};
            shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageInfo.module = compositingShaderModule;
            shaderStageInfo.pName = "main";

            VkComputePipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            pipelineInfo.stage = shaderStageInfo;
            pipelineInfo.layout = compositingPipelineLayout;

            auto result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &compositingPipeline);

            if (result != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create ray tracing compute pipeline!");
            }
        }
        #pragma endregion

	public:
		VulkanPipeline() {}

        void init()
        {
            auto start = std::chrono::high_resolution_clock::now();

            createRenderPass();
            createDescriptorSetLayout();
            createGraphicsPipeline();

            auto end1 = std::chrono::high_resolution_clock::now();

            createRayTracingImageBuffer();
            createRayTracingImageView();

            // TODO support textures
            createComputeRayTracingDescriptorSetLayout();
            createComputeRayTracingPipelineLayout();
            createComputeRayTracingPipeline();
            allocateComputeRayTracingPipelineBuffers();

            auto end2 = std::chrono::high_resolution_clock::now();
            auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start).count();
            auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - end1).count();
            debugVulkan && printf("\nVulkan pipeline\n");
            debugVulkan && printf("    Rasterization: %lld ms\n", duration1);
            debugVulkan && printf("    Compute ray tracing: %lld ms\n", duration2);

            //createCompositingDescriptorSetLayout();
            //createCompositingPipelineLayout();
            //createCompositingPipeline();
        }
	};
}