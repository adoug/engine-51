#pragma once

#include "../interfaces/IFileExporter.h"
#include <filesystem>
#include <string>

class BaseFileExporter : public IFileExporter
{
protected:
    std::filesystem::path createExportPath(const std::string& exportDir,
                                         const std::string& filename,
                                         const std::string& extension) const;

    void logSuccess(const std::string& fileName, const std::string& details = "") const;
    void logError(const std::string& fileName, const std::string& error = "") const;

    virtual bool performExport(DFSFile& dfs, int entryNo, const std::string& outputPath, bool embedTextures) = 0;
    virtual std::string getOutputExtension() const;

public:
    bool exportFile(DFSFile& dfs, int entryNo, const std::string& exportDir, bool embedTextures = true) final;
};