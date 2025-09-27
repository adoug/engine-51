#pragma once

#include "../interfaces/ICommandHandler.h"

class ListCommandHandler : public ICommandHandler
{
public:
    bool canHandle(const std::vector<std::string>& args) const override;
    int execute(DFSFile& dfs, const std::vector<std::string>& args) override;
    std::string getUsageDescription() const override;
};