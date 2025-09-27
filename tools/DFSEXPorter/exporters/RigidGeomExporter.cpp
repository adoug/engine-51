#include "RigidGeomExporter.h"
#include "gltfExporter.h"
#include "../../a51lib/DFSFile.h"
#include "../../a51lib/RigidGeom.h"
#include <filesystem>
#include <iostream>

bool RigidGeomExporter::canExport(const std::string& extension) const
{
    return extension == ".RIGIDGEOM";
}

bool RigidGeomExporter::exportFile(DFSFile& dfs, int entryNo, const std::string& exportDir, bool embedTextures)
{
    auto origFilename = dfs.getBaseFilename(entryNo);

    std::filesystem::path exportPath;
    if (!exportDir.empty()) {
        exportPath = std::filesystem::path(exportDir);
        std::filesystem::create_directories(exportPath);
    }

    std::string fileName = (exportPath / (origFilename + ".gltf")).string();

    RigidGeom rigidGeom;
    uint8_t* fileData = dfs.getFileData(entryNo);
    int fileLen = dfs.getFileSize(entryNo);
    rigidGeom.readFile(fileData, fileLen);

    try {
        exportGLTF(rigidGeom, fileName, &dfs, embedTextures);
        std::cout << "Exported rigid geometry: " << fileName << (embedTextures ? " (embedded textures)" : " (external textures)") << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to export rigid geometry: " << fileName << " - " << e.what() << std::endl;
        return false;
    }
}

std::string RigidGeomExporter::getDescription() const
{
    return "RIGIDGEOM exporter - converts rigid 3D geometry to GLTF format";
}