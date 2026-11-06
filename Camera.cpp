#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        // calculam directia fata folosind pozitia camerei si punctul catre care priveste
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        // calculam directia dreapta folosind produsul vectorial dintre directia fata si directia sus
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        
        // initializam yaw si pitch 
        this->cameraYaw = glm::degrees(atan2(this->cameraFrontDirection.z, this->cameraFrontDirection.x));
        this->cameraPitch = glm::degrees(asin(this->cameraFrontDirection.y));
    }

    // return the camera position for specular reflexions
    glm::vec3 Camera::getPosition() {
        return this->cameraPosition;
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        // returnam matricea view folosind pozitia camerei, punctul catre care priveste si vectorul sus
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, this->cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        // actualizare pozitie camera in functie de directia fata si directia dreapta
        switch (direction) {
            case MOVE_FORWARD:
                this->cameraPosition += this->cameraFrontDirection * speed;
                break;
            case MOVE_BACKWARD:
                this->cameraPosition -= this->cameraFrontDirection * speed;
                break;
            case MOVE_RIGHT:
                this->cameraPosition += this->cameraRightDirection * speed;
                break;
            case MOVE_LEFT:
                this->cameraPosition -= this->cameraRightDirection * speed;
                break;
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        
        this->cameraPitch += pitch;
        this->cameraYaw += yaw;

        // protectie pentru flip la camera
        if (this->cameraPitch > 89.0f) {
            this->cameraPitch = 89.0f;  
        }
        if (this->cameraPitch < -89.0f) {
            this->cameraPitch = -89.0f;
        }

        // calculam noua directie fata in functie de pitch si yaw
        glm::vec3 newFrontDirection;
        newFrontDirection.x = cos(glm::radians(this->cameraYaw)) * cos(glm::radians(this->cameraPitch));
        newFrontDirection.y = sin(glm::radians(this->cameraPitch));
        newFrontDirection.z = sin(glm::radians(this->cameraYaw)) * cos(glm::radians(this->cameraPitch));
        
        // actualizam vectorii camerei in functie de noua directie fata
        this->cameraFrontDirection = glm::normalize(newFrontDirection);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        this->cameraUpDirection = glm::normalize(glm::cross(this->cameraRightDirection, this->cameraFrontDirection));
    }

    void Camera::setPosition(glm::vec3 position) {
        this->cameraPosition = position;
    }

    void Camera::setTarget(glm::vec3 target) {
        this->cameraTarget = target;    // acutualizam parametrii camerei dupa actualizare tinta
        this->cameraUpDirection = glm::normalize(glm::cross(this->cameraRightDirection, this->cameraFrontDirection));
        this->cameraFrontDirection = glm::normalize(target - this->cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    glm::vec3 Camera::getTarget() {
        return this->cameraTarget;
    }
}
