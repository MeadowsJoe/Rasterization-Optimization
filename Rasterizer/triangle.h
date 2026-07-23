#pragma once

#include "mesh.h"
#include "colour.h"
#include "renderer.h"
#include "light.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <immintrin.h>
#include <thread>
#include <vector>

// Simple support class for a 2D vector
class vec2D {
public:
    float x, y;

    // Default constructor initializes both components to 0
    vec2D() { x = y = 0.f; };

    // Constructor initializes components with given values
    vec2D(float _x, float _y) : x(_x), y(_y) {}

    // Constructor initializes components from a vec4
    vec2D(vec4 v) {
        x = v[0];
        y = v[1];
    }

    // Display the vector components
    void display() { std::cout << x << '\t' << y << std::endl; }

    // Overloaded subtraction operator for vector subtraction
    vec2D operator- (vec2D& v) {
        vec2D q;
        q.x = x - v.x;
        q.y = y - v.y;
        return q;
    }
};

// Class representing a triangle for rendering purposes
class triangle {
    Vertex v[3];       // Vertices of the triangle
    float area;        // Area of the triangle
    colour col[3];     // Colors for each vertex of the triangle

public:
    // Constructor initializes the triangle with three vertices
    // Input Variables:
    // - v1, v2, v3: Vertices defining the triangle
    triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
        v[0] = v1;
        v[1] = v2;
        v[2] = v3;

        // Calculate the 2D area of the triangle
        vec2D e1 = vec2D(v[1].p - v[0].p);
        vec2D e2 = vec2D(v[2].p - v[0].p);
        area = std::fabs(e1.x * e2.y - e1.y * e2.x);
    }

    // Helper function to compute the cross product for barycentric coordinates
    // Input Variables:
    // - v1, v2: Edges defining the vector
    // - p: Point for which coordinates are being calculated
    float getC(vec2D v1, vec2D v2, vec2D p) {
        vec2D e = v2 - v1;
        vec2D q = p - v1;
        return q.y * e.x - q.x * e.y;
    }
    __m256 getCSIMD(vec2D v1, vec2D v2, __m256& vx, __m256& vy) {
        __m256 v1x = _mm256_set1_ps(v1.x);
        __m256 v1y = _mm256_set1_ps(v1.y);
        __m256 v2x = _mm256_set1_ps(v2.x);
        __m256 v2y = _mm256_set1_ps(v2.y);
        __m256 ex = _mm256_sub_ps(v2x, v1x);
        __m256 ey = _mm256_sub_ps(v2y, v1y);
        __m256 qx = _mm256_sub_ps(vx, v1x);
        __m256 qy = _mm256_sub_ps(vy, v1y);
        __m256 r1 = _mm256_mul_ps(qy, ex);
        __m256 r2 = _mm256_mul_ps(qx, ey);

        return _mm256_sub_ps(r1, r2);
    }

    // Compute barycentric coordinates for a given point
    // Input Variables:
    // - p: Point to check within the triangle
    // Output Variables:
    // - alpha, beta, gamma: Barycentric coordinates of the point
    // Returns true if the point is inside the triangle, false otherwise
    bool getCoordinates(vec2D p, float& alpha, float& beta, float& gamma) {
        alpha = getC(vec2D(v[0].p), vec2D(v[1].p), p) / area;
        beta = getC(vec2D(v[1].p), vec2D(v[2].p), p) / area;
        gamma = getC(vec2D(v[2].p), vec2D(v[0].p), p) / area;

        if (alpha < 0.f || beta < 0.f || gamma < 0.f) return false;
        return true;
    }
    __m256 getCoordinatesSIMD(__m256& vx, __m256& vy, __m256& alpha, __m256& beta, __m256& gamma) {

        alpha = getCSIMD(vec2D(v[0].p), vec2D(v[1].p), vx, vy);
        beta = getCSIMD(vec2D(v[1].p), vec2D(v[2].p), vx, vy);
        gamma = getCSIMD(vec2D(v[2].p), vec2D(v[0].p), vx, vy);
        __m256 va = _mm256_set1_ps(area);

        alpha = _mm256_div_ps(alpha, va);
        beta = _mm256_div_ps(beta, va);
        gamma = _mm256_div_ps(gamma, va);
        __m256 zero = _mm256_setzero_ps();

        __m256 test_alpha = _mm256_cmp_ps(alpha, zero, _CMP_GE_OQ);
        __m256 test_beta = _mm256_cmp_ps(beta, zero, _CMP_GE_OQ);
        __m256 test_gamma = _mm256_cmp_ps(gamma, zero, _CMP_GE_OQ);

        __m256 inside_mask = _mm256_and_ps(_mm256_and_ps(test_alpha, test_beta), test_gamma);

        return inside_mask;
    }
    bool getCoordinatesOptimised(vec2D& p, float& alpha, float& beta, float& gamma) {
        //cache each point once
        const vec2D v0p = (vec2D(v[0].p));
        const vec2D v1p = (vec2D(v[1].p));
        const vec2D v2p = (vec2D(v[2].p));

        //multiply rather than divide
        const float invArea = 1.0f / area;

        //early exit if possible
        alpha = getC(v0p, v1p, p) * invArea;
        if (alpha < 0.f) return false;
        beta = getC(v1p, v2p, p) * invArea;
        if (beta < 0.f) return false;
        gamma = getC(v2p, v0p, p) * invArea;
        if (gamma < 0.f) return false;

        return true;
    }

    // Template function to interpolate values using barycentric coordinates
    // Input Variables:
    // - alpha, beta, gamma: Barycentric coordinates
    // - a1, a2, a3: Values to interpolate
    // Returns the interpolated value
    template <typename T>
    T interpolate(float alpha, float beta, float gamma, T a1, T a2, T a3) {
        return (a1 * beta) + (a2 * gamma) + (a3 * alpha);
    }
    __m256 interpolateSIMD(__m256 alpha, __m256 beta, __m256 gamma, __m256 a1, __m256 a2, __m256 a3) {
        return _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(a1, beta), _mm256_mul_ps(a2, gamma)), _mm256_mul_ps(a3, alpha)
        );
    }

    // Draw the triangle on the canvas
    // Input Variables:
    // - renderer: Renderer object for drawing
    // - L: Light object for shading calculations
    // - ka, kd: Ambient and diffuse lighting coefficients
     // - ka, kd: Ambient and diffuse lighting coefficients
    void draw(Renderer& renderer, Light& L, float ka, float kd) {
        vec2D minV, maxV;

        // Get the screen-space bounds of the triangle
        getBoundsWindow(renderer.canvas, minV, maxV);

        // Skip very small triangles
        if (area < 1.f) return;

        // Iterate over the bounding box and check each pixel
        for (int y = (int)(minV.y); y < (int)ceil(maxV.y); y++) {
            for (int x = (int)(minV.x); x < (int)ceil(maxV.x); x++) {
                float alpha, beta, gamma;

                // Check if the pixel lies inside the triangle
                if (getCoordinates(vec2D((float)x, (float)y), alpha, beta, gamma)) {
                    // Interpolate color, depth, and normals
                    colour c = interpolate(alpha, beta, gamma, v[0].rgb, v[1].rgb, v[2].rgb);
                    c.clampColour();
                    float depth = interpolate(alpha, beta, gamma, v[0].p[2], v[1].p[2], v[2].p[2]);
                    vec4 normal = interpolate(alpha, beta, gamma, v[0].normal, v[1].normal, v[2].normal);
                    normal.normalise();

                    // Perform Z-buffer test and apply shading
                    if (renderer.zbuffer(x, y) > depth && depth > 0.001f) {
                        // typical shader begin
                        L.omega_i.normalise();
                        float dot = std::max(vec4::dot(L.omega_i, normal), 0.0f);
                        colour a = (c * kd) * (L.L * dot) + (L.ambient * ka); // using kd instead of ka for ambient
                        // typical shader end
                        unsigned char r, g, b;
                        a.toRGB(r, g, b);
                        renderer.canvas.draw(x, y, r, g, b);
                        renderer.zbuffer(x, y) = depth;
                    }
                }
            }
        }
    }
    void drawSIMDOptimised(Renderer& renderer, Light& L, float ka, float kd) {
        vec2D minV, maxV;

        // Get the screen-space bounds of the triangle
        getBoundsWindow(renderer.canvas, minV, maxV);

        // Skip very small triangles
        if (area < 1.f) return;

        __m256 v0r = _mm256_set1_ps(v[0].rgb[colour::RED]);
        __m256 v0g = _mm256_set1_ps(v[0].rgb[colour::GREEN]);
        __m256 v0b = _mm256_set1_ps(v[0].rgb[colour::BLUE]);
        __m256 v1r = _mm256_set1_ps(v[1].rgb[colour::RED]);
        __m256 v1g = _mm256_set1_ps(v[1].rgb[colour::GREEN]);
        __m256 v1b = _mm256_set1_ps(v[1].rgb[colour::BLUE]);
        __m256 v2r = _mm256_set1_ps(v[2].rgb[colour::RED]);
        __m256 v2g = _mm256_set1_ps(v[2].rgb[colour::GREEN]);
        __m256 v2b = _mm256_set1_ps(v[2].rgb[colour::BLUE]);

        __m256 v0p2 = _mm256_set1_ps(v[0].p[2]);
        __m256 v1p2 = _mm256_set1_ps(v[1].p[2]);
        __m256 v2p2 = _mm256_set1_ps(v[2].p[2]);

        __m256 n0x = _mm256_set1_ps(v[0].normal[0]);
        __m256 n0y = _mm256_set1_ps(v[0].normal[1]);
        __m256 n0z = _mm256_set1_ps(v[0].normal[2]);
        __m256 n0w = _mm256_set1_ps(v[0].normal[3]);
        __m256 n1x = _mm256_set1_ps(v[1].normal[0]);
        __m256 n1y = _mm256_set1_ps(v[1].normal[1]);
        __m256 n1z = _mm256_set1_ps(v[1].normal[2]);
        __m256 n1w = _mm256_set1_ps(v[1].normal[3]);
        __m256 n2x = _mm256_set1_ps(v[2].normal[0]);
        __m256 n2y = _mm256_set1_ps(v[2].normal[1]);
        __m256 n2z = _mm256_set1_ps(v[2].normal[2]);
        __m256 n2w = _mm256_set1_ps(v[2].normal[3]);

        L.omega_i.normalise();

        __m256 light_x = _mm256_set1_ps(L.omega_i[0]);
        __m256 light_y = _mm256_set1_ps(L.omega_i[1]);
        __m256 light_z = _mm256_set1_ps(L.omega_i[2]);
        __m256 light_w = _mm256_set1_ps(L.omega_i[3]);

        __m256 kd_val = _mm256_set1_ps(kd);
        __m256 ka_val = _mm256_set1_ps(ka);

        __m256 light_r = _mm256_set1_ps(L.L[colour::RED]);
        __m256 light_g = _mm256_set1_ps(L.L[colour::GREEN]);
        __m256 light_b = _mm256_set1_ps(L.L[colour::BLUE]);

        __m256 ambient_r = _mm256_set1_ps(L.ambient[colour::RED]);
        __m256 ambient_g = _mm256_set1_ps(L.ambient[colour::GREEN]);
        __m256 ambient_b = _mm256_set1_ps(L.ambient[colour::BLUE]);

        __m256 one = _mm256_set1_ps(1.0f);
        __m256 zero = _mm256_setzero_ps();
        __m256 min_depth = _mm256_set1_ps(0.001f);
        __m256 epsilon = _mm256_set1_ps(0.00001f);

        for (int y = (int)(minV.y); y < (int)ceil(maxV.y); y++) {
            int x = (int)(minV.x);
            int maxX = (int)ceil(maxV.x);
            for (; x < maxX; x += 8) {

                __m256 vx = _mm256_setr_ps((float)x, (float)x + 1, (float)x + 2, (float)x + 3, (float)x + 4, (float)x + 5, (float)x + 6, (float)x + 7);
                __m256 vy = _mm256_set1_ps((float)y);

                __m256 alpha, beta, gamma;
                __m256 inside_mask = getCoordinatesSIMD(vx, vy, alpha, beta, gamma);

                //early exit if all 8 pixels are outside the triangle
                if (_mm256_testz_ps(inside_mask, inside_mask)) {
                    continue;
                }

                __m256 r = interpolateSIMD(alpha, beta, gamma, v0r, v1r, v2r);
                __m256 g = interpolateSIMD(alpha, beta, gamma, v0g, v1g, v2g);
                __m256 b = interpolateSIMD(alpha, beta, gamma, v0b, v1b, v2b);

                r = _mm256_min_ps(_mm256_max_ps(r, zero), one);
                g = _mm256_min_ps(_mm256_max_ps(g, zero), one);
                b = _mm256_min_ps(_mm256_max_ps(b, zero), one);

                __m256 depth = interpolateSIMD(alpha, beta, gamma, v0p2, v1p2, v2p2);

                __m256 nx = interpolateSIMD(alpha, beta, gamma, n0x, n1x, n2x);
                __m256 ny = interpolateSIMD(alpha, beta, gamma, n0y, n1y, n2y);
                __m256 nz = interpolateSIMD(alpha, beta, gamma, n0z, n1z, n2z);
                __m256 nw = interpolateSIMD(alpha, beta, gamma, n0w, n1w, n2w);

                __m256 len_sq = _mm256_add_ps(_mm256_add_ps(_mm256_add_ps(
                    _mm256_mul_ps(nx, nx), _mm256_mul_ps(ny, ny)), _mm256_mul_ps(nz, nz)), _mm256_mul_ps(nw, nw));

                __m256 len = _mm256_sqrt_ps(len_sq);

                //taking a max to avoid divding by zero
                len = _mm256_max_ps(len, epsilon);

                //normalising the vectors
                nx = _mm256_div_ps(nx, len);
                ny = _mm256_div_ps(ny, len);
                nz = _mm256_div_ps(nz, len);
                nw = _mm256_div_ps(nw, len);

                alignas(64) float zbuffer_values[8];
                zbuffer_values[0] = renderer.zbuffer(x + 0, y);
                zbuffer_values[1] = renderer.zbuffer(x + 1, y);
                zbuffer_values[2] = renderer.zbuffer(x + 2, y);
                zbuffer_values[3] = renderer.zbuffer(x + 3, y);
                zbuffer_values[4] = renderer.zbuffer(x + 4, y);
                zbuffer_values[5] = renderer.zbuffer(x + 5, y);
                zbuffer_values[6] = renderer.zbuffer(x + 6, y);
                zbuffer_values[7] = renderer.zbuffer(x + 7, y);

                __m256 current_depth = _mm256_load_ps(zbuffer_values);

                __m256 depth_test = _mm256_and_ps(_mm256_cmp_ps(current_depth, depth, _CMP_GT_OQ), _mm256_cmp_ps(depth, min_depth, _CMP_GT_OQ));

                __m256 final_mask = _mm256_and_ps(inside_mask, depth_test);

                //if pixel inside triangle and new pixel between prev depth check and camera, then continue
                if (_mm256_testz_ps(final_mask, final_mask)) {
                    continue;
                }

                //brightness of pixel
                __m256 dot = _mm256_add_ps(_mm256_add_ps(_mm256_add_ps(
                    _mm256_mul_ps(light_x, nx),
                    _mm256_mul_ps(light_y, ny)),
                    _mm256_mul_ps(light_z, nz)),
                    _mm256_mul_ps(light_w, nw));

                //clamping
                dot = _mm256_max_ps(dot, zero);

                __m256 final_r = _mm256_add_ps(_mm256_mul_ps(
                    _mm256_mul_ps(r, kd_val), //surface colour * kd
                    _mm256_mul_ps(light_r, dot)), // light colour * dot
                    _mm256_mul_ps(ambient_r, ka_val));// ambient * ka
                __m256 final_g = _mm256_add_ps(_mm256_mul_ps(
                    _mm256_mul_ps(g, kd_val),
                    _mm256_mul_ps(light_g, dot)),
                    _mm256_mul_ps(ambient_g, ka_val));
                __m256 final_b = _mm256_add_ps(_mm256_mul_ps(
                    _mm256_mul_ps(b, kd_val),
                    _mm256_mul_ps(light_b, dot)),
                    _mm256_mul_ps(ambient_b, ka_val));

                //clamping
                final_r = _mm256_min_ps(_mm256_max_ps(final_r, zero), one);
                final_g = _mm256_min_ps(_mm256_max_ps(final_g, zero), one);
                final_b = _mm256_min_ps(_mm256_max_ps(final_b, zero), one);

                //temp arrays
                alignas(64) float mask_array[8];
                alignas(64) float r_array[8], g_array[8], b_array[8];
                alignas(64) float depth_array[8];

                //storing simd data into tempo arrays
                _mm256_store_ps(mask_array, final_mask);
                _mm256_store_ps(r_array, final_r);
                _mm256_store_ps(g_array, final_g);
                _mm256_store_ps(b_array, final_b);
                _mm256_store_ps(depth_array, depth);

                for (int i = 0; i < 8; i++) {
                    //checking if each mask passed checks
                    if (mask_array[i] != 0.0f) {
                        //converting from float to 255
                        unsigned char r_byte = (unsigned char)(r_array[i] * 255.0f);
                        unsigned char g_byte = (unsigned char)(g_array[i] * 255.0f);
                        unsigned char b_byte = (unsigned char)(b_array[i] * 255.0f);

                        renderer.canvas.draw(x + i, y, r_byte, g_byte, b_byte);
                        renderer.zbuffer(x + i, y) = depth_array[i];
                    }
                }
            }
            //complete as normal for remainin pixels
            for (; x < maxX; x++) {
                float alpha, beta, gamma;

                vec2D p((float)x, (float)y);
                // Check if the pixel lies inside the triangle
                if (getCoordinatesOptimised(p, alpha, beta, gamma)) {
                    // Interpolate color, depth, and normals
                    colour c = interpolate(alpha, beta, gamma, v[0].rgb, v[1].rgb, v[2].rgb);
                    c.clampColour();
                    float depth = interpolate(alpha, beta, gamma, v[0].p[2], v[1].p[2], v[2].p[2]);
                    vec4 normal = interpolate(alpha, beta, gamma, v[0].normal, v[1].normal, v[2].normal);
                    normal.normalise();

                    // Perform Z-buffer test and apply shading
                    if (renderer.zbuffer(x, y) > depth && depth > 0.001f) {
                        // typical shader begin
                        L.omega_i.normalise();
                        float dot = std::max(vec4::dot(L.omega_i, normal), 0.0f);
                        colour a = (c * kd) * (L.L * dot) + (L.ambient * ka); // using kd instead of ka for ambient
                        // typical shader end
                        unsigned char r, g, b;
                        a.toRGB(r, g, b);
                        renderer.canvas.draw(x, y, r, g, b);
                        renderer.zbuffer(x, y) = depth;
                    }
                }
            }
        }
    }

    void drawSIMDOptimisedStrip(Renderer& renderer, Light& L, float ka, float kd, int yMin, int yMax) {
        vec2D minV, maxV;

        // Get the screen-space bounds of the triangle
        getBoundsWindow(renderer.canvas, minV, maxV);

        // Skip very small triangles
        if (area < 1.f) return;

        __m256 v0r = _mm256_set1_ps(v[0].rgb[colour::RED]);
        __m256 v0g = _mm256_set1_ps(v[0].rgb[colour::GREEN]);
        __m256 v0b = _mm256_set1_ps(v[0].rgb[colour::BLUE]);
        __m256 v1r = _mm256_set1_ps(v[1].rgb[colour::RED]);
        __m256 v1g = _mm256_set1_ps(v[1].rgb[colour::GREEN]);
        __m256 v1b = _mm256_set1_ps(v[1].rgb[colour::BLUE]);
        __m256 v2r = _mm256_set1_ps(v[2].rgb[colour::RED]);
        __m256 v2g = _mm256_set1_ps(v[2].rgb[colour::GREEN]);
        __m256 v2b = _mm256_set1_ps(v[2].rgb[colour::BLUE]);

        __m256 v0p2 = _mm256_set1_ps(v[0].p[2]);
        __m256 v1p2 = _mm256_set1_ps(v[1].p[2]);
        __m256 v2p2 = _mm256_set1_ps(v[2].p[2]);

        __m256 n0x = _mm256_set1_ps(v[0].normal[0]);
        __m256 n0y = _mm256_set1_ps(v[0].normal[1]);
        __m256 n0z = _mm256_set1_ps(v[0].normal[2]);
        __m256 n0w = _mm256_set1_ps(v[0].normal[3]);
        __m256 n1x = _mm256_set1_ps(v[1].normal[0]);
        __m256 n1y = _mm256_set1_ps(v[1].normal[1]);
        __m256 n1z = _mm256_set1_ps(v[1].normal[2]);
        __m256 n1w = _mm256_set1_ps(v[1].normal[3]);
        __m256 n2x = _mm256_set1_ps(v[2].normal[0]);
        __m256 n2y = _mm256_set1_ps(v[2].normal[1]);
        __m256 n2z = _mm256_set1_ps(v[2].normal[2]);
        __m256 n2w = _mm256_set1_ps(v[2].normal[3]);

        L.omega_i.normalise();

        __m256 light_x = _mm256_set1_ps(L.omega_i[0]);
        __m256 light_y = _mm256_set1_ps(L.omega_i[1]);
        __m256 light_z = _mm256_set1_ps(L.omega_i[2]);
        __m256 light_w = _mm256_set1_ps(L.omega_i[3]);

        __m256 kd_val = _mm256_set1_ps(kd);
        __m256 ka_val = _mm256_set1_ps(ka);

        __m256 light_r = _mm256_set1_ps(L.L[colour::RED]);
        __m256 light_g = _mm256_set1_ps(L.L[colour::GREEN]);
        __m256 light_b = _mm256_set1_ps(L.L[colour::BLUE]);

        __m256 ambient_r = _mm256_set1_ps(L.ambient[colour::RED]);
        __m256 ambient_g = _mm256_set1_ps(L.ambient[colour::GREEN]);
        __m256 ambient_b = _mm256_set1_ps(L.ambient[colour::BLUE]);

        __m256 one = _mm256_set1_ps(1.0f);
        __m256 zero = _mm256_setzero_ps();
        __m256 min_depth = _mm256_set1_ps(0.001f);
        __m256 epsilon = _mm256_set1_ps(0.00001f);

        int startY = std::max((int)(minV.y), yMin);
        int endY = std::min((int)ceil(maxV.y), yMax);

        for (int y = startY; y < endY; y++) {
            int x = (int)(minV.x);
            int maxX = (int)ceil(maxV.x);
            for (; x < maxX; x += 8) {

                __m256 vx = _mm256_setr_ps((float)x, (float)x + 1, (float)x + 2, (float)x + 3, (float)x + 4, (float)x + 5, (float)x + 6, (float)x + 7);
                __m256 vy = _mm256_set1_ps((float)y);

                __m256 alpha, beta, gamma;
                __m256 inside_mask = getCoordinatesSIMD(vx, vy, alpha, beta, gamma);

                //early exit if all 8 pixels are outside the triangle
                if (_mm256_testz_ps(inside_mask, inside_mask)) {
                    continue;
                }

                __m256 r = interpolateSIMD(alpha, beta, gamma, v0r, v1r, v2r);
                __m256 g = interpolateSIMD(alpha, beta, gamma, v0g, v1g, v2g);
                __m256 b = interpolateSIMD(alpha, beta, gamma, v0b, v1b, v2b);

                r = _mm256_min_ps(_mm256_max_ps(r, zero), one);
                g = _mm256_min_ps(_mm256_max_ps(g, zero), one);
                b = _mm256_min_ps(_mm256_max_ps(b, zero), one);

                __m256 depth = interpolateSIMD(alpha, beta, gamma, v0p2, v1p2, v2p2);

                __m256 nx = interpolateSIMD(alpha, beta, gamma, n0x, n1x, n2x);
                __m256 ny = interpolateSIMD(alpha, beta, gamma, n0y, n1y, n2y);
                __m256 nz = interpolateSIMD(alpha, beta, gamma, n0z, n1z, n2z);
                __m256 nw = interpolateSIMD(alpha, beta, gamma, n0w, n1w, n2w);

                __m256 len_sq = _mm256_add_ps(_mm256_add_ps(_mm256_add_ps(
                    _mm256_mul_ps(nx, nx), _mm256_mul_ps(ny, ny)), _mm256_mul_ps(nz, nz)), _mm256_mul_ps(nw, nw));

                __m256 len = _mm256_sqrt_ps(len_sq);

                //taking a max to avoid divding by zero
                len = _mm256_max_ps(len, epsilon);

                //normalising the vectors
                nx = _mm256_div_ps(nx, len);
                ny = _mm256_div_ps(ny, len);
                nz = _mm256_div_ps(nz, len);
                nw = _mm256_div_ps(nw, len);

                alignas(64) float zbuffer_values[8];
                zbuffer_values[0] = renderer.zbuffer(x + 0, y);
                zbuffer_values[1] = renderer.zbuffer(x + 1, y);
                zbuffer_values[2] = renderer.zbuffer(x + 2, y);
                zbuffer_values[3] = renderer.zbuffer(x + 3, y);
                zbuffer_values[4] = renderer.zbuffer(x + 4, y);
                zbuffer_values[5] = renderer.zbuffer(x + 5, y);
                zbuffer_values[6] = renderer.zbuffer(x + 6, y);
                zbuffer_values[7] = renderer.zbuffer(x + 7, y);

                __m256 current_depth = _mm256_load_ps(zbuffer_values);

                __m256 depth_test = _mm256_and_ps(_mm256_cmp_ps(current_depth, depth, _CMP_GT_OQ), _mm256_cmp_ps(depth, min_depth, _CMP_GT_OQ));

                __m256 final_mask = _mm256_and_ps(inside_mask, depth_test);

                //if pixel inside triangle and new pixel between prev depth check and camera, then continue
                if (_mm256_testz_ps(final_mask, final_mask)) {
                    continue;
                }

                //brightness of pixel
                __m256 dot = _mm256_add_ps(_mm256_add_ps(_mm256_add_ps(
                    _mm256_mul_ps(light_x, nx),
                    _mm256_mul_ps(light_y, ny)),
                    _mm256_mul_ps(light_z, nz)),
                    _mm256_mul_ps(light_w, nw));

                //clamping
                dot = _mm256_max_ps(dot, zero);

                __m256 final_r = _mm256_add_ps(_mm256_mul_ps(
                    _mm256_mul_ps(r, kd_val), //surface colour * kd
                    _mm256_mul_ps(light_r, dot)), // light colour * dot
                    _mm256_mul_ps(ambient_r, ka_val));// ambient * ka
                __m256 final_g = _mm256_add_ps(_mm256_mul_ps(
                    _mm256_mul_ps(g, kd_val),
                    _mm256_mul_ps(light_g, dot)),
                    _mm256_mul_ps(ambient_g, ka_val));
                __m256 final_b = _mm256_add_ps(_mm256_mul_ps(
                    _mm256_mul_ps(b, kd_val),
                    _mm256_mul_ps(light_b, dot)),
                    _mm256_mul_ps(ambient_b, ka_val));

                //clamping
                final_r = _mm256_min_ps(_mm256_max_ps(final_r, zero), one);
                final_g = _mm256_min_ps(_mm256_max_ps(final_g, zero), one);
                final_b = _mm256_min_ps(_mm256_max_ps(final_b, zero), one);

                //temp arrays
                alignas(64) float mask_array[8];
                alignas(64) float r_array[8], g_array[8], b_array[8];
                alignas(64) float depth_array[8];

                //storing simd data into tempo arrays
                _mm256_store_ps(mask_array, final_mask);
                _mm256_store_ps(r_array, final_r);
                _mm256_store_ps(g_array, final_g);
                _mm256_store_ps(b_array, final_b);
                _mm256_store_ps(depth_array, depth);

                for (int i = 0; i < 8; i++) {
                    //checking if each mask passed checks
                    if (mask_array[i] != 0.0f) {
                        //converting from float to 255
                        unsigned char r_byte = (unsigned char)(r_array[i] * 255.0f);
                        unsigned char g_byte = (unsigned char)(g_array[i] * 255.0f);
                        unsigned char b_byte = (unsigned char)(b_array[i] * 255.0f);

                        renderer.canvas.draw(x + i, y, r_byte, g_byte, b_byte);
                        renderer.zbuffer(x + i, y) = depth_array[i];
                    }
                }
            }
            //complete as normal for remainin pixels
            for (; x < maxX; x++) {
                float alpha, beta, gamma;

                vec2D p((float)x, (float)y);
                // Check if the pixel lies inside the triangle
                if (getCoordinatesOptimised(p, alpha, beta, gamma)) {
                    // Interpolate color, depth, and normals
                    colour c = interpolate(alpha, beta, gamma, v[0].rgb, v[1].rgb, v[2].rgb);
                    c.clampColour();
                    float depth = interpolate(alpha, beta, gamma, v[0].p[2], v[1].p[2], v[2].p[2]);
                    vec4 normal = interpolate(alpha, beta, gamma, v[0].normal, v[1].normal, v[2].normal);
                    normal.normalise();

                    // Perform Z-buffer test and apply shading
                    if (renderer.zbuffer(x, y) > depth && depth > 0.001f) {
                        // typical shader begin
                        L.omega_i.normalise();
                        float dot = std::max(vec4::dot(L.omega_i, normal), 0.0f);
                        colour a = (c * kd) * (L.L * dot) + (L.ambient * ka); // using kd instead of ka for ambient
                        // typical shader end
                        unsigned char r, g, b;
                        a.toRGB(r, g, b);
                        renderer.canvas.draw(x, y, r, g, b);
                        renderer.zbuffer(x, y) = depth;
                    }
                }
            }
        }
    }

    // Compute the 2D bounds of the triangle
    // Output Variables:
    // - minV, maxV: Minimum and maximum bounds in 2D space
    void getBounds(vec2D& minV, vec2D& maxV) {
        minV = vec2D(v[0].p);
        maxV = vec2D(v[0].p);
        for (unsigned int i = 1; i < 3; i++) {
            minV.x = std::min(minV.x, v[i].p[0]);
            minV.y = std::min(minV.y, v[i].p[1]);
            maxV.x = std::max(maxV.x, v[i].p[0]);
            maxV.y = std::max(maxV.y, v[i].p[1]);
        }
    }

    // Compute the 2D bounds of the triangle, clipped to the canvas
    // Input Variables:
    // - canvas: Reference to the rendering canvas
    // Output Variables:
    // - minV, maxV: Clipped minimum and maximum bounds
    void getBoundsWindow(GamesEngineeringBase::Window& canvas, vec2D& minV, vec2D& maxV) {
        getBounds(minV, maxV);
        minV.x = std::max(minV.x, static_cast<float>(0));
        minV.y = std::max(minV.y, static_cast<float>(0));
        maxV.x = std::min(maxV.x, static_cast<float>(canvas.getWidth()));
        maxV.y = std::min(maxV.y, static_cast<float>(canvas.getHeight()));
    }

    // Debugging utility to display the triangle bounds on the canvas
    // Input Variables:
    // - canvas: Reference to the rendering canvas
    void drawBounds(GamesEngineeringBase::Window& canvas) {
        vec2D minV, maxV;
        getBounds(minV, maxV);

        for (int y = (int)minV.y; y < (int)maxV.y; y++) {
            for (int x = (int)minV.x; x < (int)maxV.x; x++) {
                canvas.draw(x, y, 255, 0, 0);
            }
        }
    }

    // Debugging utility to display the coordinates of the triangle vertices
    void display() {
        for (unsigned int i = 0; i < 3; i++) {
            v[i].p.display();
        }
        std::cout << std::endl;
    }
};