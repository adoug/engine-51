#pragma once

#include "../interfaces/IFileExporter.h"

class RigidGeomExporter : public IFileExporter
{
public:
    bool canExport(const std::string& extension) const override;
    bool exportFile(DFSFile& dfs, int entryNo, const std::string& exportDir, bool embedTextures = true) override;
    std::string getDescription() const override;
};