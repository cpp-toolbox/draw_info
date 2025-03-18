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

    std::vector<IVPTexturePacked> ivptps;
    int id;
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

struct VertexBoneData {
    unsigned int indices_of_bones_that_affect_this_vertex[4] = {0};
    float weight_value_of_this_vertex_wrt_bone[4] = {0.0f};

    VertexBoneData() {}

    void add_bone_data(unsigned int BoneID, float Weight);
};

class IVPNTRigged {
  public:
    IVPNTRigged(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions, std::vector<glm::vec3> normals,
                std::vector<glm::vec2> texture_coordinates, const std::string &texture,
                std::vector<VertexBoneData> bone_data, int id)
        : indices(indices), xyz_positions(xyz_positions), normals(normals), texture_coordinates(texture_coordinates),
          texture_path(texture), bone_data(bone_data), id(id) {};
    Transform transform;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texture_coordinates;
    std::string texture_path;
    std::vector<VertexBoneData> bone_data;
    int id;
};

// packed version of the above
class IVPNTPRigged {
  public:
    IVPNTPRigged(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions,
                 std::vector<glm::vec3> normals, std::vector<glm::vec2> packed_texture_coordinates,
                 int packed_texture_index, int packed_texture_bounding_box_index, const std::string &texture,
                 std::vector<VertexBoneData> bone_data)
        : indices(indices), xyz_positions(xyz_positions), normals(normals),
          packed_texture_coordinates(packed_texture_coordinates), packed_texture_index(packed_texture_index),
          packed_texture_bounding_box_index(packed_texture_bounding_box_index), texture(texture),
          bone_data(bone_data) {};
    Transform transform;
    int id = GlobalUIDGenerator::get_id();
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> packed_texture_coordinates;
    int packed_texture_index;
    int packed_texture_bounding_box_index;
    std::string texture;
    std::vector<VertexBoneData> bone_data;
};

struct BoneInfo {
    // this transformation puts the local orgin at the start (tail) of the bone and makes the bone sit on an axis so
    // that when transformations are applied it works correctly, note that it is not recursive in any sense, it
    // literally just puts it in the correct position not relative to a parent bone or anything like that

    // another name for this is the inverase bind pose because the bind pose transformation takes a bone and puts it at
    // the origin to be read for application of transformations, and this matrix is the inverse of that

    // A quick thing to jog your memory could be that it "brings the bone joint back to the origin"

    // Note that bones don't really exist, they only exist by the mapping of vertex to bone id's and then knowing the
    // mapping for each bone,

    // Questions for myself: what is a bone tip? it just shows what the rotation and scale is visually, based on a (0,
    // 0, 1) bone tip you can compute where the tip would be based on the scale and so on I think... but then again
    // bones can be larger and have no scaled in them right so then what? it might just be to compute during the auto
    // weighting process maybe check assimp if that data is stored in there if a bone is positioned at position (x, y,
    // z) and then we have a vertex at (x, y + 1, z + 1) then its new position becomes (0, 1, 1) that is it is
    // positioned relative to the bones origin
    glm::mat4 local_space_to_bone_space_in_bind_pose_transformation;
    glm::mat4 local_space_animated_transform_upto_this_bone = glm::mat4(0);

    BoneInfo(const glm::mat4 &lstbst) { local_space_to_bone_space_in_bind_pose_transformation = lstbst; }
};

class TransformedIVPNTPRGroup { // TIG
  public:
    TransformedIVPNTPRGroup() {}
    TransformedIVPNTPRGroup(std::vector<IVPNTPRigged> ivpntprs, int id) : ivpntprs(std::move(ivpntprs)), id(id) {}
    int id;
    std::vector<IVPNTPRigged> ivpntprs;
    Transform transform;
};

IndexedVertexPositions extract_indexed_vertex_positions(const IVPTextured &ivp_textured);
IndexedVertexPositions extract_indexed_vertex_positions(const IVPTexturePacked &ivp_texture_packed);

std::vector<IndexedVertexPositions>
extract_indexed_vertex_positions_vector(const std::vector<IVPTextured> &ivp_textured_vector);
std::vector<IndexedVertexPositions>
extract_indexed_vertex_positions_vector(const std::vector<IVPTexturePacked> &ivp_texture_packed_vector);

} // namespace draw_info

#endif // DRAW_INFO_HPP
