#ifndef CAMERA_H
#define CAMERA_H

#include "globals.h"

/**
 * @class Camera
 * @brief Represents a camera in 3D scene.
 */
class Camera {
public:
    glm::vec3 position; // Position of the camera
    glm::vec3 front; // Front vector of the camera
    glm::vec3 up; // Up vector of the camera
    float yaw; // Yaw of the camera
    float pitch; // Pitch of the camera
    float movementSpeed; // Speed of the camera
    float mouseSensitivity; // Sensitivity of the camera

    /**
     * @brief Default constructor.
     */
    Camera();

    /**
     * @brief Constructor with parameters.
     * @param startPosition The starting position of the camera.
     * @param startUp The starting up vector of the camera.
     * @param startYaw The starting yaw of the camera.
     * @param startPitch The starting pitch of the camera.
     */
    Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch);
    
    /**
     * @brief Returns the view matrix.
     * @return The view matrix.
     */
    glm::mat4 getViewMatrix();

    /**
     * @brief Returns the position of the camera.
     * @return The position of the camera.
     */
    glm::vec3 getPosition() const { return position; }

    /**
     * @brief Sets the position and orientation of the camera.
     * @param position The new position of the camera.
     * @param up The new up vector of the camera.
     * @param yaw The new yaw of the camera.
     * @param pitch The new pitch of the camera.
     */
    void setPositionAndOrientation(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

    /**
     * @brief Processes mouse movement.
     * @param xoffset The x offset of the mouse.
     * @param yoffset The y offset of the mouse.
     */
    void processMouseMovement(float xoffset, float yoffset);

    /**
     * @brief Moves the camera in a direction.
     * @param direction The direction to move the camera.
     */
    void moveCamera(int direction);

    /**
     * @brief Returns the projection matrix.
     * @return The projection matrix.
     */
    glm::mat4 getProjectionMatrix();

private:
    /**
     * @brief Updates the camera vectors.
     */
    void updateCameraVectors();
};

extern Camera camera;

#endif