//
// Created by kprie on 11.07.2024.
//

#include "../../include/Core/Camera.h"

Thryve::Core::Camera::Camera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& upVector,
                             float nearPlane, float farPlane, float aspectRatio) :
    m_Position{position}, m_Up{upVector}, m_Target{target}, m_NearPlane{nearPlane}, m_FarPlane{farPlane},
    m_Aspect{aspectRatio}
{
    m_ProjectionMatrix = glm::perspective<double>(glm::radians(m_Fov), m_Aspect, m_NearPlane, m_FarPlane);
}
std::array<glm::vec4, 6> Thryve::Core::Camera::CalculateFrustrumPlanes()
{
    std::array<glm::vec4, 6> _planes;

    const glm::vec3 _forward = glm::normalize(glm::cross(GetRightVector(), GetUpVector()));

    const float _tanHalfFovY = glm::tan(glm::radians(m_Fov)*.5f);

    const float _nearHalfHeight = m_NearPlane * _tanHalfFovY;
    const float _farHalfHeight = m_FarPlane * _tanHalfFovY;

    const float _nearHalfWidth = _nearHalfHeight * m_Aspect;
    const float _farHalfWidth = _farHalfHeight * m_Aspect;

    const glm::vec3 _nearCenter = GetPosition() + _forward * m_NearPlane;
    const glm::vec3 _farCenter = GetPosition() + _forward * m_FarPlane;

    const glm::vec3 _nearTopRight = _nearCenter + GetUpVector() * _nearHalfHeight + GetRightVector() * _nearHalfWidth;
    const glm::vec3 _nearBottomRight = _nearCenter - GetUpVector() * _nearHalfHeight + GetRightVector() * _nearHalfWidth;
    const glm::vec3 _nearTopLeft = _nearCenter + GetUpVector() * _nearHalfHeight - GetRightVector() * _nearHalfWidth;
    const glm::vec3 _nearBottomLeft = _nearCenter - GetUpVector() * _nearHalfHeight - GetRightVector() * _nearHalfWidth;

    const glm::vec3 _farTopRight = _farCenter + GetUpVector() * _farHalfHeight + GetRightVector() * _farHalfWidth;
    const glm::vec3 _farBottomRight = _farCenter - GetUpVector() * _farHalfHeight + GetRightVector() * _farHalfWidth;
    const glm::vec3 _farTopLeft = _farCenter + GetUpVector() * _farHalfHeight - GetRightVector() * _farHalfWidth;
    const glm::vec3 _farBottomLeft = _farCenter - GetUpVector() * _farHalfHeight - GetRightVector() * _farHalfWidth;

    auto _calculatePlaneNormal = [](const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2)
    {
      return glm::normalize(glm::cross(p1-p0, p2-p0));
    };

    const glm::vec3 _leftNormal = _calculatePlaneNormal(_farBottomLeft, _nearBottomLeft, _farTopLeft);
    _planes[0] = glm::vec4(_leftNormal, -glm::dot(_leftNormal, _farBottomLeft));

    // Bottom plane
    const glm::vec3 _bottomNormal = _calculatePlaneNormal(_farBottomRight, _nearBottomRight, _farBottomLeft);
    _planes[1] = glm::vec4(_bottomNormal, -glm::dot(_bottomNormal, _farBottomRight));

    // Right plane
    const glm::vec3 _rightNormal = _calculatePlaneNormal(_farTopRight, _nearTopRight, _farBottomRight);
    _planes[2] = glm::vec4(_rightNormal, -glm::dot(_rightNormal, _farTopRight));

    // Top plane
    const glm::vec3 _topNormal = _calculatePlaneNormal(_farTopLeft, _nearTopLeft, _farTopRight);
    _planes[3] = glm::vec4(_topNormal, -glm::dot(_topNormal, _farTopLeft));

    // Near plane
    const glm::vec3 _nearNormal = _calculatePlaneNormal(_nearTopRight, _nearTopLeft, _nearBottomRight);
    _planes[4] = glm::vec4(_nearNormal, -glm::dot(_nearNormal, _nearTopRight));

    // Far plane
    const glm::vec3 _farNormal = _calculatePlaneNormal(_farTopRight, _farBottomRight, _farTopLeft);
    _planes[5] = glm::vec4(_farNormal, -glm::dot(_farNormal, _farTopRight));

    return _planes;
}

void Thryve::Core::Camera::Move(const glm::vec3& direction, float increment)
{
    m_Position = m_Position + (direction * increment);
}

void Thryve::Core::Camera::SetPos(const glm::vec3& position)
{
    bIsUsed = true;
    m_Position = position;
}

void Thryve::Core::Camera::SetUpVector(const glm::vec3& up)
{
    bIsUsed = true;
    m_Up = glm::normalize(up);
}

void Thryve::Core::Camera::Rotate(const glm::vec2& delta, const double deltaT)
{
    const auto _deltaQuat = glm::quat(glm::vec3(-deltaT* delta.y, deltaT* delta.x, 0.0f));
    m_Orientation = _deltaQuat * m_Orientation;
    m_Orientation = glm::normalize(m_Orientation);
}

glm::mat4 Thryve::Core::Camera::GetProjectionMatrix() const
{
    return m_ProjectionMatrix;
}

glm::mat4 Thryve::Core::Camera::GetViewMatrix() const
{
    const glm::mat4 _t = glm::translate(glm::mat4(1.0f), -m_Position);
    const glm::mat4 _r = glm::mat4_cast(m_Orientation);

    return _t * _r;
}

glm::vec3 Thryve::Core::Camera::GetDirection() const
{
    const auto _view = glm::mat4_cast(m_Orientation);
    return {_view[0][2], _view[1][2], _view[2][2]};
}

glm::vec3 Thryve::Core::Camera::GetEulerAngles() const
{
    glm::vec3 _eulerAngles = glm::eulerAngles(m_Orientation);
    _eulerAngles = glm::degrees(_eulerAngles);
    return _eulerAngles;
}

void Thryve::Core::Camera::SetEulerAngles(const glm::vec3& dir)
{
    glm::vec3 _eulerAngles = glm::radians(dir);
    m_Orientation = glm::quat(_eulerAngles);
    bIsUsed = true;
}

glm::vec3 Thryve::Core::Camera::GetRightVector() const
{
    const auto view = glm::mat4_cast(m_Orientation);
    return {view[0][0], view[1][0], view[2][0]};
}

glm::vec3 Thryve::Core::Camera::GetUpVector() const
{
    return {glm::normalize(glm::cross(GetRightVector(), GetDirection()))};
}

glm::vec3 Thryve::Core::Camera::GetPosition() const
{
    return m_Position;
}
