/*
 * RenderingHelper.h -> renamed MStack.h (Z.J.W.)
 * A means of replacing openGL matrix stack uses glm 
 *
 *  Created on: Jul 28, 2011
 *      Author: Wyatt and Evan
 */

#ifndef RenderingHelper_H_
#define RenderingHelper_H_

#include <stack>

#include "glm/glm.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/gtx/component_wise.hpp"

#include "glm/gtc/matrix_access.hpp"
#include "glm/gtc/matrix_integer.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"

struct RenderingHelper {
	glm::mat4 modelViewMatrix;
	std::stack<glm::mat4> modelViewMatrixStack;

	glm::mat4 projectionMatrix;
	std::stack<glm::mat4> projectionMatrixStack;

	glm::mat4 *currentMatrix;
	std::stack<glm::mat4> *currentMatrixStack;

public:
	RenderingHelper();
	virtual ~RenderingHelper();

	void useModelViewMatrix();
	void useProjectionMatrix();

	void pushMatrix();
	void popMatrix();

	void loadIdentity();

	void translate(const glm::vec3 &offset);
	void scale(float x, float y, float z);
	void scale(float size);
	void rotate(float angle, const glm::vec3 &axis);
	void rotateWith(glm::mat4 rotMatrix);

	void multMatrix(const glm::mat4 &matrix);
	const glm::mat4 &getMatrix();

	void ortho(float left, float right, float bottom, float top, float zNear, float zFar);
	void frustum(float left, float right, float bottom, float top, float zNear, float zFar);
	void lookAt(glm::vec3 eye, glm::vec3 target, glm::vec3 up);
};

#endif /* RenderingHelper_H_ */
