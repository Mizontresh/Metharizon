#include "Camera.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera() 
    : position(0.0f, 0.0f, -3.0f) // Initial position
    , rotation(1.0f, 0.0f, 0.0f, 0.0f) // Identity quaternion
    , moveForwardPressed(false)
    , moveBackwardPressed(false)
    , moveLeftPressed(false)
    , moveRightPressed(false)
    , moveUpPressed(false)
    , moveDownPressed(false)
    , lastMousePos(0.0f, 0.0f)
    , moveSpeed(5.0f)
    , mouseSensitivity(0.002f) {
}

void Camera::moveForward(float distance) {
    position += getForward() * distance;
}

void Camera::moveRight(float distance) {
    position += getRight() * distance;
}

void Camera::moveUp(float distance) {
    position += getUp() * distance;
}

void Camera::rotateYaw(float angle) {
    glm::quat yawRotation = glm::angleAxis(angle, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation = yawRotation * rotation;
}

void Camera::rotatePitch(float angle) {
    glm::quat pitchRotation = glm::angleAxis(angle, getRight());
    rotation = pitchRotation * rotation;
}

glm::vec3 Camera::getForward() const {
    return glm::rotate(rotation, glm::vec3(0.0f, 0.0f, 1.0f)); // Forward is positive Z
}

glm::vec3 Camera::getRight() const {
    return glm::rotate(rotation, glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 Camera::getUp() const {
    return glm::rotate(rotation, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::getViewMatrix() const {
    // Create view matrix from camera rotation and position
    glm::mat4 view = glm::mat4_cast(rotation);
    view = glm::translate(glm::mat4(1.0f), -position) * view;
    return view;
}

void Camera::update(float deltaTime) {
    float moveDistance = moveSpeed * deltaTime;
    
    // Get the camera's orientation vectors
    glm::vec3 forward = getForward();
    glm::vec3 right = getRight();
    glm::vec3 up = getUp();
    
    // Movement relative to where we're looking
    if (moveForwardPressed) {
        position += forward * moveDistance; // W moves forward (towards where we're looking)
    }
    if (moveBackwardPressed) {
        position -= forward * moveDistance; // S moves backward (away from where we're looking)
    }
    if (moveLeftPressed) {
        position -= right * moveDistance; // A moves left (relative to where we're looking)
    }
    if (moveRightPressed) {
        position += right * moveDistance; // D moves right (relative to where we're looking)
    }
    if (moveUpPressed) {
        position += up * moveDistance; // Space moves up (relative to where we're looking)
    }
    if (moveDownPressed) {
        position -= up * moveDistance; // Shift moves down (relative to where we're looking)
    }
} 