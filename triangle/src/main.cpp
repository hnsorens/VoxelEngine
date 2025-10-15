


#include "VkZero/binding.hpp"
#include "VkZero/frame.hpp"
#include "VkZero/graphics_pipeline.hpp"
#include "VkZero/render_pass.hpp"
#include "VkZero/shader.hpp"
#include "VkZero/shader_bindings.hpp"
#include "VkZero/shader_group.hpp"
#include "VkZero/shader_library.hpp"
#include "VkZero/shader_resource_set.hpp"
#include "VkZero/shader_types.hpp"
#include "VkZero/types.hpp"
#include "VkZero/vk_zero.hpp"
#include "VkZero/window.hpp"

using main_frag = VkZero::Shader<"frag", "bin/frag.spv", VkZero::SHADER_FRAGMENT, 
  VkZero::ShaderAttachments<
    VkZero::ColorAttachment<"output", VkZero::Format::R8G8B8A8Srgb, 0>
  >
>;
using main_vert = VkZero::Shader<"vert", "bin/vert.spv", VkZero::SHADER_VERTEX>;
using ShaderLibrary = VkZero::ShaderLibrary<main_frag, main_vert>;

int main()
{
    VkZero::VkZeroInit();

    VkZero::Window window{1080, 1920, "Hello World"};
   ShaderLibrary shaders;

    using PushConstant = VkZero::ShaderPushConstants<>;
    using ShaderGroup = VkZero::ShaderGroup<PushConstant, main_vert, main_frag>;
    using ShaderResourceBindings = VkZero::ShaderResourceSet<>;
    using GraphicsPipeline = VkZero::GraphicsPipeline<ShaderGroup, ShaderResourceBindings>;
    using RenderpassResourceSet = VkZero::RenderPassResourceSet<VkZero::RenderPassResource<"output">>;
    using RenderPass = VkZero::GraphicsRenderpass<RenderpassResourceSet, GraphicsPipeline>;
    using Frame = VkZero::Frame<RenderPass>;

    PushConstant pushConstant;
    ShaderGroup group{pushConstant, shaders.get<"vert">(), shaders.get<"frag">()};
    ShaderResourceBindings bindings;
    GraphicsPipeline pipeline{group, bindings};
    RenderpassResourceSet attachments{
      {&window.getSwapChainImages()}
    };
    RenderPass renderPass{1920, 1080, attachments, pipeline};
    Frame frame{renderPass, window.impl};

    while (!window.shouldClose())
    {
      window.pollEvents();
      frame.draw();
    }
}