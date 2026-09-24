#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cassert>

// ---------- Vec3 and Mesh ----------
struct Vec3 {
    float x, y, z;
};

class Mesh {
public:
    std::vector<Vec3> vertices;
    std::vector<std::vector<int>> faces;

    void addVertex(const Vec3& v) {
        vertices.push_back(v);
    }

    void addFace(int a, int b, int c) {
        faces.push_back({a, b, c});
    }

    void saveOFF(const std::string& filename) {
        std::ofstream out(filename);
        out << "OFF\n";
        out << vertices.size() << " " << faces.size() << " 0\n";
        for (const auto& v : vertices) {
            out << v.x << " " << v.y << " " << v.z << "\n";
        }
        for (const auto& f : faces) {
            out << "3 " << f[0] << " " << f[1] << " " << f[2] << "\n";
        }
        out.close();
        std::cout << "Saved mesh to " << filename << "\n";
    }
};

// ---------- Non-Uniform Catmull-Rom Evaluator ----------
static std::pair<float, float> evaluateNonUniformCatmullRom(
    const std::vector<Vec3>& profile,
    float y_target)
{
    assert(!profile.empty());
    const size_t n = profile.size();

    if (n == 1) return {profile[0].x, 0.0f};
    if (n == 2) {
        float y0 = profile[0].y, y1 = profile[1].y;
        float r0 = profile[0].x, r1 = profile[1].x;
        if (std::abs(y1 - y0) < 1e-8f) return {r0, 0.0f};
        float t = (y_target - y0) / (y1 - y0);
        t = std::clamp(t, 0.0f, 1.0f);
        return {r0 + t * (r1 - r0), (r1 - r0) / (y1 - y0)};
    }

    float y_max = profile.front().y;
    float y_min = profile.back().y;
    if (y_target > y_max) y_target = y_max;
    if (y_target < y_min) y_target = y_min;

    size_t i = 0;
    while (i < n - 2 && profile[i + 1].y > y_target) ++i;

    auto getPoint = [&](int idx) -> Vec3 {
        if (idx < 0) {
            Vec3 p = profile[0];
            p.y = profile[0].y + (profile[0].y - profile[1].y);
            return p;
        }
        if (idx >= (int)n) {
            Vec3 p = profile[n - 1];
            p.y = profile[n - 1].y - (profile[n - 2].y - profile[n - 1].y);
            return p;
        }
        return profile[idx];
    };

    Vec3 p0 = getPoint((int)i - 1);
    Vec3 p1 = getPoint((int)i);
    Vec3 p2 = getPoint((int)i + 1);
    Vec3 p3 = getPoint((int)i + 2);

    float y1 = p1.y, y2 = p2.y;
    if (std::abs(y2 - y1) < 1e-8f) return {p1.x, 0.0f};

    float t = (y_target - y1) / (y2 - y1);
    t = std::clamp(t, 0.0f, 1.0f);

    float m1 = (std::abs(p2.y - p0.y) > 1e-8f) ? (p2.x - p0.x) / (p2.y - p0.y) : 0.0f;
    float m2 = (std::abs(p3.y - p1.y) > 1e-8f) ? (p3.x - p1.x) / (p3.y - p1.y) : 0.0f;

    float t2 = t * t;
    float t3 = t2 * t;
    float h00 =  2*t3 - 3*t2 + 1;
    float h10 =    t3 - 2*t2 + t;
    float h01 = -2*t3 + 3*t2;
    float h11 =    t3 -   t2;

    float r = h00 * p1.x + h10 * (y2 - y1) * m1 +
              h01 * p2.x + h11 * (y2 - y1) * m2;

    float dh00 = 6*t2 - 6*t;
    float dh10 = 3*t2 - 4*t + 1;
    float dh01 = -6*t2 + 6*t;
    float dh11 = 3*t2 - 2*t;

    float dr_dt = dh00 * p1.x + dh10 * (y2 - y1) * m1 +
                  dh01 * p2.x + dh11 * (y2 - y1) * m2;
    float dr_dy = dr_dt / (y2 - y1);

    return {r, dr_dy};
}

// ---------- Smooth Glass Generator ----------
Mesh makeSmoothGlassThick(
    const std::vector<Vec3>& outerProfile,
    const std::vector<Vec3>& innerProfile,
    int angularSlices = 128,   // increased for smooth rim
    int heightSamples = 500)
{
    Mesh mesh;

    // Sample dense profile with EXACT endpoints
    auto sampleDenseProfile = [&](const std::vector<Vec3>& profile) {
        std::vector<Vec3> sampled;
        float y_start = profile.front().y;
        float y_end = profile.back().y;
        for (int i = 0; i < heightSamples; ++i) {
            if (i == 0) {
                sampled.push_back({profile.front().x, profile.front().y, 0.0f});
            } else if (i == heightSamples - 1) {
                sampled.push_back({profile.back().x, profile.back().y, 0.0f});
            } else {
                float t = static_cast<float>(i) / (heightSamples - 1);
                float y = y_start + t * (y_end - y_start);
                auto [r, dr_dy] = evaluateNonUniformCatmullRom(profile, y);
                sampled.push_back({r, y, 0.0f});
            }
        }
        return sampled;
    };

    auto denseOuter = sampleDenseProfile(outerProfile);
    auto denseInner = sampleDenseProfile(innerProfile);

    int outerCount = denseOuter.size();
    int innerCount = denseInner.size();

    // --- Outer surface vertices ---
    size_t outerStart = mesh.vertices.size();
    for (int i = 0; i < outerCount; ++i) {
        for (int j = 0; j < angularSlices; ++j) {
            float theta = 2.0f * M_PI * j / angularSlices;
            float x = denseOuter[i].x * std::cos(theta);
            float z = denseOuter[i].x * std::sin(theta);
            mesh.addVertex({x, denseOuter[i].y, z});
        }
    }

    // --- Inner surface vertices ---
    size_t innerStart = mesh.vertices.size();
    for (int i = 0; i < innerCount; ++i) {
        for (int j = 0; j < angularSlices; ++j) {
            float theta = 2.0f * M_PI * j / angularSlices;
            float x = denseInner[i].x * std::cos(theta);
            float z = denseInner[i].x * std::sin(theta);
            mesh.addVertex({x, denseInner[i].y, z});
        }
    }

    // --- Outer faces (CCW) ---
    for (int i = 0; i < outerCount - 1; ++i) {
        for (int j = 0; j < angularSlices; ++j) {
            int j1 = (j + 1) % angularSlices;
            int v0 = outerStart + i * angularSlices + j;
            int v1 = outerStart + i * angularSlices + j1;
            int v2 = outerStart + (i + 1) * angularSlices + j1;
            int v3 = outerStart + (i + 1) * angularSlices + j;
            mesh.addFace(v0, v1, v2);
            mesh.addFace(v0, v2, v3);
        }
    }

    // --- Inner faces (CW winding for outward normals) ---
    for (int i = 0; i < innerCount - 1; ++i) {
        for (int j = 0; j < angularSlices; ++j) {
            int j1 = (j + 1) % angularSlices;
            int v0 = innerStart + i * angularSlices + j;
            int v1 = innerStart + i * angularSlices + j1;
            int v2 = innerStart + (i + 1) * angularSlices + j1;
            int v3 = innerStart + (i + 1) * angularSlices + j;
            mesh.addFace(v0, v2, v1); // reversed
            mesh.addFace(v0, v3, v2);
        }
    }

    // --- RIM: Correct non-twisted quad strip ---
    for (int j = 0; j < angularSlices; ++j) {
        int j1 = (j + 1) % angularSlices;
        int v_o0 = outerStart + j;
        int v_o1 = outerStart + j1;
        int v_i0 = innerStart + j;
        int v_i1 = innerStart + j1;

        // Two triangles per quad: [o0, o1, i1] and [o0, i1, i0]
        mesh.addFace(v_o0, v_o1, v_i1);
        mesh.addFace(v_o0, v_i1, v_i0);
    }

    // --- Bottom caps ---
    auto addCap = [&](size_t ringBase, float y_center) {
        size_t center = mesh.vertices.size();
        mesh.addVertex({0.0f, y_center, 0.0f});
        for (int j = 0; j < angularSlices; ++j) {
            int j1 = (j + 1) % angularSlices;
            mesh.addFace(center, ringBase + j1, ringBase + j);
        }
    };

    addCap(outerStart + (outerCount - 1) * angularSlices, denseOuter.back().y);
    addCap(innerStart + (innerCount - 1) * angularSlices, denseInner.back().y);

    return mesh;
}

// ---------- Main ----------
int main() {
    std::vector<Vec3> outerProfile = {
        {1.4f, 10.5f, 0.0f}, {1.7f, 9.4f, 0.0f}, {1.85f, 8.3f, 0.0f}, {1.93f, 7.5f, 0.0f},
        {1.93f, 7.0f, 0.0f}, {1.8f, 6.6f, 0.0f}, {1.6f, 6.2f, 0.0f}, {1.4f, 5.9f, 0.0f},
        {1.1f, 5.5f, 0.0f}, {0.9f, 5.4f, 0.0f}, {0.4f, 4.9f, 0.0f}, {0.3f, 4.7f, 0.0f},
        {0.2f, 4.1f, 0.0f}, {0.2f, 2.6f, 0.0f}, {0.3f, 2.3f, 0.0f}, {0.4f, 1.9f, 0.0f},
        {0.6f, 1.7f, 0.0f}, {0.9f, 1.5f, 0.0f}, {1.4f, 1.4f, 0.0f}, {1.7f, 1.3f, 0.0f},
        {1.7f, 1.2f, 0.0f}
    };

    std::vector<Vec3> innerProfile = {
        {1.3f, 10.5f, 0.0f}, {1.6f, 9.4f, 0.0f}, {1.7f, 8.3f, 0.0f}, {1.8f, 7.7f, 0.0f},
        {1.8f, 7.0f, 0.0f}, {1.7f, 6.6f, 0.0f}, {1.5f, 6.2f, 0.0f}, {1.3f, 5.9f, 0.0f},
        {1.0f, 5.5f, 0.0f}, {0.7f, 5.4f, 0.0f}, {0.2f, 5.2f, 0.0f}
    };

    Mesh smoothGlass = makeSmoothGlassThick(outerProfile, innerProfile, 128, 500);
    smoothGlass.saveOFF("glass_smooth.off");

    std::cout << "✅ Run: WAYLAND_DISPLAY= meshlab glass_smooth.off\n";
    return 0;
}