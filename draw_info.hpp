#ifndef DRAW_INFO_HPP
#define DRAW_INFO_HPP

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "sbpt_generated_includes.hpp"

/**
 *
 * draw info is a namespace containing representations of data that will eventually land on the GPU. These
 * representations are held in ram/memory, and not on the graphics card immediately.
 *
 * For every shader you make, you need some sort of CPU side representation of that data before hand, and then once you
 * have it you can pipe it into opengl so that the shader can use all the vertex attributes. Therefore this is a
 * collection of classes/structs that represent the data that eventually needs to get fed into opengl, due to some
 * shader existing.
 *
 *
 * Concretely if you had a shader that tries to do lighting by accessing the normal of a fragment to determine how much
 * the light glances off, then your model needs to at least have normals as vertex attributes.
 *
 * The purpose of having such representations is that it allows us to modify the representations and then if changes are
 * detected the geometry can be re-uploaded.
 *
 * The type of geometry we just talked about would static geometry as the geometry of the object can change, but for the
 * most part this doesn't happen, and in that case we call it static geometry, once uploaded it will never or rarely
 * change and therefore holding in in motherboard ram serves no purpose, this motivates the next paragraph:
 *
 * Eventually there will be "Hollow" or "Deffered" representations which minimize the amount of ram usage for very large
 * or complicated object, whose purpose is to just to upload to the graphics card, more on that later
 */
namespace draw_info {

template <typename T>
concept IVPLike = requires(T t) {
    // Must have these data members
    { t.xyz_positions } -> std::convertible_to<std::vector<glm::vec3>>;
    { t.indices } -> std::convertible_to<std::vector<unsigned int>>;
    { t.id } -> std::convertible_to<unsigned int>;
    { t.name } -> std::convertible_to<std::string>;

    { t.transform };
    { t.buffer_modification_tracker };
};

template <IVPLike T> void apply_translation(T &ivp_like) {
    glm::vec3 translation = ivp_like.transform.get_translation();

    for (auto &pos : ivp_like.xyz_positions) {
        pos += translation;
    }

    ivp_like.transform.reset_translation();
    ivp_like.buffer_modification_tracker.just_modified();
}

template <IVPLike T> void apply_rotation(T &ivp_like) {
    glm::mat4 rotation_mat = ivp_like.transform.get_rotation_transform_matrix();

    for (auto &pos : ivp_like.xyz_positions) {
        glm::vec4 rotated = rotation_mat * glm::vec4(pos, 1.0f);
        pos = glm::vec3(rotated);
    }

    ivp_like.transform.reset_rotation();
    ivp_like.buffer_modification_tracker.just_modified();
}

template <IVPLike T> void apply_scale(T &ivp_like) {
    glm::vec3 scale = ivp_like.transform.get_scale();

    for (auto &pos : ivp_like.xyz_positions) {
        pos *= scale;
    }

    ivp_like.transform.reset_scale();
    ivp_like.buffer_modification_tracker.just_modified();
}

template <IVPLike T> void apply_transform(T &ivp_like) {
    glm::mat4 transform_mat = ivp_like.transform.get_transform_matrix();

    for (auto &pos : ivp_like.xyz_positions) {
        glm::vec4 transformed = transform_mat * glm::vec4(pos, 1.0f);
        pos = glm::vec3(transformed);
    }

    ivp_like.transform.reset();
    ivp_like.buffer_modification_tracker.just_modified();
}

/**
 * @brief Tracks modifications to geometry data relative to its GPU buffer state.
 *
 * The `BufferModificationTracker` is used to determine when data on the CPU side has been modified
 * and needs to be rebuffered (uploaded) to the GPU. It keeps track of whether the associated geometry
 * currently exists in GPU memory and whether it has been changed since the last time it was buffered.
 *
 * Typical usage pattern:
 * - Call `just_modified()` whenever CPU-side geometry data changes.
 * - During rendering, check `has_been_modified_since_last_buffering()` to determine whether to rebuffer.
 * - Call `just_buffered_data()` after reuploading the data to mark it as up-to-date.
 * - Call `free_buffered_data()` when the GPU buffer is deleted or invalidated.
 */
class BufferModificationTracker {
  public:
    /**
     * @brief Marks the data as modified since the last GPU buffering.
     *
     * Should be called whenever the CPU-side data changes and needs to be reuploaded
     * to the GPU before the next render.
     *
     * @note if there is no data in the buffer then this does nothing
     */
    void just_modified() {
        if (has_data_in_buffer())
            has_been_modified_since_last_buffered_ = true;
    }

    /**
     * @brief Marks the data as successfully buffered to the GPU.
     *
     * Sets the internal flags to indicate that the geometry data now exists in the GPU buffer
     * and is synchronized with the CPU-side data.
     */
    void just_buffered_data() {
        has_data_in_buffer_ = true;
        has_been_modified_since_last_buffered_ = false;
    }

    /**
     * @brief Marks the buffered GPU data as freed or invalidated.
     *
     * Should be called when the GPU buffer is deleted, for example during cleanup
     * or when resizing or reinitializing a buffer.
     */
    void free_buffered_data() { has_data_in_buffer_ = false; }

    /**
     * @brief Checks whether there is currently valid data in the GPU buffer.
     *
     * @return true if the data exists in GPU memory, false otherwise.
     */
    bool has_data_in_buffer() { return has_data_in_buffer_; }

    /**
     * @brief Checks if the CPU-side data has been modified since it was last buffered to the GPU.
     *
     * @note If the data has never been buffered before, this function returns false, since
     * there is no prior buffering to compare against.
     *
     * @return true if the data has been modified since the last buffering; false otherwise.
     */
    bool has_been_modified_since_last_buffering() const {
        if (has_data_in_buffer_) {
            return has_been_modified_since_last_buffered_;
        } else {
            // NOTE: if there's no data in the buffer yet, then it has not been modified
            // since last buffering because there was no last buffering.
            return false;
        }
    }

    /**
     * @brief Converts the internal state of the tracker to a human-readable string.
     *
     * @return A string representation of the tracker's current state.
     */
    std::string to_string() const {
        return "BufferModificationTracker { " + std::string("has_data_in_buffer = ") +
               (has_data_in_buffer_ ? "true" : "false") + ", " +
               std::string("has_been_modified_since_last_buffered_ = ") +
               (has_been_modified_since_last_buffered_ ? "true" : "false") + " }";
    }

  private:
    bool has_data_in_buffer_ = false; ///< Whether geometry data currently exists in the GPU buffer.
    bool has_been_modified_since_last_buffered_ =
        false; ///< Whether CPU-side data has changed since the last buffering.
};

/**
 * @brief Represents the minimal geometric data required to render a mesh using glDrawElements or an equivalent.
 *
 * The `IndexedVertexPositions` (IVP) class holds a set of vertex positions and their corresponding
 * indices that define how vertices connect into primitives (typically triangles or lines).
 *
 * This structure is designed to be the *minimal* representation of something drawable —
 * just enough to issue a draw call once the data has been uploaded to the GPU.
 * Typically, to visualize this geometry meaningfully, additional shader data such as
 * color uniforms or materials would be needed.
 *
 * It is most commonly used for solid-color objects, wireframes, or quick debug visualization.
 *
 * @note This class includes a `BufferModificationTracker` to manage synchronization between
 * CPU-side geometry and GPU buffers.
 *
 * @see BufferModificationTracker
 */
class IndexedVertexPositions { // IVP
  public:
    /**
     * @brief Constructs an empty IndexedVertexPositions object.
     *
     * Initializes with no vertices or indices and a default ID of `-1`.
     */
    IndexedVertexPositions() : id(-1) {};

    /**
     * @brief Constructs an IndexedVertexPositions object from vertex and index data.
     *
     * @param indices A list of vertex indices that define how vertices form primitives (e.g., triangles).
     * @param xyz_positions A list of vertex positions in 3D space.
     * @param id Optional unique identifier for this mesh (defaults to -1).
     * @param name Optional human-readable name for identifying the mesh.
     */
    IndexedVertexPositions(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions, int id = -1,
                           const std::string &name = "")
        : indices(indices), xyz_positions(xyz_positions), id(id), name(name) {};

    /**
     * @brief Constructs an IndexedVertexPositions object from another IVP-like type.
     *
     * This constructor allows interoperability with other objects that conform to
     * the `IVPLike` concept — meaning they expose the same fields (`indices`, `xyz_positions`,
     * `id`, `transform`, `buffer_modification_tracker`, and `name`).
     *
     * @tparam T Type satisfying the `IVPLike` concept.
     * @param src The source object to copy data from.
     */
    template <IVPLike T>
    IndexedVertexPositions(const T &src)
        : indices(src.indices), xyz_positions(src.xyz_positions), id(src.id), transform(src.transform),
          buffer_modification_tracker(src.buffer_modification_tracker), name(src.name) {}

    /**
     * @brief Transformation applied to the vertex positions before rendering.
     *
     * @see Transform
     */
    Transform transform;

    /**
     * @brief Unique identifier for this ivp. If other systems use this object then this can be the handle back into
     * that system.
     */
    int id;

    /**
     * @brief Optional name for identifying the mesh.
     */
    std::string name;

    /**
     * @brief Index list defining how vertices are connected.
     *
     * Typically represents triangle or line connectivity (e.g., three indices per triangle).
     */
    std::vector<unsigned int> indices;

    /**
     * @brief Vertex positions in object (model) space.
     */
    std::vector<glm::vec3> xyz_positions;

    BufferModificationTracker buffer_modification_tracker;

    /**
     * @brief Stream output operator for debugging and logging.
     *
     * @param os Output stream to write to.
     * @param ivp The IndexedVertexPositions instance to print.
     * @return Reference to the modified output stream.
     */
    friend std::ostream &operator<<(std::ostream &os, const IndexedVertexPositions &ivp) {
        os << "IndexedVertexPositions("
           << "indices.size=" << ivp.indices.size() << ", "
           << "xyz_positions.size=" << ivp.xyz_positions.size() << ", "
           << "transform=" << ivp.transform << ")";
        return os;
    }
};

/**
 * @brief Represents a mesh with indexed vertex positions and normals.
 *
 * The `IVPNormals` (Indexed Vertex Positions with Normals) class extends the idea of
 * `IndexedVertexPositions` by including per-vertex normal vectors. Normals are essential
 * for lighting calculations and for rendering meshes with shaded surfaces rather than
 * just flat colors or wireframes.
 *
 * This class provides the minimal geometric information required to render a lit 3D object,
 * including vertex positions, indices, and surface normals. It is suitable for basic shading
 * and lighting models such as Lambertian or Phong illumination.
 *
 * @note Like `IndexedVertexPositions`, this class includes a `BufferModificationTracker`
 * to manage synchronization between CPU-side data and GPU buffers.
 *
 * @see IndexedVertexPositions
 * @see BufferModificationTracker
 */
class IVPNormals {
  public:
    IVPNormals() : id(-1) {};

    IVPNormals(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions, std::vector<glm::vec3> normals,
               int id = -1, const std::string &name = "")
        : indices(indices), xyz_positions(xyz_positions), normals(normals), id(id), name(name) {};

    Transform transform;
    int id;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    std::vector<glm::vec3> normals;
    std::string name;
    BufferModificationTracker buffer_modification_tracker;

    friend std::ostream &operator<<(std::ostream &os, const IVPNormals &ivp) {
        os << "IVPNormals("
           << "indices.size=" << ivp.indices.size() << ", "
           << "xyz_positions.size=" << ivp.xyz_positions.size() << ", "
           << "normals.size=" << ivp.normals.size() << ", "
           << "transform=" << ivp.transform << ")";
        return os;
    }
};

IndexedVertexPositions ivpn_to_ivpn(const IVPNormals &ivpn);

/**
 * @brief Represents a group of drawable geometries sharing a common transformation.
 *
 * The `TransformedIVPGroup` (TIG) class groups together multiple
 * `IndexedVertexPositions` (IVP) objects that are transformed collectively using a single
 * `Transform`. This is useful when several pieces of geometry should move, rotate,
 * or scale together — for example, parts of a model that form a single logical object.
 *
 * The group itself does not define how rendering occurs; rather, it serves as a convenient
 * container for organizing and managing related drawable components under one transform.
 *
 * @note Each contained `IndexedVertexPositions` object may still have its own internal
 * transform or buffer modification tracker.
 *
 * @see IndexedVertexPositions
 * @see Transform
 */
class TransformedIVPGroup {
  public:
    TransformedIVPGroup() {}
    TransformedIVPGroup(std::vector<IndexedVertexPositions> ivps, int id) : ivps(std::move(ivps)), id(id) {}
    int id;
    std::vector<IndexedVertexPositions> ivps;
    Transform transform;
};

/**
 * @brief Represents an indexed mesh with per-vertex color data.
 *
 * The `IVPColor` (Indexed Vertex Positions with Color) class extends the concept of
 * `IndexedVertexPositions` by adding per-vertex color information. This allows each vertex
 * to have its own color, making it useful for rendering solid-colored objects, vertex-colored
 * meshes, or simple debug visualizations.
 *
 * It stores indices, vertex positions, and corresponding RGB colors. Each vertex in
 * `xyz_positions` has a corresponding color entry in `rgb_colors`.
 *
 * @note Like other IVP-based structures, `IVPColor` uses a `BufferModificationTracker`
 * to track whether data has changed and needs to be rebuffered to the GPU.
 *
 * @see IndexedVertexPositions
 * @see BufferModificationTracker
 *
 * @todo it would be nice one day to be able to load an obj with texture and flatten those textures and then use
 * ivpcolor instead for really fast performance.
 *
 */
class IVPColor { // IVPSC
  public:
    bool logging_enabled = false;
    IVPColor() : id(-1) {};

    /**
     * @brief Constructs a uniformly colored IVPColor object from an existing IVP.
     *
     * All vertices of the provided `IndexedVertexPositions` are assigned the same color.
     *
     * @param ivp Source `IndexedVertexPositions` containing vertex and index data.
     * @param color A uniform RGB color to apply to all vertices.
     */
    IVPColor(draw_info::IndexedVertexPositions ivp, glm::vec3 color)
        : IVPColor(ivp, std::vector<glm::vec3>(ivp.xyz_positions.size(), color), ivp.id, ivp.name) {}

    /**
     * @brief Constructs an IVPColor object from an existing IVP and per-vertex color data.
     *
     * @param ivp Source `IndexedVertexPositions` containing vertex and index data.
     * @param rgb_colors Per-vertex colors corresponding to each vertex in `ivp.xyz_positions`.
     * @param id Optional unique identifier for this mesh (defaults to -1).
     * @param name Optional name for identifying the mesh.
     */
    IVPColor(draw_info::IndexedVertexPositions ivp, std::vector<glm::vec3> rgb_colors, int id = -1,
             const std::string &name = "")
        : indices(ivp.indices), xyz_positions(ivp.xyz_positions), rgb_colors(rgb_colors), id(id), name(name) {};

    /**
     * @brief Constructs an IVPColor object directly from raw geometry and color data.
     *
     * @param indices A list of vertex indices defining primitive connectivity (e.g., triangles).
     * @param xyz_positions A list of vertex positions in 3D space.
     * @param rgb_colors A list of vertex colors corresponding to each position.
     * @param id Optional unique identifier for this mesh (defaults to -1).
     * @param name Optional human-readable name for identifying the mesh.
     */
    IVPColor(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions, std::vector<glm::vec3> rgb_colors,
             int id = -1, const std::string &name = "")
        : indices(indices), xyz_positions(xyz_positions), rgb_colors(rgb_colors), id(id), name(name) {};

    void copy_draw_data_from(const IVPColor &other) {
        LogSection _(*global_logger, "ivpc copy constructor", logging_enabled);
        if (this == &other)
            return;

        // only drawing related data copied
        indices = other.indices;
        xyz_positions = other.xyz_positions;
        rgb_colors = other.rgb_colors;

        buffer_modification_tracker.just_modified();
    }

    // TODO: remove
    Transform transform;
    int id;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    /**
     * @brief Per-vertex RGB color values.
     *
     * Each entry corresponds one-to-one with a vertex in `xyz_positions`.
     */
    std::vector<glm::vec3> rgb_colors;
    std::string name;

    BufferModificationTracker buffer_modification_tracker;
};
/**
 * @brief Represents an indexed mesh with normals and per-vertex color data.
 *
 * The `IVPNColor` (Indexed Vertex Positions with Normals and Color) class extends `IVPNormals`
 * by adding per-vertex RGB color information. Each vertex has a position, a normal, and
 * a color, making this structure suitable for rendering lit and colored 3D meshes.
 *
 * @note Includes a `BufferModificationTracker` to track modifications to the CPU-side data
 * and whether it needs to be rebuffered to the GPU.
 *
 * @see IVPNormals
 * @see BufferModificationTracker
 */
class IVPNColor {
  public:
    /**
     * @brief Constructs an IVPNColor with a uniform color from an existing IVPNormals mesh.
     *
     * All vertices in the mesh are assigned the same RGB color.
     *
     * @param ivpn Source IVPNormals mesh containing positions and normals.
     * @param color RGB color to apply uniformly to all vertices.
     */
    IVPNColor(IVPNormals ivpn, glm::vec3 color)
        : IVPNColor(ivpn, std::vector<glm::vec3>(ivpn.xyz_positions.size(), color)) {}

    /**
     * @brief Constructs an IVPNColor with per-vertex color data from an existing IVPNormals mesh.
     *
     * @param ivpn Source IVPNormals mesh containing positions and normals.
     * @param rgb_colors Per-vertex RGB colors corresponding to each vertex.
     */
    IVPNColor(IVPNormals ivpn, std::vector<glm::vec3> rgb_colors)
        : transform(ivpn.transform), indices(ivpn.indices), xyz_positions(ivpn.xyz_positions), normals(ivpn.normals),
          id(ivpn.id), rgb_colors(rgb_colors) {};

    /**
     * @brief Constructs an IVPNColor directly from raw vertex, normal, index, and color data.
     *
     * @param indices Vertex indices defining how vertices form primitives.
     * @param xyz_positions Vertex positions in 3D space.
     * @param normals Vertex normals for lighting calculations.
     * @param colors Per-vertex RGB colors.
     * @param id Optional unique identifier (defaults to -1).
     * @param name Optional human-readable name for the mesh.
     */
    IVPNColor(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions, std::vector<glm::vec3> normals,
              std::vector<glm::vec3> colors, int id = -1, const std::string &name = "")
        : indices(indices), xyz_positions(xyz_positions), normals(normals), id(id), name(name) {};
    Transform transform;
    int id;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    /**
     * @brief Per-vertex normals for lighting calculations.
     */
    std::vector<glm::vec3> normals;

    std::vector<glm::vec3> rgb_colors;
    std::string name;

    BufferModificationTracker buffer_modification_tracker;

    friend std::ostream &operator<<(std::ostream &os, const IVPNColor &ivp) {
        os << "IndexedVertexPositions("
           << "indices.size=" << ivp.indices.size() << ", "
           << "xyz_positions.size=" << ivp.xyz_positions.size() << ", "
           << "normals.size=" << ivp.normals.size() << ", "
           << "rgb_colors.size=" << ivp.rgb_colors.size() << ", "
           << "transform=" << ivp.transform << ")";
        return os;
    }
};

/**
 * @brief Represents an indexed mesh with texture coordinates and an optional texture.
 *
 * The `IVPTextured` (Indexed Vertex Positions with Texture) class extends the concept of
 * `IndexedVertexPositions` by including per-vertex 2D texture coordinates and an optional
 * texture file path. This is used for rendering textured 3D meshes, typically with
 * diffuse textures applied to the surface. So this would be the "smallest" draw info that can be used to render
 * textures on an object.
 *
 * Each vertex has a position (`xyz_positions`) and a corresponding texture coordinate
 * (`texture_coordinates`) that maps to a point on a 2D texture image. The `texture_path`
 * optionally specifies the file path to the texture to be applied.
 *
 * @note This class is generally used for meshes with diffuse textures. For other types of
 * textures (e.g., normal maps, specular maps), additional data structures may be required.
 *
 * @note Includes a `BufferModificationTracker` to track modifications to the CPU-side data
 * and whether it needs to be rebuffered to the GPU.
 *
 * @see IndexedVertexPositions
 * @see BufferModificationTracker
 */
class IVPTextured {
  public:
    /**
     * @brief Constructs a textured IVP mesh.
     *
     * @param indices Vertex indices defining connectivity for primitives.
     * @param xyz_positions Vertex positions in 3D space.
     * @param texture_coordinates Per-vertex 2D texture coordinates.
     * @param texture Optional texture file path. Defaults to an empty string.
     * @param id Optional unique identifier for this mesh. Defaults to a generated global ID.
     * @param name Optional name for identifying the mesh. Useful for mapping or tracking.
     *
     * @todo need to recall why texture defaults to = "" when in reality it should always be there because in my mind
     * that's not optional
     */
    IVPTextured(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions,
                std::vector<glm::vec2> texture_coordinates, const std::string &texture = "",
                int id = GlobalUIDGenerator::get_id(), const std::string &name = "")
        : indices(indices), xyz_positions(xyz_positions), texture_coordinates(texture_coordinates),
          texture_path(texture), id(id), name(name) {};

    Transform transform;
    int id;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    std::vector<glm::vec2> texture_coordinates;
    std::string texture_path;

    // NOTE: this was added because IVPTextured is what are used while mapping, while mapping we want the ability to
    // pass custom information for a particular mesh, an easy way to get the information is to use the name of a mesh to
    // pass this information, additionally assimp exposes this information to us for free
    std::string name = "";

    BufferModificationTracker buffer_modification_tracker;
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

    /**
     * @brief Per-vertex 2D texture coordinates.
     *
     * Each entry corresponds one-to-one with a vertex in `xyz_positions`.
     */
    std::vector<glm::vec2> texture_coordinates;

    /**
     * @brief File path to a texture image.
     *
     * Typically refers to a diffuse texture used to color the mesh.
     */
    std::string texture_path;

    BufferModificationTracker buffer_modification_tracker;
};

/**
 * @brief Represents an indexed mesh with packed texture coordinates.
 *
 * The `IVPTexturePacked` (Indexed Vertex Positions with Packed Texture) class is used
 * for meshes whose textures are packed into a texture atlas. It stores both the original
 * per-vertex texture coordinates and the modified (packed) coordinates that correspond
 * to the location of the mesh in the packed atlas.
 *
 * This is useful in scenarios where multiple textures are combined into a single large
 * texture to reduce draw calls and improve GPU performance. If the packed texture atlas
 * changes (e.g., dynamically during a game), the original coordinates can be used to
 * recompute the packed coordinates without modifying the geometry.
 *
 * @note Includes a `BufferModificationTracker` to track modifications to CPU-side data
 * and whether it needs to be rebuffered to the GPU.
 *
 * @see IndexedVertexPositions
 * @see BufferModificationTracker
 */
class IVPTexturePacked {
  public:
    /**
     * @brief Constructs an IVPTexturePacked object from raw data.
     *
     * @param indices Vertex indices defining connectivity.
     * @param xyz_positions Vertex positions in 3D space.
     * @param original_texture_coordinates Original per-vertex 2D texture coordinates.
     * @param packed_texture_coordinates Per-vertex coordinates after packing into the texture atlas.
     * @param packed_texture_index Index of the packed texture in the atlas.
     * @param packed_texture_bounding_box_index Index of the bounding box in the atlas for this texture.
     * @param texture File path to the texture (usually a diffuse texture).
     * @param id Optional unique identifier for this mesh (defaults to -1).
     * @param name Optional human-readable name for the mesh.
     */
    IVPTexturePacked(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions,
                     std::vector<glm::vec2> original_texture_coordinates,
                     std::vector<glm::vec2> packed_texture_coordinates, int packed_texture_index,
                     int packed_texture_bounding_box_index, const std::string &texture, int id = -1,
                     const std::string &name = "")
        : indices(indices), xyz_positions(xyz_positions), original_texture_coordinates(original_texture_coordinates),
          packed_texture_coordinates(packed_texture_coordinates), packed_texture_index(packed_texture_index),
          packed_texture_bounding_box_index(packed_texture_bounding_box_index), texture_path(texture), id(id),
          name(name) {};

    /**
     * @brief Constructs an IVPTexturePacked object from an existing `IndexedVertexPositions` mesh.
     *
     * @param ivp Source mesh providing vertex and index data.
     * @param original_texture_coordinates Original per-vertex 2D texture coordinates.
     * @param packed_texture_coordinates Per-vertex coordinates after packing into the texture atlas.
     * @param packed_texture_index Index of the packed texture in the atlas.
     * @param packed_texture_bounding_box_index Index of the bounding box in the atlas for this texture.
     * @param texture File path to the texture (usually a diffuse texture).
     * @param id Optional unique identifier (defaults to a generated global ID).
     * @param name Optional human-readable name for the mesh.
     */
    IVPTexturePacked(const IndexedVertexPositions &ivp, std::vector<glm::vec2> original_texture_coordinates,
                     std::vector<glm::vec2> packed_texture_coordinates, int packed_texture_index,
                     int packed_texture_bounding_box_index, const std::string &texture,
                     int id = GlobalUIDGenerator::get_id(), const std::string &name = "")
        : indices(ivp.indices), xyz_positions(ivp.xyz_positions),
          original_texture_coordinates(original_texture_coordinates),
          packed_texture_coordinates(packed_texture_coordinates), packed_texture_index(packed_texture_index),
          packed_texture_bounding_box_index(packed_texture_bounding_box_index), texture_path(texture), id(id),
          name(name) {};

    // TODO: remove these instead rely on TransformedIVPTPGroup, do this change later
    Transform transform;
    int id;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    /**
     * @brief Original per-vertex 2D texture coordinates.
     *
     * Used to recompute packed coordinates if the texture atlas changes dynamically.
     */
    std::vector<glm::vec2> original_texture_coordinates;
    /**
     * @brief Per-vertex texture coordinates after packing into the atlas.
     */
    std::vector<glm::vec2> packed_texture_coordinates;
    /**
     * @brief Index of the packed texture in the texture atlas.
     */
    int packed_texture_index;
    /**
     * @brief Index of the bounding box in the texture atlas for this texture.
     */
    int packed_texture_bounding_box_index;
    std::string texture_path;

    // TODO: needs to be set during import
    std::string name;

    BufferModificationTracker buffer_modification_tracker;
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

/**
 * @brief Represents an indexed mesh with normals and packed texture coordinates.
 *
 * The `IVPNTexturePacked` (Indexed Vertex Positions with Normals and Packed Texture) class
 * is used for meshes that have normals for lighting calculations and textures packed
 * into a texture atlas. It stores both the original per-vertex texture coordinates and the
 * modified (packed) coordinates corresponding to the mesh’s location in the packed atlas.
 *
 * This is useful when multiple textures are combined into a single texture atlas to reduce
 * draw calls. The original coordinates are retained so that packed coordinates can be
 * regenerated if the atlas changes dynamically.
 *
 * Typically, this structure is used for meshes with diffuse textures where lighting
 * (via normals) is required.
 *
 * @note Includes a `BufferModificationTracker` to track modifications to CPU-side data
 * and whether it needs to be rebuffered to the GPU.
 *
 * @see IVPNormals
 * @see IVPTexturePacked
 * @see BufferModificationTracker
 */
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
    int id = GlobalUIDGenerator::get_id();
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> original_texture_coordinates;
    std::vector<glm::vec2> packed_texture_coordinates;
    int packed_texture_index;
    int packed_texture_bounding_box_index;
    std::string texture_path;

    BufferModificationTracker buffer_modification_tracker;
};

/**
 * @brief Stores bone influence data for a single vertex in skeletal animation.
 *
 * @warning Each vertex can be affected by up to 4 bones. This struct keeps track of the indices
 * of the bones that influence the vertex and the corresponding weight values. In the future the number of bones may be
 * extended, but note the shader needs updating as well
 *
 * The weights define how much each bone affects the vertex's final position during
 * skeletal animation (skin deformation). The sum of the weights is usually expected
 * to be 1.0 for proper interpolation.
 *
 * @todo rename to
 */
struct VertexBoneData {
    /**
     * @brief Array storing indices of bones that affect this vertex.
     *
     * Only the first `n` entries (depending on how many bones actually influence the vertex)
     * are considered. The rest are zero-initialized.
     */
    unsigned int indices_of_bones_that_affect_this_vertex[4] = {0};
    /**
     * @brief Array storing the corresponding weight of each bone for this vertex.
     *
     * The weight represents how much influence the bone has on the vertex. The sum of all
     * weights should typically be 1.0.
     */
    float weight_value_of_this_vertex_wrt_bone[4] = {0.0f};

    /**
     * @brief Default constructor.
     *
     * Initializes all bone indices to 0 and all weights to 0.0.
     */
    VertexBoneData() {}

    /**
     * @brief Adds bone influence data to this vertex.
     *
     * @param BoneID Index of the bone influencing this vertex.
     * @param Weight Influence weight of the bone. Should typically be between 0 and 1.
     *
     * @note Only the first 4 bones influencing a vertex are stored. If more than 4 calls
     * are made, the additional influences may be ignored or require custom handling.
     */
    void add_bone_data(unsigned int BoneID, float Weight);
};
/**
 * @brief Represents a rigged, indexed 3D mesh with texture coordinates and bone data.
 *
 * The `IVPNTRigged` (Indexed Vertex Positions with Normals, Texture coordinates, and Rigging) class
 * extends the concept of `IndexedVertexPositions` by including per-vertex normals, 2D texture
 * coordinates, optional texture, and skeletal animation data (bone weights and indices). This
 * is used for rendering skinned 3D meshes, typically with diffuse textures and skeletal animation.
 *
 * Each vertex has a position (`xyz_positions`), a normal (`normals`) for lighting calculations,
 * a texture coordinate (`texture_coordinates`) that maps to a point on a 2D texture image, and
 * optional bone influence data (`bone_data`) for skinning. The `texture_path` optionally specifies
 * the file path to the texture to be applied.
 *
 * @note This class is generally used for meshes that require skeletal animation and texturing.
 * For static meshes without bones, simpler classes like `IVPTextured` may suffice.
 *
 * @note Each instance has a `Transform` that defines its position, rotation, and scale in the world.
 *
 * @see IVPTextured
 * @see IVPNormals
 * @see VertexBoneData
 */
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

/**
 * @brief Represents a rigged, indexed 3d mesh with that works with packed texture atlases and bone data.
 *
 * @see IVPNTRigged
 * @see IVPTexturePacked
 */
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

/**
 * @brief Stores transformation data for a single bone in a skeletal mesh.
 *
 * The `BoneInfo` struct contains matrices used to transform vertices for skeletal animation.
 * Bones themselves are abstract—they exist only as mappings from vertices to bone indices.
 * This struct helps compute both the bone's space in the bind pose and its animated transformation.
 *
 * @note The `local_space_to_bone_space_in_bind_pose_transformation` is also known as the
 * "inverse bind pose." It transforms the bone so that its local origin is at the start (tail)
 * of the bone and aligns it along a canonical axis. This is not relative to any parent bone;
 * it simply positions the bone correctly for animation calculations.
 *
 * @note The `local_space_animated_transform_upto_this_bone` stores the bone's animated
 * transformation applied to vertices, in local space. This allows vertices influenced by
 * this bone to be properly moved during skeletal animation.
 *
 * @remark Bones do not physically exist; their effects are entirely determined by the mapping
 * of vertex indices to bone indices and the corresponding transformations.
 *
 * @see glm::mat4
 */
struct BoneInfo {
    /**
     * this transformation puts the local orgin at the start (tail) of the bone and makes the bone sit on an axis so
     * that when transformations are applied it works correctly, note that it is not recursive in any sense, it
     * literally just puts it in the correct position not relative to a parent bone or anything like that
     *
     * another name for this is the inverase bind pose because the bind pose transformation takes a bone and puts it
     * at the origin to be read for application of transformations, and this matrix is the inverse of that
     *
     * A quick thing to jog your memory could be that it "brings the bone joint back to the origin"
     *
     * Note that bones don't really exist, they only exist by the mapping of vertex to bone id's and then knowing
     * the mapping for each bone,
     *
     * Questions for myself: what is a bone tip? it just shows what the rotation and scale is visually, based on a
     * (0, 0, 1) bone tip you can compute where the tip would be based on the scale and so on I think... but then
     * again bones can be larger and have no scaled in them right so then what? it might just be to compute during
     * the auto weighting process maybe check assimp if that data is stored in there if a bone is positioned at
     * position (x, y, z) and then we have a vertex at (x, y + 1, z + 1) then its new position becomes (0, 1, 1)
     * that is it is positioned relative to the bones origin
     */
    glm::mat4 local_space_to_bone_space_in_bind_pose_transformation;
    // this transoformation takes a vertex in local space, and moves it to its animated position in local space
    glm::mat4 local_space_animated_transform_upto_this_bone = glm::mat4(0);

    /**
     * @brief Constructs a BoneInfo with a specified bind pose transformation.
     *
     * @param lstbst The local-space-to-bone-space transformation in the bind pose.
     */
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
