#include "ListCommandHandler.h"
#include "../../a51lib/DFSFile.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

bool ListCommandHandler::canHandle(const std::vector<std::string>& args) const
{
    return std::find(args.begin(), args.end(), "-l") != args.end();
}

int ListCommandHandler::execute(DFSFile& dfs, const std::vector<std::string>& args)
{
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

    return 0;
}

std::string ListCommandHandler::getUsageDescription() const
{
    return "  -l              List all files in the DFS archive";
}