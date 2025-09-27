#include "ExporterFactory.h"
#include "../exporters/XBMPExporter.h"
#include "../exporters/RigidGeomExporter.h"
#include "../exporters/SkinGeomExporter.h"
#include "../exporters/PlaySurfaceExporter.h"
#include "../exporters/GenericFileExporter.h"

#include <vector>

ExporterFactory ExporterFactory::instance;

ExporterFactory::ExporterFactory()
{
    registerExporter<XBMPExporter>(".XBMP");
    registerExporter<RigidGeomExporter>(".RIGIDGEOM");
    registerExporter<SkinGeomExporter>(".SKINGEOM");
    registerExporter<PlaySurfaceExporter>(".PLAYSURFACE");
    registerExporter<GenericFileExporter>(".UNKNOWN");
}

ExporterFactory& ExporterFactory::getInstance()
{
    return instance;
}

void ExporterFactory::registerExporter(const std::string& extension, ExporterCreator creator)
{
    creators[extension] = creator;
}

std::unique_ptr<IFileExporter> ExporterFactory::createExporter(const std::string& extension) const
{
    auto it = creators.find(extension);
    if (it != creators.end()) {
        return it->second();
    }
    return nullptr;
}

std::vector<std::string> ExporterFactory::getSupportedExtensions() const
{
    std::vector<std::string> extensions;
    for (const auto& pair : creators) {
        extensions.push_back(pair.first);
    }
    return extensions;
}