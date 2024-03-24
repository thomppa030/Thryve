//
// Created by thomppa on 3/23/24.
//

#ifndef SERVICE_H
#define SERVICE_H
#include "Configuration.h"
#include "Ref.h"

struct ServiceConfiguration : Configuration {
    //TODO Should be properly done
};

namespace Thryve::Core{
    class IService : public ReferenceCounted {
    public:
        IService() = default;
        ~IService() override = default;

        virtual void Init(ServiceConfiguration* configuration){}
        virtual void ShutDown(){}
    };
}

#endif //SERVICE_H
