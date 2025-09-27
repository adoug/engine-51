#include "ExportManager.h"
#include "ExporterFactory.h"
#include "../exporters/GenericFileExporter.h"
#include "../../../a51lib/DFSFile.h"

ExportManager::ExportManager()
{
    fallbackExporter = std::make_unique<GenericFileExporter>();
}

bool ExportManager::exportFile(DFSFile& dfs, int entryNo, const std::string& exportDir, bool embedTextures)
{
    auto extension = dfs.getFileExtension(entryNo);

    auto exporter = ExporterFactory::getInstance().createExporter(extension);

    if (exporter && exporter->canExport(extension)) {
        return exporter->exportFile(dfs, entryNo, exportDir, embedTextures);
    }

    if (fallbackExporter && fallbackExporter->canExport(extension)) {
        return fallbackExporter->exportFile(dfs, entryNo, exportDir, embedTextures);
    }

    return false;
}

std::vector<std::string> ExportManager::getSupportedTypes() const
{
    return ExporterFactory::getInstance().getSupportedExtensions();
}

std::vector<std::string> ExportManager::getExporterDescriptions() const
{
    std::vector<std::string> descriptions;
    auto extensions = getSupportedTypes();

    for (const auto& ext : extensions) {
        auto exporter = ExporterFactory::getInstance().createExporter(ext);
        if (exporter) {
            descriptions.push_back(exporter->getDescription());
        }
    }

    if (fallbackExporter) {
        descriptions.push_back(fallbackExporter->getDescription());
    }

    return descriptions;
}