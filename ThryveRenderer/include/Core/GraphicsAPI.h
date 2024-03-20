//
// Created by kprie on 20.03.2024.
//

#ifndef GRAPHICSAPI_H
#define GRAPHICSAPI_H

namespace Thryve::Core {
	enum class GraphicsAPIType {
		NONE,
		VULKAN
	};

	class RenderAPI {
	public:
		virtual void Init() = 0;

		static GraphicsAPIType CurrentAPI() {return s_CurrentGraphicsAPI;}

	private:
		inline static GraphicsAPIType s_CurrentGraphicsAPI = GraphicsAPIType::VULKAN;
	};
}

#endif //GRAPHICSAPI_H
