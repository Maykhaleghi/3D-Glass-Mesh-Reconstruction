#include <iostream>
#include <fstream>
#include <vector>

struct Vec3 { float x, y, z; };

int main() {
    std::vector<Vec3> vertices = {
        {-0.5f, -0.5f, -0.5f},
        { 0.5f, -0.5f, -0.5f},
        { 0.5f,  0.5f, -0.5f},
        {-0.5f,  0.5f, -0.5f},
        {-0.5f, -0.5f,  0.5f},
        { 0.5f, -0.5f,  0.5f},
        { 0.5f,  0.5f,  0.5f},
        {-0.5f,  0.5f,  0.5f}
    };

    std::vector<std::vector<int>> faces = {
        {0, 1, 2, 3},
        {4, 5, 6, 7},
        {0, 1, 5, 4},
        {2, 3, 7, 6},
        {0, 3, 7, 4},
        {1, 2, 6, 5}
    };

    std::ofstream out("cube.off");
    if (!out) {
        std::cerr << "Cannot open file for writing!\n";
        return 1;
    }

    out << "OFF\n";
    out << vertices.size() << " " << faces.size() << " 0\n";

    for (const auto& v : vertices)
        out << v.x << " " << v.y << " " << v.z << "\n";

    for (const auto& f : faces) {
        out << f.size() << " ";
        for (int idx : f) out << idx << " ";
        out << "\n";
    }

    out.close();
    std::cout << "cube.off generated successfully.\n";
    return 0;
}

