#pragma once

#include "../interfaces/ICommandHandler.h"
#include <memory>
#include <vector>
#include <string>

class ExportManager;

class DFSExporterApplication
{
private:
    std::shared_ptr<ExportManager> exportManager;
    std::vector<std::unique_ptr<ICommandHandler>> commandHandlers;

    void initializeCommandHandlers();
    void showUsage() const;
    void showDFSInfo(class DFSFile& dfs) const;

public:
    DFSExporterApplication();
    ~DFSExporterApplication() = default;

    int run(int argc, char* argv[]);
};