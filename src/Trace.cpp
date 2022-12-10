#include <iostream>
#include <glm/glm.hpp>

class Trace
{
private:
    float x;
    float y;
    float z;

public:
    Trace()
    {
        x = 0;
        y = 0;
        z = 0;
    }

    Trace(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    ~Trace()
    {
    }

    glm::vec4 getPos()
    {
        return glm::vec4(x, y, z, 0.0);
    }

    void updatePos(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }
};