#pragma once

#include <string>

class DFSFile;

class IFileExporter
{
public:
    virtual ~IFileExporter() = default;
    virtual bool canExport(const std::string& extension) const = 0;
    virtual bool exportFile(DFSFile& dfs, int entryNo, const std::string& exportDir, bool embedTextures = true) = 0;
    virtual std::string getDescription() const = 0;
};