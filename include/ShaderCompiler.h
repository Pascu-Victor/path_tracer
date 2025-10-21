#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

class ShaderCompiler {
public:
  // Compile GLSL shader to SPIR-V using glslc or glslang
  static std::vector<uint32_t> compileShader(const std::string &shaderPath,
                                             VkShaderStageFlagBits stage);

  // Get shader stage name for logging
  static std::string getShaderStageName(VkShaderStageFlagBits stage);

private:
  // Helper to run external compiler
  static std::vector<uint32_t> compileWithGlslc(const std::string &shaderPath);

  // Helper to read compiled SPIR-V file
  static std::vector<uint32_t> readSPIRVFile(const std::string &filePath);
};
