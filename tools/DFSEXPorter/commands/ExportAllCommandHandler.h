#pragma once

#include "../interfaces/ICommandHandler.h"
#include <memory>

class ExportManager;
class IExportContext;

class ExportAllCommandHandler : public ICommandHandler
{
private:
    std::shared_ptr<ExportManager> exportManager;

public:
    explicit ExportAllCommandHandler(std::shared_ptr<ExportManager> manager);
    bool canHandle(const std::vector<std::string>& args) const override;
    int execute(DFSFile& dfs, const std::vector<std::string>& args) override;
    std::string getUsageDescription() const override;

private:
    std::string parseExportDirectory(const std::vector<std::string>& args) const;
    bool parseEmbedTextures(const std::vector<std::string>& args) const;
};