#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>

#include "GamesEngineeringBase.h" // Include the GamesEngineeringBase header
#include <algorithm>
#include <chrono>

#include <cmath>
#include "matrix.h"
#include "colour.h"
#include "mesh.h"
#include "zbuffer.h"
#include "renderer.h"
#include "RNG.h"
#include "light.h"
#include "triangle.h"
#include "ThreadPool.h"

// Main rendering function that processes a mesh, transforms its vertices, applies lighting, and draws triangles on the canvas.
// Input Variables:
// - renderer: The Renderer object used for drawing.
// - mesh: Pointer to the Mesh object containing vertices and triangles to render.
// - camera: Matrix representing the camera's transformation.
// - L: Light object representing the lighting parameters.
void render(Renderer& renderer, Mesh* mesh, matrix& camera, Light& L) {
    // Combine perspective, camera, and world transformations for the mesh
    matrix p = renderer.perspective * camera * mesh->world;
    float width = static_cast<float>(renderer.canvas.getWidth());
    float height = static_cast<float>(renderer.canvas.getHeight());


    // Iterate through all triangles in the mesh
    for (triIndices& ind : mesh->triangles) {
        Vertex t[3]; // Temporary array to store transformed triangle vertices

        // Transform each vertex of the triangle
        for (unsigned int i = 0; i < 3; i++) {
            t[i].p = p * mesh->vertices[ind.v[i]].p; // Apply transformations
            t[i].p.divideW(); // Perspective division to normalize coordinates

            // Transform normals into world space for accurate lighting
            // no need for perspective correction as no shearing or non-uniform scaling
            t[i].normal = mesh->world * mesh->vertices[ind.v[i]].normal; 
            t[i].normal.normalise();

            // Map normalized device coordinates to screen space
            t[i].p[0] = (t[i].p[0] + 1.f) * 0.5f * width;
            t[i].p[1] = (t[i].p[1] + 1.f) * 0.5f * height;
            t[i].p[1] = height - t[i].p[1]; // Invert y-axis

            // Copy vertex colours
            t[i].rgb = mesh->vertices[ind.v[i]].rgb;
        }

        // Clip triangles with Z-values outside [-1, 1]
        if (fabs(t[0].p[2]) > 1.0f || fabs(t[1].p[2]) > 1.0f || fabs(t[2].p[2]) > 1.0f) continue;

        // Create a triangle object and render it
        triangle tri(t[0], t[1], t[2]);
        tri.draw(renderer, L, mesh->ka, mesh->kd);
    }
}

void renderMT(Renderer& renderer, std::vector<Mesh*>& meshes, matrix& camera, Light& L) {
    matrix p_cam = renderer.perspective * camera;
    float width = static_cast<float>(renderer.canvas.getWidth());
    float height = static_cast<float>(renderer.canvas.getHeight());

    struct TriangleJob {
        triangle tri;
        float ka, kd;
    };
    std::vector<TriangleJob> jobs;

    for (Mesh* mesh : meshes) {
        matrix p = p_cam * mesh->world;
        for (triIndices& ind : mesh->triangles) {
            Vertex t[3];
            for (unsigned int i = 0; i < 3; i++) {
                t[i].p = p * mesh->vertices[ind.v[i]].p;
                t[i].p.divideW();
                t[i].normal = mesh->world * mesh->vertices[ind.v[i]].normal;
                t[i].normal.normalise();
                t[i].p[0] = (t[i].p[0] + 1.f) * 0.5f * width;
                t[i].p[1] = (t[i].p[1] + 1.f) * 0.5f * height;
                t[i].p[1] = height - t[i].p[1];
                t[i].rgb = mesh->vertices[ind.v[i]].rgb;
            }
            if (fabs(t[0].p[2]) > 1.0f || fabs(t[1].p[2]) > 1.0f || fabs(t[2].p[2]) > 1.0f)
                continue;
            jobs.push_back({ triangle(t[0], t[1], t[2]), mesh->ka, mesh->kd });
        }
    }

    //get number of triangles to check if MT needed
    size_t triangleCount = jobs.size();

    if (triangleCount < 200) {
        for (auto& job : jobs) {
            job.tri.drawSIMDOptimised(renderer, L, job.ka, job.kd);
        }
        return;
    }

    //after testing - ideal threads for this code is around 150 triangles per 1 thread
    unsigned int numThreads = std::min((unsigned int)std::jthread::hardware_concurrency(), (unsigned int)triangleCount / 150);
    std::vector<std::jthread> threads(numThreads);

    int canvasHeight = renderer.canvas.getHeight();
    int stripHeight = canvasHeight / numThreads;

    for (unsigned int i = 0; i < numThreads; i++) {
        threads[i] = std::jthread([i, stripHeight, numThreads, canvasHeight, &jobs, &renderer, &L]() {
            int yMin = i * stripHeight;
            int yMax = (i == numThreads - 1) ? canvasHeight : (i + 1) * stripHeight;

            for (auto& job : jobs)
                job.tri.drawSIMDOptimisedStrip(renderer, L, job.ka, job.kd, yMin, yMax);
        });
    }

   /* for (unsigned int i = 0; i < numThreads; i++) {
        threads[i] = std::jthread([i, numThreads, &jobs, &renderer, &L]() {
            for (size_t j = i; j < jobs.size(); j += numThreads) {
                jobs[j].tri.drawSIMDOptimised(renderer, L, jobs[j].ka, jobs[j].kd);
            }
            });
    }*/
}

void renderMTPool(ThreadPool& pool, Renderer& renderer, std::vector<Mesh*>& meshes, matrix& camera, Light& L) {
    matrix p_cam = renderer.perspective * camera;
    float width = static_cast<float>(renderer.canvas.getWidth());
    float height = static_cast<float>(renderer.canvas.getHeight());
    int numThreads = pool.threads.size();

    struct TriangleJob {
        triangle tri;
        float ka, kd;
    };
    std::vector<TriangleJob> jobs;

    for (Mesh* mesh : meshes) {
        matrix p = p_cam * mesh->world;
        for (triIndices& ind : mesh->triangles) {
            Vertex t[3];
            for (unsigned int i = 0; i < 3; i++) {
                t[i].p = p * mesh->vertices[ind.v[i]].p;
                t[i].p.divideW();
                t[i].normal = mesh->world * mesh->vertices[ind.v[i]].normal;
                t[i].normal.normalise();
                t[i].p[0] = (t[i].p[0] + 1.f) * 0.5f * width;
                t[i].p[1] = (t[i].p[1] + 1.f) * 0.5f * height;
                t[i].p[1] = height - t[i].p[1];
                t[i].rgb = mesh->vertices[ind.v[i]].rgb;
            }
            if (fabs(t[0].p[2]) > 1.0f || fabs(t[1].p[2]) > 1.0f || fabs(t[2].p[2]) > 1.0f)
                continue;
            jobs.push_back({ triangle(t[0], t[1], t[2]), mesh->ka, mesh->kd });
        }
    }

    //get number of triangles to check if MT needed
    size_t triangleCount = jobs.size();

    if (triangleCount < 200) {
        for (auto& job : jobs) {
            job.tri.drawSIMDOptimised(renderer, L, job.ka, job.kd);
        }
        return;
    }

    int canvasHeight = renderer.canvas.getHeight();
    int stripHeight = canvasHeight / numThreads;

    for (int i = 0; i < numThreads; i++) {
        pool.enqueue([i, stripHeight, numThreads, canvasHeight, &jobs, &renderer, &L]() mutable {
            int yMin = i * stripHeight;
            int yMax = (i == numThreads - 1) ? canvasHeight : (i + 1) * stripHeight;
            for(auto& job : jobs)
                job.tri.drawSIMDOptimisedStrip(renderer, L, job.ka, job.kd, yMin, yMax) ;
            });
    }
    /*for (int i = 0; i < numThreads; i++) {
        pool.enqueue([i, &jobs, &renderer, &L]() mutable {
            for (auto& job : jobs)
                job.tri.drawSIMDOptimised(renderer, L, job.ka, job.kd);
            });
    }*/
    pool.waitForCompletion();
}


// Test scene function to demonstrate rendering with user-controlled transformations
// No input variables
void sceneTest() {
    Renderer renderer;
    // create light source {direction, diffuse intensity, ambient intensity}
    Light L{ vec4(0.f, 1.f, 1.f, 0.f), colour(1.0f, 1.0f, 1.0f), colour(0.2f, 0.2f, 0.2f) };
    // camera is just a matrix
    matrix camera = matrix::makeIdentity(); // Initialize the camera with identity matrix

    bool running = true; // Main loop control variable

    std::vector<Mesh*> scene; // Vector to store scene objects

    // Create a sphere and a rectangle mesh
    Mesh mesh = Mesh::makeSphere(1.0f, 10, 20);
    //Mesh mesh2 = Mesh::makeRectangle(-2, -1, 2, 1);

    // add meshes to scene
    scene.push_back(&mesh);
   // scene.push_back(&mesh2); 

    float x = 0.0f, y = 0.0f, z = -4.0f; // Initial translation parameters
    mesh.world = matrix::makeTranslation(x, y, z);
    //mesh2.world = matrix::makeTranslation(x, y, z) * matrix::makeRotateX(0.01f);

    // Main rendering loop
    while (running) {
        renderer.canvas.checkInput(); // Handle user input
        renderer.clear(); // Clear the canvas for the next frame

        // Apply transformations to the meshes
     //   mesh2.world = matrix::makeTranslation(x, y, z) * matrix::makeRotateX(0.01f);
        mesh.world = matrix::makeTranslation(x, y, z);

        // Handle user inputs for transformations
        if (renderer.canvas.keyPressed(VK_ESCAPE)) break;
        if (renderer.canvas.keyPressed('A')) x += -0.1f;
        if (renderer.canvas.keyPressed('D')) x += 0.1f;
        if (renderer.canvas.keyPressed('W')) y += 0.1f;
        if (renderer.canvas.keyPressed('S')) y += -0.1f;
        if (renderer.canvas.keyPressed('Q')) z += 0.1f;
        if (renderer.canvas.keyPressed('E')) z += -0.1f;

        // Render each object in the scene
        for (auto& m : scene)
            render(renderer, m, camera, L);

        renderer.present(); // Display the rendered frame
    }
}

// Utility function to generate a random rotation matrix
// No input variables
matrix makeRandomRotation() {
    RandomNumberGenerator& rng = RandomNumberGenerator::getInstance();
    unsigned int r = rng.getRandomInt(0, 3);
    float rot = rng.getRandomFloat(0.f, 2.0f * M_PI);
    float sinRot = std::sin(rot);
    float cosRot = std::cos(rot);

    switch (r) {
    case 0: return matrix::makeRotateX(sinRot, cosRot);
    case 1: return matrix::makeRotateY(sinRot, cosRot);
    case 2: return matrix::makeRotateZ(sinRot, cosRot);
    default: return matrix::makeIdentity();
    }
}

// Function to render a scene with multiple objects and dynamic transformations
// No input variables
void scene1() {
    Renderer renderer;
    matrix camera;
    Light L{ vec4(0.f, 1.f, 1.f, 0.f), colour(1.0f, 1.0f, 1.0f), colour(0.2f, 0.2f, 0.2f) };

    /*unsigned int numThreads = (unsigned int)std::jthread::hardware_concurrency();
    numThreads = 3;
    ThreadPool pool(numThreads);*/

    bool running = true;

    std::vector<Mesh*> scene;

    // Create a scene of 40 cubes with random rotations
    for (unsigned int i = 0; i < 20; i++) {
        Mesh* m = new Mesh();
        *m = Mesh::makeCube(1.f);
        m->world = matrix::makeTranslation(-2.0f, 0.0f, (-3 * static_cast<float>(i))) * makeRandomRotation();
        scene.push_back(m);
        m = new Mesh();
        *m = Mesh::makeCube(1.f);
        m->world = matrix::makeTranslation(2.0f, 0.0f, (-3 * static_cast<float>(i))) * makeRandomRotation();
        scene.push_back(m);
    }

    float zoffset = 8.0f; // Initial camera Z-offset
    float step = -0.1f;  // Step size for camera movement

    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::time_point<std::chrono::high_resolution_clock> end;
    int cycle = 0;

    // Main rendering loop
    while (running) {
        renderer.canvas.checkInput();
        renderer.clear();

        camera = matrix::makeTranslation(0, 0, -zoffset); // Update camera position

        // Rotate the first two cubes in the scene
        scene[0]->world = scene[0]->world * matrix::makeRotateXYZ(0.1f, 0.1f, 0.0f);
        scene[1]->world = scene[1]->world * matrix::makeRotateXYZ(0.0f, 0.1f, 0.2f);


        zoffset += step;
        if (zoffset < -60.f || zoffset > 8.f) {
            step *= -1.f;
            if (++cycle % 2 == 0) {
                end = std::chrono::high_resolution_clock::now();
                std::cout << std::chrono::duration<double, std::milli>(end - start).count() << "\n";
                start = std::chrono::high_resolution_clock::now();
            }
        }

        if (renderer.canvas.keyPressed(VK_ESCAPE)) break;

        /*for (auto& m : scene)
            render(renderer, m, camera, L);*/
        renderMT(renderer, scene, camera, L);
        //renderMTPool(pool, renderer, scene, camera, L);

        renderer.present();

        //breaks after 10 cycles
        if (cycle >= 20) break;
    }

    for (auto& m : scene)
        delete m;
}

// Scene with a grid of cubes and a moving sphere
// No input variables
void scene2() {
    Renderer renderer;
    matrix camera = matrix::makeIdentity();
    Light L{ vec4(0.f, 1.f, 1.f, 0.f), colour(1.0f, 1.0f, 1.0f), colour(0.2f, 0.2f, 0.2f) };

    unsigned int numThreads = (unsigned int)std::jthread::hardware_concurrency();
    numThreads = 4;
    ThreadPool pool(numThreads);

    std::vector<Mesh*> scene;

    struct rRot { float x; float y; float z; }; // Structure to store random rotation parameters
    std::vector<rRot> rotations;

    RandomNumberGenerator& rng = RandomNumberGenerator::getInstance();

    // Create a grid of cubes with random rotations
    for (unsigned int y = 0; y < 6; y++) {
        for (unsigned int x = 0; x < 8; x++) {
            Mesh* m = new Mesh();
            *m = Mesh::makeCube(1.f);
            scene.push_back(m);
            m->world = matrix::makeTranslation(-7.0f + (static_cast<float>(x) * 2.f), 5.0f - (static_cast<float>(y) * 2.f), -8.f);
            rRot r{ rng.getRandomFloat(-.1f, .1f), rng.getRandomFloat(-.1f, .1f), rng.getRandomFloat(-.1f, .1f) };
            rotations.push_back(r);
        }
    }

    // Create a sphere and add it to the scene
    Mesh* sphere = new Mesh();
    *sphere = Mesh::makeSphere(1.0f, 10, 20);
    scene.push_back(sphere);
    float sphereOffset = -6.f;
    float sphereStep = 0.1f;
    sphere->world = matrix::makeTranslation(sphereOffset, 0.f, -6.f);

    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::time_point<std::chrono::high_resolution_clock> end;
    int cycle = 0;

    bool running = true;
    while (running) {
        renderer.canvas.checkInput();
        renderer.clear();

        // Rotate each cube in the grid
        for (unsigned int i = 0; i < rotations.size(); i++)
            scene[i]->world = scene[i]->world * matrix::makeRotateXYZ(rotations[i].x, rotations[i].y, rotations[i].z);

        // Move the sphere back and forth
        sphereOffset += sphereStep;
        sphere->world = matrix::makeTranslation(sphereOffset, 0.f, -6.f);
        if (sphereOffset > 6.0f || sphereOffset < -6.0f) {
            sphereStep *= -1.f;
            if (++cycle % 2 == 0) {
                end = std::chrono::high_resolution_clock::now();
                std::cout << std::chrono::duration<double, std::milli>(end - start).count() << "\n";
                start = std::chrono::high_resolution_clock::now();
            }
        }

        if (renderer.canvas.keyPressed(VK_ESCAPE)) break;

       /* for (auto& m : scene)
            render(renderer, m, camera, L);*/
        //renderMT(renderer, scene, camera, L);
        renderMTPool(pool, renderer, scene, camera, L);

        renderer.present();
        //breaks after 10 cycles
        if (cycle >= 20) break;
    }

    for (auto& m : scene)
        delete m;
}

void scene3() {
    Renderer renderer;
    matrix camera = matrix::makeIdentity();
    Light L{ vec4(0.f, 1.f, 1.f, 0.f), colour(1.0f, 1.0f, 1.0f), colour(0.2f, 0.2f, 0.2f) };

    unsigned int numThreads = (unsigned int)std::jthread::hardware_concurrency();
    numThreads = 6;
    ThreadPool pool(numThreads);

    std::vector<Mesh*> scene;

    struct rRot { float x; float y; float z; }; // Structure to store random rotation parameters
    std::vector<rRot> rotations;

    RandomNumberGenerator& rng = RandomNumberGenerator::getInstance();

    // Create a grid of cubes with random rotations
    for (unsigned int y = 0; y < 15; y++) {
        for (unsigned int x = 0; x < 18; x++) {
            Mesh* m = new Mesh();
            *m = Mesh::makeCube(1.f);
            scene.push_back(m);
            m->world = matrix::makeTranslation(-20.0f + (static_cast<float>(x) * 2.f), 15.0f - (static_cast<float>(y) * 2.f), -20.f);
            rRot r{ rng.getRandomFloat(-.1f, .1f), rng.getRandomFloat(-.1f, .1f), rng.getRandomFloat(-.1f, .1f) };
            rotations.push_back(r);
        }
    }

    // Create a sphere and add it to the scene
   /* Mesh* sphere = new Mesh();
    *sphere = Mesh::makeSphere(1.0f, 10, 20);
    scene.push_back(sphere);
    float sphereOffset = -6.f;
    sphere->world = matrix::makeTranslation(sphereOffset, 0.f, -10.f);*/

    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::time_point<std::chrono::high_resolution_clock> end;
    int cycle = 0;
    float xoffset = -15;
    float step = 0.1f;

    GamesEngineeringBase::Timer tim;

    float time = 0;

    bool running = true;
    while (running) {
        renderer.canvas.checkInput();
        renderer.clear();

        time += tim.dt();

        camera = matrix::makeTranslation(xoffset, 0, 0); // Update camera position
        xoffset += step;
        if (xoffset > 15 || xoffset < -15) {
            step *= -1.f;
            if (++cycle % 2 == 0) {
                end = std::chrono::high_resolution_clock::now();
                std::cout << std::chrono::duration<double, std::milli>(end - start).count() << "\n";
                start = std::chrono::high_resolution_clock::now();
            }
        }

        // Rotate each cube in the grid
        for (unsigned int i = 0; i < rotations.size(); i++)
            scene[i]->world = scene[i]->world * matrix::makeRotateXYZ(rotations[i].x, rotations[i].y, rotations[i].z);

        //sphere->world = matrix::makeTranslation(sphereOffset + 10.0f * sin(time), 8.0f * sin(time * 1.5f + 1.0f), -15.f + 5 * sin(time * 0.5f));

        if (renderer.canvas.keyPressed(VK_ESCAPE)) break;

       /* for (auto& m : scene)
            render(renderer, m, camera, L);*/
        //renderMT(renderer, scene, camera, L);
        renderMTPool(pool, renderer, scene, camera, L);

        renderer.present();
        //breaks after 10 cycles
        if (cycle >= 20) break;
    }

    for (auto& m : scene)
        delete m;
}

// Entry point of the application
// No input variables
int main() {
    // Uncomment the desired scene function to run
    //scene1();
    //scene2();
    scene3();
    //sceneTest(); 
    

    return 0;
}