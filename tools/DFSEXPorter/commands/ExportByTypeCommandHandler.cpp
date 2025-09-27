#include "ExportByTypeCommandHandler.h"
#include "../core/ExportManager.h"
#include "../../a51lib/DFSFile.h"
#include <iostream>
#include <algorithm>
#include <set>
#include <vector>

ExportByTypeCommandHandler::ExportByTypeCommandHandler(std::shared_ptr<ExportManager> manager)
    : exportManager(manager) {}

bool ExportByTypeCommandHandler::canHandle(const std::vector<std::string>& args) const
{
    return std::find(args.begin(), args.end(), "-t") != args.end();
}

int ExportByTypeCommandHandler::execute(DFSFile& dfs, const std::vector<std::string>& args)
{
    std::string fileType = parseFileType(args);
    std::string exportDir = parseExportDirectory(args);
    bool embedTextures = parseEmbedTextures(args);

    if (fileType.empty()) {
        std::cerr << "Error: -t option requires a file type" << std::endl;
        return 1;
    }

    std::cout << "\n=== Exporting Files by Type ===" << std::endl;
    std::cout << "File type: " << fileType << std::endl;
    if (!exportDir.empty()) {
        std::cout << "Export directory: " << exportDir << std::endl;
    } else {
        std::cout << "Export directory: current directory" << std::endl;
    }

    std::vector<int> matchingFiles;
    int fileCount = dfs.numFiles();

    for (int i = 0; i < fileCount; ++i) {
        std::string extension = dfs.getFileExtension(i);
        std::string currentFileType = extension;
        if (!currentFileType.empty() && currentFileType[0] == '.') {
            currentFileType = currentFileType.substr(1);
        }
        std::transform(currentFileType.begin(), currentFileType.end(), currentFileType.begin(), ::toupper);

        if (currentFileType == fileType) {
            matchingFiles.push_back(i);
        }
    }

    if (matchingFiles.empty()) {
        std::cerr << "Error: No files found with type '" << fileType << "'" << std::endl;
        std::cout << "\nAvailable file types:" << std::endl;

        std::set<std::string> availableTypes;
        for (int i = 0; i < fileCount; ++i) {
            std::string extension = dfs.getFileExtension(i);
            if (!extension.empty() && extension[0] == '.') {
                std::string type = extension.substr(1);
                std::transform(type.begin(), type.end(), type.begin(), ::toupper);
                availableTypes.insert(type);
            }
        }

        for (const auto& type : availableTypes) {
            std::cout << "  " << type << std::endl;
        }
        return 1;
    }

    std::cout << "Found " << matchingFiles.size() << " files to export." << std::endl;
    std::cout << std::endl;

    int successCount = 0;
    for (size_t i = 0; i < matchingFiles.size(); ++i) {
        int entryNo = matchingFiles[i];
        std::cout << "[" << (i + 1) << "/" << matchingFiles.size() << "] ";
        if (exportManager->exportFile(dfs, entryNo, exportDir, embedTextures)) {
            successCount++;
        }
    }

    std::cout << "\nExport completed! Successfully exported " << successCount << "/" << matchingFiles.size() << " files." << std::endl;
    return successCount == matchingFiles.size() ? 0 : 1;
}

std::string ExportByTypeCommandHandler::getUsageDescription() const
{
    return "  -t <type> [dir] Export all files of specific type (e.g., RIGIDGEOM, SKINGEOM, XBMP)";
}

std::string ExportByTypeCommandHandler::parseFileType(const std::vector<std::string>& args) const
{
    auto it = std::find(args.begin(), args.end(), "-t");
    if (it != args.end() && (it + 1) != args.end()) {
        std::string fileType = *(it + 1);
        std::transform(fileType.begin(), fileType.end(), fileType.begin(), ::toupper);
        return fileType;
    }
    return "";
}

std::string ExportByTypeCommandHandler::parseExportDirectory(const std::vector<std::string>& args) const
{
    auto it = std::find(args.begin(), args.end(), "-t");
    if (it != args.end() && (it + 2) != args.end()) {
        std::string nextArg = *(it + 2);
        if (!nextArg.empty() && nextArg[0] != '-') {
            return nextArg;
        }
    }
    return "";
}

bool ExportByTypeCommandHandler::parseEmbedTextures(const std::vector<std::string>& args) const
{
    return std::find(args.begin(), args.end(), "--external-tex") == args.end();
}