#include "XBMPExporter.h"
#include "../../../a51lib/DFSFile.h"
#include "../../../a51lib/Bitmap.h"
#include "../../../a51lib/gltf/stb_image_write.h"
#include <sstream>

bool XBMPExporter::canExport(const std::string& extension) const
{
    return extension == ".XBMP";
}

bool XBMPExporter::performExport(DFSFile& dfs, int entryNo, const std::string& outputPath, bool embedTextures)
{
    uint8_t* fileData = dfs.getFileData(entryNo);
    int fileLen = dfs.getFileSize(entryNo);

    if (!fileData || fileLen == 0) {
        logError(outputPath, "Invalid file data");
        return false;
    }

    Bitmap bitmap;
    const bool oldVersion = dfs.getVersion() == 1;

    if (!bitmap.readFile(fileData, fileLen, oldVersion)) {
        logError(outputPath, "Failed to read bitmap data");
        return false;
    }

    // Convert to RGB format for PNG export
    bitmap.convertFormat(Bitmap::FMT_24_RGB_888);

    // Export as proper PNG using stb_image_write
    if (stbi_write_png(outputPath.c_str(), bitmap.getWidth(), bitmap.getHeight(), 3,
                      bitmap.data.pixelData, bitmap.getWidth() * 3)) {

        // Create detailed success message
        std::ostringstream details;
        details << "PNG texture (" << bitmap.getWidth() << "x" << bitmap.getHeight() << ")";
        logSuccess(outputPath, details.str());
        return true;
    } else {
        logError(outputPath, "PNG encoding failed");
        return false;
    }
}

std::string XBMPExporter::getDescription() const
{
    return "XBMP texture exporter - converts Xbox bitmap textures to PNG format with improved error handling";
}