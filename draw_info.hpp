#ifndef DRAW_INFO_HPP
#define DRAW_INFO_HPP

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "sbpt_generated_includes.hpp"

namespace draw_info {

class IndexedVertexPositions {
  public:
    IndexedVertexPositions(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions)
        : indices(indices), xyz_positions(xyz_positions) {};
    Transform transform;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
};

class IVPSolidColor {
  public:
    IVPSolidColor(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions,
                  std::vector<glm::vec3> rgb_colors)
        : indices(indices), xyz_positions(xyz_positions), rgb_colors(rgb_colors) {};
    Transform transform;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    std::vector<glm::vec2> texture_coordinates;
    std::vector<glm::vec3> rgb_colors;
};

class IVPTextured {
  public:
    IVPTextured(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions,
                std::vector<glm::vec2> texture_coordinates, const std::string &texture = "")
        : indices(indices), xyz_positions(xyz_positions), texture_coordinates(texture_coordinates),
          texture_path(texture) {};
    Transform transform;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    std::vector<glm::vec2> texture_coordinates;
    std::string texture_path;
};

// with normals
class IVPNTextured {
  public:
    IVPNTextured(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions,
                 std::vector<glm::vec3> normals, std::vector<glm::vec2> texture_coordinates,
                 const std::string &texture = "")
        : indices(indices), xyz_positions(xyz_positions), normals(normals), texture_coordinates(texture_coordinates),
          texture_path(texture) {};
    Transform transform;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texture_coordinates;
    std::string texture_path;
};

class IVPTexturePacked {
  public:
    IVPTexturePacked(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions,
                     std::vector<glm::vec2> original_texture_coordinates,
                     std::vector<glm::vec2> packed_texture_coordinates, int packed_texture_index,
                     int packed_texture_bounding_box_index, const std::string &texture)
        : indices(indices), xyz_positions(xyz_positions), original_texture_coordinates(original_texture_coordinates),
          packed_texture_coordinates(packed_texture_coordinates), packed_texture_index(packed_texture_index),
          packed_texture_bounding_box_index(packed_texture_bounding_box_index), texture_path(texture) {};
    Transform transform;
    int id = UniqueIDGenerator::generate();
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    std::vector<glm::vec2> original_texture_coordinates;
    std::vector<glm::vec2> packed_texture_coordinates;
    int packed_texture_index;
    int packed_texture_bounding_box_index;
    std::string texture_path;
};

class IVPNTexturePacked {
  public:
    IVPNTexturePacked(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions,
                      std::vector<glm::vec3> normals, std::vector<glm::vec2> original_texture_coordinates,
                      std::vector<glm::vec2> packed_texture_coordinates, int packed_texture_index,
                      int packed_texture_bounding_box_index, const std::string &texture)
        : indices(indices), xyz_positions(xyz_positions), normals(normals),
          original_texture_coordinates(original_texture_coordinates),
          packed_texture_coordinates(packed_texture_coordinates), packed_texture_index(packed_texture_index),
          packed_texture_bounding_box_index(packed_texture_bounding_box_index), texture_path(texture) {};
    Transform transform;
    int id = UniqueIDGenerator::generate();
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    std::vector<glm::vec3> normals;
    // the reason we need this is so that we can regenerate based on the original ones
    std::vector<glm::vec2> original_texture_coordinates;
    std::vector<glm::vec2> packed_texture_coordinates;
    int packed_texture_index;
    int packed_texture_bounding_box_index;
    std::string texture_path;
};

IndexedVertexPositions extract_indexed_vertex_positions(const IVPTextured &ivp_textured);
IndexedVertexPositions extract_indexed_vertex_positions(const IVPTexturePacked &ivp_texture_packed);

std::vector<IndexedVertexPositions>
extract_indexed_vertex_positions_vector(const std::vector<IVPTextured> &ivp_textured_vector);
std::vector<IndexedVertexPositions>
extract_indexed_vertex_positions_vector(const std::vector<IVPTexturePacked> &ivp_texture_packed_vector);

} // namespace draw_info

#endif // DRAW_INFO_HPP
