//
// Created by kprie on 20.03.2024.
//

#ifndef RENDERCONTEXT_H
#define RENDERCONTEXT_H
#include "Core/Ref.h"


namespace Thryve::Rendering {
	class RenderContext : public Core::ReferenceCounted{
	public:
		RenderContext() = default;
		~RenderContext() override = default;

		virtual void Init() = 0;
		virtual void Run() = 0;

		static Core::SharedRef<RenderContext> Create();
	};

}



#endif //RENDERCONTEXT_H
