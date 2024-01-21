#ifndef __CAR__
#define __CAR__

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <queue>
enum D
{
    CAR_FORWARD,
    CAR_BACKWARD,
    CAR_LEFT,
    CAR_RIGHT,
    CAR_NO
};
class Car
{
public:
    glm::vec3 Position;
    glm::vec3 Front;
    float Yaw;
    float MovementSpeed;
    float RoundSpeed;
    float a;

    Car(glm::vec3 position, glm::vec3 front) : MovementSpeed(0.0f), RoundSpeed(90.0f)
    {
        Position = position;
        Front = front;
        if (front.x != 0)
        {
            Yaw = glm::degrees(glm::atan(front.z / front.x));
            if (front.x < 0)
            {
                Yaw += 180.0f;
            }
        }
        else
        {
            if (front.z > 0)
            {
                Yaw = 90.0f;
            }
            else if (front.z < 0)
            {
                Yaw = 270.0f;
            }
            else
            {
                Yaw = 0.0f;
            }
        }
        // updateFront();
    }

    void ProcessKeyboard(D direction, float deltaTime, glm::vec3 Camera_Front)
    {
        if (direction == CAR_FORWARD)
        {
            if (MovementSpeed <= 10.0f && MovementSpeed >= 0)
            {
                a = 50.0f;
            }
            else
            {
                a = 500.0f / (MovementSpeed);
                if(MovementSpeed<=0)
                {
                    a = 50.0f;
                }
                if (a < 0)
                    a = -a;
            }
            glm::vec3 temp = glm::vec3(Camera_Front.x, 0.0f, Camera_Front.z);
            Position += glm::normalize(temp) * (MovementSpeed * deltaTime + 0.5f * a * deltaTime * deltaTime);
            MovementSpeed = MovementSpeed + a * deltaTime;
        }
        if (direction == CAR_BACKWARD)
        {
            if (MovementSpeed >= -10.0f && MovementSpeed <=0)
            {
                a = -30.0f;
            }
            else
            {
                a = 300.0f / (MovementSpeed);
                if(MovementSpeed >= 0)
                {
                    a = -50.0f;
                }
                if (a > 0)
                    a = -a;
            }
            glm::vec3 temp = glm::vec3(Camera_Front.x, 0.0f, Camera_Front.z);
            Position += glm::normalize(temp) * (MovementSpeed * deltaTime + 0.5f * a * deltaTime * deltaTime);
            MovementSpeed = MovementSpeed + a * deltaTime;
        }
        if (direction == CAR_NO)
        {
            if (MovementSpeed > 0.0f)
            {
                a = -50.0f;
                glm::vec3 temp = glm::vec3(Camera_Front.x, 0.0f, Camera_Front.z);
                Position += glm::normalize(temp) * (MovementSpeed * deltaTime + 0.5f * a * deltaTime * deltaTime);
                MovementSpeed = MovementSpeed + a * deltaTime;
                if (MovementSpeed < 0.0f)
                    MovementSpeed = 0.0f;
            }
            else if (MovementSpeed < 0.0f)
            {
                a = 50.0f;
                glm::vec3 temp = glm::vec3(Camera_Front.x, 0.0f, Camera_Front.z);
                Position += glm::normalize(temp) * (MovementSpeed * deltaTime + 0.5f * a * deltaTime * deltaTime);
                MovementSpeed = MovementSpeed + a * deltaTime;
                if (MovementSpeed > 0)
                    MovementSpeed = 0.0f;
            }
        }
        if (direction == CAR_LEFT)
        {
            if (MovementSpeed != 0.0f)
            {
                Yaw -= RoundSpeed * deltaTime;
                if (Yaw >= 360.0f)
                {
                    // Yaw -= 360.0f;
                }
            }
        }
        if (direction == CAR_RIGHT)
        {
            if (MovementSpeed != 0.0f)
            {
                Yaw += RoundSpeed * deltaTime;
                if (Yaw >= 360.0f)
                {
                    // Yaw -= 360.0f;
                }
            }
        }
        updateFront();
    }
    glm::vec3 getPosition()
    {
        return Position;
    }
    float getYaw()
    {
        return Yaw;
    }
    glm::vec3 getFront()
    {
        return Front;
    }

    void updateFront()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw));
        front.y = 0.0f;
        front.z = sin(glm::radians(Yaw));
        Front = glm::normalize(front);
    }
};

#endif