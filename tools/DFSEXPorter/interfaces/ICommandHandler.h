#pragma once

#include <string>
#include <vector>

class DFSFile;

class ICommandHandler
{
public:
    virtual ~ICommandHandler() = default;
    virtual bool canHandle(const std::vector<std::string>& args) const = 0;
    virtual int execute(DFSFile& dfs, const std::vector<std::string>& args) = 0;
    virtual std::string getUsageDescription() const = 0;
};