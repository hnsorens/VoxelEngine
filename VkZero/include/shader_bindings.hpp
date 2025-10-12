#pragma once

#include <tuple>

namespace VkZero {
template <typename... Bindings> struct ShaderBindings {
  using Options = std::tuple<Bindings...>;
};

template <typename... Attachments> struct ShaderAttachments {
  // TODO: verify that all attachments are either ShaderInput, ShaderPreserve,
  // ShaderDepthAttachment, or ShaderColorAttachment
  using Options = std::tuple<Attachments...>;
};

template <typename PushConstantType> struct ShaderPushConstant {
  // TODO: verify that all attachments are either ShaderInput, ShaderPreserve,
  // ShaderDepthAttachment, or ShaderColorAttachment
  using Options = PushConstantType;
};
} // namespace VkZero
