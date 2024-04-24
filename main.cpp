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

    auto _loggingService = Thryve::Core::ServiceRegistry::RegisterService<Thryve::Core::DevelopmentLogger>("Debug");
    _loggingService->Init(&_devLogConfig);

    auto _validationLoggerService = Thryve::Core::ServiceRegistry::RegisterService<Thryve::Core::ValidationLayerLogger>("Validation");
    _validationLoggerService->Init(&_valLogConfig);

    auto _profilingService = Thryve::Core::ServiceRegistry::RegisterService<Thryve::Core::ProfilingService>();
    _profilingService->Init(nullptr);

    auto* _coreApp = new Thryve::Core::App();

    try {
        // TODO EEEEWWWWWW Really needs to be changed, is done right now to Also render UI.
        _coreApp->GetWindow()->GetRenderContext()->Run();
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
