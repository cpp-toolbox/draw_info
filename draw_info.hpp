#ifndef DRAW_INFO_HPP
#define DRAW_INFO_HPP

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "sbpt_generated_includes.hpp"

namespace draw_info {

class IndexedVertexPositions { // IVP
  public:
    IndexedVertexPositions() {};
    IndexedVertexPositions(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions,
                           int id = GlobalUIDGenerator::get_id())
        : indices(indices), xyz_positions(xyz_positions), id(id) {};
    Transform transform;
    int id;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;

    friend std::ostream &operator<<(std::ostream &os, const IndexedVertexPositions &ivp) {
        os << "IndexedVertexPositions("
           << "indices.size=" << ivp.indices.size() << ", "
           << "xyz_positions.size=" << ivp.xyz_positions.size() << ", "
           << "transform=" << ivp.transform << ")";
        return os;
    }
};

class TransformedIVPGroup { // TIG
  public:
    TransformedIVPGroup() {}
    TransformedIVPGroup(std::vector<IndexedVertexPositions> ivps, int id) : ivps(std::move(ivps)), id(id) {}
    int id;
    std::vector<IndexedVertexPositions> ivps;
    Transform transform;
};

class IVPSolidColor { // IVPSC
  public:
    IVPSolidColor() {};
    IVPSolidColor(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions,
                  std::vector<glm::vec3> rgb_colors, int id = GlobalUIDGenerator::get_id())
        : indices(indices), xyz_positions(xyz_positions), rgb_colors(rgb_colors), id(id) {};

    // TODO: remove
    Transform transform;
    int id;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    std::vector<glm::vec2> texture_coordinates;
    std::vector<glm::vec3> rgb_colors;
};

class IVPTextured { // IVPT
  public:
    IVPTextured(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions,
                std::vector<glm::vec2> texture_coordinates, const std::string &texture = "",
                int id = GlobalUIDGenerator::get_id())
        : indices(indices), xyz_positions(xyz_positions), texture_coordinates(texture_coordinates),
          texture_path(texture), id(id) {};
    // TODO: remove this just make a new class for it wrapping like the tig
    Transform transform;
    int id;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    std::vector<glm::vec2> texture_coordinates;
    std::string texture_path;
};

class IVPNTextured { // IVP Normals
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

class IVPTexturePacked { // IVPTP
  public:
    IVPTexturePacked(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions,
                     std::vector<glm::vec2> original_texture_coordinates,
                     std::vector<glm::vec2> packed_texture_coordinates, int packed_texture_index,
                     int packed_texture_bounding_box_index, const std::string &texture,
                     int id = GlobalUIDGenerator::get_id())
        : indices(indices), xyz_positions(xyz_positions), original_texture_coordinates(original_texture_coordinates),
          packed_texture_coordinates(packed_texture_coordinates), packed_texture_index(packed_texture_index),
          packed_texture_bounding_box_index(packed_texture_bounding_box_index), texture_path(texture), id(id) {};
    // TODO: remove these instead rely on TransformedIVPTPGroup, do this change later
    Transform transform;
    int id;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    std::vector<glm::vec2> original_texture_coordinates;
    std::vector<glm::vec2> packed_texture_coordinates;
    int packed_texture_index;
    int packed_texture_bounding_box_index;
    std::string texture_path;
};

// TODO: batcher_draw_info_integration
class TransformedIVPTPGroup { // TIG
  public:
    TransformedIVPTPGroup() {}
    TransformedIVPTPGroup(std::vector<IVPTexturePacked> ivptp, int id) : ivptps(std::move(ivptp)), id(id) {}
    int id;
    std::vector<IVPTexturePacked> ivptps;
    Transform transform;

    void regenerate_ids(IDGenerator &tig_id_generator, IDGenerator &ivptp_id_generator) {
        id = tig_id_generator.get_id();
        for (auto &ivptp : ivptps) {
            ivptp.id = ivptp_id_generator.get_id();
        }
    }
};

class IVPNTexturePacked { // IVPTP
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
    int id = GlobalUIDGenerator::get_id();
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
