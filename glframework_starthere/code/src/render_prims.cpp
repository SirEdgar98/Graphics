#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>

// Boolean variables allow to show/hide the primitives
bool renderSphere = true;
bool renderCapsule = true;
bool renderParticles = false;
bool renderCloth = false;
bool renderCube = true;

namespace Sphere {
	extern void setupSphere(glm::vec3 pos = glm::vec3(0.f, 1.f, 0.f), float radius = 1.f);
	extern void cleanupSphere();
	extern void updateSphere(glm::vec3 pos, float radius = 1.f);
	extern void drawSphere();
}

namespace TrumpModel {
	extern void setupModel();
	extern void cleanupModel();
	extern void updateModel(const glm::mat4& transform);
	extern void drawModel(float currentTime);
}

namespace PolloModel {
	void setupModel();
	void cleanupModel();
	void updateModel(const glm::mat4& transform);
	void drawModel(float currentTime);
}

namespace CabinModel {
	void setupModel();
	void cleanupModel();
	void updateModel(const glm::mat4& transform);
	void drawModel(float currentTime);
}

namespace NoriaBodyModel {
	void setupModel();
	void cleanupModel();
	void updateModel(const glm::mat4& transform);
	void drawModel(float currentTime);
}

void setupPrims() {
	Sphere::setupSphere();

	TrumpModel::setupModel();
	PolloModel::setupModel();
	CabinModel::setupModel();
}
void cleanupPrims() {
	Sphere::cleanupSphere();

	TrumpModel::cleanupModel();
	PolloModel::cleanupModel();
	CabinModel::cleanupModel();
}

void renderPrims() {
	if (renderSphere)
		Sphere::drawSphere();

	//if (renderCube)
	//	TrumpModel::drawModel(0.33);
	//	PolloModel::drawModel(0.33);
	//	CabinModel::drawModel(0.33);
}
