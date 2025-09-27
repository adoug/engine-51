#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <vector>
#include <set>
#include "gltfExporter.h"
#include "../../a51lib/DFSFile.h"
#include "../../a51lib/Bitmap.h"
#include "../../a51lib/RigidGeom.h"
#include "../../a51lib/SkinGeom.h"
#include "../../a51lib/Playsurface.h"
#include "../../a51lib/BinLevel.h"
#include "../../a51lib/LevelTemplate.h"
#include "../../a51lib/dataUtil/Bitstream.h"
#include "../../a51lib/gltf/json.hpp"

using nlohmann::json;

std::string getPropStringVal(PropertyType type, Bitstream& bs);

void showUsage() {
    std::cout << "DFS Exporter - Console Export Tool" << std::endl;
    std::cout << "Usage: DFSEXPorter [options] <dfs_file>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -l              List all files in the DFS archive" << std::endl;
    std::cout << "  -x [dir]        Export all files to directory (current dir if not specified)" << std::endl;
    std::cout << "  -e <filename>   Export specific file by name" << std::endl;
    std::cout << "  -t <type> [dir] Export all files of specific type (e.g., RIGIDGEOM, SKINGEOM, XBMP)" << std::endl;
    std::cout << "  --external-tex  Use external PNG files for textures instead of embedding" << std::endl;
    std::cout << "  (no flags)      Show DFS header and file list" << std::endl;
}

void exportJSON(Playsurface& playSurface, const std::string& filename)
{
    json zones;
    for (auto& zi : playSurface.zones) {
        if (zi.numSurfaces > 0) {
            json zone;
            for (auto& surface : zi.surfaces) {
                json surf;
                surf["geomName"] = playSurface.geoms.at(surface.GeomNameIndex);

                json localToWorld;
                for (int r = 0; r < 4; ++r) {
                    for (int c = 0; c < 4; ++c) {
                        localToWorld.push_back(surface.L2W.cells[r][c]);
                    }
                }
                surf["localToWorld"] = localToWorld;
                zone["surfaces"].push_back(surf);
            }
            zones.push_back(zone);
        }
    }
    json output;
    output["zones"] = zones;
    auto outputString = output.dump(2);
    std::ofstream outputFile(filename);
    if (outputFile.is_open()) {
        outputFile << outputString << std::endl;
    }
}

void exportJSON(BinLevel& binLevel, const std::string& filename)
{
    Bitstream bs;
    bs.init(binLevel.bitstreamData, binLevel.bitstreamLen);

    json objects;
    for (auto& object : binLevel.objects) {
        json jsonObject;
        jsonObject["type"] = binLevel.dictionary[object.typeIndex];
        json properties;
        for (int ip = object.iProperty; ip < object.iProperty + object.nProperty; ++ip){
            auto& property = binLevel.properties[ip];
            properties[binLevel.dictionary[property.nameIndex]] = getPropStringVal(property.type, bs);
        }
        jsonObject["properties"] = properties;
        objects.push_back(jsonObject);
    }
    json output;
    output["objects"] = objects;
    auto outputString = output.dump(2);
    std::ofstream outputFile(filename);
    if (outputFile.is_open()) {
        outputFile << outputString << std::endl;
    }
}

void exportJSON(LevelTemplate& levelTemplate, const std::string& filename)
{
    Bitstream bs;
    bs.init(levelTemplate.bitstreamData, levelTemplate.bitstreamLen);

    json jsTemplates;
    for (auto& entry : levelTemplate.templates)
    {
        json jsTemplate;
        jsTemplate["name"] = levelTemplate.dictionary[entry.nameIndex];
        std::vector<float> apvec;
        apvec.push_back(entry.anchorPos.x);
        apvec.push_back(entry.anchorPos.y);
        apvec.push_back(entry.anchorPos.z);
        jsTemplate["anchorpos"] = apvec;

        bs.setCursor(entry.iStartBitStream);
        json objects;
        for (int io = entry.iObject; io < entry.iObject + entry.nObjects; ++io){
            auto& object = levelTemplate.objects[io];
            json jsonObject;
            jsonObject["type"] = levelTemplate.dictionary[object.typeIndex];
            json properties;
            for (int ip = object.iProperty; ip < object.iProperty + object.nProperty; ++ip){
                auto& property = levelTemplate.properties[ip];
                properties[levelTemplate.dictionary[property.nameIndex]] = getPropStringVal(property.type, bs);
            }
            jsonObject["properties"] = properties;
            objects.push_back(jsonObject);
        }
        jsTemplate["objects"] = objects;
        jsTemplates.push_back(jsTemplate);
    }
    json output;
    output["templates"] = jsTemplates;
    auto outputString = output.dump(2);
    std::ofstream outputFile(filename);
    if (outputFile.is_open()) {
        outputFile << outputString << std::endl;
    }
}

void exportFile(DFSFile& dfs, int entryNo, const std::string& exportDir, bool embedTextures = true)
{
    auto extension = dfs.getFileExtension(entryNo);
    auto origFilename = dfs.getBaseFilename(entryNo);

    std::filesystem::path exportPath;
    if (!exportDir.empty()) {
        exportPath = std::filesystem::path(exportDir);
        std::filesystem::create_directories(exportPath);
    }

    if (extension == ".XBMP") {
        std::string fileName = (exportPath / (origFilename + ".png")).string();

        uint8_t* fileData = dfs.getFileData(entryNo);
        int fileLen = dfs.getFileSize(entryNo);
        Bitmap bitmap;
        const bool oldVersion = dfs.getVersion() == 1;
        bitmap.readFile(fileData, fileLen, oldVersion);
        bitmap.convertFormat(Bitmap::FMT_32_ARGB_8888);

        // Save bitmap data as raw RGBA for now (PNG export would need image library)
        std::ofstream outFile(fileName, std::ios::binary);
        if (outFile.is_open()) {
            outFile.write((const char*)bitmap.data.pixelData, bitmap.width * bitmap.height * 4);
            std::cout << "Exported bitmap: " << fileName << " (" << bitmap.width << "x" << bitmap.height << ")" << std::endl;
        }
    } else if (extension == ".RIGIDGEOM") {
        std::string fileName = (exportPath / (origFilename + ".gltf")).string();

        RigidGeom rigidGeom;
        uint8_t* fileData = dfs.getFileData(entryNo);
        int fileLen = dfs.getFileSize(entryNo);
        rigidGeom.readFile(fileData, fileLen);
        exportGLTF(rigidGeom, fileName, &dfs, embedTextures);
        std::cout << "Exported rigid geometry: " << fileName << (embedTextures ? " (embedded textures)" : " (external textures)") << std::endl;
    } else if (extension == ".SKINGEOM") {
        std::string fileName = (exportPath / (origFilename + ".gltf")).string();

        SkinGeom geom;
        uint8_t* fileData = dfs.getFileData(entryNo);
        int fileLen = dfs.getFileSize(entryNo);
        geom.readFile(fileData, fileLen);
        exportGLTF(geom, fileName, &dfs, embedTextures);
        std::cout << "Exported skin geometry: " << fileName << (embedTextures ? " (embedded textures)" : " (external textures)") << std::endl;
    } else if (extension == ".PLAYSURFACE") {
        std::string fileName = (exportPath / (origFilename + ".json")).string();

        Playsurface playSurface;
        uint8_t* fileData = dfs.getFileData(entryNo);
        int fileLen = dfs.getFileSize(entryNo);
        playSurface.readFile(fileData, fileLen);
        exportJSON(playSurface, fileName);
        std::cout << "Exported playsurface: " << fileName << std::endl;
    } else if (extension == ".BIN_LEVEL") {
        std::string fileName = (exportPath / (origFilename + ".json")).string();

        int dictEntryNo = dfs.findEntry(dfs.getBaseFilename(entryNo), ".LEV_DICT");
        if (dictEntryNo >= 0) {
            uint8_t* dictFileData = dfs.getFileData(dictEntryNo);
            int dictFileLen = dfs.getFileSize(dictEntryNo);

            uint8_t* fileData = dfs.getFileData(entryNo);
            int fileLen = dfs.getFileSize(entryNo);
            BinLevel binLevel;
            binLevel.readFile(fileData, fileLen, dictFileData, dictFileLen);
            exportJSON(binLevel, fileName);
            std::cout << "Exported bin level: " << fileName << std::endl;
        } else {
            std::cerr << "Warning: Could not find dictionary file for " << origFilename << ".LEV_DICT" << std::endl;
        }
    } else if (extension == ".TEMPLATES") {
        std::string fileName = (exportPath / (origFilename + ".json")).string();

        int dictEntryNo = dfs.findEntry(dfs.getBaseFilename(entryNo), ".TMPL_DCT");
        if (dictEntryNo >= 0) {
            uint8_t* dictFileData = dfs.getFileData(dictEntryNo);
            int dictFileLen = dfs.getFileSize(dictEntryNo);

            uint8_t* fileData = dfs.getFileData(entryNo);
            int fileLen = dfs.getFileSize(entryNo);
            LevelTemplate levelTemplate;
            levelTemplate.readFile(fileData, fileLen, dictFileData, dictFileLen);
            exportJSON(levelTemplate, fileName);
            std::cout << "Exported templates: " << fileName << std::endl;
        } else {
            std::cerr << "Warning: Could not find dictionary file for " << origFilename << ".TMPL_DCT" << std::endl;
        }
    } else {
        // For other files, just extract raw data
        std::string fileName = (exportPath / (origFilename + extension)).string();
        uint8_t* fileData = dfs.getFileData(entryNo);
        int fileLen = dfs.getFileSize(entryNo);

        std::ofstream outFile(fileName, std::ios::binary);
        if (outFile.is_open()) {
            outFile.write((const char*)fileData, fileLen);
            std::cout << "Extracted raw file: " << fileName << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        showUsage();
        return 1;
    }

    bool listMode = false;
    bool exportMode = false;
    bool exportSingleMode = false;
    bool exportByTypeMode = false;
    bool embedTextures = true;
    std::string exportDir;
    std::string exportFileName;
    std::string exportFileType;
    std::string dfsFile;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-l") {
            listMode = true;
        } else if (arg == "-x") {
            exportMode = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                exportDir = argv[++i];
            }
        } else if (arg == "-e") {
            exportSingleMode = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                exportFileName = argv[++i];
            } else {
                std::cerr << "Error: -e option requires a filename" << std::endl;
                showUsage();
                return 1;
            }
        } else if (arg == "-t") {
            exportByTypeMode = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                exportFileType = argv[++i];
                // Convert to uppercase for consistency
                std::transform(exportFileType.begin(), exportFileType.end(), exportFileType.begin(), ::toupper);

                // Check if next argument is a directory (optional)
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    exportDir = argv[++i];
                }
            } else {
                std::cerr << "Error: -t option requires a file type" << std::endl;
                showUsage();
                return 1;
            }
        } else if (arg == "--external-tex") {
            embedTextures = false;
        } else if (arg[0] != '-') {
            dfsFile = arg;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            showUsage();
            return 1;
        }
    }

    if (dfsFile.empty()) {
        std::cerr << "Error: Please provide a DFS file path" << std::endl;
        showUsage();
        return 1;
    }
    
    
    std::cout << "Opening DFS file: " << dfsFile << std::endl;

    // Create DFSFile instance and read the archive
    DFSFile dfs(0);
    bool headerOnly = !(exportMode || exportSingleMode || exportByTypeMode); // Read full data if exporting
    if (!dfs.read(dfsFile, headerOnly)) {
        std::cerr << "Error: Failed to read DFS file: " << dfsFile << std::endl;
        return 1;
    }
    
    int fileCount = dfs.numFiles();

    if (listMode) {
        // List mode: just show file list
        std::cout << "\n=== Files in Archive ===" << std::endl;
        std::cout << "Total files: " << fileCount << std::endl;
        std::cout << std::endl;

        if (fileCount > 0) {
            std::cout << std::setw(6) << "Index" << " | "
                      << std::setw(25) << "Name" << " | "
                      << std::setw(12) << "Extension" << " | "
                      << std::setw(10) << "Size (bytes)" << std::endl;
            std::cout << std::string(70, '-') << std::endl;

            for (int i = 0; i < fileCount; ++i) {
                std::string baseName = dfs.getBaseFilename(i);
                std::string extension = dfs.getFileExtension(i);
                int fileSize = dfs.getFileSize(i);

                std::cout << std::setw(6) << i << " | "
                          << std::setw(25) << baseName << " | "
                          << std::setw(12) << extension << " | "
                          << std::setw(10) << fileSize << std::endl;
            }
        } else {
            std::cout << "No files found in the archive." << std::endl;
        }
    } else if (exportMode) {
        // Export mode: export all files
        std::cout << "\n=== Exporting All Files ===" << std::endl;
        if (!exportDir.empty()) {
            std::cout << "Export directory: " << exportDir << std::endl;
        } else {
            std::cout << "Export directory: current directory" << std::endl;
        }
        std::cout << "Total files to export: " << fileCount << std::endl;
        std::cout << std::endl;

        for (int i = 0; i < fileCount; ++i) {
            std::cout << "[" << (i + 1) << "/" << fileCount << "] ";
            exportFile(dfs, i, exportDir, embedTextures);
        }

        std::cout << "\nExport completed!" << std::endl;
    } else if (exportSingleMode) {
        // Export single file mode
        std::cout << "\n=== Exporting Single File ===" << std::endl;
        std::cout << "Looking for file: " << exportFileName << std::endl;

        int targetEntryNo = -1;
        std::string searchName = exportFileName;

        // Convert search name to lowercase for case-insensitive comparison
        std::transform(searchName.begin(), searchName.end(), searchName.begin(), ::tolower);

        for (int i = 0; i < fileCount; ++i) {
            std::string baseName = dfs.getBaseFilename(i);
            std::string extension = dfs.getFileExtension(i);
            std::string fullName = baseName + extension;

            // Convert to lowercase for comparison
            std::string lowerBaseName = baseName;
            std::string lowerFullName = fullName;
            std::transform(lowerBaseName.begin(), lowerBaseName.end(), lowerBaseName.begin(), ::tolower);
            std::transform(lowerFullName.begin(), lowerFullName.end(), lowerFullName.begin(), ::tolower);

            // Match either base name or full name (with extension)
            if (lowerBaseName == searchName || lowerFullName == searchName) {
                targetEntryNo = i;
                break;
            }
        }

        if (targetEntryNo >= 0) {
            std::cout << "Found file at index " << targetEntryNo << std::endl;
            exportFile(dfs, targetEntryNo, "", embedTextures);
            std::cout << "\nExport completed!" << std::endl;
        } else {
            std::cerr << "Error: File '" << exportFileName << "' not found in archive" << std::endl;
            std::cout << "\nAvailable files:" << std::endl;
            for (int i = 0; i < fileCount; ++i) {
                std::cout << "  " << dfs.getBaseFilename(i) << dfs.getFileExtension(i) << std::endl;
            }
            return 1;
        }
    } else if (exportByTypeMode) {
        // Export by type mode
        std::cout << "\n=== Exporting Files by Type ===" << std::endl;
        std::cout << "File type: " << exportFileType << std::endl;
        if (!exportDir.empty()) {
            std::cout << "Export directory: " << exportDir << std::endl;
        } else {
            std::cout << "Export directory: current directory" << std::endl;
        }

        // Find all files of the specified type
        std::vector<int> matchingFiles;
        for (int i = 0; i < fileCount; ++i) {
            std::string extension = dfs.getFileExtension(i);

            // Remove the leading dot and convert to uppercase for comparison
            std::string fileType = extension;
            if (!fileType.empty() && fileType[0] == '.') {
                fileType = fileType.substr(1);
            }
            std::transform(fileType.begin(), fileType.end(), fileType.begin(), ::toupper);

            if (fileType == exportFileType) {
                matchingFiles.push_back(i);
            }
        }

        if (matchingFiles.empty()) {
            std::cerr << "Error: No files found with type '" << exportFileType << "'" << std::endl;
            std::cout << "\nAvailable file types:" << std::endl;

            // Collect and display available types
            std::set<std::string> availableTypes;
            for (int i = 0; i < fileCount; ++i) {
                std::string extension = dfs.getFileExtension(i);
                if (!extension.empty() && extension[0] == '.') {
                    std::string fileType = extension.substr(1);
                    std::transform(fileType.begin(), fileType.end(), fileType.begin(), ::toupper);
                    availableTypes.insert(fileType);
                }
            }

            for (const auto& type : availableTypes) {
                std::cout << "  " << type << std::endl;
            }
            return 1;
        }

        std::cout << "Found " << matchingFiles.size() << " files of type " << exportFileType << std::endl;
        std::cout << std::endl;

        for (size_t i = 0; i < matchingFiles.size(); ++i) {
            int entryNo = matchingFiles[i];
            std::cout << "[" << (i + 1) << "/" << matchingFiles.size() << "] ";
            exportFile(dfs, entryNo, exportDir, embedTextures);
        }

        std::cout << "\nExport completed! Exported " << matchingFiles.size() << " files." << std::endl;
    } else {
        // Default mode: show header and file list
        std::cout << "\n=== DFS Archive Information ===" << std::endl;
        dfs.logHeader();

        std::cout << "\n=== Files in Archive ===" << std::endl;
        std::cout << "Total files: " << fileCount << std::endl;
        std::cout << std::endl;

        if (fileCount > 0) {
            std::cout << std::setw(6) << "Index" << " | "
                      << std::setw(25) << "Name" << " | "
                      << std::setw(12) << "Extension" << " | "
                      << std::setw(10) << "Size (bytes)" << std::endl;
            std::cout << std::string(70, '-') << std::endl;

            for (int i = 0; i < fileCount; ++i) {
                std::string baseName = dfs.getBaseFilename(i);
                std::string extension = dfs.getFileExtension(i);
                int fileSize = dfs.getFileSize(i);

                std::cout << std::setw(6) << i << " | "
                          << std::setw(25) << baseName << " | "
                          << std::setw(12) << extension << " | "
                          << std::setw(10) << fileSize << std::endl;
            }
        } else {
            std::cout << "No files found in the archive." << std::endl;
        }

        std::cout << "\nUse -l to list files only, or -x [dir] to export all files." << std::endl;
    }
    
    return 0;
}

