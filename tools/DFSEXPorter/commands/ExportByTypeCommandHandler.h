#pragma once

#include "../interfaces/ICommandHandler.h"
#include <memory>

class ExportManager;

class ExportByTypeCommandHandler : public ICommandHandler
{
private:
    std::shared_ptr<ExportManager> exportManager;

public:
    explicit ExportByTypeCommandHandler(std::shared_ptr<ExportManager> manager);
    bool canHandle(const std::vector<std::string>& args) const override;
    int execute(DFSFile& dfs, const std::vector<std::string>& args) override;
    std::string getUsageDescription() const override;

private:
    std::string parseFileType(const std::vector<std::string>& args) const;
    std::string parseExportDirectory(const std::vector<std::string>& args) const;
    bool parseEmbedTextures(const std::vector<std::string>& args) const;
};