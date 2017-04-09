#include "BezierCurve.h"

#include <math.h>
#include <iostream>

BezierCurve::BezierCurve(int _numPoints, glm::vec3* _Points) {
  numPoints = _numPoints;
  Points = _Points;
  std::cout << Points[3].x << Points[3].y << Points[3].z << std::endl;
  T = 0.0f;
}

BezierCurve::BezierCurve() {
  numPoints = 0;
  Points = NULL;
  T = 0.0f;
}

int factorial(int n)
{
	int rtn = 1;
	while(n >= 1){
		rtn = rtn * n;
		n--;
	}
	return rtn;
}

int Binomial(int n, int k){
	int tmp = factorial(n) / (factorial(n-k) * factorial(k));
	return tmp;
}

glm::vec3 BezierCurve::GetNextValue() const {
	glm::vec3 newPos = glm::vec3(0.0f, 0.0f, 0.0f);
	for(int i=0; i<numPoints; i++){
		float pointx = (Points[i][0]);
		float pointy = (Points[i][1]);
		float pointz = (Points[i][2]);
		float binomial = ((float)Binomial(numPoints-1, i));
		float pow1 = ((float)pow(1-T, numPoints - 1 - i));
		float pow2 = ((float)pow(T, i));
		newPos.x += pointx * binomial * pow1 * pow2;
		newPos.y += pointy * binomial * pow1 * pow2;
		newPos.z += pointz * binomial * pow1 * pow2;

	}

	return newPos;

}