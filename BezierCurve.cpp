#include "BezierCurve.h"

#include <math.h>

BezierCurve::BezierCurve(int _numPoints, glm::vec3* _Points) {
  numPoints = _numPoints;
  Points = _Points;
  T = 0.0f;
}

BezierCurve::BezierCurve() {
  numPoints = 0;
  Points = NULL;
  T = 0.0f;
}

int factorial(int n)
{
  return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n;
}

float Binomial(int n, int k){
	return factorial(n) / (factorial(n-k) * factorial(k));
}

glm::vec3 BezierCurve::GetNextValue() {
	glm::vec3 newPos = glm::vec3(0.0f, 0.0f, 0.0f);
	for(int i=0; i<numPoints; i++){
		newPos += Points[i] * Binomial(i, numPoints) * (float)pow(1-T, numPoints-i) * (float)pow(T,i);

	}

	T = T + 0.01f;

	return newPos;

}