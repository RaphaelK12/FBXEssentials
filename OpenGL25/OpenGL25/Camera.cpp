#include <glm\glm.hpp>
#include <glm\vec3.hpp>
#include <glm\vec4.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Camera.h"

glm::mat4 LookAt(const glm::vec3& Eye, const glm::vec3& Center, const glm::vec3& Up)
{
	glm::mat4 Matrix;
	glm::vec3 X, Y, Z;
	Z = Eye - Center;
	glm::normalize(Z); //Z.glm::Normalize();
	Y = Up;
	
	X = glm::cross(Y, Z); //X = Y.Cross( Z );
	Y = glm::cross(Z, X); //Y = Z.Cross( X );

	glm::normalize(X); //X.Normalize();
	glm::normalize(Y); //Y.Normalize();
	Matrix[0][0] = X.x;
	Matrix[1][0] = X.y;
	Matrix[2][0] = X.z;

	Matrix[3][0] = glm::dot(-X, Eye); //Matrix[3][0] = -X.Dot(Eye);

	Matrix[0][1] = Y.x;
	Matrix[1][1] = Y.y;
	Matrix[2][1] = Y.z;
	
	Matrix[3][1] = glm::dot(-Y, Eye); //Matrix[3][1] = -Y.Dot(Eye);
	
	Matrix[0][2] = Z.x;
	Matrix[1][2] = Z.y;
	Matrix[2][2] = Z.z;
	Matrix[3][2] = glm::dot(-Z, Eye); //Matrix[3][2] = -Z.Dot(Eye);
	Matrix[0][3] = 0;
	Matrix[1][3] = 0;
	Matrix[2][3] = 0;
	Matrix[3][3] = 1.0f;
	return Matrix;
}