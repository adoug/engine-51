#include <iostream>
#include <string>
#include <iomanip>
#include "gltfExporter.h"
#include "../../a51lib/DFSFile.h"

int main(int argc, char *argv[])
{
    std::cout << "DFS Exporter - Console Export Tool" << std::endl;
    std::cout << "Usage: DFSEXPorter <dfs_file>" << std::endl;
    
    if (argc != 2) {
        std::cerr << "Error: Please provide a DFS file path" << std::endl;
        std::cerr << "Usage: DFSEXPorter <dfs_file>" << std::endl;
        return 1;
    }
    
    std::string dfsFile = argv[1];
    
    std::cout << "Opening DFS file: " << dfsFile << std::endl;
    
    // Create DFSFile instance and read the archive
    DFSFile dfs(0);
    if (!dfs.read(dfsFile, true)) {
        std::cerr << "Error: Failed to read DFS file: " << dfsFile << std::endl;
        return 1;
    }
    
    // Display DFS file information
    std::cout << "\n=== DFS Archive Information ===" << std::endl;
    dfs.logHeader();
    
    // List all files in the archive
    int fileCount = dfs.numFiles();
    std::cout << "\n=== Files in Archive ===" << std::endl;
    std::cout << "Total files: " << fileCount << std::endl;
    std::cout << std::endl;
    
    if (fileCount > 0) {
        std::cout << std::setw(6) << "Index" << " | " 
                  << std::setw(20) << "Name" << " | " 
                  << std::setw(10) << "Extension" << " | " 
                  << std::setw(10) << "Size (bytes)" << std::endl;
        std::cout << std::string(60, '-') << std::endl;
        
        for (int i = 0; i < fileCount; ++i) {
            std::string baseName = dfs.getBaseFilename(i);
            std::string extension = dfs.getFileExtension(i);
            int fileSize = dfs.getFileSize(i);
            
            std::cout << std::setw(6) << i << " | " 
                      << std::setw(20) << baseName << " | " 
                      << std::setw(10) << extension << " | " 
                      << std::setw(10) << fileSize << std::endl;
        }
    } else {
        std::cout << "No files found in the archive." << std::endl;
    }
    
    std::cout << "\nExport functionality to be implemented..." << std::endl;
    
    return 0;
}
