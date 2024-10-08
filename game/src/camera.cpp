#include "camera.h"

// Constructor
Camera::Camera() {
    position = glm::vec3(0.0f, 0.0f, 0.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    yaw = -90.0f;
    pitch = 0.0f;
    front = glm::vec3(0.0f, 0.0f, -1.0f);
    movementSpeed = 2.5f;
    mouseSensitivity = 0.1f;
}

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch)
    : front(glm::vec3(0.0f, 0.0f, -1.0f)), 
      movementSpeed(2.5f), 
      mouseSensitivity(0.1f) {
    position = startPosition;
    up = startUp;
    yaw = startYaw;
    pitch = startPitch;
    updateCameraVectors();
}

// Returns the view matrix
glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(position, position + front, up);
}

void Camera::setPositionAndOrientation(glm::vec3 position, glm::vec3 up, float yaw, float pitch) {
    this->position = position;
    this->up = up;
    this->yaw = yaw;
    this->pitch = pitch;
}

// Processes input received from a mouse input system
void Camera::processMouseMovement(float xoffset, float yoffset) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Clamp the pitch
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    updateCameraVectors();
}

void Camera::moveCamera(int direction) {
    float velocity = movementSpeed / 100; // You can also use deltaTime for frame-rate independent movement
    if (direction == 0) { // Forward
        position += front * velocity;
    }
    if (direction == 1) { // Backward
        position -= front * velocity;
    }
    if (direction == 2) { // Left
        position -= glm::normalize(glm::cross(front, up)) * velocity;
    }
    if (direction == 3) { // Right
        position += glm::normalize(glm::cross(front, up)) * velocity;
    }
    if (direction == 4) { // Up (W key)
        position += up * velocity;
    }
    if (direction == 5) { // Down (S key)
        position -= up * velocity;
    }
}

// Updates the camera vectors based on the current yaw and pitch
void Camera::updateCameraVectors() {
    glm::vec3 frontVector;
    frontVector.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    frontVector.y = sin(glm::radians(pitch));
    frontVector.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(frontVector);
    up = glm::normalize(glm::cross(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)), front));
}

glm::mat4 Camera::getProjectionMatrix() {
    return glm::perspective(glm::radians(45.0f), WIDTH / HEIGHT, 0.1f, 100.0f);
}