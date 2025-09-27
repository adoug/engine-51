#include "BaseFileExporter.h"
#include "../../../a51lib/DFSFile.h"
#include <iostream>

std::filesystem::path BaseFileExporter::createExportPath(const std::string& exportDir,
                                                        const std::string& filename,
                                                        const std::string& extension) const
{
    std::filesystem::path exportPath;
    if (!exportDir.empty()) {
        exportPath = std::filesystem::path(exportDir);
        std::filesystem::create_directories(exportPath);
    }
    return exportPath / (filename + extension);
}

void BaseFileExporter::logSuccess(const std::string& fileName, const std::string& details) const
{
    std::cout << "Exported: " << fileName;
    if (!details.empty()) {
        std::cout << " " << details;
    }
    std::cout << std::endl;
}

void BaseFileExporter::logError(const std::string& fileName, const std::string& error) const
{
    std::cerr << "Failed to export: " << fileName;
    if (!error.empty()) {
        std::cerr << " - " << error;
    }
    std::cerr << std::endl;
}

bool BaseFileExporter::exportFile(DFSFile& dfs, int entryNo, const std::string& exportDir, bool embedTextures)
{
    try {
        auto origFilename = dfs.getBaseFilename(entryNo);
        auto extension = dfs.getFileExtension(entryNo);

        std::string outputPath = createExportPath(exportDir, origFilename, getOutputExtension()).string();
        return performExport(dfs, entryNo, outputPath, embedTextures);

    } catch (const std::exception& e) {
        logError(dfs.getBaseFilename(entryNo), e.what());
        return false;
    }
}

std::string BaseFileExporter::getOutputExtension() const
{
    return ".out"; // Default fallback
}