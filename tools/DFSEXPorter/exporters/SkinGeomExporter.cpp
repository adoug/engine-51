#include "SkinGeomExporter.h"
#include "gltfExporter.h"
#include "../../a51lib/DFSFile.h"
#include "../../a51lib/SkinGeom.h"
#include <filesystem>
#include <iostream>

bool SkinGeomExporter::canExport(const std::string& extension) const
{
    return extension == ".SKINGEOM";
}

bool SkinGeomExporter::exportFile(DFSFile& dfs, int entryNo, const std::string& exportDir, bool embedTextures)
{
    auto origFilename = dfs.getBaseFilename(entryNo);

    std::filesystem::path exportPath;
    if (!exportDir.empty()) {
        exportPath = std::filesystem::path(exportDir);
        std::filesystem::create_directories(exportPath);
    }

    std::string fileName = (exportPath / (origFilename + ".gltf")).string();

    SkinGeom geom;
    uint8_t* fileData = dfs.getFileData(entryNo);
    int fileLen = dfs.getFileSize(entryNo);
    geom.readFile(fileData, fileLen);

    try {
        exportGLTF(geom, fileName, &dfs, embedTextures);
        std::cout << "Exported skin geometry: " << fileName << (embedTextures ? " (embedded textures)" : " (external textures)") << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to export skin geometry: " << fileName << " - " << e.what() << std::endl;
        return false;
    }
}

std::string SkinGeomExporter::getDescription() const
{
    return "SKINGEOM exporter - converts skinned/animated 3D geometry to GLTF format";
}