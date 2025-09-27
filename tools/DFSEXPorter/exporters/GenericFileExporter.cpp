#include "GenericFileExporter.h"
#include "../../a51lib/DFSFile.h"
#include <filesystem>
#include <fstream>
#include <iostream>

bool GenericFileExporter::canExport(const std::string& extension) const
{
    return true;
}

bool GenericFileExporter::exportFile(DFSFile& dfs, int entryNo, const std::string& exportDir, bool embedTextures)
{
    auto origFilename = dfs.getBaseFilename(entryNo);
    auto extension = dfs.getFileExtension(entryNo);

    std::filesystem::path exportPath;
    if (!exportDir.empty()) {
        exportPath = std::filesystem::path(exportDir);
        std::filesystem::create_directories(exportPath);
    }

    std::string fileName = (exportPath / (origFilename + extension)).string();
    uint8_t* fileData = dfs.getFileData(entryNo);
    int fileLen = dfs.getFileSize(entryNo);

    std::ofstream outFile(fileName, std::ios::binary);
    if (outFile.is_open()) {
        outFile.write((const char*)fileData, fileLen);
        std::cout << "Extracted raw file: " << fileName << std::endl;
        return true;
    } else {
        std::cerr << "Failed to extract file: " << fileName << std::endl;
        return false;
    }
}

std::string GenericFileExporter::getDescription() const
{
    return "Generic file exporter - extracts raw file data for unsupported formats";
}