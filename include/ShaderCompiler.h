#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

class ShaderCompiler {
public:
  // Compile GLSL shader to SPIR-V using glslc or glslang
  static std::vector<uint32_t> compileShader(const std::string &shaderPath,
                                             VkShaderStageFlagBits stage);

  // Load surface shaders from directory and inject into main shader
  // Returns the modified shader source and fills outPathToIndex with
  // path->index mapping
  static std::string loadAndInjectSurfaceShaders(
      const std::string &mainShaderPath, const std::string &surfaceShaderDir,
      std::unordered_map<std::string, int> &outPathToIndex);

  // Get shader stage name for logging
  static std::string getShaderStageName(VkShaderStageFlagBits stage);

private:
  // Helper to run external compiler
  static std::vector<uint32_t> compileWithGlslc(const std::string &shaderPath);

  // Helper to read compiled SPIR-V file
  static std::vector<uint32_t> readSPIRVFile(const std::string &filePath);

  // Helper to read text file
  static std::string readTextFile(const std::string &filePath);

  // Helper to discover and load surface shaders
  // Returns map of shader path -> shader code
  static std::map<std::string, std::string>
  loadSurfaceShaderFiles(const std::string &directory);
};
