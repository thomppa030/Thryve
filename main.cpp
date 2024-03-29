#include <iostream>

#include "Core/App.h"
#include "Core/Log.h"
#include "Core/ServiceRegistry.h"
#include "ThryveApplication.h"

int main() {

    Thryve::Core::DevelopmentLoggerConfiguration _devLogConfig = {};
    _devLogConfig.ConsoleOutputEnabled = true;

    Thryve::Core::ValidationLayerLoggerConfiguration _valLogConfig = {};
    _valLogConfig.ConsoleOutputEnabled = true;

    auto _loggingService = Thryve::Core::SharedRef<Thryve::Core::DevelopmentLogger>::Create("Core");
    _loggingService->Init(&_devLogConfig);

    auto _validationLoggerService = Thryve::Core::SharedRef<Thryve::Core::ValidationLayerLogger>::Create("Validation");
    _validationLoggerService->Init(&_valLogConfig);

    Thryve::Core::ServiceRegistry::RegisterService(_loggingService);
    Thryve::Core::ServiceRegistry::RegisterService(_validationLoggerService);

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
