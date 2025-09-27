#pragma once

#include "BaseFileExporter.h"

class XBMPExporter : public BaseFileExporter
{
protected:
    bool performExport(DFSFile& dfs, int entryNo, const std::string& outputPath, bool embedTextures) override;
    std::string getOutputExtension() const override { return ".png"; }

public:
    bool canExport(const std::string& extension) const override;
    std::string getDescription() const override;
};