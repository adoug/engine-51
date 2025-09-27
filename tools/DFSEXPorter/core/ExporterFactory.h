#pragma once

#include "../interfaces/IFileExporter.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

class ExporterFactory
{
public:
    using ExporterCreator = std::function<std::unique_ptr<IFileExporter>()>;

private:
    std::unordered_map<std::string, ExporterCreator> creators;
    static ExporterFactory instance;

    ExporterFactory();

public:
    static ExporterFactory& getInstance();

    void registerExporter(const std::string& extension, ExporterCreator creator);

    std::unique_ptr<IFileExporter> createExporter(const std::string& extension) const;

    std::vector<std::string> getSupportedExtensions() const;

    template<typename T>
    void registerExporter(const std::string& extension)
    {
        registerExporter(extension, []() { return std::make_unique<T>(); });
    }
};