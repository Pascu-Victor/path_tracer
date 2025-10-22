#include "ShaderCompiler.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

std::vector<uint32_t>
ShaderCompiler::compileShader(const std::string &shaderPath,
                              VkShaderStageFlagBits stage)
{
  return compileWithGlslc(shaderPath);
}

std::string ShaderCompiler::getShaderStageName(VkShaderStageFlagBits stage)
{
  switch (stage)
  {
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
ShaderCompiler::compileWithGlslc(const std::string &shaderPath)
{
  std::string outputPath = shaderPath + ".spv";

  std::string command = std::string("glslc ") + shaderPath + " -o " +
                        outputPath + " -O" + " --target-env=vulkan1.4" +
                        " -fshader-stage=comp";

  std::cout << "Compiling shader: " << command << std::endl;

  int result = system(command.c_str());
  if (result != 0)
  {
    throw std::runtime_error("Failed to compile shader: " + shaderPath);
  }

  // Read compiled SPIR-V
  return readSPIRVFile(outputPath);
}

std::vector<uint32_t>
ShaderCompiler::readSPIRVFile(const std::string &filePath)
{
  std::ifstream file(filePath, std::ios::ate | std::ios::binary);

  if (!file.is_open())
  {
    throw std::runtime_error("Failed to open SPIR-V file: " + filePath);
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

  file.seekg(0);
  file.read((char *)buffer.data(), fileSize);
  file.close();

  return buffer;
}

std::string ShaderCompiler::readTextFile(const std::string &filePath)
{
  std::ifstream file(filePath);
  if (!file.is_open())
  {
    throw std::runtime_error("Failed to open file: " + filePath);
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

std::map<std::string, std::string>
ShaderCompiler::loadSurfaceShaderFiles(const std::string &directory)
{
  std::map<std::string, std::string> shaders;

  if (!fs::exists(directory))
  {
    std::cout << "Surface shader directory not found: " << directory
              << std::endl;
    return shaders;
  }

  for (const auto &entry : fs::directory_iterator(directory))
  {
    if (entry.is_regular_file() && entry.path().extension() == ".glsl")
    {
      std::string filepath = entry.path().string();
      std::string shaderCode = readTextFile(filepath);
      shaders[filepath] = shaderCode;
      std::cout << "Loaded surface shader: " << entry.path().filename().string()
                << " from " << filepath << std::endl;
    }
  }

  return shaders;
}

std::string ShaderCompiler::loadAndInjectSurfaceShaders(
    const std::string &mainShaderPath, const std::string &surfaceShaderDir,
    std::unordered_map<std::string, int> &outPathToIndex)
{
  // Read main shader template
  std::string mainShader = readTextFile(mainShaderPath);

  // Load all surface shaders from directory
  auto surfaceShaders = loadSurfaceShaderFiles(surfaceShaderDir);

  // Build path to index mapping and injected shader code
  outPathToIndex.clear();
  int shaderIndex = 1; // Start at 1 (0 is reserved for Phong)

  std::stringstream injectedCode;
  std::stringstream dispatchCode;
  std::stringstream dispatchCodeSphere; // For sphere emissive sampling

  bool first = true;
  bool firstSphere = true;
  for (const auto &[filepath, code] : surfaceShaders)
  {
    outPathToIndex[filepath] = shaderIndex;

    // Add shader function code
    injectedCode << "// Shader Index " << shaderIndex << " - " << filepath
                 << "\n";
    injectedCode << code << "\n\n";

    // Extract function name from the shader code
    std::string functionName;
    size_t funcPos = code.find("SurfaceShaderResult ");
    if (funcPos != std::string::npos)
    {
      size_t nameStart = funcPos + 20; // After "SurfaceShaderResult "
      size_t nameEnd = code.find("(", nameStart);
      if (nameEnd != std::string::npos)
      {
        functionName = code.substr(nameStart, nameEnd - nameStart);
      }
    }

    // Build dispatch chain for regular surface shading
    if (!first)
    {
      dispatchCode << " else ";
    }
    dispatchCode << "if (shaderFunctionIndex == " << shaderIndex << ") {\n";
    dispatchCode << "            shaderResult = " << functionName
                 << "(shaderData);\n";
    dispatchCode << "        }";

    // Build dispatch chain for sphere emissive sampling
    if (!firstSphere)
    {
      dispatchCodeSphere << " else ";
    }
    dispatchCodeSphere << "if (sphereShaderIndex == " << shaderIndex << ") {\n";
    dispatchCodeSphere << "                    sphereShaderResult = "
                       << functionName << "(sphereShaderData);\n";
    dispatchCodeSphere << "                }";

    first = false;
    firstSphere = false;
    shaderIndex++;
  }

  // Replace shader functions placeholder with injected code
  const std::string functionsPlaceholder = "// SURFACE_SHADERS_PLACEHOLDER";
  size_t pos = mainShader.find(functionsPlaceholder);
  if (pos != std::string::npos)
  {
    mainShader.replace(pos, functionsPlaceholder.length(), injectedCode.str());
    std::cout << "Injected " << surfaceShaders.size()
              << " surface shaders into main shader" << std::endl;
  }
  else
  {
    std::cout
        << "Warning: Could not find SURFACE_SHADERS_PLACEHOLDER in main shader"
        << std::endl;
  }

  // Replace dispatch placeholder with generated dispatch code
  const std::string dispatchPlaceholder =
      "// SURFACE_SHADER_DISPATCH_PLACEHOLDER";
  pos = mainShader.find(dispatchPlaceholder);
  if (pos != std::string::npos)
  {
    mainShader.replace(pos, dispatchPlaceholder.length(), dispatchCode.str());
    std::cout << "Generated dispatch code for " << surfaceShaders.size()
              << " surface shaders" << std::endl;
  }
  else
  {
    std::cout << "Warning: Could not find SURFACE_SHADER_DISPATCH_PLACEHOLDER "
                 "in main shader"
              << std::endl;
  }

  // Replace sphere emissive dispatch placeholder
  const std::string dispatchSphereEmissivePlaceholder =
      "// SURFACE_SHADER_DISPATCH_FOR_SPHERE_EMISSIVE";
  pos = mainShader.find(dispatchSphereEmissivePlaceholder);
  if (pos != std::string::npos)
  {
    mainShader.replace(pos, dispatchSphereEmissivePlaceholder.length(),
                       dispatchCodeSphere.str());
    std::cout << "Generated sphere emissive dispatch code for "
              << surfaceShaders.size() << " surface shaders" << std::endl;
  }
  else
  {
    std::cout << "Warning: Could not find "
                 "SURFACE_SHADER_DISPATCH_FOR_SPHERE_EMISSIVE in main shader"
              << std::endl;
  }

  return mainShader;
}
