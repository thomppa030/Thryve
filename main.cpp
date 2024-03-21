#include <iostream>

#include "ThryveApplication.h"
#include "Core/App.h"

int main() {

    auto* _coreApp = new Thryve::Core::App();

    try {
        _coreApp->Run();

    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::out_of_range& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    delete _coreApp;

    return EXIT_SUCCESS;
}
