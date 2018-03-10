#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>

// Boolean variables allow to show/hide the primitives
bool renderCube = true;

namespace Cube {
	extern void setupCube();
	extern void cleanupCube();
	extern void updateCube(const glm::mat4& transform);
	extern void drawCube();
}

void setupPrims() {
	Cube::setupCube();
}
void cleanupPrims() {
	
	Cube::cleanupCube();
}

void renderPrims() {
	
	if (renderCube)
		Cube::drawCube();
}
