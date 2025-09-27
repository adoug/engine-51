#include "gltfExporter.h"
#include "../../a51lib/RigidGeom.h"
#include "../../a51lib/SkinGeom.h"
#include "../../a51lib/Bitmap.h"
#include "../../a51lib/DFSFile.h"

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "../../a51lib/gltf/tiny_gltf.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

// Helper function to get directory from file path
std::string getDirectory(const std::string& filePath)
{
    std::filesystem::path path(filePath);
    return path.parent_path().string();
}

// Simplified texture handling - just use external files for now
bool setupTexture(DFSFile* dfsFile, const std::string& textureName, tinygltf::Image& image)
{
    // Create PNG filename
    std::string pngName = textureName;
    if (pngName.length() >= 4) {
        pngName.replace(pngName.end() - 4, pngName.end(), "png");
    }
    for (auto& c : pngName) {
        c = toupper(c);
    }

    // For now, just use external PNG file reference
    image = tinygltf::Image();
    image.uri = pngName;
    
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
        
        if (setupTexture(dfsFile, tfn, image)) {
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
    std::cout << "Exporting SkinGeom to GLTF: " << fileName << std::endl;
    
    // For now, just output a message that this function needs implementation
    std::cout << "SkinGeom export not yet implemented" << std::endl;
}