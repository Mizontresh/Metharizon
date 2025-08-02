#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Camera {
public:
    Camera();
    
    // Movement
    void moveForward(float distance);
    void moveRight(float distance);
    void moveUp(float distance);
    
    // Rotation
    void rotateYaw(float angle);
    void rotatePitch(float angle);
    
    // Getters
    glm::vec3 getPosition() const { return position; }
    glm::quat getRotation() const { return rotation; }
    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    glm::vec3 getUp() const;
    
    // View matrix
    glm::mat4 getViewMatrix() const;
    
    // Update
    void update(float deltaTime);
    
    // Public state for input handling (Made public)
    bool moveForwardPressed;
    bool moveBackwardPressed;
    bool moveLeftPressed;
    bool moveRightPressed;
    bool moveUpPressed;
    bool moveDownPressed;
    glm::vec2 lastMousePos;
    float moveSpeed;
    float mouseSensitivity;
    
private:
    glm::vec3 position;
    glm::quat rotation;
}; 