#include <windows.h>
#include <GL\glew.h>
#include <SDL2\SDL.h>
#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <cstdio>
#include "GL_framework.h"


/*extern void PhysicsInit();
extern void PhysicsUpdate(float dt);
extern void PhysicsCleanup();*/
extern void GUI();

extern void GLmousecb(MouseEvent ev);
extern void GLResize(int width, int height);
extern void GLinit(int width, int height);
extern void GLcleanup();
extern void GLrender(double currentTime);


int currentDayNight;
extern int currentBulb;
extern int currentExcercice;
extern int currentCameraShot;
extern bool light_moves_M;
extern bool sunActive_M;
extern bool moonActive_M;
extern bool bulbActive_M;
extern bool bulbMove_M;
extern bool secondWheel_M;
extern bool toon_M;
extern bool modelTranition_M;
extern bool contourOn_M;
extern bool contourTrump_M;
extern bool contourPollo_M;



extern void myRenderCode(double currentTime);
//extern void myCleanupCode(void);
//extern void myInitCode(void);


//////
namespace {
	const int expected_fps = 30;
	const double expected_frametime = 1.0 / expected_fps;
	const uint32_t expected_frametime_ms = (uint32_t) (1e3 * expected_frametime);
	uint32_t prev_frametimestamp = 0;
	uint32_t curr_frametimestamp = 0;

	void waitforFrameEnd() {
		curr_frametimestamp = SDL_GetTicks();
		DWORD wait = 0;
		if ((curr_frametimestamp - prev_frametimestamp) < expected_frametime_ms) {
			wait = expected_frametime_ms - (curr_frametimestamp - prev_frametimestamp);
		}
		if(wait > 0) {
			Sleep(wait);
		}
		prev_frametimestamp = SDL_GetTicks();
	}
}

int main(int argc, char** argv) {
	//Init GLFW
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		SDL_Quit();
		return -1;
	}
	// Create window
	SDL_Window *mainwindow;
	SDL_GLContext maincontext;

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);


	mainwindow = SDL_CreateWindow("GL_framework", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		if (!mainwindow) { /* Die if creation failed */
			SDL_Log("Couldn't create SDL window: %s", SDL_GetError());
			SDL_Quit();
			return -1;
		}

	/* Create our opengl context and attach it to our window */
	maincontext = SDL_GL_CreateContext(mainwindow);

	// Init GLEW
	GLenum err = glewInit();
	if(GLEW_OK != err) {
		SDL_Log("Glew error: %s\n", glewGetErrorString(err));
	}
	SDL_Log("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	// Disable V-Sync
	SDL_GL_SetSwapInterval(0);

	int display_w, display_h;
	SDL_GL_GetDrawableSize(mainwindow, &display_w, &display_h);
	// Init scene
	GLinit(display_w, display_h);
	//PhysicsInit();

	//myInitCode();

	
	// Setup ImGui binding
	ImGui_ImplSdlGL3_Init(mainwindow);

	currentExcercice = 0;
	currentCameraShot = 0; 
	currentDayNight = 0; 
	currentBulb = 0;
	light_moves_M = false;
	sunActive_M = false;
	moonActive_M = false;
	bulbActive_M = false;
	bulbMove_M = false; 
	secondWheel_M = false;
	toon_M = false;
	modelTranition_M = true;
	contourOn_M = false;
	contourPollo_M = false;

	bool quit_app = false;
	while (!quit_app) {
		SDL_Event eve;
		while (SDL_PollEvent(&eve)) {
			ImGui_ImplSdlGL3_ProcessEvent(&eve);
			switch (eve.type) {
			case SDL_WINDOWEVENT:
				if (eve.window.event == SDL_WINDOWEVENT_RESIZED) {
					GLResize(eve.window.data1, eve.window.data2);
				}
				break;
			case SDL_QUIT:
				quit_app = true;
				break;
			case SDL_KEYDOWN:
				switch (eve.key.keysym.sym)
				{
				case SDLK_a: //Following exercise
					currentExcercice++;
					break;
				case SDLK_z: //Previous exercise
					currentExcercice--;
					break;
				case SDLK_d: //Trigger Day-Night transition
					if(currentDayNight == 0)
						light_moves_M = !light_moves_M; 
					if (currentDayNight == 1)
						sunActive_M = !sunActive_M;
					if (currentDayNight == 2)
						moonActive_M = !moonActive_M;
					currentDayNight++;
					currentDayNight %= 3;
					break;
				case SDLK_b: //Trigger Bulb Light variants
					currentBulb++;
					currentBulb %= 3;
					if (currentBulb == 0) {
						bulbActive_M = false;
						bulbMove_M = false;
					}
					if (currentBulb == 1) {
						bulbActive_M = true;
						bulbMove_M = false;
					}
					if (currentBulb == 2) {
						bulbActive_M = true;
						bulbMove_M = true;
					}
					currentBulb++;
					currentBulb %= 3;
					break;
				case SDLK_t: //Trigger Toon Shading variants
					toon_M = !toon_M;
					contourPollo_M = !contourPollo_M;
					break;
				case SDLK_c: // Trigger Camara position variants
					currentCameraShot++;
					currentCameraShot %= 4;
					break;
				case SDLK_m: //Model transitioning
					modelTranition_M = !modelTranition_M;
					break;
				case SDLK_s: //Ever-falling wheel
					secondWheel_M = !secondWheel_M;
					contourOn_M = !contourOn_M;
					contourPollo_M = !contourPollo_M;
					break;
				default:
					break;
				}
				break;
			}

			//Excercice control
			if (currentExcercice == 13) currentExcercice = 0;
			if (currentExcercice < 0) currentExcercice = 0;
			switch (currentExcercice)
			{
			case 0:
				break;
			case 1:
				currentCameraShot = 0;
				modelTranition_M = true;
				contourOn_M = false;
				break;
			case 2:
				currentCameraShot = 0;
				modelTranition_M = false;
				break;
			case 3:
				currentCameraShot = 1;
				break;
			case 4:
				currentCameraShot = 3; 
				break;
			case 5:
				currentCameraShot = 0;
				light_moves_M = true;
				sunActive_M = true;
				moonActive_M = true;
				break;
			case 6:
				currentCameraShot = 0;
				light_moves_M = true;
				sunActive_M = false;
				moonActive_M = true;
				bulbActive_M = true;
				break;
			case 7:
				currentCameraShot = 1;
				light_moves_M = false;
				sunActive_M = false;
				moonActive_M = false;
				bulbActive_M = true;
			case 8:
				secondWheel_M = true;
				break;
			case 9:
				currentCameraShot = 1;
				toon_M = true;
				light_moves_M = true;
				sunActive_M = true;
				moonActive_M = false;
				secondWheel_M = false;
			case 10:
				toon_M = true;
				light_moves_M = true;
				sunActive_M = true;
				moonActive_M = true;
				secondWheel_M = false;
				break;
			case 11:
				toon_M = true;
				light_moves_M = false;
				sunActive_M = false;
				moonActive_M = true;
				secondWheel_M = false;
				contourOn_M = false;
				break;
			case 12:
				secondWheel_M = true;
				contourOn_M = true;

				break;
			default:
				break;
			}
		}
		ImGui_ImplSdlGL3_NewFrame(mainwindow);

		ImGuiIO& io = ImGui::GetIO();
		
		 
		GUI();
		//PhysicsUpdate((float)expected_frametime);
		if(!io.WantCaptureMouse) {
			MouseEvent ev = {io.MousePos.x, io.MousePos.y, 
				(io.MouseDown[0] ? MouseEvent::Button::Left : 
				(io.MouseDown[1] ? MouseEvent::Button::Right :
				(io.MouseDown[2] ? MouseEvent::Button::Middle :
				MouseEvent::Button::None)))};
			GLmousecb(ev);
		}


		double currentTime = (double)SDL_GetTicks() / 1000.0;
		GLrender(currentTime);
		
		//double currentTime = (double) SDL_GetTicks() / 1000.0;
		//myRenderCode(currentTime);
		


		SDL_GL_SwapWindow(mainwindow);
		waitforFrameEnd();
	}

	//myCleanupCode();

	GLcleanup();

	ImGui_ImplSdlGL3_Shutdown();
	SDL_GL_DeleteContext(maincontext);
	SDL_DestroyWindow(mainwindow);
	SDL_Quit();
	return 0;
}
