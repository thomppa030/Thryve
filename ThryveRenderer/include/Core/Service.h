//
// Created by thomppa on 3/23/24.
//

#ifndef SERVICE_H
#define SERVICE_H
#include "Configuration.h"

struct ServiceConfiguration : Configuration {
    //TODO Should be properly done
};

namespace Thryve::Core{
    struct Service {
        virtual ~Service() = default;

        virtual void Init(const ServiceConfiguration* configuration){}
        virtual void ShutDown(){}
    };
}

#endif //SERVICE_H
