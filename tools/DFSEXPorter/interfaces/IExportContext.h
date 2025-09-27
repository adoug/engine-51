#pragma once

#include <string>

class IExportContext
{
public:
    virtual ~IExportContext() = default;
    virtual std::string getExportDirectory() const = 0;
    virtual bool shouldEmbedTextures() const = 0;
    virtual void setExportDirectory(const std::string& dir) = 0;
    virtual void setEmbedTextures(bool embed) = 0;
};