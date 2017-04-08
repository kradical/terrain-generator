#pragma once

#include <glm/glm.hpp>

class BezierCurve
{
public:

  // Constructor
  BezierCurve(int _numPoints, glm::vec3* _Points);
  BezierCurve();
  glm::vec3 GetNextValue();

private:
    glm::vec3* Points;
    int numPoints;
    float T;
};