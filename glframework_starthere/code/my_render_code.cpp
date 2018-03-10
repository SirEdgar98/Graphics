
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <cstdio>
#include <cassert>
#include<imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <ctime>

#include "GL_framework.h"



namespace ImGui {
	void RenderGUI();
}
namespace Box {
	void setupCube();
	void cleanupCube();
	void drawCube();
}
namespace Axis {
	void setupAxis();
	void cleanupAxis();
	void drawAxis();
}

namespace Cube {
	void setupCube();
	void cleanupCube();
	void updateCube(const glm::mat4& transform);
	void drawCube();
	void DrawScene(double currentTime);
	void updateColor(const glm::vec4 newColor);
}

namespace RenderVars {
	 float FOV = glm::radians(65.0f);
	const float zNear = 0.f;
	const float zFar = 50.f;

	const float margin = 30;
	float travelingSpeed = 0.3;;
	float fovSpeed =  0.3;
	float zoomSpeed = 0.05;
	float posX = -30;
	float fov = 65;
	float zoom = - 10;
	static time_t rep;

	int width;
	int height;


	glm::mat4 _projection;
	glm::mat4 _modelView;
	glm::mat4 _MVP;
	glm::mat4 _inv_modelview;
	glm::vec4 _cameraPoint;

	float panv[3] = { 0.f, -5.f, -15.f };
	float rota[2] = { 0.f, 0.f };
}
namespace RV = RenderVars;


enum class scene{TRAVELING, ZOOM, FOV, DOLLY };
static int selected = 0;
scene s = scene::TRAVELING; 

namespace ImGui {
	void RenderGUI()
	{
		bool show = true;
		ImGui::Begin("Escenari Selection", &show, 0);
		{
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::RadioButton("Traveling", &selected, 0); 
			ImGui::RadioButton("Zoom", &selected, 1); 
			ImGui::RadioButton("FOV", &selected, 2);
			ImGui::RadioButton("Dolly Effect", &selected, 3);
		}
		ImGui::End();

	}
}

void myInitCode(int width, int height) {
	glViewport(0, 0, width, height);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	RV::rep = clock() + 4000;

	Cube::setupCube();
}
void myRenderCode(double currentTime, int width, int height) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (selected)
	{
	case 0:
		 s = scene::TRAVELING;
		break;
	case 1:
		 s = scene::ZOOM;
		break;
	case 2:
		 s = scene::FOV;
		break;
	case 3:
		 s = scene::DOLLY;
		break;
	default:
		break;
	}

	glm::mat4 CameraMat;
	float scale = 50;


	//Sitch for each scene
	switch (s) {
	case scene::TRAVELING:	
		RV::_projection = glm::ortho(-(float)width / scale, (float)width / scale, -(float)height / scale, (float)width / scale, RV::zNear, RV::zFar);
		CameraMat = glm::lookAt(glm::vec3{ RV::posX,20.0,-20.0 }, glm::vec3{ RV::posX + 5,5.0,0 }, glm::vec3{ 0.0,1.0,0.0 });
		break;

	case scene::ZOOM:
		RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);
	
		CameraMat = glm::lookAt(glm::vec3{ 0,3,RV::zoom }, glm::vec3{ 0 ,3,0 }, glm::vec3{ 0.0,1.0,0.0 });
		break;

	case scene::FOV:
		RV::_projection = glm::perspective(RV::FOV, (float)1920 / (float)1080, RV::zNear, RV::zFar);
		CameraMat = glm::lookAt(glm::vec3{ 0,3,-10 }, glm::vec3{ 0 ,3,0 }, glm::vec3{ 0.0,1.0,0.0 });
		break;

	case scene::DOLLY:
		RV::_projection = glm::perspective(RV::FOV, (float)1920 / (float)1080, RV::zNear, RV::zFar);
		CameraMat = glm::lookAt(glm::vec3{ 0,3,RV::zoom }, glm::vec3{ 0 ,3,0 }, glm::vec3{ 0.0,1.0,0.0 });
		break;

	}

	if (RV::posX >= RV::margin)
		RV::posX = -30;

	if (RV::rep <= clock()) {
		RV::fov = 65;
		RV::rep = clock() + 4000;
		RV::zoom = -10;
	}


	RV::posX += RV::travelingSpeed;
	RV::zoom += RV::zoomSpeed;
	RV::fov  += RV::fovSpeed;

	RV::FOV = glm::radians(RV::fov);
	
	RV::_MVP = RV::_projection * CameraMat; 



	//Render scene and GUI
	Cube::DrawScene(currentTime);
	ImGui::Render();

 }

void myCleanupCode(void) {

	Box::cleanupCube();
	Axis::cleanupAxis();
	Cube::cleanupCube();
 }



///////////////////////////////////////COMPILE AND LINK//////////////////////////////////////// 

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

////////////////////////////////////////////////// BOX
namespace Box {
	GLuint cubeVao;
	GLuint cubeVbo[2];
	GLuint cubeShaders[2];
	GLuint cubeProgram;

	float cubeVerts[] = {
		// -5,0,-5 -- 5, 10, 5
		-5.f,  0.f, -5.f,
		5.f,  0.f, -5.f,
		5.f,  0.f,  5.f,
		-5.f,  0.f,  5.f,
		-5.f, 10.f, -5.f,
		5.f, 10.f, -5.f,
		5.f, 10.f,  5.f,
		-5.f, 10.f,  5.f,
	};
	GLubyte cubeIdx[] = {
		1, 0, 2, 3, // Floor - TriangleStrip
		0, 1, 5, 4, // Wall - Lines
		1, 2, 6, 5, // Wall - Lines
		2, 3, 7, 6, // Wall - Lines
		3, 0, 4, 7  // Wall - Lines
	};

	const char* vertShader_xform =
		"#version 330\n\
in vec3 in_Position;\n\
uniform mat4 mvpMat;\n\
void main() {\n\
	gl_Position = mvpMat * vec4(in_Position, 1.0);\n\
}";
	const char* fragShader_flatColor =
		"#version 330\n\
out vec4 out_Color;\n\
uniform vec4 color;\n\
void main() {\n\
	out_Color = color;\n\
}";

	void setupCube() {
		glGenVertexArrays(1, &cubeVao);
		glBindVertexArray(cubeVao);
		glGenBuffers(2, cubeVbo);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, cubeVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeVbo[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 20, cubeIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		cubeShaders[0] = compileShader(vertShader_xform, GL_VERTEX_SHADER, "cubeVert");
		cubeShaders[1] = compileShader(fragShader_flatColor, GL_FRAGMENT_SHADER, "cubeFrag");

		cubeProgram = glCreateProgram();
		glAttachShader(cubeProgram, cubeShaders[0]);
		glAttachShader(cubeProgram, cubeShaders[1]);
		glBindAttribLocation(cubeProgram, 0, "in_Position");
		linkProgram(cubeProgram);
	}
	void cleanupCube() {
		glDeleteBuffers(2, cubeVbo);
		glDeleteVertexArrays(1, &cubeVao);

		glDeleteProgram(cubeProgram);
		glDeleteShader(cubeShaders[0]);
		glDeleteShader(cubeShaders[1]);
	}
	void drawCube() {
		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
		// FLOOR
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.6f, 0.6f, 0.6f, 1.f);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);
		// WALLS
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.f, 0.f, 0.f, 1.f);
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(sizeof(GLubyte) * 4));
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(sizeof(GLubyte) * 8));
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(sizeof(GLubyte) * 12));
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(sizeof(GLubyte) * 16));

		glUseProgram(0);
		glBindVertexArray(0);
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


////////////////////////////////////////////////// CUBE
namespace Cube {
	GLuint cubeVao;
	GLuint cubeVbo[3];
	GLuint cubeShaders[2];
	GLuint cubeProgram;
	glm::mat4 objMat;

	//Change color variable
	glm::vec4 myColor = { 0.0f, 0.5f, 1.0f, 1.0f }; 

	extern const float halfW = 0.5f;
	int numVerts = 24 + 6; // 4 vertex/face * 6 faces + 6 PRIMITIVE RESTART

						   //   4---------7
						   //  /|        /|
						   // / |       / |
						   //5---------6  |
						   //|  0------|--3
						   //| /       | /
						   //|/        |/
						   //1---------2
	glm::vec3 verts[] = {
		glm::vec3(-halfW, -halfW, -halfW),
		glm::vec3(-halfW, -halfW,  halfW),
		glm::vec3(halfW, -halfW,  halfW),
		glm::vec3(halfW, -halfW, -halfW),
		glm::vec3(-halfW,  halfW, -halfW),
		glm::vec3(-halfW,  halfW,  halfW),
		glm::vec3(halfW,  halfW,  halfW),
		glm::vec3(halfW,  halfW, -halfW)
	};
	glm::vec3 norms[] = {
		glm::vec3(0.f, -1.f,  0.f),
		glm::vec3(0.f,  1.f,  0.f),
		glm::vec3(-1.f,  0.f,  0.f),
		glm::vec3(1.f,  0.f,  0.f),
		glm::vec3(0.f,  0.f, -1.f),
		glm::vec3(0.f,  0.f,  1.f)
	};

	glm::vec3 cubeVerts[] = {
		verts[1], verts[0], verts[2], verts[3],
		verts[5], verts[6], verts[4], verts[7],
		verts[1], verts[5], verts[0], verts[4],
		verts[2], verts[3], verts[6], verts[7],
		verts[0], verts[4], verts[3], verts[7],
		verts[1], verts[2], verts[5], verts[6]
	};
	glm::vec3 cubeNorms[] = {
		norms[0], norms[0], norms[0], norms[0],
		norms[1], norms[1], norms[1], norms[1],
		norms[2], norms[2], norms[2], norms[2],
		norms[3], norms[3], norms[3], norms[3],
		norms[4], norms[4], norms[4], norms[4],
		norms[5], norms[5], norms[5], norms[5]
	};
	GLubyte cubeIdx[] = {
		0, 1, 2, 3, UCHAR_MAX,
		4, 5, 6, 7, UCHAR_MAX,
		8, 9, 10, 11, UCHAR_MAX,
		12, 13, 14, 15, UCHAR_MAX,
		16, 17, 18, 19, UCHAR_MAX,
		20, 21, 22, 23, UCHAR_MAX
	};




	const char* cube_vertShader =
		"#version 330\n\
	in vec3 in_Position;\n\
	in vec3 in_Normal;\n\
	out vec4 vert_Normal;\n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
		vert_Normal = mv_Mat * objMat * vec4(in_Normal, 0.0);\n\
	}";


	const char* cube_fragShader =
		"#version 330\n\
in vec4 vert_Normal;\n\
out vec4 out_Color;\n\
uniform mat4 mv_Mat;\n\
uniform vec4 color;\n\
void main() {\n\
	out_Color = vec4(color.xyz * dot(vert_Normal, mv_Mat*vec4(0.0, 1.0, 0.0, 0.0)) + color.xyz * 0.3, 1.0 );\n\
}";
	void setupCube() {
		glGenVertexArrays(1, &cubeVao);
		glBindVertexArray(cubeVao);
		glGenBuffers(3, cubeVbo);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNorms), cubeNorms, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glPrimitiveRestartIndex(UCHAR_MAX);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeVbo[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIdx), cubeIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		cubeShaders[0] = compileShader(cube_vertShader, GL_VERTEX_SHADER, "cubeVert");
		cubeShaders[1] = compileShader(cube_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		cubeProgram = glCreateProgram();
		glAttachShader(cubeProgram, cubeShaders[0]);
		glAttachShader(cubeProgram, cubeShaders[1]);
		glBindAttribLocation(cubeProgram, 0, "in_Position");
		glBindAttribLocation(cubeProgram, 1, "in_Normal");
		linkProgram(cubeProgram);
	}
	void cleanupCube() {
		glDeleteBuffers(3, cubeVbo);
		glDeleteVertexArrays(1, &cubeVao);

		glDeleteProgram(cubeProgram);
		glDeleteShader(cubeShaders[0]);
		glDeleteShader(cubeShaders[1]);
	}
	void updateCube(const glm::mat4& transform) {
		objMat = transform;
	}
	void drawCube() {
		glEnable(GL_PRIMITIVE_RESTART);
		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.1f, 1.f, 1.f, 0.f);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_PRIMITIVE_RESTART);
	}

	void DrawScene(double currentTime) {

		glEnable(GL_PRIMITIVE_RESTART);
		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);

		glm::vec4 newColor;

		////
		newColor = { 1.0f, 0.8f, 0.5f, 1.0f }; //Color selection
		Cube::updateColor(newColor);
		////


		glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)); //transformacio
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)); //Scale
		objMat = translate * scale;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		////

		objMat = glm::translate(glm::mat4(1.0f), glm::vec3(-1.25f,0.5f, -2.0f)); //Transfrom
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		////

		objMat = glm::translate(glm::mat4(1.0f), glm::vec3(1.25f, 0.5f, -2.0f)); //Transfrom
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		////

		////
		newColor = { 1.0f, 0.0f, 0.5f, 1.0f }; //seleccio de color
		Cube::updateColor(newColor);
		////

		objMat = glm::translate(glm::mat4(1.0f), glm::vec3(2.5f, 0.5f, 0.0f)); //Transfrom
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		////


		objMat = glm::translate(glm::mat4(1.0f), glm::vec3(-2.5f, 0.5f, 0.0f)); //Transfrom
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		////

		translate = glm::translate(glm::mat4(1.0f), glm::vec3(-1.25f, 0.5f, 1.5f)); //Transfrom
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 2.0f, 1.0f)); //Scale
		objMat = translate * scale;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		////

		translate = glm::translate(glm::mat4(1.0f), glm::vec3(1.25f, 0.5f, 1.5f)); //Transfrom
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 2.0f, 1.0f)); //Scale
		objMat = translate * scale;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		////

		translate = glm::translate(glm::mat4(1.0f), glm::vec3(2.5f, 0.5f, 2.5f)); //Transfrom
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 3.0f, 1.0f)); //Scale
		objMat = translate * scale;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		////

		translate = glm::translate(glm::mat4(1.0f), glm::vec3(-2.5f, 0.5f, 2.5f)); //Transfrom
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 3.0f, 1.0f)); //Scale
		objMat = translate * scale;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		////

		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_PRIMITIVE_RESTART);
	}

	void updateColor(const glm::vec4 newColor) {
		myColor = newColor;
	}


}