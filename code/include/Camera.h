#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Car.h>
#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    NO
};

// Default camera values
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;
const float ZOOM_RANGE = 10.0f;
const float ZOOM_SPEED = 0.05f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    float R;
    float Zoom;

    Camera(glm::vec3 position = glm::vec3(0.0f, 50.0f, 50.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f)) : Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        glm::vec3 front = target - position;
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
        if (front.x != 0)
        {
            Yaw = glm::degrees(glm::atan(front.z / front.x));
            if (front.x < 0)
            {
                Yaw += 180.0f;
            }
            Pitch = glm::degrees(glm::atan(front.y / (glm::sqrt(front.x * front.x + front.z * front.z))));
        }
        else
        {
            if (front.z > 0)
            {
                Yaw = 90.0f;
                Pitch = glm::degrees(glm::atan(front.y / (glm::sqrt(front.x * front.x + front.z * front.z))));
            }
            else if (front.z < 0)
            {
                Yaw = 270.0f;
                Pitch = glm::degrees(glm::atan(front.y / (glm::sqrt(front.x * front.x + front.z * front.z))));
            }
            else
            {
                Yaw = 0.0f;
                if (front.y > 0)
                {
                    Pitch = 90.0f;
                }
                else if (front.y < 0)
                {
                    Pitch = -90.0f;
                }
                else
                {
                    Pitch = 0.0f;
                }
            }
        }
        R = glm::sqrt((position.x - target.x) * (position.x - target.x) + (position.z - target.z) * (position.z - target.z));
        // Pitch = glm::degrees(glm::atan(front.y/(glm::sqrt(front.x*front.x+front.z*front.z))));
        // updateCameraVectors();
    }
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        if (direction == FORWARD)
        {
            ZoomOut();
        }
        if (direction == BACKWARD)
        {
            ZoomIn();
        }
        if (direction == NO)
        {
            ZoomRecover();
        }
        if (direction == LEFT)
        {
        }
        if (direction == RIGHT)
        {
        }
    }

    glm::mat4 GetViewMatrix(Car car)
    {
        glm::vec3 Target = car.Position;
        float car_yaw = car.getYaw();
        glm::vec3 new_Position;
        if (car_yaw < Yaw)
        {
            if (Yaw - car_yaw <= 0.09f)
            {
                Yaw = car_yaw;
            }
            else
            {
                Yaw -= 0.09f;
            }
            updateCameraVectors();
        }
        else if (car_yaw > Yaw)
        {
            if (car_yaw - Yaw <= 0.09f)
            {
                Yaw = car_yaw;
            }
            else
            {
                Yaw += 0.09f;
            }
            updateCameraVectors();
        }
        glm::vec3 front_temp;
        front_temp.x = cos(glm::radians(Yaw));
        front_temp.y = 0.0f;
        front_temp.z = sin(glm::radians(Yaw));
        glm::vec3 Front_temp = glm::normalize(front_temp);
        new_Position = Target - R * Front_temp;
        new_Position.y = Position.y;
        Position = new_Position;

        return glm::lookAt(Position, Target, WorldUp);
    }
    void ZoomIn()
    {
        if (Zoom >= 40.0f)
            Zoom -= 0.05f;
    }

    void ZoomOut()
    {
        if (Zoom <= 60.0f)
            Zoom += 0.12f;
    }

    void ZoomRecover()
    {
        if (Zoom < ZOOM)
            Zoom += ZOOM_SPEED;
        if (Zoom > ZOOM)
            Zoom -= ZOOM_SPEED;
    }
    glm::vec3 getFront()
    {
        return Front;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        // std::cout<<front.x<<","<<front.y<<","<<front.z<<std::endl;
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp)); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
#endif