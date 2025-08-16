#include "ResourceManager.hpp"
#include "VulkanContext.hpp"
// #include "image.hpp"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include <type_traits>
#include <string>
#include <vulkan/vulkan_core.h>
#include "Binding.hpp"
#include "DescriptorPool.hpp"
#include "FixedString.hpp"


enum ShaderType
{
    SHADER_VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
    SHADER_FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
    SHADER_GEOMETRY = VK_SHADER_STAGE_GEOMETRY_BIT,
    SHADER_RMISS = VK_SHADER_STAGE_MISS_BIT_KHR,
    SHADER_RGEN = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
};

template <typename... Bindings>
struct ShaderBindings
{
    using Options = std::tuple<Bindings...>;
};

template <typename... Attachments>
struct ShaderAttachments
{
    // TODO: verify that all attachments are either ShaderInput, ShaderPreserve, ShaderDepthAttachment, or ShaderColorAttachment
    using Options = std::tuple<Attachments...>;
};

namespace ShaderDetails
{
    template <typename... Shaders>
    struct has_duplicate_shader_bindings;

    template <>
    struct has_duplicate_shader_bindings<> {
        static constexpr bool value = false;
    };

    template <typename First, typename... Rest>
    struct has_duplicate_shader_bindings<First, Rest...>
    {
        static constexpr bool value = 
            ((First::get_binding_set() == Rest::get_binding_set() || First::get_binding() == Rest::get_binding() || First::get_binding_count() == Rest::get_binding_count()) || ...) ||
            has_duplicate_shader_bindings<Rest...>::value;
    };

    template <typename options>
    struct find_bindings;

    template <>
    struct find_bindings<std::tuple<>>
    {
        using type = ShaderBindings<>;
    };


    // Recursive case: check first element
    template <typename First, typename... Rest>
    struct find_bindings<std::tuple<First, Rest...>> {
    private:
        // Helper: true if First is a ShaderBindings<...>
        template <typename T>
        struct is_shader_bindings : std::false_type {};

        template <typename... Bs>
        struct is_shader_bindings<ShaderBindings<Bs...>> : std::true_type {};

    public:
        using type = std::conditional_t<
            is_shader_bindings<First>::value,
            First,
            typename find_bindings<std::tuple<Rest...>>::type
        >;
    };

    template <typename options>
    struct find_attachments;

    template <>
    struct find_attachments<std::tuple<>>
    {
        using type = ShaderAttachments<>;
    };

    template <typename First, typename... Rest>
    struct find_attachments<std::tuple<First, Rest...>> {
    private:
        // Helper: true if First is a ShaderBindings<...>
        template <typename T>
        struct is_shader_attachment : std::false_type {};

        template <typename... Bs>
        struct is_shader_attachment<ShaderAttachments<Bs...>> : std::true_type {};

    public:
        using type = std::conditional_t<
            is_shader_attachment<First>::value,
            First,
            typename find_attachments<std::tuple<Rest...>>::type
        >;
    };
}

template <FixedString ShaderName, FixedString Path, ShaderType Type, typename... ShaderOptions>
class Shader {
public:
    using Bindings = ShaderDetails::find_bindings<std::tuple<ShaderOptions...>>::type::Options;
    using Attachments = ShaderDetails::find_attachments<std::tuple<ShaderOptions...>>::type::Options;
    static constexpr FixedString name = ShaderName.value;
    static constexpr FixedString path = Path.value;

    using BindingsList = Bindings;

    Shader(std::unique_ptr<VulkanContext>& ctx) 
    {
        auto shaderCode = ResourceManager::readFile(std::string{path.value});

        shaderModule = createShaderModule(ctx->getDevice(), shaderCode);

        shaderInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderInfo.stage = (VkShaderStageFlagBits)Type;
        shaderInfo.module = shaderModule;
        shaderInfo.pName = "main";
        shaderInfo.flags = 0;
    }

    static constexpr ShaderType get_type() { return Type; }

    VkPipelineShaderStageCreateInfo getShaderInfo() { return shaderInfo; }
    
private:

    static_assert(std::tuple_size<Attachments>::value > 0 ? Type == SHADER_FRAGMENT : true, "Only fragment shaders can contain attachments");
    static_assert(!ShaderDetails::has_duplicate_shader_bindings<Bindings>::value, "Shader cannot have duplicate bindings");

    VkShaderModule createShaderModule(VkDevice device,
                                        const std::vector<char> &code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) !=
            VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    VkShaderModule shaderModule;
    VkPipelineShaderStageCreateInfo shaderInfo {};
};


namespace ShaderGroupDetails
{
    template <typename... Shaders>
    struct validate_shader_bindings {
    private:
        // Combine all shaders' BindingsList tuple types into one tuple type
        using all_bindings_tuple = decltype(std::tuple_cat(
            std::declval<typename Shaders::BindingsList>()...
        ));

        // Check compatibility of two binding types
        template <typename A, typename B>
        struct compatible {
            static constexpr bool same_slot =
                (A::get_binding_set() == B::get_binding_set()) &&
                (A::get_binding()     == B::get_binding());

            static constexpr bool same_type  =
                A::type() == B::type();

            static constexpr bool same_count =
                (A::get_descriptor_count() == B::get_descriptor_count());

            static constexpr bool value = !same_slot || (same_type && same_count);
        };

        // Recursive pairwise checker
        template <typename... Ts>
        struct check_all {
            static constexpr bool value = true; // Empty pack or single element is fine
        };

        template <typename First, typename... Rest>
        struct check_all<First, Rest...> {
            static constexpr bool value =
                ((compatible<First, Rest>::value) && ...) && // Compare First with all Rest
                check_all<Rest...>::value;                   // Then recurse
        };

        // Turn tuple<Ts...> into check_all<Ts...>::value
        template <typename Tuple>
        struct tuple_check;

        template <typename... Ts>
        struct tuple_check<std::tuple<Ts...>> {
            static constexpr bool value = check_all<Ts...>::value;
        };

    public:
        static constexpr bool value = tuple_check<all_bindings_tuple>::value;
    };

    template <typename Attachments>
    struct collect_attachments;

    template <>
    struct collect_attachments<std::tuple<>>
    {
        using value = std::tuple<>;
    };

    template <typename First, typename... Rest>
    struct collect_attachments<std::tuple<First, Rest...>>
    {
        using value = decltype(std::tuple_cat(
            std::declval<typename First::Attachments>(),
            std::declval<typename collect_attachments<std::tuple<Rest...>>::value>() 
        ));
    };
}

template <typename... Shaders>
class ShaderGroup
{
    // static_assert(all_shader_types_unique<Shaders...>::value,
    //              "Shader types conflict - multiple shaders with the same shader type!");
    static_assert(ShaderGroupDetails::validate_shader_bindings<Shaders...>::value,
                 "Shader bindings conflict - same binding number with different resource type or different binding counts!");
public:

    
    using shaders = std::tuple<Shaders...>;
    using Attachments = ShaderGroupDetails::collect_attachments<shaders>::value;
    
    // Original constructor
    ShaderGroup(Shaders&... shaders) : 
    m_shaders{shaders.getShaderInfo()...} {}

    size_t size()
    {
        return m_shaders.size();
    }

    VkPipelineShaderStageCreateInfo* data()
    {
        return m_shaders.data();
    }
    
private:

    std::vector<VkPipelineShaderStageCreateInfo> m_shaders;
};

namespace ShaderResourceSetDetails
{
    template <typename... Shaders>
    struct has_duplicate_bindings;

    template <>
    struct has_duplicate_bindings<> {
        static constexpr bool value = false;
    };

    template <typename First, typename... Rest>
    struct has_duplicate_bindings<First, Rest...>
    {
        static constexpr bool value = 
            ((std::is_same_v<typename First::resourceType, typename Rest::resourceType> || First::get_type() == Rest::get_type() || First::get_binding() == Rest::get_binding() || First::get_descriptor_count() == Rest::get_descriptor_count()) || ...) ||
            has_duplicate_bindings<Rest...>::value;
    };
}

template <typename... Bindings>
class ShaderResourceSet
{

    static_assert(!ShaderResourceSetDetails::has_duplicate_bindings<Bindings...>::value, "Shader Resource Set cannot have duplicate bindings!");

public:

    using BindingResources = std::tuple<Bindings...>;

    ShaderResourceSet(std::unique_ptr<VulkanContext>& ctx, Bindings&&... bindings) :
    descriptorSetLayout([&]() {
        printf("Creating descriptor set layout\n");
        fflush(stdout);

        std::vector<VkDescriptorSetLayoutBinding> descriptorBindings;

        // Fold expression over Bindings...
        (void(
            [&] {
                using BindingType = Bindings;
                VkDescriptorSetLayoutBinding bindInfo{};
                bindInfo.binding = BindingType::get_binding();
                bindInfo.descriptorCount = BindingType::get_descriptor_count();
                bindInfo.descriptorType = BindingType::type();
                bindInfo.stageFlags = BindingType::get_stages();
                bindInfo.pImmutableSamplers = nullptr;
                descriptorBindings.push_back(bindInfo);
            }()
        ), ...); // this iterates over each Binding type

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(descriptorBindings.size());
        descriptorSetLayoutInfo.pBindings = descriptorBindings.data();
        descriptorSetLayoutInfo.pNext = nullptr;

        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(ctx->getDevice(), &descriptorSetLayoutInfo, nullptr, &layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        return layout;
    }()),
    descriptorSets([&](){
        printf("Creating descriptor sets\n");
        fflush(stdout);
        VkDescriptorSetAllocateInfo allocInfo = {};
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{MAX_FRAMES_IN_FLIGHT, descriptorSetLayout};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = DescriptorPool::instance().get();
        allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
        allocInfo.pSetLayouts = descriptorSetLayouts.data();

        std::vector<VkDescriptorSet> descriptorSets(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(ctx->getDevice(), &allocInfo, descriptorSets.data())) {
            throw std::runtime_error("Failed to create raytracing descriptor set!");
        }

        return std::move(descriptorSets);
    }()),
    bindings(
        ([&](auto&& b) {
            printf("Creating bindings\n");
        fflush(stdout);
            b.writeAll(ctx->getDevice(), descriptorSets);
            return std::move(b);
        }(std::forward<Bindings>(bindings)))...
    )
    {}

    VkDescriptorSetLayout getLayout() { return descriptorSetLayout; }

private:

    VkDescriptorSetLayout descriptorSetLayout;
    std::vector<VkDescriptorSet> descriptorSets;
    std::tuple<Bindings...> bindings;

    friend class PipelineManager;
};



template <int Stage, int SetIndex, typename ShaderBinding, typename ResourceTuple>
struct vgp_find_invalid_binding;

template <int Stage, int SetIndex, typename ShaderBinding>
struct vgp_find_invalid_binding<Stage, SetIndex, ShaderBinding, std::tuple<>> {
    static constexpr bool value = false;
};

template <int Stage, int SetIndex, typename ShaderBinding, typename First, typename... Rest>
struct vgp_find_invalid_binding<Stage, SetIndex, ShaderBinding, std::tuple<First, Rest...>> {
    static constexpr bool value =
        (ShaderBinding::get_binding() != First::get_binding() ||
         ShaderBinding::get_descriptor_count() != First::get_descriptor_count() ||
         ShaderBinding::type() != First::type()) ||
         (Stage & First::get_stages()) == 0 ||
        vgp_find_invalid_binding<Stage, SetIndex, ShaderBinding, std::tuple<Rest...>>::value;
};


// 2. Check a ShaderBinding against all ResourceSets
template <int Stage, int SetIndex, typename ShaderBinding, typename... Sets>
struct vgp_shader_invalid_resource;

template <int Stage, int SetIndex, typename ShaderBinding>
struct vgp_shader_invalid_resource<Stage, SetIndex, ShaderBinding> {
    static constexpr bool value = false;
};

template <int Stage, int SetIndex, typename ShaderBinding, typename FirstSet, typename... RestSets>
struct vgp_shader_invalid_resource<Stage, SetIndex, ShaderBinding, FirstSet, RestSets...> {
    static constexpr bool value =
        vgp_find_invalid_binding<Stage, SetIndex, ShaderBinding, typename FirstSet::BindingResources>::value ||
        vgp_shader_invalid_resource<Stage, SetIndex + 1, ShaderBinding, RestSets...>::value;
};


// 3. Check all bindings in a shader
template <int Stage, typename ShaderBindingsTuple, typename... Sets>
struct vgp_shader_invalid;

template <int Stage, typename... Sets>
struct vgp_shader_invalid<Stage, std::tuple<>, Sets...> {
    static constexpr bool value = false;
};

template <int Stage, typename FirstBinding, typename... RestBindings, typename... Sets>
struct vgp_shader_invalid<Stage, std::tuple<FirstBinding, RestBindings...>, Sets...> {
    static constexpr bool value =
        vgp_shader_invalid_resource<Stage, 0, FirstBinding, Sets...>::value ||
        vgp_shader_invalid<Stage, std::tuple<RestBindings...>, Sets...>::value;
};


// 4. Iterate over all shaders in the ShaderGroup
template <typename ShaderTuple, typename... Sets>
struct vgp_invalid;

template <typename... Sets>
struct vgp_invalid<std::tuple<>, Sets...> {
    static constexpr bool value = false;
};

template <typename FirstShader, typename... RestShaders, typename... Sets>
struct vgp_invalid<std::tuple<FirstShader, RestShaders...>, Sets...> {
    static constexpr bool value =
        vgp_shader_invalid<FirstShader::get_type(), typename FirstShader::BindingsList, Sets...>::value ||
        vgp_invalid<std::tuple<RestShaders...>, Sets...>::value;
};


// ----------------------------
// Step 2: Main struct wrapper
// ----------------------------
template <typename ShaderGroup, typename... ResourceSets>
struct validate_graphics_pipeline {
    static constexpr bool value = !vgp_invalid<typename ShaderGroup::shaders, ResourceSets...>::value;
};

template <typename ShaderGroup, typename... ShaderResourcesBindings>
class GraphicsPipeline
{
    static_assert(validate_graphics_pipeline<ShaderGroup, ShaderResourcesBindings...>::value, "Graphics Pipeline Invalid");

public:

    using Attachments = ShaderGroup::Attachments;

    GraphicsPipeline(std::unique_ptr<VulkanContext>& ctx, ShaderGroup& shaderGroup, ShaderResourcesBindings&... resources) :
    m_shaderGroup(shaderGroup),
    pipelineLayout([&](){

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

        (descriptorSetLayouts.push_back(resources.getLayout()), ...);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

        printf("Pipeline Layout SIze: %d\n", descriptorSetLayouts.size());

        VkPipelineLayout layout;
        if (vkCreatePipelineLayout(ctx->getDevice(), &pipelineLayoutInfo, nullptr,
                                    &layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        return layout;
    }())
    {
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        // TODO save the shader arrays, so if it goes out of scope the references are still there
        pipelineInfo.stageCount = m_shaderGroup.size();
        pipelineInfo.pStages = m_shaderGroup.data();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.layout = pipelineLayout;
    }

    void create_pipeline(VkDevice device, VkRenderPass renderPass)
    {
        if (pipeline)
        {
            throw std::runtime_error("Cannot use pipeline in more than one renderpass!");
        }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    VkPipelineViewportStateCreateInfo viewportState{};
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    VkPipelineMultisampleStateCreateInfo multisampling{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    VkPipelineDynamicStateCreateInfo dynamicState{};

    std::vector<VkDynamicState> dynamicStates;

     vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    pipelineInfo.pVertexInputState = &vertexInputInfo;

    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    pipelineInfo.pInputAssemblyState = &inputAssembly;

    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    pipelineInfo.pViewportState = &viewportState;

    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    pipelineInfo.pRasterizationState = &rasterizer;

    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineInfo.pMultisampleState = &multisampling;

    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;
    pipelineInfo.pColorBlendState = &colorBlending;
  
    dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    pipelineInfo.pDynamicState = &dynamicState;

        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;

        pipelineInfo.renderPass = renderPass;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
        nullptr, &pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        // Frees all memory in shader group because it cannot be used anymore
        ShaderGroup group = std::move(m_shaderGroup);
    }

private:
    
    ShaderGroup m_shaderGroup;
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline = VK_NULL_HANDLE;

    friend class PipelineManager;
};





// Shader index finder
template <FixedString Name, typename... Shaders>
struct shader_index;

// Base case (not found)
template <FixedString Name>
struct shader_index<Name> {
    static_assert("Shader not found in registry");
    static constexpr int value = -1;
};

// Recursive case
template <FixedString Name, typename First, typename... Rest>
struct shader_index<Name, First, Rest...> {
private:
    static constexpr bool matches = []() {
        constexpr FixedString first_name = First::name;
        return first_name == Name;
    }();
    
public:
    static constexpr int value = matches ? 0 : (1 + shader_index<Name, Rest...>::value);
};

// ShaderTypes class
template <typename... Shaders>
class ShaderTypes {
private:
    std::tuple<Shaders...> shaders;

public:
    using types = std::tuple<Shaders...>;

    ShaderTypes(std::unique_ptr<VulkanContext>& ctx) : shaders(Shaders(ctx)...) {}

    template <FixedString Name>
    constexpr auto& get() {
        constexpr size_t index = shader_index<Name, Shaders...>::value;
        return std::get<index>(shaders);
    }
};









