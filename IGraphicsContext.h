//
// Created by kprie on 15.03.2024.
//

#ifndef IGRAPHICSCONTEXT_H
#define IGRAPHICSCONTEXT_H

class IGraphicsContext {
public:
    virtual ~IGraphicsContext() = default;

    virtual void run() = 0;
    // Add more functions as needed, such as cleanup, setting up scenes, etc.
};

#endif //IGRAPHICSCONTEXT_H
