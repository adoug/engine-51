#include "PlaySurfaceExporter.h"
#include "../../a51lib/DFSFile.h"
#include "../../a51lib/Playsurface.h"
#include "../utilities/ExportUtilities.h"
#include <filesystem>
#include <iostream>
#include <fstream>

bool PlaySurfaceExporter::canExport(const std::string& extension) const
{
    return extension == ".PLAYSURFACE";
}

bool PlaySurfaceExporter::exportFile(DFSFile& dfs, int entryNo, const std::string& exportDir, bool embedTextures)
{
    auto origFilename = dfs.getBaseFilename(entryNo);

    std::filesystem::path exportPath;
    if (!exportDir.empty()) {
        exportPath = std::filesystem::path(exportDir);
        std::filesystem::create_directories(exportPath);
    }

    std::string fileName = (exportPath / (origFilename + ".json")).string();

    Playsurface playSurface;
    uint8_t* fileData = dfs.getFileData(entryNo);
    int fileLen = dfs.getFileSize(entryNo);
    playSurface.readFile(fileData, fileLen);

    try {
        exportJSON(playSurface, fileName);
        std::cout << "Exported playsurface: " << fileName << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to export playsurface: " << fileName << " - " << e.what() << std::endl;
        return false;
    }
}

std::string PlaySurfaceExporter::getDescription() const
{
    return "PLAYSURFACE exporter - converts playsurface data to JSON format";
}