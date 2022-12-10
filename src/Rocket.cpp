#include <iostream>
#include <glm/glm.hpp>

#define GRAVITY 0.00098
#define yMin -45

using namespace std;

class Rocket
{

private:
    float velocity;
    float x;
    float y;
    float z;
    float finalY;
    float explosionRadius;
    int explosed; // explosed flag
    float delayLaunch;
    float timeAfterExplosion;

public:
    static float random(float min, float max)
    {
        return ((float)rand() / RAND_MAX) * (max - min) + min;
    }

    static float highestPoint(float y, float velocity)
    {
        return y + velocity * velocity / (2 * GRAVITY);
    }

    Rocket()
    {
        velocity = random(0.25, 0.40);
        x = random(-60, 60);
        y = yMin;
        z = -1;
        explosed = 0;
        // physics formula, get the highest point
        finalY = highestPoint(y, velocity);
        timeAfterExplosion = random(0.5, 1.0);
        delayLaunch = random(0.0, 0.5);
        explosionRadius = random(5, 20);
    }

    ~Rocket()
    {
    }

    glm::vec4 getPos()
    {
        return glm::vec4(x, y, z, 0.0);
    }

    void updatePos()
    {
        if (velocity > -0.01)
        {
            y += velocity;
            // with gravity
            velocity -= GRAVITY;
        }
        else
        {
            explosed = 1;
        }
    };

    float getVelocity()
    {
        return velocity;
    };

    int isExplosed()
    {
        return explosed;
    };

    void updateLife()
    {
        timeAfterExplosion -= 0.001;
    };

    void updateDelayLaunch()
    {
        if (delayLaunch > 0)
            delayLaunch -= 0.001;
    };

    void reinitPos()
    {
        velocity = random(0.25, 0.40);
        x = random(-60, 60);
        y = yMin;
        z = -1;
        explosed = 0;
        finalY = highestPoint(y, velocity);
        timeAfterExplosion = random(0.5, 1.0);
        explosionRadius = random(5, 20);
    };

    float getFinalY()
    {
        return finalY;
    };

    float getTimeAfterExplosion()
    {
        return timeAfterExplosion;
    };

    int ready()
    {
        return delayLaunch <= 0;
    }

    float getExplosionRadius()
    {
        return explosionRadius;
    }
};