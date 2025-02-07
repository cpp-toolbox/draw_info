#include "draw_info.hpp"

namespace draw_info {

IndexedVertexPositions extract_indexed_vertex_positions(const IVPTextured &ivp_textured) {
    return IndexedVertexPositions(ivp_textured.indices, ivp_textured.xyz_positions);
}

IndexedVertexPositions extract_indexed_vertex_positions(const IVPTexturePacked &ivp_texture_packed) {
    return IndexedVertexPositions(ivp_texture_packed.indices, ivp_texture_packed.xyz_positions);
}

std::vector<IndexedVertexPositions>
extract_indexed_vertex_positions_vector(const std::vector<IVPTextured> &ivp_textured_vector) {

    std::vector<IndexedVertexPositions> result;
    result.reserve(ivp_textured_vector.size()); // Reserve space for efficiency

    for (const auto &ivp_textured : ivp_textured_vector) {
        result.emplace_back(ivp_textured.indices, ivp_textured.xyz_positions);
    }

    return result;
}

std::vector<IndexedVertexPositions>
extract_indexed_vertex_positions_vector(const std::vector<IVPTexturePacked> &ivp_texture_packed_vector) {

    std::vector<IndexedVertexPositions> result;
    result.reserve(ivp_texture_packed_vector.size()); // Reserve space for efficiency

    for (const auto &ivp_texture_packed : ivp_texture_packed_vector) {
        result.emplace_back(ivp_texture_packed.indices, ivp_texture_packed.xyz_positions);
    }

    return result;
}

} // namespace draw_info
