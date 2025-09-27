// Suppress CRT security warnings for stb_image_write
#define _CRT_SECURE_NO_WARNINGS

#include "gltfExporter.h"
#include "../../a51lib/RigidGeom.h"
#include "../../a51lib/SkinGeom.h"
#include "../../a51lib/Bitmap.h"
#include "../../a51lib/DFSFile.h"

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "../../a51lib/gltf/tiny_gltf.h"

// Include stb_image_write separately for PNG creation
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../a51lib/gltf/stb_image_write.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>

// Base64 encoding table
static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// Helper function to encode binary data to base64
std::string base64_encode(const unsigned char* data, size_t len)
{
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (len--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';
    }

    return ret;
}

// PNG write callback for stb_image_write to capture PNG data in memory
struct PNGWriteData {
    std::vector<unsigned char> data;
};

void png_write_callback(void* context, void* data, int size)
{
    PNGWriteData* png_data = static_cast<PNGWriteData*>(context);
    unsigned char* bytes = static_cast<unsigned char*>(data);
    png_data->data.insert(png_data->data.end(), bytes, bytes + size);
}

// Helper function to extract texture data and embed as PNG in glTF image (following legacy approach)
bool extractAndEmbedTexture(DFSFile* dfsFile, const std::string& textureName, tinygltf::Image& image)
{
    // Extract base name and extension from textureName
    std::string textureBaseName = textureName;
    std::string textureExtension;

    size_t dotPos = textureName.find_last_of('.');
    if (dotPos != std::string::npos) {
        textureBaseName = textureName.substr(0, dotPos);
        textureExtension = textureName.substr(dotPos);
    }

    // Convert to uppercase for comparison
    std::transform(textureBaseName.begin(), textureBaseName.end(), textureBaseName.begin(), ::toupper);
    std::transform(textureExtension.begin(), textureExtension.end(), textureExtension.begin(), ::toupper);

    // Find the texture file in the DFS
    int textureIndex = -1;
    for (int i = 0; i < dfsFile->numFiles(); ++i) {
        std::string baseName = dfsFile->getBaseFilename(i);
        std::string extension = dfsFile->getFileExtension(i);

        // Convert to uppercase for comparison
        std::transform(baseName.begin(), baseName.end(), baseName.begin(), ::toupper);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::toupper);

        if (baseName == textureBaseName && extension == textureExtension) {
            textureIndex = i;
            break;
        }
    }

    if (textureIndex == -1) {
        return false; // Texture not found
    }

    // Load the texture data
    uint8_t* fileData = dfsFile->getFileData(textureIndex);
    int fileLen = dfsFile->getFileSize(textureIndex);

    if (!fileData || fileLen == 0) {
        return false;
    }

    // Load into Bitmap
    Bitmap bitmap;
    const bool oldVersion = dfsFile->getVersion() == 1;
    if (!bitmap.readFile(fileData, fileLen, oldVersion)) {
        return false;
    }

    // Convert to RGB888 format (following legacy approach)
    bitmap.convertFormat(Bitmap::FMT_24_RGB_888);

    // Create PNG data using stb_image_write
    PNGWriteData pngData;
    if (!stbi_write_png_to_func(png_write_callback, &pngData,
                                bitmap.getWidth(), bitmap.getHeight(), 3,
                                bitmap.data.pixelData, bitmap.getWidth() * 3)) {
        return false;
    }

    // Embed the PNG data in the glTF image using base64 data URI (following legacy pattern)
    std::string base64Data = base64_encode(pngData.data.data(), pngData.data.size());
    std::string dataUri = "data:image/png;base64," + base64Data;

    // Clear all other properties and set only the data URI (following legacy)
    image = tinygltf::Image(); // Reset to clean state
    image.uri = dataUri;

    return true;
}

// Helper function to get directory from file path
std::string getDirectory(const std::string& filePath)
{
    std::filesystem::path path(filePath);
    return path.parent_path().string();
}

// Helper function to extract texture data and save as external PNG file
bool extractAndSavePNG(DFSFile* dfsFile, const std::string& textureName, const std::string& outputPath)
{
    tinygltf::Image tempImage;
    if (extractAndEmbedTexture(dfsFile, textureName, tempImage)) {
        // We have the image embedded as base64 data URI, need to extract PNG data
        std::string dataUri = tempImage.uri;

        // Find the base64 data after "data:image/png;base64,"
        size_t commaPos = dataUri.find(',');
        if (commaPos != std::string::npos) {
            std::string base64Data = dataUri.substr(commaPos + 1);

            // Decode base64 data (simplified for this use case)
            std::vector<unsigned char> pngData;

            // Simple base64 decoding
            std::string cleaned_input;
            for (char c : base64Data) {
                if (c != '=' && c != '\n' && c != '\r' && c != ' ' && c != '\t') {
                    cleaned_input += c;
                }
            }

            for (size_t i = 0; i < cleaned_input.size(); i += 4) {
                uint32_t value = 0;
                for (int j = 0; j < 4 && i + j < cleaned_input.size(); ++j) {
                    char c = cleaned_input[i + j];
                    uint8_t val = 0;
                    if (c >= 'A' && c <= 'Z') val = c - 'A';
                    else if (c >= 'a' && c <= 'z') val = c - 'a' + 26;
                    else if (c >= '0' && c <= '9') val = c - '0' + 52;
                    else if (c == '+') val = 62;
                    else if (c == '/') val = 63;
                    value = (value << 6) | val;
                }

                if (i + 1 < cleaned_input.size()) pngData.push_back((value >> 16) & 0xFF);
                if (i + 2 < cleaned_input.size()) pngData.push_back((value >> 8) & 0xFF);
                if (i + 3 < cleaned_input.size()) pngData.push_back(value & 0xFF);
            }

            std::ofstream file(outputPath, std::ios::binary);
            if (file.is_open()) {
                file.write(reinterpret_cast<const char*>(pngData.data()), pngData.size());
                return true;
            }
        }
    }
    return false;
}

// Helper function to setup texture for GLTF (following legacy approach)
bool setupTexture(DFSFile* dfsFile, const std::string& textureName, tinygltf::Image& image, const std::string& outputDir, bool embedTextures)
{
    if (!dfsFile) {
        return false;
    }

    // Create PNG filename (following legacy pattern)
    std::string pngName = textureName;
    if (pngName.length() >= 4) {
        pngName.replace(pngName.end() - 4, pngName.end(), "png");
    }
    for (auto& c : pngName) {
        c = toupper(c);
    }

    if (embedTextures && extractAndEmbedTexture(dfsFile, textureName, image)) {
        // Successfully embedded - image object is already populated with base64 data URI
        std::cout << "Embedded texture as PNG data URI: " << textureName << std::endl;
    } else {
        // Use external PNG file
        image = tinygltf::Image(); // Reset to clean state
        image.uri = pngName;

        // Extract and save PNG file
        std::filesystem::path outputPath;
        if (!outputDir.empty()) {
            outputPath = std::filesystem::path(outputDir) / pngName;
        } else {
            outputPath = std::filesystem::path(pngName);
        }

        if (extractAndSavePNG(dfsFile, textureName, outputPath.string())) {
            std::cout << "Exported texture as PNG: " << outputPath.string() << std::endl;
        } else {
            std::cerr << "Warning: Failed to export texture: " << textureName << std::endl;
            return false;
        }
    }

    return true;
}

void exportGLTF(RigidGeom& rigidGeom, const std::string& fileName, DFSFile* dfsFile, bool embedTextures)
{
    std::cout << "Exporting RigidGeom to GLTF: " << fileName << std::endl;
    
    tinygltf::Model m;
    tinygltf::Scene scene;

    int numMeshes = rigidGeom.getNumMeshes();
    int accessorIdx = 0;
    int viewIdx = 0;
    int materialIdx = 0;
    int textureIdx = 0;

    // Get output directory for external PNG files
    std::string outputDir = getDirectory(fileName);

    // Process textures
    for (int texNo = 0; texNo < rigidGeom.getNumTextures(); ++texNo) {
        tinygltf::Image image;
        std::string tfn = rigidGeom.getTextureFilename(texNo);
        
        if (setupTexture(dfsFile, tfn, image, outputDir, embedTextures)) {
            m.images.push_back(image);
        }
    }

    int nodeMeshIdx = 0;
    int buffersIdx = 0;
    
    for (int meshNo = 0; meshNo < numMeshes; ++meshNo) {
        Mesh& mesh = rigidGeom.meshes[meshNo];
        for (int submeshIdx = mesh.iSubMesh; submeshIdx < mesh.iSubMesh + mesh.nSubMeshes; ++submeshIdx) {

            int    numVertices = rigidGeom.getNumSubmeshVertices(submeshIdx);
            float* vertexData = rigidGeom.getSubmeshVerticesPUV(submeshIdx);
            float* normals = rigidGeom.getSubmeshVertexNormals(submeshIdx);

            // Create vertex data buffer
            const unsigned char* pcvd = (unsigned char*)vertexData;
            tinygltf::Buffer buffer;
            buffer.data = std::vector<unsigned char>(pcvd, pcvd + numVertices * 20);
            delete[] vertexData;
            m.buffers.push_back(buffer);
            int vBufIdx = buffersIdx++;

            // Create normals buffer
            const unsigned char* pcnd = (unsigned char*)normals;
            tinygltf::Buffer normalsBuffer;
            normalsBuffer.data = std::vector<unsigned char>(pcnd, pcnd + numVertices * 12);
            delete[] normals;
            m.buffers.push_back(normalsBuffer);
            int nBufIdx = buffersIdx++;

            // Create buffer views
            tinygltf::BufferView posBufferView;
            posBufferView.buffer = vBufIdx;
            posBufferView.byteOffset = 0;
            posBufferView.byteLength = numVertices * 12;
            posBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

            tinygltf::BufferView uvBufferView;
            uvBufferView.buffer = vBufIdx;
            uvBufferView.byteOffset = numVertices * 12;
            uvBufferView.byteLength = numVertices * 8;
            uvBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

            m.bufferViews.push_back(posBufferView);
            int posBufferViewIdx = viewIdx++;

            m.bufferViews.push_back(uvBufferView);
            int uvBufferViewIdx = viewIdx++;

            tinygltf::BufferView normalsBufferView;
            normalsBufferView.buffer = nBufIdx;
            normalsBufferView.byteOffset = 0;
            normalsBufferView.byteLength = numVertices * 12;
            normalsBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

            m.bufferViews.push_back(normalsBufferView);
            int normalsBufferViewIdx = viewIdx++;

            // Create accessors
            tinygltf::Accessor posAccessor;
            posAccessor.bufferView = posBufferViewIdx;
            posAccessor.byteOffset = 0;
            posAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            posAccessor.count = numVertices;
            posAccessor.type = TINYGLTF_TYPE_VEC3;
            const BBox& bbox = rigidGeom.getBoundingBox(meshNo);
            posAccessor.maxValues = {bbox.max.x, bbox.max.y, bbox.max.z};
            posAccessor.minValues = {bbox.min.x, bbox.min.y, bbox.min.z};

            tinygltf::Accessor uvAccessor;
            uvAccessor.bufferView = uvBufferViewIdx;
            uvAccessor.byteOffset = 0;
            uvAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            uvAccessor.count = numVertices;
            uvAccessor.type = TINYGLTF_TYPE_VEC2;
            uvAccessor.maxValues = {1.0, 1.0};
            uvAccessor.minValues = {-1.0, -1.0};

            tinygltf::Accessor normalsAccessor;
            normalsAccessor.bufferView = normalsBufferViewIdx;
            normalsAccessor.byteOffset = 0;
            normalsAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            normalsAccessor.count = numVertices;
            normalsAccessor.type = TINYGLTF_TYPE_VEC3;
            normalsAccessor.maxValues = {1.0, 1.0, 1.0};
            normalsAccessor.minValues = {-1.0, -1.0, -1.0};

            m.accessors.push_back(posAccessor);
            int posAccessorId = accessorIdx++;

            m.accessors.push_back(uvAccessor);
            int uvAccessorId = accessorIdx++;

            m.accessors.push_back(normalsAccessor);
            int normalsAccessorId = accessorIdx++;

            // Create texture
            tinygltf::Texture texture;
            int meshMatIdx = rigidGeom.subMeshes[submeshIdx].iMaterial;
            texture.source = rigidGeom.materials[meshMatIdx].iTexture;
            m.textures.push_back(texture);
            int currentTextureIdx = m.textures.size() - 1;

            // Create material
            tinygltf::Material mat;
            mat.pbrMetallicRoughness.baseColorFactor = {1.0f, 0.9f, 0.9f, 1.0f};
            mat.pbrMetallicRoughness.baseColorTexture.index = currentTextureIdx;
            mat.doubleSided = true;
            m.materials.push_back(mat);
            int theMaterialIdx = materialIdx++;
            textureIdx++;

            // Create mesh primitive
            tinygltf::Primitive primitive;
            primitive.attributes["POSITION"] = posAccessorId;
            primitive.attributes["TEXCOORD_0"] = uvAccessorId;
            primitive.attributes["NORMAL"] = normalsAccessorId;
            primitive.material = theMaterialIdx;
            primitive.mode = TINYGLTF_MODE_TRIANGLES;
            tinygltf::Mesh mesh;
            mesh.primitives.push_back(primitive);
            m.meshes.push_back(mesh);

            // Create node
            tinygltf::Node node;
            node.mesh = nodeMeshIdx;
            scene.nodes.push_back(nodeMeshIdx++);
            m.nodes.push_back(node);
        }
    }
    
    m.scenes.push_back(scene);

    // Define the asset
    tinygltf::Asset asset;
    asset.version = "2.0";
    asset.generator = "DFSEXPorter";
    m.asset = asset;

    // Save to file
    tinygltf::TinyGLTF gltf;
    bool success = gltf.WriteGltfSceneToFile(&m, fileName,
                              false,  // embedImages
                              true,   // embedBuffers
                              true,   // pretty print
                              false); // write binary
                              
    if (success) {
        std::cout << "Successfully exported GLTF file: " << fileName << std::endl;
    } else {
        std::cerr << "Failed to export GLTF file: " << fileName << std::endl;
    }
}

void exportGLTF(SkinGeom& geom, const std::string& fileName, DFSFile* dfsFile, bool embedTextures)
{
    tinygltf::Model m;
    tinygltf::Scene scene;

    int numMeshes = geom.getNumMeshes();
    int accessorIdx = 0;
    int viewIdx = 0;
    int materialIdx = 0;
    int textureIdx = 0;

    // Get output directory for external files
    std::string outputDir = getDirectory(fileName);

    // Process textures and create mapping for successful exports
    std::map<int, int> textureIndexMap; // Maps original texture index to new image index
    for (int texNo = 0; texNo < geom.getNumTextures(); ++texNo) {
        tinygltf::Image image;
        std::string tfn = geom.getTextureFilename(texNo);

        if (setupTexture(dfsFile, tfn, image, outputDir, embedTextures)) {
            textureIndexMap[texNo] = m.images.size();
            m.images.push_back(image);
        }
    }

    int nodeMeshIdx = 0;
    int buffersIdx = 0;
    for (int meshNo = 0; meshNo < numMeshes; ++meshNo) {
        Mesh& mesh = geom.meshes[meshNo];
        for (int submeshIdx = mesh.iSubMesh; submeshIdx < mesh.iSubMesh + mesh.nSubMeshes; ++submeshIdx) {

            int    numVertices = geom.getNumSubmeshVertices(submeshIdx);
            float* vertexData = geom.getSubmeshVerticesPUV(submeshIdx);
            float* normals = geom.getSubmeshVertexNormals(submeshIdx);

            // Calculate UV bounds from actual data before creating buffers
            float minU = 1e6f, maxU = -1e6f, minV = 1e6f, maxV = -1e6f;
            for (int v = 0; v < numVertices; ++v) {
                float u = vertexData[v * 5 + 3]; // UV is at offset 3,4 in PUV format
                float vCoord = vertexData[v * 5 + 4];
                minU = std::min(minU, u);
                maxU = std::max(maxU, u);
                minV = std::min(minV, vCoord);
                maxV = std::max(maxV, vCoord);
            }

            // Calculate normal bounds from actual data
            float minNX = 1e6f, maxNX = -1e6f, minNY = 1e6f, maxNY = -1e6f, minNZ = 1e6f, maxNZ = -1e6f;
            for (int v = 0; v < numVertices; ++v) {
                float nx = normals[v * 3 + 0];
                float ny = normals[v * 3 + 1];
                float nz = normals[v * 3 + 2];
                minNX = std::min(minNX, nx);
                maxNX = std::max(maxNX, nx);
                minNY = std::min(minNY, ny);
                maxNY = std::max(maxNY, ny);
                minNZ = std::min(minNZ, nz);
                maxNZ = std::max(maxNZ, nz);
            }

            // Create buffers
            const unsigned char* pcvd = (unsigned char*)vertexData;
            tinygltf::Buffer     buffer;
            buffer.data = std::vector<unsigned char>(pcvd, pcvd + numVertices * 20);
            delete[] vertexData;
            m.buffers.push_back(buffer);
            int vBufIdx = buffersIdx++;

            const unsigned char* pcnd = (unsigned char*)normals;
            tinygltf::Buffer     normalsBuffer;
            normalsBuffer.data = std::vector<unsigned char>(pcnd, pcnd + numVertices * 12);
            delete[] normals;
            m.buffers.push_back(normalsBuffer);
            int nBufIndx = buffersIdx++;

            tinygltf::BufferView posBufferView;
            posBufferView.buffer = vBufIdx;
            posBufferView.byteOffset = 0;
            posBufferView.byteLength = numVertices * 12;
            posBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

            tinygltf::BufferView uvBufferView;
            uvBufferView.buffer = vBufIdx;
            uvBufferView.byteOffset = numVertices * 12;
            uvBufferView.byteLength = numVertices * 8;
            uvBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

            m.bufferViews.push_back(posBufferView);
            int posBufferViewIdx = viewIdx++;

            m.bufferViews.push_back(uvBufferView);
            int uvBufferViewIdx = viewIdx++;

            tinygltf::BufferView normalsBufferView;
            normalsBufferView.buffer = nBufIndx;
            normalsBufferView.byteOffset = 0;
            normalsBufferView.byteLength = numVertices * 12;
            normalsBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

            m.bufferViews.push_back(normalsBufferView);
            int normalsBufferViewIdx = viewIdx++;

            // Describe the layout of posBufferView, the vertices themself
            tinygltf::Accessor posAccessor;
            posAccessor.bufferView = posBufferViewIdx;
            posAccessor.byteOffset = 0;
            posAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            posAccessor.count = numVertices;
            posAccessor.type = TINYGLTF_TYPE_VEC3;
            const BBox& bbox = geom.getBoundingBox(meshNo);
            posAccessor.maxValues = {bbox.max.x, bbox.max.y, bbox.max.z};
            posAccessor.minValues = {bbox.min.x, bbox.min.y, bbox.min.z};

            tinygltf::Accessor uvAccessor;
            uvAccessor.bufferView = uvBufferViewIdx;
            uvAccessor.byteOffset = 0;
            uvAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            uvAccessor.count = numVertices;
            uvAccessor.type = TINYGLTF_TYPE_VEC2;
            uvAccessor.maxValues = {maxU, maxV};
            uvAccessor.minValues = {minU, minV};

            tinygltf::Accessor normalsAccessor;
            normalsAccessor.bufferView = normalsBufferViewIdx;
            normalsAccessor.byteOffset = 0;
            normalsAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            normalsAccessor.count = numVertices;
            normalsAccessor.type = TINYGLTF_TYPE_VEC3;
            normalsAccessor.maxValues = {maxNX, maxNY, maxNZ};
            normalsAccessor.minValues = {minNX, minNY, minNZ};

            m.accessors.push_back(posAccessor);
            int posAccessorId = accessorIdx++;

            m.accessors.push_back(uvAccessor);
            int uvAccessorId = accessorIdx++;

            m.accessors.push_back(normalsAccessor);
            int normalsAccessorId = accessorIdx++;

            // Create texture and material per submesh with proper mapping
            int meshMatIdx = geom.subMeshes[submeshIdx].iMaterial;
            int originalTextureIdx = geom.materials[meshMatIdx].iTexture;

            // Create a simple material
            tinygltf::Material mat;
            mat.pbrMetallicRoughness.baseColorFactor = {1.0f, 0.9f, 0.9f, 1.0f};

            // Only reference texture if it was successfully exported
            if (textureIndexMap.find(originalTextureIdx) != textureIndexMap.end()) {
                tinygltf::Texture texture;
                texture.source = textureIndexMap[originalTextureIdx]; // Use mapped image index
                m.textures.push_back(texture);
                int currentTextureIdx = m.textures.size() - 1;
                mat.pbrMetallicRoughness.baseColorTexture.index = currentTextureIdx;
            }
            // If texture wasn't found, material will use base color only

            mat.doubleSided = true;
            m.materials.push_back(mat);
            int theMaterialIdx = materialIdx++;
            textureIdx++;

            // Build the mesh primitive and add it to the mesh
            tinygltf::Primitive primitive;
            primitive.attributes["POSITION"] = posAccessorId;
            primitive.attributes["TEXCOORD_0"] = uvAccessorId;
            primitive.attributes["NORMAL"] = normalsAccessorId;
            primitive.material = theMaterialIdx;
            primitive.mode = TINYGLTF_MODE_TRIANGLES;
            tinygltf::Mesh mesh;
            mesh.primitives.push_back(primitive);
            m.meshes.push_back(mesh);

            // Other tie ups
            tinygltf::Node node;
            node.mesh = nodeMeshIdx;
            scene.nodes.push_back(nodeMeshIdx++); // Default scene
            m.nodes.push_back(node);
        }
    }
    m.scenes.push_back(scene);

    // Define the asset. The version is required
    tinygltf::Asset asset;
    asset.version = "2.0";
    asset.generator = "DFSEXPorter";
    m.asset = asset;

    // Save it to a file
    tinygltf::TinyGLTF gltf;
    bool success = gltf.WriteGltfSceneToFile(&m, fileName,
                              embedTextures,  // embedImages
                              true,   // embedBuffers
                              true,   // pretty print
                              false); // write binary

    if (success) {
        std::cout << "Successfully exported SkinGeom to: " << fileName << std::endl;
    } else {
        std::cerr << "Failed to export SkinGeom to: " << fileName << std::endl;
    }
}