#include "draw_info.hpp"

namespace draw_info {

IndexedVertexPositions ivpn_to_ivpn(const IVPNormals &ivpn) { return {ivpn.indices, ivpn.xyz_positions, ivpn.id}; }

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

void VertexBoneData::add_bone_data(unsigned int BoneID, float Weight) {
    for (unsigned int i = 0; i < 4; i++) {
        if (weight_value_of_this_vertex_wrt_bone[i] == 0.0) {
            indices_of_bones_that_affect_this_vertex[i] = BoneID;
            weight_value_of_this_vertex_wrt_bone[i] = Weight;
            /*std::cout << "Bone ID " << BoneID << " weight " << Weight << " stored at local index " << i <<
             * std::endl;*/
            return;
        }
    }

    bool logging = false;
    if (logging) {
        std::cout << "was about to add bone data, but we've already associated 4 weights, not adding" << std::endl;
    }
    /*assert(false); // Should never get here if we have enough space for bones, otherwise we need to increment the
     * num*/
    /*// bones count*/
}

} // namespace draw_info
