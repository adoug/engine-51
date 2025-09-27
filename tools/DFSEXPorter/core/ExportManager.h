#pragma once

#include "../interfaces/IFileExporter.h"
#include <memory>
#include <string>
#include <vector>

class DFSFile;

class ExportManager
{
private:
    std::unique_ptr<IFileExporter> fallbackExporter;

public:
    ExportManager();
    ~ExportManager() = default;

    bool exportFile(DFSFile& dfs, int entryNo, const std::string& exportDir, bool embedTextures = true);

    std::vector<std::string> getSupportedTypes() const;

    std::vector<std::string> getExporterDescriptions() const;
};