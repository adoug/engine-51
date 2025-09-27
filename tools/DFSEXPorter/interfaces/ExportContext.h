#pragma once

#include "IExportContext.h"

class ExportContext : public IExportContext
{
private:
    std::string exportDirectory;
    bool embedTextures;

public:
    ExportContext(const std::string& dir = "", bool embed = true)
        : exportDirectory(dir), embedTextures(embed) {}

    std::string getExportDirectory() const override { return exportDirectory; }
    bool shouldEmbedTextures() const override { return embedTextures; }
    void setExportDirectory(const std::string& dir) override { exportDirectory = dir; }
    void setEmbedTextures(bool embed) override { embedTextures = embed; }
};