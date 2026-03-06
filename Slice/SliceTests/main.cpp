#include <GL/glew.h>
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "Starting SliceTests..." << std::endl;
    
    Catch::Session session;
    
    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0) {
        return returnCode;
    }
    
    std::cout << "Running tests..." << std::endl;
    return session.run();
}
