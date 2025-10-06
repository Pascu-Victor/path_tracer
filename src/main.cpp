#include <SDL3/SDL.h>
#include <vector>
#include <memory>
#include <iostream>

#include "Vec3.h"
#include "Ray.h"
#include "Camera.h"
#include "Sphere.h"
#include "Material.h"
#include "Object.h"

// Window dimensions
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Ray tracing function
Vec3 traceRay(const Ray &ray, const std::vector<std::shared_ptr<Object>> &objects)
{
    HitRecord record;
    HitRecord tempRecord;
    bool hitAnything = false;
    double closestSoFar = 1e10;

    // Check all objects for intersection
    for (const auto &object : objects)
    {
        if (object->hit(ray, 0.001, closestSoFar, tempRecord))
        {
            hitAnything = true;
            closestSoFar = tempRecord.t;
            record = tempRecord;
        }
    }

    if (hitAnything)
    {
        // Return the material color
        return record.material->color;
    }

    // Background gradient (sky)
    Vec3 unitDirection = ray.direction.normalized();
    double t = 0.5 * (unitDirection.y + 1.0);
    return (1.0 - t) * Vec3(1.0, 1.0, 1.0) + t * Vec3(0.5, 0.7, 1.0);
}

int main(int argc, char *argv[])
{
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create window
    SDL_Window *window = SDL_CreateWindow(
        "Ray Tracer",
        WINDOW_WIDTH, WINDOW_HEIGHT,
        0);

    if (!window)
    {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer)
    {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create texture for rendering
    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        WINDOW_WIDTH,
        WINDOW_HEIGHT);

    if (!texture)
    {
        std::cerr << "Texture creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Setup camera
    double aspectRatio = static_cast<double>(WINDOW_WIDTH) / WINDOW_HEIGHT;
    Camera camera(
        Vec3(0, 0, 2), // Camera position
        Vec3(0, 0, 0), // Look at point
        Vec3(0, 1, 0), // Up vector
        60.0,          // Vertical field of view
        aspectRatio);

    // Create scene with objects
    std::vector<std::shared_ptr<Object>> objects;

    // Add spheres with different materials
    auto redMaterial = std::make_shared<Material>(0.8, 0.2, 0.2);
    auto greenMaterial = std::make_shared<Material>(0.2, 0.8, 0.2);
    auto blueMaterial = std::make_shared<Material>(0.2, 0.2, 0.8);
    auto yellowMaterial = std::make_shared<Material>(0.8, 0.8, 0.2);

    objects.push_back(std::make_shared<Sphere>(Vec3(0, 0, -1), 0.5, redMaterial));
    objects.push_back(std::make_shared<Sphere>(Vec3(-1, 0, -1), 0.3, greenMaterial));
    objects.push_back(std::make_shared<Sphere>(Vec3(1, 0, -1), 0.3, blueMaterial));
    objects.push_back(std::make_shared<Sphere>(Vec3(0, -100.5, -1), 100, yellowMaterial)); // Ground

    // Render the scene
    std::cout << "Rendering scene..." << std::endl;

    std::vector<unsigned char> pixels(WINDOW_WIDTH * WINDOW_HEIGHT * 3);

    for (int j = 0; j < WINDOW_HEIGHT; j++)
    {
        for (int i = 0; i < WINDOW_WIDTH; i++)
        {
            double u = static_cast<double>(i) / (WINDOW_WIDTH - 1);
            double v = static_cast<double>(WINDOW_HEIGHT - 1 - j) / (WINDOW_HEIGHT - 1);

            Ray ray = camera.getRay(u, v);
            Vec3 color = traceRay(ray, objects);

            int pixelIndex = (j * WINDOW_WIDTH + i) * 3;
            pixels[pixelIndex + 0] = static_cast<unsigned char>(std::clamp(color.x, 0.0, 1.0) * 255.99);
            pixels[pixelIndex + 1] = static_cast<unsigned char>(std::clamp(color.y, 0.0, 1.0) * 255.99);
            pixels[pixelIndex + 2] = static_cast<unsigned char>(std::clamp(color.z, 0.0, 1.0) * 255.99);
        }
    }

    std::cout << "Rendering complete!" << std::endl;

    // Update texture with rendered pixels
    SDL_UpdateTexture(texture, nullptr, pixels.data(), WINDOW_WIDTH * 3);

    // Main loop
    bool running = true;
    SDL_Event event;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (event.key.key == SDLK_ESCAPE)
                {
                    running = false;
                }
            }
        }

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
