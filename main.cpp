#include <iostream>

#include "ThryveApplication.h"
#include "Core/App.h"

int main() {

    try {
        auto* _coreApp = new Thryve::Core::App();
        const ThryveApplication _app;

        _app.Run();
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

    return EXIT_SUCCESS;
}
