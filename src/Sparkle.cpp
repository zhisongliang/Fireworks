#include <iostream>
#include <math.h>
#include <glm/glm.hpp>

class Sparkle
{
private:
    float x;
    float y;
    float z;
    float vx;
    float vy;
    float life;
    float explosionRadius;
    int died; // died flag

public:
    static float random(float min, float max)
    {
        return ((float)rand() / RAND_MAX) * (max - min) + min;
    }

    Sparkle()
    {
        // according to rocket final position, set the sparkle position
        x = 0;
        y = 0;
        z = 0;
        float theta = random(0, 2 * M_PI);
        float v = random(0.0001, 0.03);
        vx = v * cos(theta);
        vy = v * sin(theta);
        life = random(0.3, 0.6);
        died = 0;
    }

    Sparkle(float x, float y, float z, float explosionRadius)
    {
        // according to rocket final position, set the sparkle position
        this->x = x;
        this->y = y;
        this->z = z;
        this->explosionRadius = explosionRadius;
        float theta = random(0, 2 * M_PI);
        float v = random(0.0001, 0.003 * explosionRadius);
        vx = v * cos(theta);
        vy = v * sin(theta);
        life = random(0.3, 0.6);
        died = 0;
    };

    ~Sparkle(){};

    void updatePos(int isExplosed, float rockety)
    {
        if (!isExplosed)
        {
            y = rockety;
            // y = rockety - random(0.01, 2);
        }
        else if (!died)
        {
            if (life > 0.0)
            {
                x += vx;
                y += vy;
                life -= 0.001;
            }
            else
            {
                died = 1;
            }
        }
    }

    void setPos(float xVal, float yVal, float zVal)
    {
        x = xVal;
        y = yVal;
        z = zVal;
    };

    void reinit(float x, float y, float z, float explosionRadius)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->explosionRadius = explosionRadius;
        float theta = random(0, 2 * M_PI);
        float v = random(0.0001, 0.003 * explosionRadius);
        vx = v * cos(theta);
        vy = v * sin(theta);
        life = random(0.3, 0.6);
        died = 0;
    }

    glm::vec4 getPos()
    {
        return glm::vec4(x, y, z, 0.0);
    };

    int getDied()
    {
        return died;
    };
};
