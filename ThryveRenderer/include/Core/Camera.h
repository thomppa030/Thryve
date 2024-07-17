//
// Created by kprie on 11.07.2024.
//
#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/type_aligned.hpp>

struct UniformTransform {
    glm::aligned_mat4 Model;
    glm::aligned_mat4 View;
    glm::aligned_mat4 Projection;
    glm::aligned_mat4 PrevViewMat = glm::mat4(1.0f);
    glm::aligned_mat4 Jitter = glm::mat4(1.0f);
};

namespace Thryve::Core {
    class Camera final {
    public:
        explicit Camera(const glm::vec3& position = glm::vec3(-9.f, 2.f, 2.f),
                        const glm::vec3& target = glm::vec3(0.0f, 0.f, 0.f),
                        const glm::vec3& upVector = glm::vec3(0.0f, 1.0f, 0.0f), float nearPlane = .1f,
                        float farPlane = 4000.0f, float aspectRatio = 800.f / 600.f);

        std::array<glm::vec4, 6> CalculateFrustrumPlanes();

        void Move(const glm::vec3& direction, float increment);

        void SetPos(const glm::vec3& position);

        void SetUpVector(const glm::vec3& up);

        void Rotate(const glm::vec2& delta, double deltaT = 4.0f);

        [[nodiscard]] glm::mat4 GetProjectionMatrix() const;

        [[nodiscard]] glm::mat4 GetViewMatrix() const;

        [[nodiscard]] glm::vec3 GetDirection() const;

        [[nodiscard]] glm::vec3 GetEulerAngles() const;

        void SetEulerAngles(const glm::vec3& dir);

        [[nodiscard]] glm::vec3 GetRightVector() const;
        [[nodiscard]] glm::vec3 GetUpVector() const;
        [[nodiscard]] glm::vec3 GetPosition() const;

    private:
        glm::vec3 m_Position;
        glm::vec3 m_Up;
        glm::vec3 m_Target;
        glm::quat m_Orientation;
        glm::mat4 m_ProjectionMatrix{1.0f};
        float m_NearPlane = 10.f;
        float m_FarPlane = 4000.0f;
        float m_Fov = 60.0f;
        float m_Aspect = 800.0 / 600.0;

        bool bIsUsed{false};
    };
}