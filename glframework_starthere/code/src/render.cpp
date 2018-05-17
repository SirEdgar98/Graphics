#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <cassert>
#include <math.h>
#include <ctime>
#include "GL_framework.h"
#include <vector>
#include <iostream>

#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>

#define TWO_PI 
//variables to load an object:
//Trump Vectors
std::vector< glm::vec3 > TrumpVertices;
std::vector< glm::vec2 > TrumpUvs;
std::vector< glm::vec3 > TrumpNormals;
//Pollo Vectors
std::vector< glm::vec3 > PolloVertices;
std::vector< glm::vec2 > PolloUvs;
std::vector< glm::vec3 > PolloNormals;
//Cabin Vectors
std::vector< glm::vec3 > CabinVertices;
std::vector< glm::vec2 > CabinUvs;
std::vector< glm::vec3 > CabinNormals;
//Noria Body Vectors
std::vector< glm::vec3 > NoriaBodyVertices;
std::vector< glm::vec2 > NoriaBodyUvs;
std::vector< glm::vec3 > NoriaBodyNormals;
//Noria Legs Vectors
std::vector< glm::vec3 > NoriaLegsVertices;
std::vector< glm::vec2 > NoriaLegsUvs;
std::vector< glm::vec3 > NoriaLegsNormals;


//Light Variables 
// Sun
glm::vec3 lightPos;
float lightRadius = 10.0f;

glm::vec3 lightMoonPos;
float moonRadius = 10.0f;

//Inside Cabin Light
glm::vec3 inCabinlightPos;
float inCabinlightRadius = 1.0;

glm::vec3 sunlight;
glm::vec3 moonlight;






extern bool loadOBJ(const char * path,
	std::vector < glm::vec3 > & out_vertices,
	std::vector < glm::vec2 > & out_uvs,
	std::vector < glm::vec3 > & out_normals
);

float toRadians(float d) { return d * 3.14 / 180.0; }
enum class cameraPlane
{
	GENERAL_SHOT,
	COUNTER_SHOT,
	LATERAL_SHOT,
	GODS_EYE_SHOT
};





//COLORS
namespace colors {
	const glm::vec3 red = glm::vec3(0.7,0.1,0.0);
	const glm::vec3 black = glm::vec3(0.0 ,0.0, 0.0);
	const glm::vec3 white = glm::vec3(0.8, 0.8, 0.8);
	const glm::vec3 green = glm::vec3(0.1, 0.7, 0.0);
	const glm::vec3 blue = glm::vec3(0.0, 0.1, 0.7);
	const glm::vec3 orange = glm::vec3(0.8, 0.4, 0.0);
	const glm::vec3 yellow = glm::vec3(0.8, 0.8, 0.0);
	const glm::vec3 sunlightcolor = glm::vec3(0.5, 0.5, 0.5);
	const glm::vec3 sunlightredcolor = glm::vec3(1.0, 0.3, 0.0);
	const glm::vec3 moonlightcolor = glm::vec3(0.5, 0.5, 0.8);

}


glm::vec3 interpolate(glm::vec3 first, glm::vec3 second, float alpha){

	glm::vec3 vec = second - first;

	return first + vec * alpha;
}


int currentCameraCounter = 0;
cameraPlane currentCamera = cameraPlane::GENERAL_SHOT;

float transitionTime = 2.0;
float nextTime = clock() + transitionTime * 1000; 
bool trumpFocus = true;


bool show_test_window = false;



const char* model_vertShader =
"#version 330\n\
	in vec3 in_Position;\n\
	in vec3 in_Normal;\n\
	uniform vec3 lPos;\n\
	uniform vec3 moonPos;\n\
	uniform vec3 inlPos;\n\
	out vec3 lDir;\n\
	out vec3 moonDir;\n\
	out vec3 inlDir;\n\
	out vec4 vert_Normal;\n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		vec4 worldPos =  objMat * vec4(in_Position, 1.0);\n\
		gl_Position = mvpMat * worldPos;\n\
		vert_Normal = mv_Mat * objMat * vec4(in_Normal, 0.0);\n\
		lDir = normalize(lPos - worldPos.xyz);\n\
		inlDir = normalize(inlPos - worldPos.xyz);\n\
		moonDir = normalize(inlPos - worldPos.xyz);\n\
	}";

const char* model_fragShader =
"#version 330\n\
		in vec4 vert_Normal;\n\
		in vec3 lDir;\n\
		in vec3 moonDir;\n\
		in vec3 inlDir;\n\
		out vec4 out_Color;\n\
		uniform mat4 mv_Mat;\n\
		uniform vec4 color;\n\
		uniform vec3 sunColor;\n\
		uniform vec3 moonColor;\n\
		uniform vec3 inColor;\n\
		void main() {\n\
			out_Color =  vec4(color.xyz * \n\
                (sunColor * dot(normalize(vert_Normal), mv_Mat*vec4(lDir.x, lDir.y, lDir.z, 0.0)) +\n\
                moonColor * dot(normalize(vert_Normal), mv_Mat*vec4(moonDir.x, moonDir.y, moonDir.z, 0.0)) +\n\
				inColor * dot(normalize(vert_Normal), mv_Mat*vec4(inlDir.x, inlDir.y, inlDir.z, 0.0)) +\n\
                vec3(0.0, 0.0, 0.0)), 1.0);\n\
		\n\
		//float U = dot(normalize(vert_Normal), mv_Mat*vec4(lDir.x, lDir.y, lDir.z, 0.0)); \n\
		//if (U < 0.2) U = 0.1;\n\
		//if (U >= 0.3 && U < 0.5) U = 0.4;\n\
		//if (U >= 0.5 && U < 0.7) U = 0.6;\n\
		//if (U >= 0.9) U = 1.0;\n\
			//out_Color = vec4(color.xyz * U, 1.0 );\n\
}";



bool light_moves = true;
bool sunActive = true;
bool moonActive = true;
bool bulbActive = true;
bool secondWheel = false;

void GUI() {
	bool show = true;
	ImGui::Begin("Simulation Parameters", &show, 0);
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate


		if (ImGui::Button("Toggle Light Move")) {
			light_moves = !light_moves;
		}
		

		if (ImGui::Button("Toggle Sun Light")) {
			sunActive = !sunActive;
		}

		if (ImGui::Button("Toggle Moon Light")) {
			moonActive = !moonActive;
		}

		if (ImGui::Button("Toggle Bulb Light")) {
			bulbActive = !bulbActive;
		}

		if (ImGui::Button("Toggle Second Wheel")) {
			secondWheel = !secondWheel;
		}



		if (ImGui::Button("Camera")) {
			currentCameraCounter++;
			currentCameraCounter %= 4;

			switch (currentCameraCounter)
			{
			case 0: currentCamera = cameraPlane::GENERAL_SHOT; break;
			case 1: currentCamera = cameraPlane::COUNTER_SHOT; break;
			case 2: currentCamera = cameraPlane::LATERAL_SHOT; break;
			case 3: currentCamera = cameraPlane::GODS_EYE_SHOT; break;
			}

		}

	}
	// .........................

	ImGui::End();

	// Example code -- ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 60), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

///////// fw decl
namespace ImGui {
	void Render();
}
namespace Axis {
	void setupAxis();
	void cleanupAxis();
	void drawAxis();
}

namespace TrumpModel {
	void setupModel();
	void cleanupModel();
	void updateModel(const glm::mat4& transform);
	void drawModel(glm::vec3 color,  glm::vec3 inColor);
}

namespace PolloModel {
	void setupModel();
	void cleanupModel();
	void updateModel(const glm::mat4& transform);
	void drawModel(glm::vec3 color, glm::vec3 inColor);
}

namespace CabinModel {
	void setupModel();
	void cleanupModel();
	void updateModel(const glm::mat4& transform);
	void drawModel(glm::vec3 color, glm::vec3 inColor);
}

namespace NoriaBodyModel {
	void setupModel();
	void cleanupModel();
	void updateModel(const glm::mat4& transform);
	void drawModel(glm::vec3 color, glm::vec3 inColor);
}

namespace NoriaLegsModel {
	void setupModel();
	void cleanupModel();
	void updateModel(const glm::mat4& transform);
	void drawModel(glm::vec3 color, glm::vec3 inColor);
}

namespace Sphere {
	void setupSphere(glm::vec3 pos, float radius);
	void cleanupSphere();
	void updateSphere(glm::vec3 pos, float radius);
	void drawSphere();
}

namespace MyFirstShader {
	GLuint myShaderCompile(void);
	void myInitCode(void);
	void myRenderCode(double currentTime);
	void myCleanupCode(void);

	GLuint myRenderProgram;
	GLuint myVAO; // VertexArrayObject
}


////////////////

namespace RenderVars {
	const float FOV = glm::radians(65.f);
	float zNear = 1.f;
	const float zFar = 100000.0f;

	glm::mat4 _projection;
	glm::mat4 _modelView;
	glm::mat4 _MVP;
	glm::mat4 _inv_modelview;
	glm::vec4 _cameraPoint;

	struct prevMouse {
		float lastx, lasty;
		MouseEvent::Button button = MouseEvent::Button::None;
		bool waspressed = false;
	} prevMouse;

	float panv[3] = { 0.f, -20.f, -50.f };
	float rota[2] = { 0.f, 0.f };
}
namespace RV = RenderVars;

void GLResize(int width, int height) {
	glViewport(0, 0, width, height);
	if (height != 0) RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);
	else RV::_projection = glm::perspective(RV::FOV, 0.f, RV::zNear, RV::zFar);
}

void GLmousecb(MouseEvent ev) {
	if (RV::prevMouse.waspressed && RV::prevMouse.button == ev.button) {
		float diffx = ev.posx - RV::prevMouse.lastx;
		float diffy = ev.posy - RV::prevMouse.lasty;
		switch (ev.button) {
		case MouseEvent::Button::Left: // ROTATE
			RV::rota[0] += diffx * 0.005f;
			RV::rota[1] += diffy * 0.005f;
			break;
		case MouseEvent::Button::Right: // MOVE XY
			RV::panv[0] += diffx * 0.03f;
			RV::panv[1] -= diffy * 0.03f;
			break;
		case MouseEvent::Button::Middle: // MOVE Z
			RV::panv[2] += diffy * 0.05f;
			break;
		default: break;
		}
	}
	else {
		RV::prevMouse.button = ev.button;
		RV::prevMouse.waspressed = true;
	}
	RV::prevMouse.lastx = ev.posx;
	RV::prevMouse.lasty = ev.posy;
}

void GLinit(int width, int height) {
	glViewport(0, 0, width, height);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);

	// Setup shaders & geometry

	bool res = loadOBJ("trump.obj", TrumpVertices, TrumpUvs, TrumpNormals);
	res = loadOBJ("pollo.obj", PolloVertices, PolloUvs, PolloNormals);
	res = loadOBJ("wheel_cabin.obj", CabinVertices, CabinUvs, CabinNormals); 
	res = loadOBJ("wheel.obj",NoriaBodyVertices,NoriaBodyUvs,NoriaBodyNormals);
	res = loadOBJ("legs.obj", NoriaLegsVertices, NoriaLegsUvs, NoriaLegsNormals);

	TrumpModel::setupModel();
	PolloModel::setupModel();
	CabinModel::setupModel(); 
	NoriaBodyModel::setupModel();
	NoriaLegsModel::setupModel();

	Sphere::setupSphere(lightPos, 1.0f);

	MyFirstShader::myInitCode();




}

void GLcleanup() {
	TrumpModel::cleanupModel();
	PolloModel::cleanupModel();
	CabinModel::cleanupModel();
	NoriaBodyModel::cleanupModel();
	NoriaLegsModel::cleanupModel();
	Sphere::cleanupSphere();
	MyFirstShader::myCleanupCode();

}


void GLrender(double currentTime) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RV::_modelView = glm::mat4(1.f);
	RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1], RV::panv[2]));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1], glm::vec3(1.f, 0.f, 0.f));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));

	//Transform variables
	float v = currentTime / 150;
	float r = 170.0;
	float sunRad = 1000.0; 
	float sunVel =( currentTime * 6.28) / 20;
	float moonVel = (currentTime * 6.28) / 18;
	int numCab = 20;
	glm::vec3 noriaScale = glm::vec3(0.03, 0.03, 0.03);
	glm::vec3 legsScale = glm::vec3(0.03, 0.03, 0.03);
	glm::vec3 noria2Offset = glm::vec3(10000.0, 0.0, 0.0);



	
	//Sun Movement
	if (light_moves) {
		lightPos = glm::vec3(0.0, sunRad * cos(sunVel) - r, sunRad * sin(sunVel));

		if (cos(sunVel) > 0)
			sunlight = interpolate( colors::sunlightredcolor, colors::sunlightcolor, cos(sunVel));
		else
			sunlight = interpolate(colors::sunlightredcolor, colors::black, -cos(sunVel));


		//SE CRUZAN EN 1 y -1
		lightMoonPos = glm::vec3(cos(toRadians(30.0)) * sunRad * sin(-moonVel), sunRad * cos(-moonVel) - r, sin(toRadians(30.0)) * sunRad * sin(-moonVel));

	

		moonlight = interpolate(colors::black, colors::moonlightcolor,   -(cos(moonVel) + 1.0)/2.0);

	}
	
	if (!sunActive)
		sunlight = colors::black;

	if (!moonActive)
		moonlight = colors::black;

	
	for (int i = 0; i < numCab; i++) {
		glm::mat4 noriaMat = glm::translate(glm::mat4(1.f), glm::vec3(r*(cos((6.26 * v) + ((6.26 / numCab)*i))), r*(sin((6.26*v) + ((6.26 / numCab)*i))), 0.0));
		glm::mat4 noria2Mat = glm::translate(glm::mat4(1.f), glm::vec3(r*(cos((6.26 * -v) + ((6.26 / numCab)*i))), r*(sin((6.26*-v) + ((6.26 / numCab)*i))), 0.0));
		noriaMat = glm::translate(noriaMat, glm::vec3(0.0, -5.0, 0.0));
		noria2Mat = glm::translate(noria2Mat, glm::vec3(0.0, -5.0, 0.0));
		
		if (i == 0) {

			inCabinlightPos = glm::vec3(noriaMat[3][0], noriaMat[3][1] - 4 *  glm::abs(cos(currentTime)) - 4, noriaMat[3][2] + 8 *sin(currentTime));
			

			//Set Trump and Pollo in cabin position
			glm::mat4 polloMat = glm::translate(noriaMat, glm::vec3(6.0,-10.0,0.0));
			glm::mat4 trumpMat = glm::translate(noriaMat, glm::vec3(-5.0, -10.0, 0.0));

			polloMat = glm::rotate(polloMat, toRadians(-90.0), glm::vec3(0.0, 1.0, 0.0));
			trumpMat = glm::rotate(trumpMat, toRadians(90.0), glm::vec3(0.0, 1.0, 0.0));
			
			polloMat = glm::scale(polloMat, glm::vec3(0.1, 0.1, 0.1));
			trumpMat = glm::scale(trumpMat, glm::vec3(0.05, 0.05, 0.05));

			PolloModel::updateModel(polloMat);
			TrumpModel::updateModel(trumpMat);

			glm::vec3 polloLoc = glm::vec3(polloMat[3][0], polloMat[3][1], polloMat[3][2]);
			glm::vec3 trumpLoc = glm::vec3(trumpMat[3][0], trumpMat[3][1], trumpMat[3][2]);
			glm::vec3 polloOfsset = glm::vec3(-4.0, 5.0, 3.0);
			glm::vec3 trumpOfsset = glm::vec3(3.0, 3.0, 3.0);
			glm::vec3 upOfssetTrump = glm::vec3(0.0, 8.0, 0.0);
			glm::vec3 upOfssetPollo = glm::vec3(0.0, 5.0, 0.0);

			glm::vec3 cabinLoc = glm::vec3(noriaMat[3][0], noriaMat[3][1], noriaMat[3][2]);
			glm::vec3 cabinOffset = glm::vec3(0.0, 9.0, 0.001);


			if (clock() > nextTime)
			{

				if (trumpFocus)
					trumpFocus = false;
				else
					trumpFocus = true;
				nextTime = clock() + transitionTime * 1000;
				
			}

			switch (currentCamera) {
				//Camara de lejos
			case cameraPlane::GENERAL_SHOT:
				RV::_modelView = glm::lookAt(glm::vec3(350.0, 50.0, -180.0), glm::vec3(60.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0));
				break;

				//Camara de cerca
			case cameraPlane::COUNTER_SHOT:
				if(trumpFocus)
				RV::_modelView = glm::lookAt(polloLoc + polloOfsset, trumpLoc + upOfssetTrump, glm::vec3(0.0, 1.0, 0));
				else
				RV::_modelView = glm::lookAt(trumpLoc + trumpOfsset, polloLoc + upOfssetPollo, glm::vec3(0.0, 1.0, 0));
				break;

			case cameraPlane::LATERAL_SHOT:
				RV::_modelView = glm::lookAt(glm::vec3(0.0, 0.0, -5000.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0));
				break;

			case cameraPlane::GODS_EYE_SHOT:
				RV::_modelView = glm::lookAt(cabinLoc + cabinOffset, cabinLoc - cabinOffset, glm::vec3(sin(currentTime), 0.0, cos(currentTime)));
				break;

			}

			RV::_MVP = RV::_projection * RV::_modelView;


			if (bulbActive) {
				PolloModel::drawModel(colors::orange, colors::white);
				TrumpModel::drawModel(colors::yellow, colors::white);
			}
			else {
				PolloModel::drawModel(colors::orange, colors::black);
				TrumpModel::drawModel(colors::yellow, colors::black);
			}
		}

		
		noriaMat = glm::scale(noriaMat, noriaScale);
		CabinModel::updateModel(noriaMat);
		CabinModel::drawModel(colors::white, colors::black);

		if (secondWheel) {
			noria2Mat = glm::scale(noria2Mat, noriaScale);
			noria2Mat = glm::translate(noria2Mat, glm::vec3(0.0, 0.0, 3000.0));
			CabinModel::updateModel(noria2Mat);
			CabinModel::drawModel(colors::white, colors::black);

		}

	
	}

	
		glm::mat4 noriaMat = glm::rotate(glm::mat4(1.f), (float)(6.26f * v), glm::vec3(0.0, 0.0, 1.0));
		
		

		noriaMat = glm::scale(noriaMat, noriaScale);

	NoriaBodyModel::updateModel(noriaMat);
	NoriaBodyModel::drawModel(colors::white, colors::black);

	if (secondWheel) {
		glm::mat4 noria2Mat = glm::rotate(glm::mat4(1.f), (float)(6.26f * -v), glm::vec3(0.0, 0.0, 1.0));
		noria2Mat = glm::scale(noria2Mat, noriaScale);
		noria2Mat = glm::translate(noria2Mat, noria2Offset);
		NoriaBodyModel::updateModel(noria2Mat);
		NoriaBodyModel::drawModel(colors::white, colors::black);
	}

	glm::mat4 legsMat = glm::mat4(1.f);
	legsMat = glm::translate(legsMat, glm::vec3(0.0, -r/2, 0.0));
	legsMat = glm::scale(legsMat, legsScale);
	legsMat = glm::rotate(legsMat, 3.14f, glm::vec3(0.0, 1.0, 0.0));
	

	NoriaLegsModel::updateModel(legsMat);
	NoriaLegsModel::drawModel(colors::white, colors::black);

	if (secondWheel) {
		legsMat = glm::translate(legsMat, -noria2Offset);
		NoriaLegsModel::updateModel(legsMat);
		NoriaLegsModel::drawModel(colors::white, colors::black);
	}
	
	//Sun sphere
	Sphere::updateSphere(lightPos, lightRadius);
	Sphere::drawSphere();
	//Moon sphere
	Sphere::updateSphere(lightMoonPos, moonRadius);
	Sphere::drawSphere();
	//Inside cabin sphere
	Sphere::updateSphere(inCabinlightPos, inCabinlightRadius);
	Sphere::drawSphere();
	

	ImGui::Render();
}


//////////////////////////////////// COMPILE AND LINK
GLuint compileShader(const char* shaderStr, GLenum shaderType, const char* name = "") {
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderStr, NULL);
	glCompileShader(shader);
	GLint res;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &res);
		char *buff = new char[res];
		glGetShaderInfoLog(shader, res, &res, buff);
		fprintf(stderr, "Error Shader %s: %s", name, buff);
		delete[] buff;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}
void linkProgram(GLuint program) {
	glLinkProgram(program);
	GLint res;
	glGetProgramiv(program, GL_LINK_STATUS, &res);
	if (res == GL_FALSE) {
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &res);
		char *buff = new char[res];
		glGetProgramInfoLog(program, res, &res, buff);
		fprintf(stderr, "Error Link: %s", buff);
		delete[] buff;
	}
}


////////////////////////////////////////////////// AXIS
namespace Axis {
	GLuint AxisVao;
	GLuint AxisVbo[3];
	GLuint AxisShader[2];
	GLuint AxisProgram;

	float AxisVerts[] = {
		0.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0
	};
	float AxisColors[] = {
		1.0, 0.0, 0.0, 1.0,
		1.0, 0.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 0.0, 1.0, 1.0,
		0.0, 0.0, 1.0, 1.0
	};
	GLubyte AxisIdx[] = {
		0, 1,
		2, 3,
		4, 5
	};
	const char* Axis_vertShader =
		"#version 330\n\
in vec3 in_Position;\n\
in vec4 in_Color;\n\
out vec4 vert_color;\n\
uniform mat4 mvpMat;\n\
void main() {\n\
	vert_color = in_Color;\n\
	gl_Position = mvpMat * vec4(in_Position, 1.0);\n\
}";
	const char* Axis_fragShader =
		"#version 330\n\
in vec4 vert_color;\n\
out vec4 out_Color;\n\
void main() {\n\
	out_Color = vert_color;\n\
}";

	void setupAxis() {
		glGenVertexArrays(1, &AxisVao);
		glBindVertexArray(AxisVao);
		glGenBuffers(3, AxisVbo);

		glBindBuffer(GL_ARRAY_BUFFER, AxisVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, AxisVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, AxisVbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, AxisColors, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 4, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, AxisVbo[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 6, AxisIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		AxisShader[0] = compileShader(Axis_vertShader, GL_VERTEX_SHADER, "AxisVert");
		AxisShader[1] = compileShader(Axis_fragShader, GL_FRAGMENT_SHADER, "AxisFrag");

		AxisProgram = glCreateProgram();
		glAttachShader(AxisProgram, AxisShader[0]);
		glAttachShader(AxisProgram, AxisShader[1]);
		glBindAttribLocation(AxisProgram, 0, "in_Position");
		glBindAttribLocation(AxisProgram, 1, "in_Color");
		linkProgram(AxisProgram);
	}
	void cleanupAxis() {
		glDeleteBuffers(3, AxisVbo);
		glDeleteVertexArrays(1, &AxisVao);

		glDeleteProgram(AxisProgram);
		glDeleteShader(AxisShader[0]);
		glDeleteShader(AxisShader[1]);
	}
	void drawAxis() {
		glBindVertexArray(AxisVao);
		glUseProgram(AxisProgram);
		glUniformMatrix4fv(glGetUniformLocation(AxisProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
		glDrawElements(GL_LINES, 6, GL_UNSIGNED_BYTE, 0);

		glUseProgram(0);
		glBindVertexArray(0);
	}
}

////////////////////////////////////////////////// SPHERE
namespace Sphere {
	GLuint sphereVao;
	GLuint sphereVbo;
	GLuint sphereShaders[3];
	GLuint sphereProgram;
	float radius;

	const char* sphere_vertShader =
		"#version 330\n\
in vec3 in_Position;\n\
uniform mat4 mv_Mat;\n\
void main() {\n\
	gl_Position = mv_Mat * vec4(in_Position, 1.0);\n\
}";
	const char* sphere_geomShader =
		"#version 330\n\
layout(points) in;\n\
layout(triangle_strip, max_vertices = 4) out;\n\
out vec4 eyePos;\n\
out vec4 centerEyePos;\n\
uniform mat4 projMat;\n\
uniform float radius;\n\
vec4 nu_verts[4];\n\
void main() {\n\
	vec3 n = normalize(-gl_in[0].gl_Position.xyz);\n\
	vec3 up = vec3(0.0, 1.0, 0.0);\n\
	vec3 u = normalize(cross(up, n));\n\
	vec3 v = normalize(cross(n, u));\n\
	nu_verts[0] = vec4(-radius*u - radius*v, 0.0); \n\
	nu_verts[1] = vec4( radius*u - radius*v, 0.0); \n\
	nu_verts[2] = vec4(-radius*u + radius*v, 0.0); \n\
	nu_verts[3] = vec4( radius*u + radius*v, 0.0); \n\
	centerEyePos = gl_in[0].gl_Position;\n\
	for (int i = 0; i < 4; ++i) {\n\
		eyePos = (gl_in[0].gl_Position + nu_verts[i]);\n\
		gl_Position = projMat * eyePos;\n\
		EmitVertex();\n\
	}\n\
	EndPrimitive();\n\
}";
	const char* sphere_fragShader_flatColor =
		"#version 330\n\
in vec4 eyePos;\n\
in vec4 centerEyePos;\n\
out vec4 out_Color;\n\
uniform mat4 projMat;\n\
uniform mat4 mv_Mat;\n\
uniform vec4 color;\n\
uniform float radius;\n\
void main() {\n\
	vec4 diff = eyePos - centerEyePos;\n\
	float distSq2C = dot(diff, diff);\n\
	if (distSq2C > (radius*radius)) discard;\n\
	float h = sqrt(radius*radius - distSq2C);\n\
	vec4 nuEyePos = vec4(eyePos.xy, eyePos.z + h, 1.0);\n\
	vec4 nuPos = projMat * nuEyePos;\n\
	gl_FragDepth = ((nuPos.z / nuPos.w) + 1) * 0.5;\n\
	vec3 normal = normalize(nuEyePos - centerEyePos).xyz;\n\
	out_Color = vec4(color.xyz * dot(normal, (mv_Mat*vec4(0.0, 1.0, 0.0, 0.0)).xyz) + color.xyz * 0.3, 1.0 );\n\
}";

	bool shadersCreated = false;
	void createSphereShaderAndProgram() {
		if (shadersCreated) return;

		sphereShaders[0] = compileShader(sphere_vertShader, GL_VERTEX_SHADER, "sphereVert");
		sphereShaders[1] = compileShader(sphere_geomShader, GL_GEOMETRY_SHADER, "sphereGeom");
		sphereShaders[2] = compileShader(sphere_fragShader_flatColor, GL_FRAGMENT_SHADER, "sphereFrag");

		sphereProgram = glCreateProgram();
		glAttachShader(sphereProgram, sphereShaders[0]);
		glAttachShader(sphereProgram, sphereShaders[1]);
		glAttachShader(sphereProgram, sphereShaders[2]);
		glBindAttribLocation(sphereProgram, 0, "in_Position");
		linkProgram(sphereProgram);

		shadersCreated = true;
	}
	void cleanupSphereShaderAndProgram() {
		if (!shadersCreated) return;
		glDeleteProgram(sphereProgram);
		glDeleteShader(sphereShaders[0]);
		glDeleteShader(sphereShaders[1]);
		glDeleteShader(sphereShaders[2]);
		shadersCreated = false;
	}

	void setupSphere(glm::vec3 pos, float radius) {
		Sphere::radius = radius;
		glGenVertexArrays(1, &sphereVao);
		glBindVertexArray(sphereVao);
		glGenBuffers(1, &sphereVbo);

		glBindBuffer(GL_ARRAY_BUFFER, sphereVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3, &pos, GL_DYNAMIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		createSphereShaderAndProgram();
	}
	void cleanupSphere() {
		glDeleteBuffers(1, &sphereVbo);
		glDeleteVertexArrays(1, &sphereVao);

		cleanupSphereShaderAndProgram();
	}
	void updateSphere(glm::vec3 pos, float radius) {
		glBindBuffer(GL_ARRAY_BUFFER, sphereVbo);
		float* buff = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		buff[0] = pos.x;
		buff[1] = pos.y;
		buff[2] = pos.z;
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		Sphere::radius = radius;
	}
	void drawSphere() {
		glBindVertexArray(sphereVao);
		glUseProgram(sphereProgram);
		glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
		glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RV::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "projMat"), 1, GL_FALSE, glm::value_ptr(RV::_projection));
		glUniform4f(glGetUniformLocation(sphereProgram, "color"), 0.6f, 0.1f, 0.1f, 1.f);
		glUniform1f(glGetUniformLocation(sphereProgram, "radius"), Sphere::radius);
		glDrawArrays(GL_POINTS, 0, 1);

		glUseProgram(0);
		glBindVertexArray(0);
	}
}

////////////////////////////////////////////////// MyModels
namespace CabinModel {
	GLuint modelVao;
	GLuint modelVbo[3];
	GLuint modelShaders[2];
	GLuint modelProgram;
	glm::mat4 objMat = glm::mat4(1.f);


	void setupModel() {


		glGenVertexArrays(1, &modelVao);
		glBindVertexArray(modelVao);
		glGenBuffers(3, modelVbo);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[0]);

		glBufferData(GL_ARRAY_BUFFER, CabinVertices.size() * sizeof(glm::vec3), &CabinVertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[1]);

		glBufferData(GL_ARRAY_BUFFER, CabinNormals.size() * sizeof(glm::vec3), &CabinNormals[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);



		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		modelShaders[0] = compileShader(model_vertShader, GL_VERTEX_SHADER, "cubeVert");
		modelShaders[1] = compileShader(model_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		modelProgram = glCreateProgram();
		glAttachShader(modelProgram, modelShaders[0]);
		glAttachShader(modelProgram, modelShaders[1]);
		glBindAttribLocation(modelProgram, 0, "in_Position");
		glBindAttribLocation(modelProgram, 1, "in_Normal");
		linkProgram(modelProgram);
	}
	void cleanupModel() {

		glDeleteBuffers(2, modelVbo);
		glDeleteVertexArrays(1, &modelVao);

		glDeleteProgram(modelProgram);
		glDeleteShader(modelShaders[0]);
		glDeleteShader(modelShaders[1]);
	}
	void updateModel(const glm::mat4& transform) {
		objMat = transform;
	}
	void drawModel(glm::vec3 color, glm::vec3 inColor) {
		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);
		
		

			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

			glUniform3f(glGetUniformLocation(modelProgram, "lPos"), lightPos.x, lightPos.y, lightPos.z);
			glUniform3f(glGetUniformLocation(modelProgram, "inlPos"), inCabinlightPos.x, inCabinlightPos.y, inCabinlightPos.z);
			glUniform3f(glGetUniformLocation(modelProgram, "moonPos"), lightMoonPos.x, lightMoonPos.y, lightMoonPos.z);

			glUniform4f(glGetUniformLocation(modelProgram, "color"), color.r, color.g, color.b, 0.f);
			glUniform3f(glGetUniformLocation(modelProgram, "sunColor"), sunlight.r, sunlight.g, sunlight.b);
			glUniform3f(glGetUniformLocation(modelProgram, "moonColor"), moonlight.r, moonlight.g, moonlight.b);
			glUniform3f(glGetUniformLocation(modelProgram, "inColor"), inColor.r, inColor.g, inColor.b);

			glDrawArrays(GL_TRIANGLES, 0, 100000);



		glUseProgram(0);
		glBindVertexArray(0);

	}


}

namespace NoriaBodyModel {
	GLuint modelVao;
	GLuint modelVbo[3];
	GLuint modelShaders[2];
	GLuint modelProgram;
	glm::mat4 objMat = glm::mat4(1.f);



	void setupModel() {
		glGenVertexArrays(1, &modelVao);
		glBindVertexArray(modelVao);
		glGenBuffers(3, modelVbo);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[0]);

		glBufferData(GL_ARRAY_BUFFER, NoriaBodyVertices.size() * sizeof(glm::vec3), &NoriaBodyVertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[1]);

		glBufferData(GL_ARRAY_BUFFER, NoriaBodyNormals.size() * sizeof(glm::vec3), &NoriaBodyNormals[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);



		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		modelShaders[0] = compileShader(model_vertShader, GL_VERTEX_SHADER, "cubeVert");
		modelShaders[1] = compileShader(model_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		modelProgram = glCreateProgram();
		glAttachShader(modelProgram, modelShaders[0]);
		glAttachShader(modelProgram, modelShaders[1]);
		glBindAttribLocation(modelProgram, 0, "in_Position");
		glBindAttribLocation(modelProgram, 1, "in_Normal");
		linkProgram(modelProgram);
	}
	void cleanupModel() {

		glDeleteBuffers(2, modelVbo);
		glDeleteVertexArrays(1, &modelVao);

		glDeleteProgram(modelProgram);
		glDeleteShader(modelShaders[0]);
		glDeleteShader(modelShaders[1]);
	}
	void updateModel(const glm::mat4& transform) {
		objMat = transform;
	}
	void drawModel(glm::vec3 color, glm::vec3 inColor) {
		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);


			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

			glUniform3f(glGetUniformLocation(modelProgram, "lPos"), lightPos.x, lightPos.y, lightPos.z);
			glUniform3f(glGetUniformLocation(modelProgram, "moonPos"), lightMoonPos.x, lightMoonPos.y, lightMoonPos.z);

			glUniform4f(glGetUniformLocation(modelProgram, "color"), color.r, color.g, color.b, 0.f);
			glUniform3f(glGetUniformLocation(modelProgram, "sunColor"), sunlight.r, sunlight.g, sunlight.b);
			glUniform3f(glGetUniformLocation(modelProgram, "moonColor"), moonlight.r, moonlight.g, moonlight.b);
			glUniform3f(glGetUniformLocation(modelProgram, "inColor"), inColor.r, inColor.g, inColor.b);

			glDrawArrays(GL_TRIANGLES, 0, 10000000);



		glUseProgram(0);
		glBindVertexArray(0);

	}


}

namespace NoriaLegsModel {
	GLuint modelVao;
	GLuint modelVbo[3];
	GLuint modelShaders[2];
	GLuint modelProgram;
	glm::mat4 objMat = glm::mat4(1.f);




	void setupModel() {
		glGenVertexArrays(1, &modelVao);
		glBindVertexArray(modelVao);
		glGenBuffers(3, modelVbo);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[0]);

		glBufferData(GL_ARRAY_BUFFER, NoriaLegsVertices.size() * sizeof(glm::vec3), &NoriaLegsVertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[1]);

		glBufferData(GL_ARRAY_BUFFER, NoriaLegsNormals.size() * sizeof(glm::vec3), &NoriaLegsNormals[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);



		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		modelShaders[0] = compileShader(model_vertShader, GL_VERTEX_SHADER, "cubeVert");
		modelShaders[1] = compileShader(model_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		modelProgram = glCreateProgram();
		glAttachShader(modelProgram, modelShaders[0]);
		glAttachShader(modelProgram, modelShaders[1]);
		glBindAttribLocation(modelProgram, 0, "in_Position");
		glBindAttribLocation(modelProgram, 1, "in_Normal");
		linkProgram(modelProgram);
	}
	void cleanupModel() {

		glDeleteBuffers(2, modelVbo);
		glDeleteVertexArrays(1, &modelVao);

		glDeleteProgram(modelProgram);
		glDeleteShader(modelShaders[0]);
		glDeleteShader(modelShaders[1]);
	}
	void updateModel(const glm::mat4& transform) {
		objMat = transform;
	}
	void drawModel(glm::vec3 color, glm::vec3 inColor) {
		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);


		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

		glUniform3f(glGetUniformLocation(modelProgram, "lPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(modelProgram, "moonPos"), lightMoonPos.x, lightMoonPos.y, lightMoonPos.z);

		glUniform4f(glGetUniformLocation(modelProgram, "color"), color.r, color.g, color.b, 0.f);
		glUniform3f(glGetUniformLocation(modelProgram, "sunColor"), sunlight.r, sunlight.g, sunlight.b);
		glUniform3f(glGetUniformLocation(modelProgram, "moonColor"), moonlight.r, moonlight.g, moonlight.b);
		glUniform3f(glGetUniformLocation(modelProgram, "inColor"), inColor.r, inColor.g, inColor.b);

		glDrawArrays(GL_TRIANGLES, 0, 100000);



		glUseProgram(0);
		glBindVertexArray(0);

	}


}

namespace PolloModel {
	GLuint modelVao;
	GLuint modelVbo[3];
	GLuint modelShaders[2];
	GLuint modelProgram;
	glm::mat4 objMat = glm::mat4(1.f);






	
	void setupModel() {
		glGenVertexArrays(1, &modelVao);
		glBindVertexArray(modelVao);
		glGenBuffers(3, modelVbo);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[0]);

		glBufferData(GL_ARRAY_BUFFER, PolloVertices.size() * sizeof(glm::vec3), &PolloVertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[1]);

		glBufferData(GL_ARRAY_BUFFER, PolloNormals.size() * sizeof(glm::vec3), &PolloNormals[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);



		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		modelShaders[0] = compileShader(model_vertShader, GL_VERTEX_SHADER, "cubeVert");
		modelShaders[1] = compileShader(model_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		modelProgram = glCreateProgram();
		glAttachShader(modelProgram, modelShaders[0]);
		glAttachShader(modelProgram, modelShaders[1]);
		glBindAttribLocation(modelProgram, 0, "in_Position");
		glBindAttribLocation(modelProgram, 1, "in_Normal");
		linkProgram(modelProgram);
	}
	void cleanupModel() {

		glDeleteBuffers(2, modelVbo);
		glDeleteVertexArrays(1, &modelVao);

		glDeleteProgram(modelProgram);
		glDeleteShader(modelShaders[0]);
		glDeleteShader(modelShaders[1]);
	}
	void updateModel(const glm::mat4& transform) {
		objMat = transform;
	}
	void drawModel(glm::vec3 color, glm::vec3 inColor) {

		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);


		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

		glUniform3f(glGetUniformLocation(modelProgram, "lPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(modelProgram, "inlPos"), inCabinlightPos.x, inCabinlightPos.y, inCabinlightPos.z);
		glUniform3f(glGetUniformLocation(modelProgram, "moonPos"), lightMoonPos.x, lightMoonPos.y, lightMoonPos.z);

		glUniform4f(glGetUniformLocation(modelProgram, "color"), color.r, color.g, color.b, 0.f);
		glUniform3f(glGetUniformLocation(modelProgram, "sunColor"), sunlight.r, sunlight.g, sunlight.b);
		glUniform3f(glGetUniformLocation(modelProgram, "moonColor"), moonlight.r, moonlight.g, moonlight.b);
		glUniform3f(glGetUniformLocation(modelProgram, "inColor"), inColor.r, inColor.g, inColor.b);

		glDrawArrays(GL_TRIANGLES, 0, 100000);



		glUseProgram(0);
		glBindVertexArray(0);

	}
}

namespace TrumpModel {
	GLuint modelVao;
	GLuint modelVbo[3];
	GLuint modelShaders[2];
	GLuint modelProgram;
	glm::mat4 objMat = glm::mat4(1.f);





	void setupModel() {
		glGenVertexArrays(1, &modelVao);
		glBindVertexArray(modelVao);
		glGenBuffers(3, modelVbo);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[0]);

		glBufferData(GL_ARRAY_BUFFER, TrumpVertices.size() * sizeof(glm::vec3), &TrumpVertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[1]);

		glBufferData(GL_ARRAY_BUFFER, TrumpNormals.size() * sizeof(glm::vec3), &TrumpNormals[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);



		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		modelShaders[0] = compileShader(model_vertShader, GL_VERTEX_SHADER, "cubeVert");
		modelShaders[1] = compileShader(model_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		modelProgram = glCreateProgram();
		glAttachShader(modelProgram, modelShaders[0]);
		glAttachShader(modelProgram, modelShaders[1]);
		glBindAttribLocation(modelProgram, 0, "in_Position");
		glBindAttribLocation(modelProgram, 1, "in_Normal");
		linkProgram(modelProgram);
	}
	void cleanupModel() {

		glDeleteBuffers(2, modelVbo);
		glDeleteVertexArrays(1, &modelVao);

		glDeleteProgram(modelProgram);
		glDeleteShader(modelShaders[0]);
		glDeleteShader(modelShaders[1]);
	}
	void updateModel(const glm::mat4& transform) {
		objMat = transform;
	}
	void drawModel(glm::vec3 color, glm::vec3 inColor) {

		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);

		
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

			glUniform3f(glGetUniformLocation(modelProgram, "lPos"), lightPos.x, lightPos.y, lightPos.z);
			glUniform3f(glGetUniformLocation(modelProgram, "moonPos"), lightMoonPos.x, lightMoonPos.y, lightMoonPos.z);
			glUniform3f(glGetUniformLocation(modelProgram, "inlPos"), inCabinlightPos.x, inCabinlightPos.y, inCabinlightPos.z);
			
			glUniform4f(glGetUniformLocation(modelProgram, "color"), color.r, color.g, color.b, 0.f);
			glUniform3f(glGetUniformLocation(modelProgram, "sunColor"), sunlight.r, sunlight.g, sunlight.b);
			glUniform3f(glGetUniformLocation(modelProgram, "moonColor"), moonlight.r, moonlight.g, moonlight.b);
			glUniform3f(glGetUniformLocation(modelProgram, "inColor"), inColor.r, inColor.g, inColor.b);


			glDrawArrays(GL_TRIANGLES, 0, 100000);

		
		glUseProgram(0);
		glBindVertexArray(0);

	}


}

namespace MyFirstShader {


	//1.  Declare shader
	//la GPU treballa amb strings
	//glPosition = vec4(x,y,z,w);
	//IMPORTANTE: pintar los puntos en orden y sentido counter clockwise o pintara la figura por la cara que no se ve

	//2.  Compile and link shader
	GLuint myShaderCompile(void) {
		static const GLchar * vertex_shader_source[] =
		{
			"#version 330										\n\
		\n\
		void main() {\n\
		const vec4 vertices[3] = vec4[3](vec4( 0.25, -0.25, 0.5, 1.0),\n\
									   vec4(0.25, 0.25, 0.5, 1.0),\n\
										vec4( -0.25,  -0.25, 0.5, 1.0));\n\
		gl_Position = vertices[gl_VertexID];\n\
		}"
		};

		//the fragment shader considers gl_PrimitiveID:

		static const GLchar * fragment_shader_source[] =
		{
			"#version 330\n\
			\n\
			out vec4 color;\n\
			\n\
			void main() {\n\
			const vec4 colors[6] = vec4[6]( vec4( 0.4, 0.0, .9, 1.0),\n\
											vec4(0.25, 0.0, 0.8, 1.0),\n\
											vec4( 0.5, 0.0, 0.8, 1.0),\n\
											vec4(0.2, 0.0, 1.0, 1.0),\n\
											vec4(0.5, 0.0, 1.0, 1.0),\n\
											vec4( 0.3, 0.0, 0.5, 1.0));\n\
			color = colors[gl_PrimitiveID];\n\
			}" };


		//in the geometry shader notice how gl_PrimitiveID is assigned before emitting each vertex

		static const GLchar * geom_shader_source[] = {
			"#version 330 \n\
			uniform mat4 trans;\n\
			layout(triangles) in;\n\
			layout(triangle_strip, max_vertices = 24) out;\n\
			void main()\n\
			{\n\
				const vec4 vertices[4] = vec4[4](vec4(1.41, -1.41, 1.41, 1.0),\n\
										vec4(1.41, 1.41, 1.41, 1.0),\n\
										vec4(-1.41, -1.41, 1.41, 1.0),\n\
										vec4(-1.41, 1.41, 1.41, 1.0));\n\
				\n\
				//CARA 1\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = trans*vertices[i]+gl_in[0].gl_Position;\n\
					gl_PrimitiveID = 0;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA 2\n\
				const vec4 vertices2[4]= vec4[4](vec4(1.41, 1.41, 1.41, 1.0),\n\
										vec4(1.41, 1.41, -1.41, 1.0),\n\
										vec4(-1.41, 1.41, 1.41, 1.0),\n\
										vec4(-1.41, 1.41, -1.41, 1.0));\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = trans*vertices2[i]+gl_in[0].gl_Position;\n\
				gl_PrimitiveID = 1;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				//CARA 3\n\
				const vec4 vertices3[4]= vec4[4](vec4(-1.41, -1.41, 1.41, 1.0),\n\
										vec4(-1.41, 1.41, 1.41, 1.0),\n\
										vec4(-1.41, -1.41, -1.41, 1.0),\n\
										vec4(-1.41, 1.41, -1.41, 1.0));\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = trans*vertices3[i]+gl_in[0].gl_Position;\n\
				gl_PrimitiveID = 2;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				//CARA 4\n\
				const vec4 vertices4[4]= vec4[4](vec4(-1.41, -1.41, -1.41, 1.0),\n\
										vec4(-1.41, 1.41, -1.41, 1.0),\n\
										vec4(1.41, -1.41, -1.41, 1.0),\n\
										vec4(1.41, 1.41, -1.41, 1.0));\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = trans*vertices4[i]+gl_in[0].gl_Position;\n\
				gl_PrimitiveID = 3;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				//CARA 5\n\
				const vec4 vertices5[4]= vec4[4](vec4(-1.41, -1.41, 1.41, 1.0),\n\
										vec4(-1.41, -1.41, -1.41, 1.0),\n\
										vec4(1.41, -1.41, 1.41, 1.0),\n\
										vec4(1.41, -1.41, -1.41, 1.0));\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = trans*vertices5[i]+gl_in[0].gl_Position;\n\
				gl_PrimitiveID = 4;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				//CARA 6\n\
				const vec4 vertices6[4]= vec4[4](vec4(1.41, -1.41, -1.41, 1.0),\n\
										vec4(1.41, 1.41, -1.41, 1.0),\n\
										vec4(1.41, -1.41, 1.41, 1.0),\n\
										vec4(1.41, 1.41, 1.41, 1.0));\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = trans*vertices6[i]+gl_in[0].gl_Position;\n\
				gl_PrimitiveID = 5;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
			}"
		};


		GLuint vertex_shader;
		GLuint fragment_shader;
		GLuint geom_shader;
		GLuint program;

		vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
		glCompileShader(vertex_shader);


		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
		glCompileShader(fragment_shader);

		geom_shader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geom_shader, 1, geom_shader_source, NULL);
		glCompileShader(geom_shader);

		program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glAttachShader(program, geom_shader);
		glLinkProgram(program);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		glDeleteShader(geom_shader);

		return program;
	}


	void  myInitCode(void) {

		myRenderProgram = myShaderCompile();
		glCreateVertexArrays(1, &myVAO);
		glBindVertexArray(myVAO);
	}




	void myRenderCode(double currentTime) {

		glm::mat4 myTransformM = RV::_MVP;

		glUseProgram(myRenderProgram);

		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "trans"), 1, GL_FALSE, glm::value_ptr(myTransformM));

		glDrawArrays(GL_TRIANGLES, 0, 3);


	}





	//5. Cleanup shader
	void myCleanupCode(void) {
		glDeleteVertexArrays(1, &myVAO);
		glDeleteProgram(myRenderProgram);

	}

}