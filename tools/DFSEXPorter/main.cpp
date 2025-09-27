#include <iostream>
#include <string>
#include <memory>

#include "core/DFSExporterApplication.h"

int main(int argc, char* argv[])
{
    try {
        DFSExporterApplication app;
        return app.run(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return 1;
    }
}