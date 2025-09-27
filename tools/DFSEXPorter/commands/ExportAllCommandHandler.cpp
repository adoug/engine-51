#include "ExportAllCommandHandler.h"
#include "../core/ExportManager.h"
#include "../../a51lib/DFSFile.h"
#include <iostream>
#include <algorithm>

ExportAllCommandHandler::ExportAllCommandHandler(std::shared_ptr<ExportManager> manager)
    : exportManager(manager) {}

bool ExportAllCommandHandler::canHandle(const std::vector<std::string>& args) const
{
    return std::find(args.begin(), args.end(), "-x") != args.end();
}

int ExportAllCommandHandler::execute(DFSFile& dfs, const std::vector<std::string>& args)
{
    std::string exportDir = parseExportDirectory(args);
    bool embedTextures = parseEmbedTextures(args);
    int fileCount = dfs.numFiles();

    std::cout << "\n=== Exporting All Files ===" << std::endl;
    if (!exportDir.empty()) {
        std::cout << "Export directory: " << exportDir << std::endl;
    } else {
        std::cout << "Export directory: current directory" << std::endl;
    }
    std::cout << "Total files to export: " << fileCount << std::endl;
    std::cout << std::endl;

    int successCount = 0;
    for (int i = 0; i < fileCount; ++i) {
        std::cout << "[" << (i + 1) << "/" << fileCount << "] ";
        if (exportManager->exportFile(dfs, i, exportDir, embedTextures)) {
            successCount++;
        }
    }

    std::cout << "\nExport completed! Successfully exported " << successCount << "/" << fileCount << " files." << std::endl;
    return successCount == fileCount ? 0 : 1;
}

std::string ExportAllCommandHandler::getUsageDescription() const
{
    return "  -x [dir]        Export all files to directory (current dir if not specified)";
}

std::string ExportAllCommandHandler::parseExportDirectory(const std::vector<std::string>& args) const
{
    auto it = std::find(args.begin(), args.end(), "-x");
    if (it != args.end() && (it + 1) != args.end()) {
        std::string nextArg = *(it + 1);
        if (!nextArg.empty() && nextArg[0] != '-') {
            return nextArg;
        }
    }
    return "";
}

bool ExportAllCommandHandler::parseEmbedTextures(const std::vector<std::string>& args) const
{
    return std::find(args.begin(), args.end(), "--external-tex") == args.end();
}