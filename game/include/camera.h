#ifndef CAMERA_H
#define CAMERA_H

#include "globals.h"

class Camera {
public:
    // Camera attributes
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;

    // Constructor
    Camera();
    Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch);
    
    // Returns the view matrix
    glm::mat4 getViewMatrix();

    glm::vec3 getPosition() const { return position; }

    void setPositionAndOrientation(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

    // Processes input received from a mouse input system
    void processMouseMovement(float xoffset, float yoffset);
    void moveCamera(int direction);

    glm::mat4 getProjectionMatrix();

private:
    void updateCameraVectors();
};

extern Camera camera;

#endif