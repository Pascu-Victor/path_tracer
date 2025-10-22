#include "ShaderCompiler.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>

std::vector<uint32_t>
ShaderCompiler::compileShader(const std::string &shaderPath,
                              VkShaderStageFlagBits stage) {
  return compileWithGlslc(shaderPath);
}

std::string ShaderCompiler::getShaderStageName(VkShaderStageFlagBits stage) {
  switch (stage) {
  case VK_SHADER_STAGE_VERTEX_BIT:
    return "VERTEX";
  case VK_SHADER_STAGE_FRAGMENT_BIT:
    return "FRAGMENT";
  case VK_SHADER_STAGE_COMPUTE_BIT:
    return "COMPUTE";
  case VK_SHADER_STAGE_GEOMETRY_BIT:
    return "GEOMETRY";
  case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
    return "TESS_CONTROL";
  case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
    return "TESS_EVAL";
  default:
    return "UNKNOWN";
  }
}

std::vector<uint32_t>
ShaderCompiler::compileWithGlslc(const std::string &shaderPath) {
  // Compile GLSL to SPIR-V using glslc with optimization
  std::string outputPath = shaderPath + ".spv";
  std::string command =
      std::string("glslc ") + shaderPath + " -o " + outputPath + " -O";

  std::cout << "Compiling shader: " << command << std::endl;

  int result = system(command.c_str());
  if (result != 0) {
    throw std::runtime_error("Failed to compile shader: " + shaderPath);
  }

  // Read compiled SPIR-V
  return readSPIRVFile(outputPath);
}

std::vector<uint32_t>
ShaderCompiler::readSPIRVFile(const std::string &filePath) {
  std::ifstream file(filePath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open SPIR-V file: " + filePath);
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

  file.seekg(0);
  file.read((char *)buffer.data(), fileSize);
  file.close();

  return buffer;
}
