#include <SDL3/SDL.h>
#include <iostream>
#include <memory>
#include <vector>

#include "Camera.h"
#include "Light.h"
#include "Material.h"
#include "Object.h"
#include "Ray.h"
#include "Sphere.h"
#include "Vec3.h"

// Window dimensions
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Maximum recursion depth for bounce lighting
const int MAX_DEPTH = 5;

// Helper function to check if a point is in shadow
bool isInShadow(const Vec3 &point, const Vec3 &lightPos,
                const std::vector<std::shared_ptr<Object>> &objects) {
  Vec3 shadowRayDir = (lightPos - point).normalized();
  double distanceToLight = (lightPos - point).length();
  Ray shadowRay(point, shadowRayDir);

  HitRecord tempRecord;
  for (const auto &object : objects) {
    if (object->hit(shadowRay, 0.001, distanceToLight - 0.001, tempRecord)) {
      return true; // In shadow
    }
  }
  return false; // Not in shadow
}

// Calculate lighting at a hit point
Vec3 calculateLighting(const HitRecord &record, const Vec3 &viewDir,
                       const std::vector<Light> &lights,
                       const std::vector<std::shared_ptr<Object>> &objects) {
  Vec3 finalColor(0, 0, 0);
  Material *mat = record.material;

  // Ambient component
  Vec3 ambient = mat->ambient * mat->color;
  finalColor += ambient;

  // Process each light source
  for (const auto &light : lights) {
    // Check if point is in shadow
    if (isInShadow(record.point, light.position, objects)) {
      continue; // Skip this light if in shadow
    }

    Vec3 lightDir = (light.position - record.point).normalized();
    double distance = (light.position - record.point).length();
    double attenuation =
        light.intensity / (1.0 + 0.09 * distance + 0.032 * distance * distance);

    // Diffuse component
    double diffuseStrength = std::max(0.0, record.normal.dot(lightDir));
    Vec3 diffuse = mat->diffuse * diffuseStrength * mat->color;

    // Apply light color and attenuation
    diffuse.x *= light.color.x * attenuation;
    diffuse.y *= light.color.y * attenuation;
    diffuse.z *= light.color.z * attenuation;

    // Specular component
    Vec3 halfDir = (lightDir + viewDir).normalized();
    double specularStrength =
        std::pow(std::max(0.0, record.normal.dot(halfDir)), mat->shininess);
    Vec3 specular =
        mat->specular * specularStrength * light.color * attenuation;

    finalColor += diffuse + specular;
  }

  return finalColor;
}

// Recursive ray tracing function with bounce lighting
Vec3 traceRay(const Ray &ray,
              const std::vector<std::shared_ptr<Object>> &objects,
              const std::vector<Light> &lights, int depth) {
  // Base case: max depth reached
  if (depth <= 0) {
    return Vec3(0, 0, 0);
  }

  HitRecord record;
  HitRecord tempRecord;
  bool hitAnything = false;
  double closestSoFar = 1e10;

  // Check all objects for intersection
  for (const auto &object : objects) {
    if (object->hit(ray, 0.001, closestSoFar, tempRecord)) {
      hitAnything = true;
      closestSoFar = tempRecord.t;
      record = tempRecord;
    }
  }

  if (hitAnything) {
    Vec3 viewDir = (ray.origin - record.point).normalized();

    // Calculate direct lighting
    Vec3 color = calculateLighting(record, viewDir, lights, objects);

    // Add bounce lighting (reflection)
    if (record.material->reflectivity > 0.0) {
      Vec3 reflectDir = ray.direction -
                        2.0 * ray.direction.dot(record.normal) * record.normal;
      reflectDir = reflectDir.normalized();
      Ray reflectRay(record.point, reflectDir);

      Vec3 reflectColor = traceRay(reflectRay, objects, lights, depth - 1);
      color = (1.0 - record.material->reflectivity) * color +
              record.material->reflectivity * reflectColor;
    }

    return color;
  }

  // Background gradient (sky)
  Vec3 unitDirection = ray.direction.normalized();
  double t = 0.5 * (unitDirection.y + 1.0);
  return (1.0 - t) * Vec3(1.0, 1.0, 1.0) + t * Vec3(0.5, 0.7, 1.0);
}

void render(std::vector<unsigned char> &pixels, const Camera &camera,
            const std::vector<std::shared_ptr<Object>> &objects,
            const std::vector<Light> &lights) {
  for (int j = 0; j < WINDOW_HEIGHT; j++) {
    for (int i = 0; i < WINDOW_WIDTH; i++) {
      double u = static_cast<double>(i) / (WINDOW_WIDTH - 1);
      double v =
          static_cast<double>(WINDOW_HEIGHT - 1 - j) / (WINDOW_HEIGHT - 1);

      Ray ray = camera.getRay(u, v);
      Vec3 color = traceRay(ray, objects, lights, MAX_DEPTH);

      int pixelIndex = (j * WINDOW_WIDTH + i) * 3;
      pixels[pixelIndex + 0] =
          static_cast<unsigned char>(std::clamp(color.x, 0.0, 1.0) * 255.99);
      pixels[pixelIndex + 1] =
          static_cast<unsigned char>(std::clamp(color.y, 0.0, 1.0) * 255.99);
      pixels[pixelIndex + 2] =
          static_cast<unsigned char>(std::clamp(color.z, 0.0, 1.0) * 255.99);
    }
  }
}

int main(int argc, char *argv[]) {
  // Initialize SDL
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
    return 1;
  }

  // Create window
  SDL_Window *window =
      SDL_CreateWindow("Ray Tracer", WINDOW_WIDTH, WINDOW_HEIGHT, 0);

  if (!window) {
    std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return 1;
  }

  // Create renderer
  SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
  if (!renderer) {
    std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  // Create texture for rendering
  SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           WINDOW_WIDTH, WINDOW_HEIGHT);

  if (!texture) {
    std::cerr << "Texture creation failed: " << SDL_GetError() << std::endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  // Setup camera
  double aspectRatio = static_cast<double>(WINDOW_WIDTH) / WINDOW_HEIGHT;
  const Vec3 cameraOrigin = Vec3(0, 0, 2);

  Camera camera(cameraOrigin,  // Camera position
                Vec3(0, 0, 0), // Look at point
                Vec3(0, 1, 0), // Up vector
                60.0,          // Vertical field of view
                aspectRatio);

  // Create scene with objects
  std::vector<std::shared_ptr<Object>> objects;

  // Add spheres with different materials (color, ambient, diffuse, specular,
  // shininess, reflectivity)
  auto redMaterial =
      std::make_shared<Material>(Vec3(0.8, 0.2, 0.2), 0.1, 0.7, 0.3, 32.0, 0.0);
  auto greenMaterial =
      std::make_shared<Material>(Vec3(0.2, 0.8, 0.2), 0.1, 0.7, 0.3, 32.0, 0.0);
  auto blueMaterial =
      std::make_shared<Material>(Vec3(0.2, 0.2, 0.8), 0.1, 0.7, 0.3, 32.0, 0.0);
  auto yellowMaterial =
      std::make_shared<Material>(Vec3(0.8, 0.8, 0.2), 0.1, 0.6, 0.1, 16.0, 0.0);
  auto mirrorMaterial = std::make_shared<Material>(Vec3(0.9, 0.9, 0.9), 0.1,
                                                   0.2, 0.8, 128.0, 0.7);

  objects.push_back(std::make_shared<Sphere>(Vec3(0, 0, -1), 0.5, redMaterial));
  objects.push_back(
      std::make_shared<Sphere>(Vec3(-1, 0, -1), 0.3, greenMaterial));
  objects.push_back(
      std::make_shared<Sphere>(Vec3(1, 0, -1), 0.3, blueMaterial));
  objects.push_back(std::make_shared<Sphere>(Vec3(0, -100.5, -1), 100,
                                             yellowMaterial)); // Ground
  objects.push_back(std::make_shared<Sphere>(
      Vec3(0.5, 0.3, -0.5), 0.2, mirrorMaterial)); // Reflective sphere

  // Create light sources with different colors
  std::vector<Light> lights;
  lights.push_back(
      Light(Vec3(2, 2, 1), Vec3(1.0, 0.9, 0.8), 5.0)); // Warm white light
  lights.push_back(
      Light(Vec3(-2, 1, 0), Vec3(0.3, 0.5, 1.0), 3.0)); // Blue light
  lights.push_back(
      Light(Vec3(0, -0.2, 0.5), Vec3(1.0, 0.4, 0.4), 2.0)); // Red light

  std::vector<unsigned char> pixels(WINDOW_WIDTH * WINDOW_HEIGHT * 3);

  // Main loop
  bool running = true;
  SDL_Event event;

  double theta = 0.0;
  const double orbitRadius = 2.0;
  Vec3 cameraLookFrom;
  cameraLookFrom.y = cameraOrigin.y;
  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      } else if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_ESCAPE) {
          running = false;
        }
      }
    }

    double x = cameraOrigin.x + orbitRadius * std::cos(theta);
    double z = cameraOrigin.y + orbitRadius * std::sin(theta);

    cameraLookFrom.x = x;
    cameraLookFrom.z = z;

    camera.setLookFrom(cameraLookFrom);

    theta += 1.0 / 180.0;

    render(pixels, camera, objects, lights);

    // Update texture with rendered pixels
    SDL_UpdateTexture(texture, nullptr, pixels.data(), WINDOW_WIDTH * 3);

    // Clear and render
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    SDL_Delay(16); // ~60 FPS
  }

  // Cleanup
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
