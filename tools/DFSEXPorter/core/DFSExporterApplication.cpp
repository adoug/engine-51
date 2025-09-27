#include "DFSExporterApplication.h"
#include "ExportManager.h"
#include "../commands/ListCommandHandler.h"
#include "../commands/ExportAllCommandHandler.h"
#include "../commands/ExportByTypeCommandHandler.h"
#include "../../a51lib/DFSFile.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

DFSExporterApplication::DFSExporterApplication()
    : exportManager(std::make_shared<ExportManager>())
{
    initializeCommandHandlers();
}

void DFSExporterApplication::initializeCommandHandlers()
{
    commandHandlers.push_back(std::make_unique<ListCommandHandler>());
    commandHandlers.push_back(std::make_unique<ExportAllCommandHandler>(exportManager));
    commandHandlers.push_back(std::make_unique<ExportByTypeCommandHandler>(exportManager));
}

int DFSExporterApplication::run(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Error: Please provide a DFS file path" << std::endl;
        showUsage();
        return 1;
    }

    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    std::string dfsFile;
    for (auto it = args.rbegin(); it != args.rend(); ++it) {
        if (!it->empty() && (*it)[0] != '-') {
            dfsFile = *it;
            break;
        }
    }

    if (dfsFile.empty()) {
        std::cerr << "Error: Please provide a DFS file path" << std::endl;
        showUsage();
        return 1;
    }

    std::cout << "Opening DFS file: " << dfsFile << std::endl;

    DFSFile dfs(0);
    bool hasCommands = false;
    for (const auto& handler : commandHandlers) {
        if (handler->canHandle(args)) {
            hasCommands = true;
            break;
        }
    }

    bool headerOnly = !hasCommands;
    if (!dfs.read(dfsFile, headerOnly)) {
        std::cerr << "Error: Failed to read DFS file: " << dfsFile << std::endl;
        return 1;
    }

    for (const auto& handler : commandHandlers) {
        if (handler->canHandle(args)) {
            return handler->execute(dfs, args);
        }
    }

    showDFSInfo(dfs);
    return 0;
}

void DFSExporterApplication::showUsage() const
{
    std::cout << "DFS Exporter - Console Export Tool" << std::endl;
    std::cout << "Usage: DFSEXPorter [options] <dfs_file>" << std::endl;
    std::cout << "Options:" << std::endl;

    for (const auto& handler : commandHandlers) {
        std::cout << handler->getUsageDescription() << std::endl;
    }

    std::cout << "  --external-tex  Use external PNG files for textures instead of embedding" << std::endl;
    std::cout << "  (no flags)      Show DFS header and file list" << std::endl;
}

void DFSExporterApplication::showDFSInfo(DFSFile& dfs) const
{
    std::cout << "\n=== DFS Archive Information ===" << std::endl;
    dfs.logHeader();

    int fileCount = dfs.numFiles();
    std::cout << "\n=== Files in Archive ===" << std::endl;
    std::cout << "Total files: " << fileCount << std::endl;
    std::cout << std::endl;

    if (fileCount > 0) {
        std::cout << std::setw(6) << "Index" << " | "
                  << std::setw(25) << "Name" << " | "
                  << std::setw(12) << "Extension" << " | "
                  << std::setw(10) << "Size" << std::endl;
        std::cout << std::string(60, '-') << std::endl;

        int displayCount = std::min(fileCount, 20);
        for (int i = 0; i < displayCount; ++i) {
            std::string baseName = dfs.getBaseFilename(i);
            std::string extension = dfs.getFileExtension(i);
            int fileSize = dfs.getFileSize(i);

            std::cout << std::setw(6) << i << " | "
                      << std::setw(25) << baseName << " | "
                      << std::setw(12) << extension << " | "
                      << std::setw(10) << fileSize << std::endl;
        }

        if (fileCount > 20) {
            std::cout << "... and " << (fileCount - 20) << " more files. Use -l to list all." << std::endl;
        }
    } else {
        std::cout << "No files found in the archive." << std::endl;
    }
}