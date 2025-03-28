﻿/************************************************************************
	 File:        TrainView.cpp

	 Author:
				  Michael Gleicher, gleicher@cs.wisc.edu

	 Modifier
				  Yu-Chi Lai, yu-chi@cs.wisc.edu

	 Comment:
						The TrainView is the window that actually shows the
						train. Its a
						GL display canvas (Fl_Gl_Window).  It is held within
						a TrainWindow
						that is the outer window with all the widgets.
						The TrainView needs
						to be aware of the window - since it might need to
						check the widgets to see how to draw

	  Note:        we need to have pointers to this, but maybe not know
						about it (beware circular references)

	 Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#include <iostream>
#include <algorithm>
#include <time.h>
#include <chrono>	// for random
#include <Fl/fl.h>

// we will need OpenGL, and OpenGL needs windows.h
#include <windows.h>
//#include "GL/gl.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "GL/glu.h"

#include "TrainView.H"
#include "TrainWindow.H"
#include "Utilities/3DUtils.H"

#include "MathHelper.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define SIMPLE_OBJECT_VERT_PATH "assets/shaders/simpleObject.vert"
#define SIMPLE_OBJECT_FRAG_PATH "assets/shaders/simpleObject.frag"
#define INSTANCE_OBJECT_VERT_PATH "assets/shaders/instanceObject.vert"
#define PIER_FRAG_PATH "assets/shaders/pier.frag"
#define SMOKE_VERT_PATH "assets/shaders/smoke.vert"
#define SMOKE_FRAG_PATH "assets/shaders/smoke.frag"
#define WATER_VERT_PATH "assets/shaders/water.vert"
#define WATER_FRAG_PATH "assets/shaders/water.frag"
#define MODEL_VERT_PATH "assets/shaders/model_loading.vert"
#define MODEL_FRAG_PATH "assets/shaders/model_loading.frag"
#define PARTICLE_VERT_PATH "assets/shaders/particle.vert"
#define PARTICLE_FRAG_PATH "assets/shaders/particle.frag"
#define ELLIPTICAL_PARTICLE_VERT_PATH "assets/shaders/ellipticalParticle.vert"
#define ELLIPTICAL_PARTICLE_FRAG_PATH "assets/shaders/ellipticalParticle.frag"
#define FRAME_VERT_PATH "assets/shaders/frame.vert"
#define FRAME_FRAG_PATH "assets/shaders/frame.frag"
#define WHITELINE_VERT_PATH "assets/shaders/whiteLine.vert"
#define WHITELINE_FRAG_PATH "assets/shaders/whiteLine.frag"
#define DRILL_VERT_PATH "assets/shaders/drill.vert"
#define DRILL_FRAG_PATH "assets/shaders/drill.frag"
#define SPEEDBG_VERT_PATH "assets/shaders/speedBg.vert"
#define SPEEDBG_FRAG_PATH "assets/shaders/speedBg.frag"
#define INSTANCE_SHADOW_VERT_PATH "assets/shaders/instanceObjectShadow.vert"
#define MODEL_SHADOW_VERT_PATH "assets/shaders/model_loading_shadow.vert"
#define OBJ_SHADOW_FRAG_PATH "assets/shaders/simpleObjectshadow.frag"
#define ISLAND_HEIGHT_VERT_PATH "assets/shaders/islandHeight.vert"
#define ISLAND_HEIGHT_FRAG_PATH "assets/shaders/islandHeight.frag"
#define SKYBOX_VERT_PATH "assets/shaders/skyBox.vert"
#define SKYBOX_FRAG_PATH "assets/shaders/skyBox.frag"

//3D models path
#define WATER_HEIGHT_PATH "assets/images/waterHeight/"
#define WATER_NORMAL_PATH "assets/images/waterNormal/"
#define OBJECT_TEXTURE_PATH "assets/images/objectTexture/"
#define BACKPACK_PATH "assets/model/backpack/backpack.obj"
#define ISLAND_PATH "assets/model/island/floating_island.obj"
#define STONE_PILLAR_PATH "assets/model/stone_pillar/stone_pillar.obj"
#define STONE_PILLAR_SECTION_PATH "assets/model/stone_pillar_section/stone_pillar_section.obj"
#define ARROW_RED_PATH "assets/model/arrow_red/arrow_boi.obj"
#define ARROW_BLUE_PATH "assets/model/arrow_blue/arrow_boi.obj"
#define CIRNO_PATH "assets/model/Cirno/cirno_fumo_3d_scan.obj"
#define TANK_PATH "assets/model/tank/tank.obj"
#define CANNON_PATH "assets/model/cannon/cannon.obj"

//sound path
#define RPG_FIRE_PATH "assets/Audios/audio_player_missile_fire_1_edited.wav"
#define SLOW_MOTION_START_PATH "assets/Audios/slowMotionStart.wav"
#define SLOW_MOTION_END_PATH "assets/Audios/slowMotionEnd.wav"
#define TARGET_EXPLOTION_PATH "assets/Audios/Explosion.wav"
#define GIGA_DRILL_BREAK_PATH "assets/Audios/GigaDrillBreak.wav"

std::vector<std::string> SKYBOX_PATH = {
	"assets/images/skybox/right.jpg",
	"assets/images/skybox/left.jpg",
	"assets/images/skybox/top.jpg",
	"assets/images/skybox/bottom.jpg",
	"assets/images/skybox/front.jpg",
	"assets/images/skybox/back.jpg"
};

#define WATER_RESOLUTION 100

#define USE_MODEL true
#define USE_WATER_ANIMATION true
Assimp::Importer importer;

//************************************************************************
//
// * Constructor to set up the GL window
//========================================================================
TrainView::
TrainView(int x, int y, int w, int h, const char* l)
	: Fl_Gl_Window(x, y, w, h, l)
	//========================================================================
{
	mode(FL_RGB | FL_ALPHA | FL_DOUBLE | FL_STENCIL);

	resetArcball();
	freeCamera.setWindow(this);
	srand(static_cast<unsigned>(time(0)));

	//set the executable file path
	exePath = getExecutableDir();

	//sound
	soundDevice = SoundDevice::get();
	RPGshot = SoundBuffer::get()->addSoundEffect((exePath + RPG_FIRE_PATH).c_str());
	slowMotionStart = SoundBuffer::get()->addSoundEffect((exePath + SLOW_MOTION_START_PATH).c_str());
	slowMotionEnd = SoundBuffer::get()->addSoundEffect((exePath + SLOW_MOTION_END_PATH).c_str());
	targetExplosion = SoundBuffer::get()->addSoundEffect((exePath + TARGET_EXPLOTION_PATH).c_str() );
	GDBEffect = SoundBuffer::get()->addSoundEffect((exePath + GIGA_DRILL_BREAK_PATH).c_str());
	soundSource_RPGshot = new SoundSource();
	soundSource_slowMotionStart = new SoundSource();
	soundSource_slowMotionEnd = new SoundSource();
	soundSource_targetExplosion = new SoundSource();
	soundSource_GDBEffect = new SoundSource();

	//set skybox path
	for (int i = 0; i < SKYBOX_PATH.size(); i++) {
		SKYBOX_PATH[i] = exePath + SKYBOX_PATH[i];
	}
}

//************************************************************************
//
// * Reset the camera to look at the world
//========================================================================
void TrainView::
resetArcball()
//========================================================================
{
	// Set up the camera to look at the world
	// these parameters might seem magical, and they kindof are
	// a little trial and error goes a long way
	arcball.setup(this, 40, 250, .2f, .4f, 0);
}
//************************************************************************
//
// * FlTk Event handler for the window
//########################################################################
// TODO: 
//       if you want to make the train respond to other events 
//       (like key presses), you might want to hack this.
//########################################################################
//========================================================================
void TrainView::aim(bool draging) {
	float currentX, currentY;
	arcball.getMouseNDC(currentX, currentY);
	camRotateX += (currentX - lastX) * 2.5;
	camRotateY += (currentY - lastY) * 2.5;
	if (camRotateY >= 1.57) camRotateY = 1.57;
	if (camRotateY <= -1.57) camRotateY = -1.57;
	if (draging) {
		if ((MathHelper::quadrant(lastX, lastY) - MathHelper::quadrant(currentX, currentY) + 4) % 4 == 1) {
			// rotate clock wise
			SpiralPower += 0.25;
		}
		else if (SpiralPower < 3 && MathHelper::quadrant(lastX, lastY) != MathHelper::quadrant(currentX, currentY))
			SpiralPower = 0;
	}
	else
		SpiralPower = 0;
	lastX = currentX;
	lastY = currentY;
}

int TrainView::handle(int event)
{
	// see if the ArcBall will handle the event - if it does, 
	// then we're done
	// note: the arcball only gets the event if we're in world view
	if (tw->worldCam->value())
		if (arcball.handle(event))
			return 1;

	// free camera
	if (tw->freeCam->value()) {
		if (freeCamera.handle(event)) {
			return 1;
		}
	}

	// remember what button was used
	static int last_push;

	switch (event) {
		// Mouse button being pushed event
	case FL_PUSH:
		last_push = Fl::event_button();
		// if the left button be pushed is left mouse button
		if (last_push == FL_LEFT_MOUSE) {
			if (tw->worldCam->value()|| tw->topCam->value() || tw->freeCam->value()) {
				doPick();
				damage(1);
				return 1;
			}
			else if(tw->trainCam->value()) {
				shoot();
			}
		};
		if (Fl::event_button() == FL_RIGHT_MOUSE && tw->trainCam->value() && Fl::event_clicks()) {
			// reset train pov
			camRotateX = 0;
			camRotateY = 0;
		}
		if (Fl::event_button() == FL_RIGHT_MOUSE && tw->trainCam->value()) {
			RenderDatabase::timeScale = RenderDatabase::BULLET_TIME_SCALE;
			soundSource_slowMotionStart->Play(slowMotionStart);
		}
		//break;

		// Mouse button release event
	case FL_RELEASE: // button release
		damage(1);
		last_push = 0;
		if (tw->trainCam->value()) {
			if (Fl::event_button() == FL_RIGHT_MOUSE && event != FL_PUSH) {
				RenderDatabase::timeScale = RenderDatabase::INIT_TIME_SCALE;
				soundSource_slowMotionEnd->Play(slowMotionEnd);
			}
			else if (SpiralPower >= 3 && Fl::event_button() == FL_LEFT_MOUSE && event != FL_PUSH) {
				// giga drill break!
				if (animationFrame == 0) {
					staticSpiralPower = SpiralPower;
					animationFrame = 1;
				}
			}
		}
		return 1;

		// Mouse button drag event
	case FL_DRAG:
		if (!tw->trainCam->value()) {
			// Compute the new control point position
			if ((last_push == FL_LEFT_MOUSE) && (selectedCube >= 0)) {
				ControlPoint* cp = &m_pTrack->points[selectedCube];

				double r1x, r1y, r1z, r2x, r2y, r2z;
				getMouseLine(r1x, r1y, r1z, r2x, r2y, r2z);

				double rx, ry, rz;
				mousePoleGo(r1x, r1y, r1z, r2x, r2y, r2z,
					static_cast<double>(cp->pos.x),
					static_cast<double>(cp->pos.y),
					static_cast<double>(cp->pos.z),
					rx, ry, rz,
					(Fl::event_state() & FL_CTRL) != 0);

				cp->pos.x = (float)rx;
				cp->pos.y = (float)ry;
				cp->pos.z = (float)rz;
				damage(1);
			}
		}
		else {
			aim(1);
		}
		break;

		// in order to get keyboard events, we need to accept focus
	case FL_FOCUS:
		return 1;

		// every time the mouse enters this window, aggressively take focus
	case FL_ENTER:
		focus(this);
		break;

	case FL_KEYBOARD:
	{
		int k = Fl::event_key();
		int ks = Fl::event_state();
		if (k == 'p') {
			// Print out the selected control point information
			if (selectedCube >= 0)
				printf("Selected(%d) (%g %g %g) (%g %g %g)\n",
					selectedCube,
					m_pTrack->points[selectedCube].pos.x,
					m_pTrack->points[selectedCube].pos.y,
					m_pTrack->points[selectedCube].pos.z,
					m_pTrack->points[selectedCube].orient.x,
					m_pTrack->points[selectedCube].orient.y,
					m_pTrack->points[selectedCube].orient.z);
			else
				printf("Nothing Selected\n");

			return 1;
		};
		break;
		// Aim with a rocket launcher
	}
	case FL_MOVE:
		if (tw->trainCam->value()) {
			aim(0);
		}
	}

	return Fl_Gl_Window::handle(event);
}

//init shader, texture, trainModel, VAO. need called under if(gladLoadGL())
void TrainView::initRander() {
	//init shader
	simpleObjectShader = new Shader((exePath + SIMPLE_OBJECT_VERT_PATH).c_str(), (exePath + SIMPLE_OBJECT_FRAG_PATH).c_str());
	simpleInstanceObjectShader = new Shader((exePath + INSTANCE_OBJECT_VERT_PATH).c_str(), (exePath + SIMPLE_OBJECT_FRAG_PATH).c_str());
	pierShader = new Shader((exePath + INSTANCE_OBJECT_VERT_PATH).c_str(), (exePath + PIER_FRAG_PATH).c_str());
	whiteLineShader = new Shader((exePath + WHITELINE_VERT_PATH).c_str(), (exePath + WHITELINE_FRAG_PATH).c_str());
	drillShader = new Shader((exePath + DRILL_VERT_PATH).c_str(), (exePath + DRILL_FRAG_PATH).c_str());
	waterShader = new Shader((exePath + WATER_VERT_PATH).c_str(), (exePath + WATER_FRAG_PATH).c_str());
	smokeShader = new Shader((exePath + SMOKE_VERT_PATH).c_str(), (exePath + SMOKE_FRAG_PATH).c_str());
	modelShader = new Shader((exePath + MODEL_VERT_PATH).c_str(), (exePath + MODEL_FRAG_PATH).c_str());
	particleShader = new Shader((exePath + PARTICLE_VERT_PATH).c_str(), (exePath + PARTICLE_FRAG_PATH).c_str());
	ellipticalParticleShader = new Shader((exePath + ELLIPTICAL_PARTICLE_VERT_PATH).c_str(), (exePath + ELLIPTICAL_PARTICLE_FRAG_PATH).c_str());
	speedBgShader = new Shader((exePath + SPEEDBG_VERT_PATH).c_str(), (exePath + SPEEDBG_FRAG_PATH).c_str());
	frameShader = new Shader((exePath + FRAME_VERT_PATH).c_str(), (exePath + FRAME_FRAG_PATH).c_str());
	instanceShadowShader = new Shader((exePath + INSTANCE_SHADOW_VERT_PATH).c_str(), (exePath + OBJ_SHADOW_FRAG_PATH).c_str());
	modelShadowShader = new Shader((exePath + MODEL_SHADOW_VERT_PATH).c_str(), (exePath + OBJ_SHADOW_FRAG_PATH).c_str());
	islandHeightShader = new Shader((exePath + ISLAND_HEIGHT_VERT_PATH).c_str(), (exePath + ISLAND_HEIGHT_FRAG_PATH).c_str());
	skyboxShader = new Shader((exePath + SKYBOX_VERT_PATH).c_str(), (exePath + SKYBOX_FRAG_PATH).c_str());

	//init texture
	printf("Loading texture...\n");
	float textureStart = std::clock();
	if (USE_WATER_ANIMATION) {
		for (int i = 0; i < 200; i++) {
			std::string zero = "00";
			if (i >= 10 && i < 100) zero = "0";
			if (i >= 100) zero = "";
			waterHeightMap[i] = RenderDatabase::loadTexture(exePath + WATER_HEIGHT_PATH + (zero + std::to_string(i) + ".png"));
		}
		for (int i = 0; i < 200; i++) {
			std::string zero = "00";
			if (i >= 10 && i < 100) zero = "0";
			if (i >= 100) zero = "";
			waterNormalMap[i] = RenderDatabase::loadTexture(exePath + WATER_NORMAL_PATH + (zero + std::to_string(i) + "_normal.png"));
		}
	}

	skyboxTexture = RenderDatabase::loadCubemap(SKYBOX_PATH);

	printf("Loading texture done. (use %.2fs)\n", (std::clock() - textureStart) / 1000);

	// set object textures
	setObjectTexture("targetImage", "targetImage.png");
	setObjectTexture("drillImage", "drillImage2.png");
	setObjectTexture("crosshair", "crosshair.png");
	setObjectTexture("speedBg", "speed_bg.png");

	//init unifrom block index
	//0 for view and project matrix
	simpleObjectShader->setBlock("Matrices", 0);
	simpleInstanceObjectShader->setBlock("Matrices", 0);
	pierShader->setBlock("Matrices", 0);
	whiteLineShader->setBlock("Matrices", 0);
	drillShader->setBlock("Matrices", 0);
	waterShader->setBlock("Matrices", 0);
	smokeShader->setBlock("Matrices", 0);
	modelShader->setBlock("Matrices", 0);
	particleShader->setBlock("Matrices", 0);
	ellipticalParticleShader->setBlock("Matrices", 0);
	speedBgShader->setBlock("Matrices", 0);
	instanceShadowShader->setBlock("Matrices", 0);
	modelShadowShader->setBlock("Matrices", 0);
	islandHeightShader->setBlock("Matrices", 0);
	skyboxShader->setBlock("Matrices", 0);

	//set ubo
	//0 for view and project matrix
	glGenBuffers(1, &uboMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

	// set some parameters
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);

	// set VAO and VBO
	setCube();
	setHollowCube();
	setCone();
	setCylinder();
	setSector();
	setWater();
	setSmoke();
	setSkybox();
	setFBOs();
	glGenVertexArrays(1, &particle);

	// set Model
	if (USE_MODEL) {
		printf("Loading model...\n");
		float start = std::clock();

		backpack = new Model(exePath + BACKPACK_PATH);
		island = new Model(exePath + ISLAND_PATH);
		stonePillar = new Model(exePath + STONE_PILLAR_PATH);
		stonePillarSection = new Model(exePath + STONE_PILLAR_SECTION_PATH);
		arrow_red = new Model(exePath + ARROW_RED_PATH);
		arrow_blue = new Model(exePath + ARROW_BLUE_PATH);
		Cirno = new Model(exePath + CIRNO_PATH);
		tank = new Model(exePath + TANK_PATH);
		cannon = new Model(exePath + CANNON_PATH);

		printf("Loading model done. (use %.2fs)\n", (std::clock() - start) / 1000);
	}

	//init particle system, need call after generate particle VAO
	particleSystem.setParticleVAO(particle);

	//color axis spring
	ParticleGenerator& g = particleSystem.addParticleGenerator(particleShader);
	g.setColor(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), 0.5);
	g.setParticleSize(0.5);
	g.setAngle(30);
	if (USE_MODEL) {
		g.setPosition(glm::vec3(0, 60, 0));
		g.setParticleLife(90);
		g.setGenerateRate(20);
	}
	else {
		g.setPosition(glm::vec3(0, 20, 0));
		g.setParticleLife(60);
	}

	g.setGravity(0.04);

	ParticleGenerator& g2 = particleSystem.addParticleGenerator(particleShader);
	if (USE_MODEL) {
		g2.setPosition(glm::vec3(0, 65, 0));
	}
	else {
		g2.setPosition(glm::vec3(0, 40, 0));
	}
	g2.setLife(5);
	g2.setParticleVelocity(2);
	g2.setParticleLife(400);
	g2.setGenerateRate(30);
	g2.setGravity(0.04);

	trainParticle1 = particleSystem.addParticleGenerator_pointer(ellipticalParticleShader);
	trainParticle2 = particleSystem.addParticleGenerator_pointer(ellipticalParticleShader);
}

//cube
void TrainView::setCube()
{
	GLfloat cubeVertices[] = {
		//down
		0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		//front -z
		0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		//right +x
		0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, 0.5f,
		//back
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		//left
		-0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		//top +y
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, 0.5f
	};
	GLfloat cubeNormal[] = {
		//down
		0, -1.0f, 0,
		0, -1.0f, 0,
		0, -1.0f, 0,
		0, -1.0f, 0,
		//front -z
		0, 0, -1.0f,
		0, 0, -1.0f,
		0, 0, -1.0f,
		0, 0, -1.0f,
		//right +x
		1.0f, 0, 0,
		1.0f, 0, 0,
		1.0f, 0, 0,
		1.0f, 0, 0,
		//back
		0, 0, 1.0f,
		0, 0, 1.0f,
		0, 0, 1.0f,
		0, 0, 1.0f,
		//left
		-1.0f, 0, 0,
		-1.0f, 0, 0,
		-1.0f, 0, 0,
		-1.0f, 0, 0,
		//top +y
		0, 1.0f, 0,
		0, 1.0f, 0,
		0, 1.0f, 0,
		0, 1.0f, 0
	};
	GLuint cubeElement[] = {
		//down
		0, 1, 2,
		0, 2, 3,
		//front -z
		4, 5, 6,
		4, 6, 7,
		//right +x
		8, 9, 10,
		8, 10, 11,
		//back
		12, 13, 14,
		12, 14, 15,
		//left
		16, 17, 18,
		16, 18, 19,
		//top +y
		20, 21, 22,
		20, 22, 23,
	};
	glGenVertexArrays(1, &cube.VAO);
	glGenBuffers(2, cube.VBO);
	glGenBuffers(1, &cube.EBO);
	glBindVertexArray(cube.VAO);
	cube.element_amount = sizeof(cubeElement) / sizeof(GLuint);
	// Position attribute
	glBindBuffer(GL_ARRAY_BUFFER, cube.VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Normal attribute
	glBindBuffer(GL_ARRAY_BUFFER, cube.VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormal), cubeNormal, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	//Element attribute
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeElement), cubeElement, GL_STATIC_DRAW);
	// Unbind VAO
	glBindVertexArray(0);
}

// a cube but its face and back are removed
void TrainView::setHollowCube()
{
	GLfloat hollowCubeVertices[] = {
		//down
		0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		//front -z
		//0.5f, -0.5f, -0.5f,
		//-0.5f, -0.5f, -0.5f,
		//-0.5f, 0.5f, -0.5f,
		//0.5f, 0.5f, -0.5f,
		//right +x
		0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, 0.5f,
		//back
		//-0.5f, -0.5f, 0.5f,
		//0.5f, -0.5f, 0.5f,
		//0.5f, 0.5f, 0.5f,
		//-0.5f, 0.5f, 0.5f,
		//left
		-0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		//top +y
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, 0.5f
	};
	GLfloat hollowCubeNormal[] = {
		//down
		0, -1.0f, 0,
		0, -1.0f, 0,
		0, -1.0f, 0,
		0, -1.0f, 0,
		//front -z
		//0, 0, -1.0f,
		//0, 0, -1.0f,
		//0, 0, -1.0f,
		//0, 0, -1.0f,
		//right +x
		1.0f, 0, 0,
		1.0f, 0, 0,
		1.0f, 0, 0,
		1.0f, 0, 0,
		//back
		//0, 0, 1.0f,
		//0, 0, 1.0f,
		//0, 0, 1.0f,
		//0, 0, 1.0f,
		//left
		-1.0f, 0, 0,
		-1.0f, 0, 0,
		-1.0f, 0, 0,
		-1.0f, 0, 0,
		//top +y
		0, 1.0f, 0,
		0, 1.0f, 0,
		0, 1.0f, 0,
		0, 1.0f, 0
	};
	GLuint hollowCubeElement[] = {
		//down
		0, 1, 2,
		0, 2, 3,
		//front -z
		//4, 5, 6,
		//4, 6, 7,
		//right +x
		8 - 4, 9 - 4, 10 - 4,
		8 - 4, 10 - 4, 11 - 4,
		//back
		//12, 13, 14,
		//12, 14, 15,
		//left
		16 - 8, 17 - 8, 18 - 8,
		16 - 8, 18 - 8, 19 - 8,
		//top +y
		20 - 8, 21 - 8, 22 - 8,
		20 - 8, 22 - 8, 23 - 8,
	};
	glGenVertexArrays(1, &hollowCube.VAO);
	glGenBuffers(2, hollowCube.VBO);
	glGenBuffers(1, &hollowCube.EBO);
	glBindVertexArray(hollowCube.VAO);
	hollowCube.element_amount = sizeof(hollowCubeElement) / sizeof(GLuint);
	// Position attribute
	glBindBuffer(GL_ARRAY_BUFFER, hollowCube.VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(hollowCubeVertices), hollowCubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Normal attribute
	glBindBuffer(GL_ARRAY_BUFFER, hollowCube.VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(hollowCubeNormal), hollowCubeNormal, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	//Element attribute
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hollowCube.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(hollowCubeElement), hollowCubeElement, GL_STATIC_DRAW);
	// Unbind VAO
	glBindVertexArray(0);
}

// cylinder, (w,h,l) = (1,1,1), face(top) to -z
void TrainView::setCylinder()
{
	int vn = circleVerticesNumber;
	GLfloat cylinderVertices[circleVerticesNumber * 12];
	GLfloat cylinderNormal[circleVerticesNumber * 12];
	GLfloat cylindertexCoord[circleVerticesNumber * 8];
	GLuint cylinderElement[(circleVerticesNumber * 2 + (circleVerticesNumber - 2) * 2) * 3];

	float pi = 3.1415926535;
	// top vertices * 2
	for (int i = 0; i < vn * 2; i += 1) {
		int index = i * 3;
		float unitAngle = (float)2 * pi / vn;
		cylinderVertices[index] = 0.5 * sin((float)unitAngle * (i % vn));
		cylinderVertices[index + 1] = 0.5 * cos((float)unitAngle * (i % vn));
		cylinderVertices[index + 2] = -0.5;
	}
	// buttom vertices * 2
	for (int i = vn * 2; i < vn * 4; i += 1) {
		int index = i * 3;
		float unitAngle = (float)2 * pi / vn;
		cylinderVertices[index] = 0.5 * sin((float)unitAngle * (i % vn));
		cylinderVertices[index + 1] = 0.5 * cos((float)unitAngle * (i % vn));
		cylinderVertices[index + 2] = 0.5;
	}
	// top normal
	for (int i = 0; i < vn; i += 1) {
		int index = i * 3;
		float unitAngle = (float)2 * pi / vn;
		cylinderNormal[index] = 0;
		cylinderNormal[index + 1] = 0;
		cylinderNormal[index + 2] = -1;
	}

	// bottom normal
	for (int i = vn * 3; i < vn * 4; i += 1) {
		int index = i * 3;
		float unitAngle = (float)2 * pi / vn;
		cylinderNormal[index] = 0;
		cylinderNormal[index + 1] = 0;
		cylinderNormal[index + 2] = 1;
	}
	// side normal
	for (int i = vn * 1; i < vn * 3; i += 1) {
		int index = i * 3;
		float unitAngle = (float)2 * pi / vn;
		cylinderNormal[index] = 1 * sin((float)unitAngle * (i % vn));
		cylinderNormal[index + 1] = 1 * cos((float)unitAngle * (i % vn));
		cylinderNormal[index + 2] = 0;
	}

	// top texCoord
	for (int i = 0; i < vn; i += 1) {
		int index = i * 2;
		float unitAngle = (float)2 * pi / vn;
		cylindertexCoord[index] = 0.25 + 0.25 * sin((float)unitAngle * (i % vn));
		cylindertexCoord[index + 1] = 0.75 + 0.25 * -cos((float)unitAngle * (i % vn));
	}

	// bottom texCoord
	for (int i = vn * 3; i < vn * 4; i += 1) {
		int index = i * 2;
		float unitAngle = (float)2 * pi / vn;
		cylindertexCoord[index] = 0.75 + 0.25 * sin((float)unitAngle * (i % vn));
		cylindertexCoord[index + 1] = 0.75 + 0.25 * -cos((float)unitAngle * (i % vn));
	}
	// side texCoord
	for (int i = vn * 1; i < vn * 3; i += 1) {
		int index = i * 2;
		float unitLength = (float)1 / vn;
		cylindertexCoord[index] = (float)unitLength * (i % vn);
		if (i < vn * 2)	// upper side
			cylindertexCoord[index + 1] = 0.5;
		else            // lower side
			cylindertexCoord[index + 1] = 0;
	}
	// side surface
	for (int i = vn; i < vn * 2; i++) {
		int index = (i - vn) * 6;
		cylinderElement[index] = i;
		cylinderElement[index + 2] = vn + i;
		cylinderElement[index + 3] = vn + i;
		if (i + 1 < 2 * vn) {
			cylinderElement[index + 1] = i + 1;
			cylinderElement[index + 4] = vn + i + 1;
			cylinderElement[index + 5] = i + 1;
		}
		else {
			cylinderElement[index + 1] = vn;
			cylinderElement[index + 4] = vn * 2;
			cylinderElement[index + 5] = vn;
		}
	}
	// top surface
	for (int i = 0; i < vn - 2; i++) {
		int index = (vn * 2 + i) * 3;
		cylinderElement[index] = 0;
		cylinderElement[index + 1] = i + 1;
		cylinderElement[index + 2] = i + 2;
	}
	// buttom surface
	for (int i = vn * 3; i < vn * 4 - 2; i++) {
		int index = (vn * 3 + (i - vn * 3) - 2) * 3;
		cylinderElement[index] = vn * 3;
		cylinderElement[index + 1] = i + 1;
		cylinderElement[index + 2] = i + 2;
	}

	glGenVertexArrays(1, &cylinder.VAO);
	glGenBuffers(3, cylinder.VBO);
	glGenBuffers(1, &cylinder.EBO);
	glBindVertexArray(cylinder.VAO);
	cylinder.element_amount = sizeof(cylinderElement) / sizeof(GLuint);
	// Position attribute
	glBindBuffer(GL_ARRAY_BUFFER, cylinder.VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cylinderVertices), cylinderVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Normal attribute
	glBindBuffer(GL_ARRAY_BUFFER, cylinder.VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cylinderNormal), cylinderNormal, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	// Image texture attribute
	glBindBuffer(GL_ARRAY_BUFFER, cylinder.VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cylindertexCoord), cylindertexCoord, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);
	//Element attribute
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinder.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cylinderElement), cylinderElement, GL_STATIC_DRAW);
	// Unbind VAO
	glBindVertexArray(0);
}

// cone, (w,h,l) = (1,1,1), face(top) to -z
void TrainView::setCone()
{
	int vn = circleVerticesNumber;
	GLfloat coneVertices[circleVerticesNumber * 9];
	GLfloat coneNormal[circleVerticesNumber * 9];
	GLfloat conetexCoord[circleVerticesNumber * 6];
	GLuint coneElement[(circleVerticesNumber * 2 + (circleVerticesNumber - 2)) * 3];
	float pi = 3.1415926535;
	// top vertices
	for (int i = 0; i < vn; i += 1) {
		int index = i * 3;
		float unitAngle = (float)2 * pi / vn;
		coneVertices[index] = 0;
		coneVertices[index + 1] = 0;
		coneVertices[index + 2] = -0.5;
	}
	// buttom vertices * 2
	for (int i = vn; i < vn * 3; i += 1) {
		int index = i * 3;
		float unitAngle = (float)2 * pi / vn;
		coneVertices[index] = 0.5 * sin((float)unitAngle * (i % vn));
		coneVertices[index + 1] = 0.5 * cos((float)unitAngle * (i % vn));
		coneVertices[index + 2] = 0.5;
	}

	// side normal
	for (int i = 0; i < vn; i += 1) {
		int index = i * 3;
		float unitAngle = (float)2 * pi / vn;
		coneNormal[index] = 0.866 * sin((float)unitAngle * i);
		coneNormal[index + 1] = 0.866 * cos((float)unitAngle * i);
		coneNormal[index + 2] = -0.5;
	}
	for (int i = vn; i < vn * 2; i += 1) {
		int index = i * 3;
		float unitAngle = (float)2 * pi / vn;
		coneNormal[index] = 0.866 * sin((float)unitAngle * i);
		coneNormal[index + 1] = 0.866 * cos((float)unitAngle * i);
		coneNormal[index + 2] = -0.5;
	}
	// buttom normal
	for (int i = vn * 2; i < vn * 3; i += 1) {
		int index = i * 3;
		coneNormal[index] = 0;
		coneNormal[index + 1] = 0;
		coneNormal[index + 2] = 1;
	}
	// top texture
	for (int i = 0; i < vn; i += 1) {
		int index = i * 2;
		conetexCoord[index] = 0.25;
		conetexCoord[index + 1] = 0.75;
	}
	// buttom(side) texture 
	for (int i = vn; i < vn * 2; i += 1) {
		int index = i * 2;
		float unitAngle = (float)2 * pi / vn;
		conetexCoord[index] = 0.25 + 0.25 * sin((float)unitAngle * i);
		conetexCoord[index + 1] = 0.75 + 0.25 * cos((float)unitAngle * i);
	}
	// buttom(really buttom) texture
	for (int i = vn * 2; i < vn * 3; i += 1) {
		int index = i * 2;
		float unitAngle = (float)2 * pi / vn;
		conetexCoord[index] = 0.75 + 0.25 * sin((float)unitAngle * i);
		conetexCoord[index + 1] = 0.75 + 0.25 * cos((float)unitAngle * i);
	}
	// side surface
	for (int i = 0; i < vn; i++) {
		int index = i * 3;
		coneElement[index] = i;
		coneElement[index + 1] = vn + i;
		if (i + 1 < vn)
			coneElement[index + 2] = vn + i + 1;
		else
			coneElement[index + 2] = vn;
	}
	// buttom surface
	for (int i = vn * 2; i < vn * 3 - 2; i++) {
		int index = (i - vn) * 3;
		coneElement[index] = vn * 2;
		coneElement[index + 1] = i + 1;
		coneElement[index + 2] = i + 2;
	}

	glGenVertexArrays(1, &cone.VAO);
	glGenBuffers(2, cone.VBO);
	glGenBuffers(1, &cone.EBO);
	glBindVertexArray(cone.VAO);
	cone.element_amount = sizeof(coneElement) / sizeof(GLuint);
	// Position attribute
	glBindBuffer(GL_ARRAY_BUFFER, cone.VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(coneVertices), coneVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Normal attribute
	glBindBuffer(GL_ARRAY_BUFFER, cone.VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(coneNormal), coneNormal, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	// Image texture attribute
	glBindBuffer(GL_ARRAY_BUFFER, cylinder.VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(conetexCoord), conetexCoord, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);
	//Element attribute
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cone.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(coneElement), coneElement, GL_STATIC_DRAW);
	// Unbind VAO
	glBindVertexArray(0);
}

// one-third sector, the angle is not fixed, (w,h,l) = (?,0.5,1), face to -z
// this is so unfixed that its use is not recommended
void TrainView::setSector()
{
	const int vn = circleVerticesNumber / 3 + 1;
	GLfloat sectorVertices[vn * 12];
	GLfloat sectorNormal[vn * 12];
	GLfloat sectortexCoord[vn * 8];
	GLuint sectorElement[(vn * 2 + (vn - 2) * 2) * 3];

	float pi = 3.1415926535;
	// top vertices * 2
	for (int i = 0; i < vn * 2; i += 1) {
		int index = i * 3;
		if (i % vn == 0) {
			// center
			sectorVertices[index] = 0;
			sectorVertices[index + 1] = 0;
			sectorVertices[index + 2] = -0.5;
		}
		else {
			// arc
			float unitAngle = (float)2 * pi / circleVerticesNumber;
			sectorVertices[index] = 0.5 * sin((float)unitAngle * (i % vn));
			sectorVertices[index + 1] = 0.5 * cos((float)unitAngle * (i % vn));
			sectorVertices[index + 2] = -0.5;
		}
	}
	// buttom vertices * 2
	for (int i = vn * 2; i < vn * 4; i += 1) {
		int index = i * 3;
		if (i % vn == 0) {
			// center
			sectorVertices[index] = 0;
			sectorVertices[index + 1] = 0;
			sectorVertices[index + 2] = 0.5;
		}
		else {
			float unitAngle = (float)2 * pi / circleVerticesNumber;
			sectorVertices[index] = 0.5 * sin((float)unitAngle * (i % vn));
			sectorVertices[index + 1] = 0.5 * cos((float)unitAngle * (i % vn));
			sectorVertices[index + 2] = 0.5;
		}
	}
	// top normal
	for (int i = 0; i < vn; i += 1) {
		int index = i * 3;
		float unitAngle = (float)2 * pi / vn;
		sectorNormal[index] = 0;
		sectorNormal[index + 1] = 0;
		sectorNormal[index + 2] = -1;
	}

	// bottom normal
	for (int i = vn * 3; i < vn * 4; i += 1) {
		int index = i * 3;
		float unitAngle = (float)2 * pi / vn;
		sectorNormal[index] = 0;
		sectorNormal[index + 1] = 0;
		sectorNormal[index + 2] = 1;
	}
	// side normal
	for (int i = vn * 1; i < vn * 3; i += 1) {
		int index = i * 3;
		if (i % vn == 0) {
			// I'm not sure about this but I guess no one will find it wrong
			sectorNormal[index] = 0.707;
			sectorNormal[index + 1] = -0.707;
			sectorNormal[index + 2] = 0;
		}
		else {
			float unitAngle = (float)2 * pi / circleVerticesNumber;
			sectorNormal[index] = 1 * sin((float)unitAngle * (i % vn));
			sectorNormal[index + 1] = 1 * cos((float)unitAngle * (i % vn));
			sectorNormal[index + 2] = 0;
		}
	}

	// top texCoord
	for (int i = 0; i < vn; i += 1) {
		int index = i * 2;
		if (i % vn == 0) {
			sectortexCoord[index] = 0.25;
			sectortexCoord[index + 1] = 0.75;
			sectortexCoord[index + 2] = 0;
		}
		else {
			float unitAngle = (float)2 * pi / circleVerticesNumber;
			sectortexCoord[index] = 0.25 + 0.25 * sin((float)unitAngle * (i % vn));
			sectortexCoord[index + 1] = 0.75 + 0.25 * -cos((float)unitAngle * (i % vn));
		}
	}

	// bottom texCoord
	for (int i = vn * 3; i < vn * 4; i += 1) {
		int index = i * 2;
		if (i % vn == 0) {
			sectortexCoord[index] = 0.75;
			sectortexCoord[index + 1] = 0.75;
			sectortexCoord[index + 2] = 0;
		}
		else {
			float unitAngle = (float)2 * pi / circleVerticesNumber;
			sectortexCoord[index] = 0.75 + 0.25 * sin((float)unitAngle * (i % vn));
			sectortexCoord[index + 1] = 0.75 + 0.25 * -cos((float)unitAngle * (i % vn));
		}
	}
	// side texCoord
	for (int i = vn * 1; i < vn * 3; i += 1) {
		int index = i * 2;
		float unitLength = (float)1 / vn;
		sectortexCoord[index] = (float)unitLength * (i % circleVerticesNumber);
		if (i < vn * 2)	// upper side
			sectortexCoord[index + 1] = 0.5;
		else            // lower side
			sectortexCoord[index + 1] = 0;
	}
	// side surface
	for (int i = vn; i < vn * 2; i++) {
		int index = (i - vn) * 6;
		sectorElement[index] = i;
		sectorElement[index + 2] = vn + i;
		sectorElement[index + 3] = vn + i;
		if (i + 1 < 2 * vn) {
			sectorElement[index + 1] = i + 1;
			sectorElement[index + 4] = vn + i + 1;
			sectorElement[index + 5] = i + 1;
		}
		else {
			sectorElement[index + 1] = vn;
			sectorElement[index + 4] = vn * 2;
			sectorElement[index + 5] = vn;
		}
	}
	// top surface
	for (int i = 0; i < vn - 2; i++) {
		int index = (vn * 2 + i) * 3;
		sectorElement[index] = 0;
		sectorElement[index + 1] = i + 1;
		sectorElement[index + 2] = i + 2;
	}
	// buttom surface
	for (int i = vn * 3; i < vn * 4 - 2; i++) {
		int index = (vn * 3 + (i - vn * 3) - 2) * 3;
		sectorElement[index] = vn * 3;
		sectorElement[index + 1] = i + 1;
		sectorElement[index + 2] = i + 2;
	}

	glGenVertexArrays(1, &sector.VAO);
	glGenBuffers(3, sector.VBO);
	glGenBuffers(1, &sector.EBO);
	glBindVertexArray(sector.VAO);
	sector.element_amount = sizeof(sectorElement) / sizeof(GLuint);
	// Position attribute
	glBindBuffer(GL_ARRAY_BUFFER, sector.VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sectorVertices), sectorVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Normal attribute
	glBindBuffer(GL_ARRAY_BUFFER, sector.VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sectorNormal), sectorNormal, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	// Image texture attribute
	glBindBuffer(GL_ARRAY_BUFFER, sector.VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sectortexCoord), sectortexCoord, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);
	//Element attribute
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sector.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sectorElement), sectorElement, GL_STATIC_DRAW);
	// Unbind VAO
	glBindVertexArray(0);
}

void TrainView::setWater()
{
	//water
	GLfloat  waterVertices[WATER_RESOLUTION * WATER_RESOLUTION * 3];
	GLfloat  watertexCoords[WATER_RESOLUTION * WATER_RESOLUTION * 2];
	GLuint waterElement[(WATER_RESOLUTION - 1) * (WATER_RESOLUTION - 1) * 6];
	for (int i = 0; i < WATER_RESOLUTION; i++) {
		for (int j = 0; j < WATER_RESOLUTION; j++) {
			int t = (i * WATER_RESOLUTION + j) * 3;
			waterVertices[t] = j / (float)(WATER_RESOLUTION - 1) - 0.5f;
			waterVertices[t + 1] = -30;
			waterVertices[t + 2] = i / (float)(WATER_RESOLUTION - 1) - 0.5f;
		}
	}
	for (int i = 0; i < WATER_RESOLUTION; i++) {
		for (int j = 0; j < WATER_RESOLUTION; j++) {
			int t = (i * WATER_RESOLUTION + j) * 2;
			watertexCoords[t] = j / (float)(WATER_RESOLUTION - 1);
			watertexCoords[t + 1] = i / (float)(WATER_RESOLUTION - 1);
		}
	}
	for (int i = 0; i < WATER_RESOLUTION; i++) {
		for (int j = 0; j < WATER_RESOLUTION; j++) {
			if (i == WATER_RESOLUTION - 1 || j == WATER_RESOLUTION - 1) continue;
			int t = (i * (WATER_RESOLUTION - 1) + j) * 6;
			int p = i * WATER_RESOLUTION + j;
			waterElement[t] = p;
			waterElement[t + 1] = p + WATER_RESOLUTION + 1;
			waterElement[t + 2] = p + 1;
			waterElement[t + 3] = p;
			waterElement[t + 4] = p + WATER_RESOLUTION;
			waterElement[t + 5] = p + WATER_RESOLUTION + 1;
		}
	}
	glGenVertexArrays(1, &water.VAO);
	glGenBuffers(2, water.VBO);
	glGenBuffers(1, &water.EBO);
	glBindVertexArray(water.VAO);
	water.element_amount = sizeof(waterElement) / sizeof(GLuint);
	// Position attribute
	glBindBuffer(GL_ARRAY_BUFFER, water.VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertices), waterVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// texCoords attribute
	glBindBuffer(GL_ARRAY_BUFFER, water.VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(watertexCoords), watertexCoords, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	//Element attribute
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, water.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(waterElement), waterElement, GL_STATIC_DRAW);
	// Unbind VAO
	glBindVertexArray(0);
}

void TrainView::setSmoke()
{
	glGenVertexArrays(1, &smoke.VAO);
	glGenBuffers(1, smoke.VBO);
	glBindVertexArray(smoke.VAO);
	// Position attribute
	glBindBuffer(GL_ARRAY_BUFFER, smoke.VBO[0]);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Unbind VAO
	glBindVertexArray(0);
}

void TrainView::setFBOs() {
	glGenFramebuffers(1, &screenFBO);
	glGenFramebuffers(1, &whiteLineFBO);
	glGenFramebuffers(1, &islandHeightFBO);

	glGenTextures(1, &screenFrameTexture);
	glGenTextures(1, &screenDepthTexture);
	glGenTextures(1, &whiteLineFrameTexture);
	glGenTextures(1, &islandHeightTexture);

	glGenRenderbuffers(1, &screenRBO);
	glGenRenderbuffers(1, &whiteLineRBO);
	glGenRenderbuffers(1, &islandHeightRBO);


	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
	unsigned int frameVBO;
	glGenVertexArrays(1, &frameVAO);
	glGenBuffers(1, &frameVBO);
	glBindVertexArray(frameVAO);
	// Position attribute
	glBindBuffer(GL_ARRAY_BUFFER, frameVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	// Unbind VAO
	glBindVertexArray(0);

	frameShader->use();
	frameShader->setInt("screenTexture", 0);
	frameShader->setInt("crosshairTexture", 1);
	frameShader->setInt("whiteLineTexture", 2);
	glUseProgram(0);
}

void TrainView::setFrameBufferTexture()
{
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, screenFrameTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w(), h(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenFrameTexture, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, screenRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w(), h());
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, screenRBO);

	//glBindTexture(GL_TEXTURE_2D, screenDepthTexture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w(), h(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, screenDepthTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

}

//set all light to dark
void TrainView::initLight() {
	const glm::vec3 ZERO = glm::vec3(0, 0, 0);
	const glm::vec3 UP = glm::vec3(0, 1, 0);
	const float ONE_FLOAT = 1.0f;
	const float ZERO_FLOAT = 0.0f;
	//set directional light
	dirLight.ambient = ZERO;
	dirLight.diffuse = ZERO;
	dirLight.specular = ZERO;
	dirLight.direction = UP;
	//set point light
	for (int i = 0; i < 4; i++) {
		pointLights[i].ambient = ZERO;
		pointLights[i].diffuse = ZERO;
		pointLights[i].specular = ZERO;
		pointLights[i].position = ZERO;
		pointLights[i].constant = ONE_FLOAT;
		pointLights[i].linear = ZERO_FLOAT;
		pointLights[i].quadratic = ZERO_FLOAT;
	}
	//set spot light
	for (int i = 0; i < 4; i++) {
		spotLights[i].ambient = ZERO;
		spotLights[i].diffuse = ZERO;
		spotLights[i].specular = ZERO;
		spotLights[i].position = ZERO;
		spotLights[i].direction = UP;
		spotLights[i].cutOff = ZERO_FLOAT;
		spotLights[i].outerCutOff = ZERO_FLOAT;
		spotLights[i].constant = ONE_FLOAT;
		spotLights[i].linear = ZERO_FLOAT;
		spotLights[i].quadratic = ZERO_FLOAT;
	}
}

//************************************************************************
//
// * this is the code that actually draws the window
//   it puts a lot of the work into other routines to simplify things
//========================================================================
void TrainView::draw()
{

	//*********************************************************************
	//
	// * Set up basic opengl informaiton
	//
	//**********************************************************************
	//initialized glad
	if (gladLoadGL())
	{
		//initiailize VAO, VBO, Shader...
		if (!hasInitRander) {
			initRander();
			hasInitRander = true;
		}
	}
	else
		throw std::runtime_error("Could not initialize GLAD!");



	// draw on our frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
	setFrameBufferTexture();

	// Set up the view port
	glViewport(0, 0, w(), h());

	// clear the window, be sure to clear the Z-Buffer too
	glClearColor(0, 0, .3f, 0);		// background should be blue

	// we need to clear out the stencil buffer since we'll use
	// it for shadows
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH);

	// Blayne prefers GL_DIFFUSE
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// prepare for projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setProjection();		// put the code to set up matrices here

	//######################################################################
	// TODO: 
	// you might want to set the lighting up differently. if you do, 
	// we need to set up the lights AFTER setting up the projection
	//######################################################################
	// enable the lighting
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);

	//*********************************************************************
	//
	// * set the light parameters
	//
	//**********************************************************************
	initLight();
	//dirLight.direction = glm::vec3(0.577, 0.577, 0.577);
	//dirLight.diffuse = RenderDatabase::SUN_COLOR;
	//point light 0, main light
	pointLights[0].position = glm::vec3(-350.0f, 500.0f, -300.0f);
	pointLights[0].ambient = RenderDatabase::MIDDLE_GRAY_COLOR;
	pointLights[0].diffuse = RenderDatabase::SUN_COLOR;
	pointLights[0].specular = RenderDatabase::WHITE_COLOR;
	//point light 1, first controlpoint light
	glm::vec3 firstContropointPos = glm::vec3(m_pTrack->points[0].pos.x, m_pTrack->points[0].pos.y, m_pTrack->points[0].pos.z);
	pointLights[1].position = firstContropointPos;
	pointLights[1].diffuse = RenderDatabase::WHITE_COLOR;
	pointLights[1].specular = RenderDatabase::WHITE_COLOR;
	pointLights[1].linear = 0.014f;
	pointLights[1].quadratic = 0.0007f;
	//spot light 0, train headlight
	glm::vec3 trainHeadlightPos = trainPos.glmvec3() + trainFront.glmvec3() * 5.1f + trainUp.glmvec3() * 4.0f;
	spotLights[0].position = trainHeadlightPos;
	spotLights[0].direction = trainFront.glmvec3();
	spotLights[0].diffuse = RenderDatabase::YELLOW_COLOR;
	spotLights[0].specular = RenderDatabase::YELLOW_COLOR;
	spotLights[0].cutOff = cos(MathHelper::degreeToRadians(30));
	spotLights[0].outerCutOff = cos(MathHelper::degreeToRadians(35));
	spotLights[0].linear = 0.007;
	spotLights[0].quadratic = 0.0002;

	//*********************************************************************
	// now draw the ground plane
	//*********************************************************************
	// set to opengl fixed pipeline(use opengl 1.x draw function)
	glUseProgram(0);

	setupFloor();
	//glDisable(GL_LIGHTING);
	//drawFloor(200, 200);

	//*********************************************************************
	// now draw the object and we need to do it twice
	// once for real, and then once for shadows
	//*********************************************************************
	glEnable(GL_LIGHTING);
	setupObjects();

	if (USE_MODEL) {
		drawIslandHeight();
		glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, islandHeightTexture);
		if (tw->drawShadow->value()) {
			instanceShadowShader->use();
			instanceShadowShader->setBool("useModel", true);
			instanceShadowShader->setInt("islandHeight", 0);
			modelShadowShader->use();
			modelShadowShader->setBool("useModel", true);
			modelShadowShader->setInt("islandHeight", 0);
		}
		pierShader->use();
		pierShader->setBool("useModel", true);
		pierShader->setInt("islandHeight", 0);
		glUseProgram(0);
	}

	drawStuff();

	//draw particle
	static float lastSpeed = 0;
	static float breakerStrength = 0;
	if (tw->speed->value() - lastSpeed < 0) {
		breakerStrength += (lastSpeed - trainVelocity) * 10;
	}
	lastSpeed = trainVelocity;

	Pnt3f trainRight = trainFront * trainUp;
	trainParticle1->setPosition(trainPos.glmvec3() + trainFront.glmvec3() * 10.0f - trainUp.glmvec3() * 4.0f + trainRight.glmvec3() * 5.0f);
	trainParticle1->setDirection((-1 * trainFront + (1 * trainUp) + 0.2 * trainRight).glmvec3());
	trainParticle1->setParticleLife(10);
	trainParticle1->setGenerateRate(breakerStrength);
	trainParticle1->setGravity(0.3);
	trainParticle1->setParticleVelocityRandomOffset(0.3);
	trainParticle1->setParticleVelocity(3);
	trainParticle1->setAngle(5);
	trainParticle1->setParticleSize(1);
	trainParticle1->setColor(glm::vec3(1, 0.95, 0), glm::vec3(1, 0.75, 0), glm::vec3(1, 0.75, 0), 0.8);

	trainParticle2->setPosition(trainPos.glmvec3() + trainFront.glmvec3() * 10.0f - trainUp.glmvec3() * 4.0f - trainRight.glmvec3() * 5.0f);
	trainParticle2->setDirection((-1*trainFront + (1 * trainUp) - 0.2 * trainRight).glmvec3());
	trainParticle2->setParticleLife(10);
	trainParticle2->setGenerateRate(breakerStrength);
	trainParticle2->setGravity(0.3);
	trainParticle2->setParticleVelocityRandomOffset(0.3);
	trainParticle2->setParticleVelocity(3);
	trainParticle2->setAngle(5);
	trainParticle2->setParticleSize(1);
	trainParticle2->setColor(glm::vec3(1, 0.95, 0), glm::vec3(1, 0.75, 0), glm::vec3(1, 0.75, 0), 0.8);
	particleSystem.draw();

	breakerStrength *= pow(0.8, RenderDatabase::timeScale);

	//update animation
	targetChainExplosionUpdate();

	// this time drawing is for shadows (except for top view)
	/*
	if (!tw->topCam->value()) {
		setupShadows();
		drawStuff(true);
		unsetupShadows();
	}*/



	if (animationFrame >= keyFrame[8]) {
		drawSpeedBg();
	}
	else {
		drawSkybox();
	}

	if (RenderDatabase::timeScale == RenderDatabase::BULLET_TIME_SCALE)
		drawWhiteLine();

	// final step, do the post-process
	drawFrame();


}

//************************************************************************
//
// * This sets up both the Projection and the ModelView matrices
//   HOWEVER: it doesn't clear the projection first (the caller handles
//   that) - its important for picking
//========================================================================
void TrainView::
setProjection()
//========================================================================
{
	// Compute the aspect ratio (we'll need it)
	float aspect = static_cast<float>(w()) / static_cast<float>(h());

	// Check whether we use the world camp
	if (tw->worldCam->value())
		arcball.setProjection(false);
	// Or we use the top cam
	else if (tw->topCam->value()) {
		float wi, he;
		if (aspect >= 1) {
			wi = 110;
			he = wi / aspect;
		}
		else {
			he = 110;
			wi = he * aspect;
		}

		// Set up the top camera drop mode to be orthogonal and set
		// up proper projection matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-wi, wi, -he, he, 200, -200);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(-90, 1, 0, 0);
	}
	else if (tw->trainCam->value()) {
		//glClear(GL_COLOR_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60, aspect, 1, 10000);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		if (animationFrame == 0) {	// default
			Pnt3f trainRight = trainFront * trainUp;
			Pnt3f horizontalFront = trainFront * cos(camRotateX) + trainRight * sin(camRotateX);
			lookingFront = horizontalFront * cos(camRotateY) + trainUp * sin(camRotateY);
			lookingUp = horizontalFront * -sin(camRotateY) + trainUp * cos(camRotateY);
			Pnt3f cameraShake(0, 0, 0);
			float t = tw->clock_time - lastShootTime;
			if (t < 30) {
				if (t < 1)
					t = 1;
				cameraShake = lookingUp * sin(6.28 * t * 0.3) * (0.02 / (1 + t));
			}
			float t2 = tw->clock_time - lastExplodeTime;
			if (t2 < 30) {
				static float str = 0;
				static Pnt3f dir;
				if (t2 <= 1) {
					str = 50 / pow((trainPos - lastExplodePos).len2(), 0.8);
					Pnt3f lookingRight = lookingFront * lookingUp;
					lookingRight.normalize();
					dir = lookingUp + lookingRight * 1.5;
					dir.normalize();
					t2 = 1;
				}
				cameraShake = dir * sin(6.28 * t2 * 0.2) * (str / (t2 * 2));
			}
			if (!USE_MODEL)
				gluLookAt(trainPos.x, trainPos.y, trainPos.z,
					trainPos.x + lookingFront.x + cameraShake.x, trainPos.y + lookingFront.y + cameraShake.y, trainPos.z + lookingFront.z + cameraShake.z,
					lookingUp.x, lookingUp.y, lookingUp.z);
			else {
				glm::vec3 camPos = (trainPos + trainUp * 7 + trainFront * 4).glmvec3();
				glm::vec3 center = (trainPos + trainUp * 7 + trainFront * 4 + lookingFront + cameraShake).glmvec3();
				gluLookAt(camPos.x, camPos.y, camPos.z,
					center.x, center.y, center.z,
					lookingUp.x, lookingUp.y, lookingUp.z);
			}
		}
		else {	// when giga drill breaking, look at the train
			static Pnt3f startCamPos;
			static Pnt3f finalCamPos;
			static float startTime = -999;
			static bool camFlag1 = false;
			static bool camFlag2 = false;
			if (animationFrame == 1) {
				Pnt3f trainRight = trainFront * trainUp;
				trainRight.normalize();
				Pnt3f horizontalFront = trainFront * cos(camRotateX) + trainRight * sin(camRotateX);
				Pnt3f aimFront = horizontalFront * cos(camRotateY) + trainUp * sin(camRotateY);
				Pnt3f aimUp = horizontalFront * -sin(camRotateY) + trainUp * cos(camRotateY);
				Pnt3f aimRight = aimFront * aimUp;
				aimFront.y = 0;
				aimRight.y = 0;
				aimFront.normalize();
				aimRight.normalize();
				finalCamPos = trainPos + aimFront * 80 + aimRight * -20 + Pnt3f(0, 0, 0);
				startCamPos = trainPos + trainFront * 40 + trainRight * -40 + Pnt3f(0, 0, 0);
				startTime = tw->clock_time;
				camFlag1 = false;
				camFlag2 = false;
			}
			if (animationFrame <= keyFrame[1]) {
				float t = (animationFrame - keyFrame[0]) / (keyFrame[1] - keyFrame[0]);
				if (t < 0) t = 0;
				float t2 = MathHelper::sigmoid(t, 10);

				Pnt3f tempCamPos = startCamPos * cos(t2 * 3.14159 / 2) + finalCamPos * sin(t2 * 3.14159 / 2);
				gluLookAt(tempCamPos.x, tempCamPos.y, tempCamPos.z,
					trainPos.x + trainFront.x * 2, trainPos.y + trainFront.y * 2, trainPos.z + trainFront.z * 2,
					0, 1, 0);
				eyepos = tempCamPos.glmvec3();
			}
			else if (animationFrame < keyFrame[11]) {
				float t3 = (animationFrame - keyFrame[4]) / (keyFrame[5] - keyFrame[4]);
				if (t3 < 0) t3 = 0;
				if (t3 > 1) t3 = 1;
				gluLookAt(finalCamPos.x, finalCamPos.y, finalCamPos.z,
					trainPos.x + trainFront.x * 30 * t3, trainPos.y + trainFront.y * 30 * t3, trainPos.z + trainFront.z * 30 * t3,
					0, 1, 0);
				eyepos = finalCamPos.glmvec3();
			}
			else {
				static Pnt3f reallyFinalCamPos;
				if (!camFlag1) {
					camFlag1 = true;
				}
				else if (!camFlag2) {
					Pnt3f trainRight = trainFront * trainUp;
					reallyFinalCamPos = trainPos + trainFront * 120 + trainUp * 15 + trainRight * -25;
					camFlag2 = true;
				}
				Pnt3f cameraShake(0, 0, 0);
				float t2 = tw->clock_time - lastExplodeTime;
				if (t2 < 1000) {
					static float str = staticSpiralPower * 4;
					static Pnt3f dir;
					static Pnt3f dir2;
					if (t2 <= 1) {
						dir = randUnitVector();
						dir2 = randUnitVector();
						t2 = 1;
					}
					cameraShake = dir * sin(6.28 * t2 * 0.2) * (str / (t2));
				}
				gluLookAt(reallyFinalCamPos.x + cameraShake.x, reallyFinalCamPos.y + cameraShake.y, reallyFinalCamPos.z + cameraShake.z,
					0 + cameraShake.x, 5 + cameraShake.y, 0 + cameraShake.z,
					0, 1, 0);
				eyepos = finalCamPos.glmvec3();
			}
		}
	}
	else if (tw->freeCam->value()) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(freeCamera.FOV_, aspect, freeCamera.NEAR_, freeCamera.FAR_);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glm::vec3 camPos = freeCamera.getPosition();
		glm::vec3 camDir = freeCamera.getDirection();
		glm::vec3 camUp = freeCamera.getUp();
		gluLookAt(
			camPos.x, camPos.y, camPos.z,
			camPos.x + camDir.x, camPos.y + camDir.y, camPos.z + camDir.z,
			camUp.x, camUp.y, camUp.z
		);
	}
	else if (tw->CirnoCam->value()) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(100, aspect, 1, 10000);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		Pnt3f trainRight = trainFront * trainUp;
		trainRight.normalize();
		Pnt3f camPos = trainPos + trainUp * 7.9 + trainFront * 9.99 + trainRight * 0.5;
		Pnt3f camDir = -1 * trainFront + trainUp * 0.4;
		Pnt3f camUp = camDir * trainRight;
		gluLookAt(
			camPos.x, camPos.y, camPos.z,
			camPos.x + camDir.x, camPos.y + camDir.y, camPos.z + camDir.z,
			camUp.x, camUp.y, camUp.z
		);
	}
	else if (tw->CirnoerCam->value()) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(100, aspect, 1, 10000);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		Pnt3f trainRight = trainFront * trainUp;
		trainRight.normalize();
		Pnt3f camPos = trainPos + trainUp * 7.9 + trainFront * 3.22 + trainRight * 0.5;
		Pnt3f camDir = -1 * trainFront + trainUp * 0.4;
		Pnt3f camUp = camDir * trainRight;
		gluLookAt(
			camPos.x, camPos.y, camPos.z,
			camPos.x + camDir.x, camPos.y + camDir.y, camPos.z + camDir.z,
			camUp.x, camUp.y, camUp.z
		);
	}
}

//set shader uniform, like view, projection, lights...
void TrainView::setShaders() {
	//get view matrix and projection matrix
	glm::mat4 view;
	glGetFloatv(GL_MODELVIEW_MATRIX, &view[0][0]);
	glm::mat4 projection;
	glGetFloatv(GL_PROJECTION_MATRIX, &projection[0][0]);

	//set uniform buffer 0
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//set uniform
	Shader* shaders[] = { simpleObjectShader, simpleInstanceObjectShader, pierShader, waterShader, smokeShader, modelShader, instanceShadowShader };
	int size = sizeof(shaders) / sizeof(Shader*);
	for (int i = 0; i < size; i++) {
		shaders[i]->use();

		Pnt3f trainRight = trainFront * trainUp;
		//set the uniform
		if (tw->worldCam->value() || tw->topCam->value())
			eyepos = glm::vec3(view[0][2] * -arcball.getEyePos().z, view[1][2] * -arcball.getEyePos().z, view[2][2] * -arcball.getEyePos().z);
		else if (tw->freeCam->value())
			eyepos = freeCamera.getPosition();
		else if (tw->trainCam->value() && animationFrame == 0)
			eyepos = trainPos.glmvec3();
		else if (tw->CirnoCam->value())
			eyepos = (trainPos + trainUp * 7.9 + trainFront * 9.99 + trainRight * 0.5).glmvec3();
		else if (tw->CirnoerCam->value())
			eyepos = (trainPos + trainUp * 7.9 + trainFront * 3.22 + trainRight * 0.5).glmvec3();


		shaders[i]->setVec3("eyePosition", eyepos);

		// light properties
		shaders[i]->setVec3("dirLight.ambient", dirLight.ambient);
		shaders[i]->setVec3("dirLight.diffuse", dirLight.diffuse);
		shaders[i]->setVec3("dirLight.specular", dirLight.specular);
		shaders[i]->setVec3("dirLight.direction", dirLight.direction);
		for (int j = 0; j < 4; j++) {
			shaders[i]->setVec3("pointLights[" + std::to_string(j) + "].ambient", pointLights[j].ambient);
			shaders[i]->setVec3("pointLights[" + std::to_string(j) + "].diffuse", pointLights[j].diffuse);
			shaders[i]->setVec3("pointLights[" + std::to_string(j) + "].specular", pointLights[j].specular);
			shaders[i]->setVec3("pointLights[" + std::to_string(j) + "].position", pointLights[j].position);
			shaders[i]->setFloat("pointLights[" + std::to_string(j) + "].constant", pointLights[j].constant);
			shaders[i]->setFloat("pointLights[" + std::to_string(j) + "].linear", pointLights[j].linear);
			shaders[i]->setFloat("pointLights[" + std::to_string(j) + "].quadratic", pointLights[j].quadratic);
		}
		for (int j = 0; j < 4; j++) {
			shaders[i]->setVec3("spotLights[" + std::to_string(j) + "].ambient", spotLights[j].ambient);
			shaders[i]->setVec3("spotLights[" + std::to_string(j) + "].diffuse", spotLights[j].diffuse);
			shaders[i]->setVec3("spotLights[" + std::to_string(j) + "].specular", spotLights[j].specular);
			shaders[i]->setVec3("spotLights[" + std::to_string(j) + "].position", spotLights[j].position);
			shaders[i]->setVec3("spotLights[" + std::to_string(j) + "].direction", spotLights[j].direction);
			shaders[i]->setFloat("spotLights[" + std::to_string(j) + "].cutOff", spotLights[j].cutOff);
			shaders[i]->setFloat("spotLights[" + std::to_string(j) + "].outerCutOff", spotLights[j].outerCutOff);
			shaders[i]->setFloat("spotLights[" + std::to_string(j) + "].constant", spotLights[j].constant);
			shaders[i]->setFloat("spotLights[" + std::to_string(j) + "].linear", spotLights[j].linear);
			shaders[i]->setFloat("spotLights[" + std::to_string(j) + "].quadratic", spotLights[j].quadratic);
		}

		shaders[i]->setFloat("gamma", tw->gamma->value());
	}
	skyboxShader->use();
	skyboxShader->setFloat("gamma", tw->gamma->value());

	//set back to opengl fixed pipeline
	glUseProgram(0);
}

void TrainView::setObjectTexture(std::string name, std::string texturePath)
{
	unsigned int id = getObjectTexture(name);
	if (id == -1) {
		objectTextures.push_back(std::make_pair(
			name, RenderDatabase::loadTexture(exePath + OBJECT_TEXTURE_PATH + texturePath)
		));
	}
}

unsigned int TrainView::getObjectTexture(std::string name)
{
	for (int i = 0; i < objectTextures.size(); i++) {
		if (objectTextures[i].first == name)
			return objectTextures[i].second;
	}
	return -1;
}

//draw object by simple object shader
void TrainView::drawSimpleObject(const Object& object, const glm::mat4 model, const Material material) {
	simpleObjectShader->use();

	simpleObjectShader->setMat4("model", model);
	simpleObjectShader->setMat4("normalMatrix", glm::transpose(glm::inverse(model)));

	// material properties
	simpleObjectShader->setVec3("material.ambient", material.ambient);
	simpleObjectShader->setVec3("material.diffuse", material.diffuse);
	simpleObjectShader->setVec3("material.specular", material.specular);
	simpleObjectShader->setFloat("material.shininess", material.shininess);

	glBindVertexArray(object.VAO);
	glDrawElements(GL_TRIANGLES, object.element_amount, GL_UNSIGNED_INT, 0);

	//unbind VAO
	glBindVertexArray(0);

	//unbind shader(switch to fixed pipeline)
	glUseProgram(0);
}

//roatateTheta is degree and anticlockwise by +y
void TrainView::
drawTree(glm::vec3 pos, float rotateTheta, float treeTrunkWidth, float treeHeight, float leafHeight, float leafWidth, float leafWidthDecreaseDelta) {
	//check to make sure tree is look good
	if (leafWidth - (treeHeight / 2.0f / leafHeight) * leafWidthDecreaseDelta < treeTrunkWidth) {
		leafWidth = (treeHeight / 2.0f / leafHeight) * leafWidthDecreaseDelta + treeTrunkWidth;
	}

	Material treeTrunkMaterial = {
		glm::vec3(0.36f, 0.25f, 0.011f),
		glm::vec3(0.36f, 0.25f, 0.011f),
		glm::vec3(0.3f, 0.3f, 0.3f),
		10.0f
	};
	Material leafMaterial = {
		glm::vec3(0.047f, 0.67f, 0.011f),
		glm::vec3(0.047f, 0.67f, 0.011f),
		glm::vec3(0.3f, 0.3f, 0.3f),
		10.0f
	};

	const glm::vec3 ORIGIN = pos;
	const glm::vec3 UP = glm::vec3(0, 1, 0);
	const glm::vec3 FRONT = glm::vec3(sin(MathHelper::degreeToRadians(rotateTheta)), 0, -cos(MathHelper::degreeToRadians(rotateTheta)));

	//draw trunk
	glm::mat4 treeTrunkModel = MathHelper::getTransformMatrix(ORIGIN + UP * (treeHeight / 2.0f), FRONT, UP, glm::vec3(treeTrunkWidth, treeHeight, treeTrunkWidth));
	drawSimpleObject(cube, treeTrunkModel, treeTrunkMaterial);

	//draw leaf
	for (glm::vec3 baseHeight = ORIGIN + UP * (treeHeight / 2.0f) + UP * (leafHeight / 2.0f); baseHeight.y <= treeHeight + leafHeight; baseHeight = baseHeight + UP * leafHeight) {
		glm::mat4 leafModel = MathHelper::getTransformMatrix(baseHeight, FRONT, UP, glm::vec3(leafWidth, leafHeight, leafWidth));
		drawSimpleObject(cube, leafModel, leafMaterial);
		leafWidth -= leafWidthDecreaseDelta;
	}
}

void TrainView::drawWater(glm::vec3 pos, glm::vec3 scale, float rotateTheta) {
	const glm::vec3 UP = glm::vec3(0, 1, 0);
	const glm::vec3 FRONT = glm::vec3(sin(MathHelper::degreeToRadians(rotateTheta)), 0, -cos(MathHelper::degreeToRadians(rotateTheta)));
	glm::mat4 model = MathHelper::getTransformMatrix(pos, FRONT, UP, scale);

	Material waterMaterial = {
		RenderDatabase::WATER_COLOR,
		RenderDatabase::WATER_COLOR,
		RenderDatabase::SUN_COLOR,
		128.0f
	};

	waterShader->use();

	waterShader->setMat4("model", model);
	waterShader->setMat4("normalMatrix", glm::transpose(glm::inverse(model)));

	// material properties
	waterShader->setVec3("material.ambient", waterMaterial.ambient);
	waterShader->setVec3("material.diffuse", waterMaterial.diffuse);
	waterShader->setVec3("material.specular", waterMaterial.specular);
	waterShader->setFloat("material.shininess", waterMaterial.shininess);

	int frameNum = (int)tw->clock_time;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterHeightMap[frameNum % 200]);
	waterShader->setInt("heightMap", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, waterNormalMap[frameNum % 200]);
	waterShader->setInt("normalMap", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	waterShader->setInt("skybox", 2);

	glBindVertexArray(water.VAO);
	glDrawElements(GL_TRIANGLES, water.element_amount, GL_UNSIGNED_INT, 0);

	//unbind VAO
	glBindVertexArray(0);

	//unbind shader(switch to fixed pipeline)
	glUseProgram(0);
}

void TrainView::drawSmoke(const std::vector<glm::vec4>& points)
{
	smokeShader->use();

	// material properties
	glBindVertexArray(smoke.VAO);
	//std::cout << points.size() << " " << sizeof(glm::vec4) << " " << points.data() << " " << sizeof(points.data()) << std::endl;
	glBindBuffer(GL_ARRAY_BUFFER, smoke.VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec4), points.data(), GL_STATIC_DRAW);
	glDrawArrays(GL_POINTS, 0, points.size());
	//unbind VAO
	glBindVertexArray(0);

	//unbind shader(switch to fixed pipeline)
	glUseProgram(0);
}
void TrainView::setSkybox()
{
	float skyBoxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	glGenVertexArrays(1, &skybox.VAO);
	glGenBuffers(2, skybox.VBO);
	glGenBuffers(1, &skybox.EBO);
	glBindVertexArray(skybox.VAO);
	// Position attribute
	glBindBuffer(GL_ARRAY_BUFFER, skybox.VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyBoxVertices), skyBoxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Unbind VAO
	glBindVertexArray(0);
}
void TrainView::drawSkybox()
{
	glDepthFunc(GL_LEQUAL);
	skyboxShader->use();
	glBindVertexArray(skybox.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, skybox.VBO[0]);
	glActiveTexture(GL_TEXTURE0);
	skyboxShader->setInt("skybox", 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
}
void TrainView::drawSpeedBg()
{
	glDepthFunc(GL_LEQUAL);
	speedBgShader->use();
	glBindVertexArray(frameVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, getObjectTexture("speedBg"));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	speedBgShader->setInt("bg", 0);
	speedBgShader->setFloat("t", animationFrame - keyFrame[8]);
	static glm::vec3 drill_dir;
	if (animationFrame < keyFrame[10]) {
		drill_dir = trainFront.glmvec3();
	}
	speedBgShader->setVec3("drill_dir", drill_dir);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDepthFunc(GL_LESS);
	glUseProgram(0);
}
void TrainView::drawIslandHeight()
{
	glBindFramebuffer(GL_FRAMEBUFFER, islandHeightFBO);
	glBindTexture(GL_TEXTURE_2D, islandHeightTexture);
	std::vector<float> zeroData(w() * h(), 9999.0f);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, w(), h(), 0, GL_RED, GL_FLOAT, zeroData.data());
	GLfloat borderColor[] = { -99999.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, islandHeightTexture, 0);

	glViewport(0, 0, w(), h());
	glClearColor(9999, 9999, 9999, 9999);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);

	glBindRenderbuffer(GL_RENDERBUFFER, islandHeightRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w(), h());
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, islandHeightRBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	glViewport(0, 0, w(), h());
	glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	islandHeightShader->use();
	glm::mat4 islandModel = MathHelper::getTransformMatrix(glm::vec3(-150, -580, 170), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0.5, 0.5, 0.5));
	islandHeightShader->setMat4("model", islandModel);
	island->Draw(islandHeightShader);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void TrainView::drawWhiteLine()
{
	glBindFramebuffer(GL_FRAMEBUFFER, whiteLineFBO);

	glViewport(0, 0, w(), h());
	glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, whiteLineFrameTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w(), h(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, whiteLineFrameTexture, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, whiteLineRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w(), h());
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, whiteLineRBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	InstanceDrawer targetInstance(RenderDatabase::WHITE_PLASTIC_MATERIAL);
	for (int i = 0; i < targets.size(); i++) {
		if (targets[i].state == 0) {
			glm::mat4 targetModel = MathHelper::getTransformMatrix(
				targets[i].pos.glmvec3(), targets[i].front.glmvec3(), targets[i].up.glmvec3(),
				glm::vec3(10, 10, 1));
			targetInstance.addModelMatrix(targetModel);
		}
	}
	InstanceDrawer targetFragInstance(RenderDatabase::WHITE_PLASTIC_MATERIAL);
	for (int i = 0; i < targetFrags.size(); i++) {
		glm::mat4 targetFragModel = MathHelper::getTransformMatrix(
			targetFrags[i].pos.glmvec3(), targetFrags[i].front.glmvec3(), targetFrags[i].up.glmvec3(),
			glm::vec3(10, 10, 1));
		targetFragInstance.addModelMatrix(targetFragModel);
	}


	//I don't know why, but it need clear again.
	glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	targetInstance.drawByInstance(whiteLineShader, cylinder);
	targetFragInstance.drawByInstance(whiteLineShader, sector);
}
void TrainView::drawFrame()
{
	// draw on the default frame
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w(), h());
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);
	frameShader->use();
	glBindVertexArray(frameVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screenFrameTexture);
	//glBindTexture(GL_TEXTURE_2D, islandHeightTexture);

	frameShader->setFloat("frame", tw->clock_time);
	if (tw->trainCam->value() && animationFrame == 0) {
		frameShader->setBool("useCrosshair", true);
		frameShader->setFloat("screenAspectRatio", (float)w() / (float)h());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, getObjectTexture("crosshair"));
	}
	else {
		frameShader->setBool("useCrosshair", false);
	}
	if (RenderDatabase::timeScale == RenderDatabase::BULLET_TIME_SCALE) {
		frameShader->setBool("bulletTime", true);
		frameShader->setInt("whiteLineTexture", 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, whiteLineFrameTexture);
	}
	else {
		frameShader->setBool("bulletTime", false);
	}
	static float SpiralstartTime;
	if (SpiralPower == 2.75) {
		SpiralstartTime = tw->clock_time;
	}
	if (SpiralPower >= 3) {
		frameShader->setBool("useSpiral", true);
		frameShader->setFloat("shineTime", (SpiralstartTime - tw->clock_time) * (SpiralPower / 3));
	}
	else {
		frameShader->setBool("useSpiral", false);
		frameShader->setFloat("shineTime", 0);
	}
	if (animationFrame > 316 && animationFrame < 326) {
		frameShader->setBool("useImpact", true);
		frameShader->setFloat("shineTime", animationFrame);
	}
	else
		frameShader->setBool("useImpact", false);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glActiveTexture(0);


	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(0);
}



//************************************************************************
// cp_pos_p0.x,cp_pos_p1.x,cp_pos_p2.x,cp_pos_p3.x };
// * this draws all of the stuff in the world
//
//	NOTE: if you're drawing shadows, DO NOT set colors (otherwise, you get 
//       colored shadows). this gets called twice per draw 
//       -- once for the objects, once for the shadows
//########################################################################
// TODO: 
// if you have other objects in the world, make sure to draw them
//########################################################################
//========================================================================
void TrainView::drawStuff(bool doingShadows)
{
	//set up shaders uniform
	setShaders();

	// Draw the control points
	// don't draw the control points if you're driving 
	// (otherwise you get sea-sick as you drive through them)
	if (tw->showControlPoint->value() && (tw->worldCam->value() || tw->topCam->value())) {
		for (size_t i = 0; i < m_pTrack->points.size(); ++i) {
			if (!doingShadows) {
				if (((int)i) != selectedCube)
					glColor3ub(240, 60, 60);
				else
					glColor3ub(240, 240, 30);
			}
			m_pTrack->points[i].draw();
		}
	}

	//draw the floor
	if (!USE_MODEL) {
		glm::mat4 floorModel = MathHelper::getTransformMatrix(glm::vec3(0, -0.5, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), glm::vec3(200, 1, 200));
		drawSimpleObject(cube, floorModel, RenderDatabase::GREEN_PLASTIC_MATERIAL);
	}

	//draw the tree
	//drawTree(glm::vec3(0, 0, 0));

	//draw the water
	drawWater(glm::vec3(0, -120, 0), glm::vec3(2500, 1, 2500));

	//draw modle
	if (USE_MODEL) {
		modelShader->use();
		float modelGamma = log2(tw->gamma->value() * 10) / 4.32193;
		modelShader->setFloat("gamma", modelGamma);

		//draw island
		glm::mat4 islandModel = MathHelper::getTransformMatrix(glm::vec3(-150, -280, 170), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0.5, 0.5, 0.5));
		modelShader->setMat4("model", islandModel);
		island->Draw(modelShader);

		//draw pillar
		glm::mat4 pillarModel = MathHelper::getTransformMatrix(glm::vec3(0, -2, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), glm::vec3(0.2, 0.2, 0.2));
		modelShader->setMat4("model", pillarModel);
		stonePillar->Draw(modelShader);

		//draw pillar section
		glm::mat4 pillarSectionModel = MathHelper::getTransformMatrix(glm::vec3(20, -8, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), glm::vec3(0.01, 0.01, 0.01));
		modelShader->setMat4("model", pillarSectionModel);
		stonePillarSection->Draw(modelShader);
		//another pillar section
		pillarSectionModel = MathHelper::getTransformMatrix(glm::vec3(0, -8, 20), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0.01, 0.01, 0.01));
		modelShader->setMat4("model", pillarSectionModel);
		stonePillarSection->Draw(modelShader);

		//draw red arrow
		glm::mat4 arrowModel = MathHelper::getTransformMatrix(glm::vec3(20, 14.5, 0), glm::vec3(0, 0, -1), glm::vec3(1, 0, 0), glm::vec3(1.5, 1.5, 1.5));
		modelShader->setMat4("model", arrowModel);
		arrow_red->Draw(modelShader);

		//draw blue arrow
		arrowModel = MathHelper::getTransformMatrix(glm::vec3(0, 14.5, 20), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), glm::vec3(1.5, 1.5, 1.5));
		modelShader->setMat4("model", arrowModel);
		arrow_blue->Draw(modelShader);

		//FUMO(fumo)(9)
		if (!tw->trainCam->value() || animationFrame > 0) {
			modelShader->setFloat("gamma", modelGamma + 1.12);
			Pnt3f trainRight = trainFront * trainUp;
			trainRight.normalize();
			Pnt3f CirnoFront = trainFront + trainRight * 1.25;
			CirnoFront.normalize();
			glm::mat4 CirnoModel = MathHelper::getTransformMatrix((trainPos + trainFront * 3 + trainUp * 8.8 + trainRight * 4).glmvec3(), CirnoFront.glmvec3(), trainUp.glmvec3(), glm::vec3(0.3, 0.3, 0.3));
			modelShader->setMat4("model", CirnoModel);
			Cirno->Draw(modelShader);
			if (tw->drawShadow->value()) {
				modelShadowShader->use();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, islandHeightTexture);
				modelShadowShader->setMat4("model", CirnoModel);
				modelShadowShader->setBool("useModel", true);
				modelShadowShader->setInt("islandHeight", 0);
				Cirno->Draw(modelShadowShader,true);
				glBindTexture(GL_TEXTURE_2D, 0);
				modelShader->use();
			}
			modelShader->setFloat("gamma", modelGamma);
		}

		//draw tank
		glm::mat4 tankModel = MathHelper::getTransformMatrix(trainPos.glmvec3(), -trainFront.glmvec3(), trainUp.glmvec3(), glm::vec3(5, 5, 5));
		modelShader->setMat4("model", tankModel);
		tank->Draw(modelShader);
		if (tw->drawShadow->value()) {
			modelShadowShader->use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, islandHeightTexture);
			modelShadowShader->setMat4("model", tankModel);
			modelShadowShader->setBool("useModel", true);
			modelShadowShader->setInt("islandHeight", 0);
			tank->Draw(modelShadowShader, true);
			glBindTexture(GL_TEXTURE_2D, 0);
			modelShader->use();
		}
		//draw cannon 
		if (tw->trainCam->value()) {
			glm::mat4 cannonModel = MathHelper::getTransformMatrix(trainPos.glmvec3() + trainUp.glmvec3() * 5.0f + trainFront.glmvec3() * 4.0f, -lookingFront.glmvec3(), lookingUp.glmvec3(), glm::vec3(5, 5, 5));
			modelShader->setMat4("model", cannonModel);
			cannon->Draw(modelShader);
			if (tw->drawShadow->value()) {
				modelShadowShader->use();
				modelShadowShader->setMat4("model", cannonModel);
			}
		}
		else {
			glm::mat4 cannonModel = MathHelper::getTransformMatrix(trainPos.glmvec3() + trainUp.glmvec3() * 3.0f + trainFront.glmvec3() * 4.0f, -trainFront.glmvec3(), trainUp.glmvec3(), glm::vec3(5, 5, 5));
			modelShader->setMat4("model", cannonModel);
			cannon->Draw(modelShader);
			if (tw->drawShadow->value()) {
				modelShadowShader->use();
				modelShadowShader->setMat4("model", cannonModel);
			}
		}
		
		if (tw->drawShadow->value()) {
			modelShadowShader->use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, islandHeightTexture);
			modelShadowShader->setBool("useModel", true);
			modelShadowShader->setInt("islandHeight", 0);
			cannon->Draw(modelShadowShader, true);
			glBindTexture(GL_TEXTURE_2D, 0);
			modelShader->use();
		}
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
	}

	// draw the track, sleeper, train
	Material trainMaterial = {
		glm::vec3(0.89225f, 0.19225f, 0.19225f),
		glm::vec3(0.80754f, 0.50754f, 0.50754f),
		glm::vec3(0.808273f, 0.508273f, 0.508273f),
		128.0f
	};
	InstanceDrawer trackInstance(RenderDatabase::SLIVER_MATERIAL);
	InstanceDrawer sleeperInstance(RenderDatabase::SLIVER_MATERIAL);
	InstanceDrawer pierInstance(RenderDatabase::SLIVER_MATERIAL);
	InstanceDrawer trainInstance(trainMaterial);

	const float track_width = 5;
	Pnt3f last_sleeper(999, 999, 999);
	Pnt3f last_pier(999, 999, 999);
	int num_point = m_pTrack->points.size();

	bool trainDrawed = false;
	float presentArcLength = 0;

	for (int i = 0; i < num_point; ++i) {

		// pos
		Pnt3f cp_pos_p0 = m_pTrack->points[(i + num_point - 1) % num_point].pos;
		Pnt3f cp_pos_p1 = m_pTrack->points[i].pos;
		Pnt3f cp_pos_p2 = m_pTrack->points[(i + 1) % num_point].pos;
		Pnt3f cp_pos_p3 = m_pTrack->points[(i + 2) % num_point].pos;
		// orient
		Pnt3f cp_orient_p0 = m_pTrack->points[(i + num_point - 1) % num_point].orient;
		Pnt3f cp_orient_p1 = m_pTrack->points[i].orient;
		Pnt3f cp_orient_p2 = m_pTrack->points[(i + 1) % num_point].orient;
		Pnt3f cp_orient_p3 = m_pTrack->points[(i + 2) % num_point].orient;

		float cp_pos_x[4] = { cp_pos_p0.x,cp_pos_p1.x,cp_pos_p2.x,cp_pos_p3.x };
		float cp_pos_y[4] = { cp_pos_p0.y,cp_pos_p1.y,cp_pos_p2.y,cp_pos_p3.y };
		float cp_pos_z[4] = { cp_pos_p0.z,cp_pos_p1.z,cp_pos_p2.z,cp_pos_p3.z };
		float cp_orient_x[4] = { cp_orient_p0.x,cp_orient_p1.x,cp_orient_p2.x,cp_orient_p3.x };
		float cp_orient_y[4] = { cp_orient_p0.y,cp_orient_p1.y,cp_orient_p2.y,cp_orient_p3.y };
		float cp_orient_z[4] = { cp_orient_p0.z,cp_orient_p1.z,cp_orient_p2.z,cp_orient_p3.z };

		//dynamic change divide line
		float DIVIDE_LINE = (MathHelper::distance(cp_pos_p0, cp_pos_p1) + MathHelper::distance(cp_pos_p1, cp_pos_p2) + MathHelper::distance(cp_pos_p2, cp_pos_p3)) * DIVIDE_LINE_SCALE;
		//printf("%f\n", DIVIDE_LINE);

		float M[16];
		float linearMatrix[16] = {
			0,0,0,0,
			0,0,-1,1,
			0,0,1,0,
			0,0,0,0
		};
		float cardinalMatrix[16] = {
			-1,2,-1,0,
			3,-5,0,2,
			-3,4,1,0,
			1,-1,0,0
		};
		float bSplineMatrix[16] = {
			-1,3,-3,1,
			3,-6,0,4,
			-3,3,3,1,
			1,0,0,0
		};
		if (tw->splineBrowser->value() == TrainWindow::LINEAR) { //linear
			std::copy(std::begin(linearMatrix), std::end(linearMatrix), std::begin(M));
			for (int i = 0; i < 16; i++) {
				M[i] /= 1.0f;
			}
		}
		else if (tw->splineBrowser->value() == TrainWindow::CARDINAL) { //cardinal
			std::copy(std::begin(cardinalMatrix), std::end(cardinalMatrix), std::begin(M));
			for (int i = 0; i < 16; i++) {
				M[i] /= 2.0f;
			}
		}
		else { // B-spline
			std::copy(std::begin(bSplineMatrix), std::end(bSplineMatrix), std::begin(M));
			for (int i = 0; i < 16; i++) {
				M[i] /= 6.0f;
			}
		}
		MathHelper::GxM(cp_pos_x, M);
		MathHelper::GxM(cp_pos_y, M);
		MathHelper::GxM(cp_pos_z, M);
		MathHelper::GxM(cp_orient_x, M);
		MathHelper::GxM(cp_orient_y, M);
		MathHelper::GxM(cp_orient_z, M);

		float percent = 1.0f / DIVIDE_LINE;
		float t = 0;
		Pnt3f qt(MathHelper::MxT(cp_pos_x, t), MathHelper::MxT(cp_pos_y, t), MathHelper::MxT(cp_pos_z, t));

		bool finalRound = false;
		while (!finalRound) {
			Pnt3f qt0 = qt;
			t += percent;
			if (t >= 1) {
				finalRound = true;
				t = 1;
			}
			qt = Pnt3f(MathHelper::MxT(cp_pos_x, t), MathHelper::MxT(cp_pos_y, t), MathHelper::MxT(cp_pos_z, t));
			Pnt3f qt1 = qt;
			glLineWidth(1);
			if (!doingShadows)
				glColor3ub(32, 32, 64);
			Pnt3f orient_t(MathHelper::MxT(cp_orient_x, t), MathHelper::MxT(cp_orient_y, t), MathHelper::MxT(cp_orient_z, t));
			orient_t.normalize();
			Pnt3f cross_t = ((qt1 + qt0 * -1) * orient_t);
			cross_t.normalize();
			cross_t = cross_t * (track_width / 2);
			//draw track
			static Pnt3f lastDir(0, -1, 0);
			static Pnt3f lastUp(0, 0, 1);
			static Pnt3f lastPos(qt1);
			Pnt3f difference = qt1 - qt0;
			Pnt3f trackUp = (cross_t * difference).glmvec3();
			float remoteness = (qt0 + (-1) * eyepos).len() * 0.2 + 100;
			float Accuracy = (remoteness) / (remoteness - 1) - 0.01;
			if ((difference.len2() > 0 && MathHelper::cos(lastDir, difference) < Accuracy || MathHelper::cos(lastUp, cross_t) < Accuracy || (lastPos - qt1).len2() > 10000) || finalRound) {
				Pnt3f trackCenter1 = (qt1 + lastPos + cross_t * 2) * 0.5f;
				Pnt3f trackCenter2 = (qt1 + lastPos + cross_t * -2) * 0.5f;
				Pnt3f trackFront = qt1 - lastPos;
				glm::mat4 trackModel1 = MathHelper::getTransformMatrix(trackCenter1.glmvec3(), trackFront.glmvec3(), trackUp.glmvec3(), glm::vec3(0.3, 0.3, trackFront.len() + 0.15));
				glm::mat4 trackModel2 = MathHelper::getTransformMatrix(trackCenter2.glmvec3(), trackFront.glmvec3(), trackUp.glmvec3(), glm::vec3(0.3, 0.3, trackFront.len() + 0.15));
				trackInstance.addModelMatrix(trackModel1);
				trackInstance.addModelMatrix(trackModel2);
				lastDir = difference;
				lastPos = qt1;
				lastUp = cross_t;
			}

			Pnt3f sleeperDistance = qt1 + (-1 * last_sleeper);
			Pnt3f pierDistance = qt1 + (-1 * last_pier);
			bool needToDrawTrain = false;
			if (sleeperDistance.len() > 5 || finalRound) {
				//draw sleeper
				glm::vec3 up = glm::cross(cross_t.glmvec3(), (qt1 + qt0 * -1).glmvec3());
				glm::mat4 sleeperModel = MathHelper::getTransformMatrix(qt1.glmvec3(), (qt1 + qt0 * -1).glmvec3(), up, glm::vec3(10, 0.5, 2));
				sleeperInstance.addModelMatrix(sleeperModel);
				last_sleeper = qt1;
			}
			if ((pierDistance.len2() > 111 || finalRound) && trackUp.y > 0) {
				// draw pier
				Pnt3f trackCenter1 = (qt0 + qt1 + cross_t * 2) * 0.5f;
				Pnt3f trackCenter2 = (qt0 + qt1 + cross_t * -2) * 0.5f;
				Pnt3f pierCenter1 = trackCenter1;
				Pnt3f pierCenter2 = trackCenter2;
				pierCenter1.y = pierCenter1.y / 2 - 50;
				pierCenter2.y = pierCenter2.y / 2 - 50;
				Pnt3f pierFront = qt1 - qt0;
				pierFront.y = 0;
				pierFront.normalize();
				glm::mat4 pierModel1 = MathHelper::getTransformMatrix(pierCenter1.glmvec3(), glm::vec3(0, 1, 0), pierFront.glmvec3(), glm::vec3(0.4, 0.4, trackCenter1.y + 100));
				glm::mat4 pierModel2 = MathHelper::getTransformMatrix(pierCenter2.glmvec3(), glm::vec3(0, 1, 0), pierFront.glmvec3(), glm::vec3(0.4, 0.4, trackCenter2.y + 100));
				pierInstance.addModelMatrix(pierModel1);
				pierInstance.addModelMatrix(pierModel2);
				last_pier = qt1;
			}
			if (tw->arcLength->value() == false) {
				if (!trainDrawed && t_time * m_pTrack->points.size() >= i + t && t_time * m_pTrack->points.size() <= i + t + percent)
					needToDrawTrain = true;
			}
			else {
				presentArcLength += (qt1 + (-1 * qt0)).len();
				if (!trainDrawed && presentArcLength / totalArcLength >= t_time)
					needToDrawTrain = true;
			}
			if (needToDrawTrain) {
				trainFront = qt1 + qt0 * -1;
				trainFront.normalize();
				trainUp = cross_t * trainFront;
				trainUp.normalize();
				trainPos = Pnt3f(qt1 + trainUp * 4);
				if (animationFrame == 0) {
					//draw train
					if (!USE_MODEL && !tw->trainCam->value()) {
						glm::mat4 trainModel = MathHelper::getTransformMatrix(trainPos.glmvec3(), trainFront.glmvec3(), trainUp.glmvec3(), glm::vec3(6, 8, 10));
						trainInstance.addModelMatrix(trainModel);
					}
				}
				Pnt3f trainLightPosition = qt1 + trainFront * 5.1 + trainUp * 1;
				float position2[] = { qt1.x + trainFront.x, qt1.y + trainFront.y, qt1.z + trainFront.z, 1.0f };
				float direction[] = { trainFront.x,trainFront.y,trainFront.z };

				//update train velocity
				float heightGradient = (qt1.y - qt0.y) / MathHelper::distance(qt1, qt0);
				trainVelocity = MathHelper::lerp(trainVelocity, tw->speed->value() - heightGradient * 10, 0.3);
				if (trainVelocity < tw->speed->value() / 5) trainVelocity = tw->speed->value() / 5;
				trainDrawed = true;
			}
		}
	}
	totalArcLength = presentArcLength;
	if (animationFrame > 0) {
		gigaDrillBreak();
	}
	if (USE_MODEL)
		pierInstance.setTexture(islandHeightTexture);
	pierInstance.drawByInstance(pierShader, hollowCube, true);
	if (tw->drawShadow->value()) {
		trackInstance.drawByInstance(simpleInstanceObjectShader, hollowCube, false);
		sleeperInstance.drawByInstance(simpleInstanceObjectShader, cube, false);
		trainInstance.drawByInstance(simpleInstanceObjectShader, cube, false);

		trackInstance.setTexture(islandHeightTexture);
		trackInstance.drawByInstance(instanceShadowShader, hollowCube);
		sleeperInstance.setTexture(islandHeightTexture);
		sleeperInstance.drawByInstance(instanceShadowShader, cube);
		trainInstance.setTexture(islandHeightTexture);
		trainInstance.drawByInstance(instanceShadowShader, cube);
	}
	else {
		trackInstance.drawByInstance(simpleInstanceObjectShader, hollowCube);
		sleeperInstance.drawByInstance(simpleInstanceObjectShader, cube);
		trainInstance.drawByInstance(simpleInstanceObjectShader, cube);
	}

	//draw rockets and targets
	InstanceDrawer rocketHeadInstance(RenderDatabase::RUBY_MATERIAL);
	InstanceDrawer rocketBodyInstance(RenderDatabase::SLIVER_MATERIAL);
	InstanceDrawer targetInstance(RenderDatabase::WHITE_PLASTIC_MATERIAL);
	InstanceDrawer targetFragInstance(RenderDatabase::WHITE_PLASTIC_MATERIAL);
	std::vector<glm::vec4> smoke;	// vec4 = (x, y, z, alpha)
	targetInstance.setTexture(this->getObjectTexture("targetImage"));
	targetFragInstance.setTexture(this->getObjectTexture("targetImage"));
	updateEntity();
	collisionJudge();
	for (int i = 0; i < rockets.size(); i++) {
		if (rockets[i].state == 0) {
			glm::mat4 HeadModel = MathHelper::getTransformMatrix(
				rockets[i].pos.glmvec3(), rockets[i].front.glmvec3(), rockets[i].up.glmvec3(),
				glm::vec3(4, 4, 3));
			glm::mat4 BodyModel = MathHelper::getTransformMatrix(
				(rockets[i].pos + rockets[i].front * -5.5).glmvec3(), rockets[i].front.glmvec3(), rockets[i].up.glmvec3(),
				glm::vec3(3.5, 3.5, 8));
			rocketHeadInstance.addModelMatrix(HeadModel);
			rocketBodyInstance.addModelMatrix(BodyModel);

			// add smoke partical
			//for (int j = 0; j < 20; j++) {
			//	for (int k = 0; k < 20; k++) {
			//		Pnt3f smokePos = rockets[i].pos + rockets[i].front * -(10 + 3 * j) + randUnitVector() * (0.5 * j);
			//		smoke.push_back(glm::vec4(smokePos.x, smokePos.y, smokePos.z, (float)(j / 20.0)));
			//	}
			//}

			if (smokeGenerator.size() <= i) {
				smokeGenerator.push_back(particleSystem.addParticleGenerator_pointer(particleShader));
			}
			smokeGenerator[i]->setPosition(rockets[i].pos.glmvec3());
			smokeGenerator[i]->setDirection(-rockets[i].front.glmvec3());
			smokeGenerator[i]->setParticleLife(10);
			smokeGenerator[i]->setGenerateRate(20);
			smokeGenerator[i]->setGravity(0);
			smokeGenerator[i]->setParticleVelocityRandomOffset(0.5);
			smokeGenerator[i]->setParticleVelocity(1);
			smokeGenerator[i]->setAngle(40);
			smokeGenerator[i]->setParticleSize(0.6);
			smokeGenerator[i]->setColor(glm::vec3(1.0, 1.0, 0.0), glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.5, 0.5, 0.5), 0.5);
		}
	}for (int i = rockets.size(); i < smokeGenerator.size(); i++) {
		smokeGenerator[i]->setGenerateRate(0);
	}
	if (tw->drawShadow->value()) {
		rocketHeadInstance.drawByInstance(simpleInstanceObjectShader, cone, false);
		rocketBodyInstance.drawByInstance(simpleInstanceObjectShader, cylinder, false);
		rocketHeadInstance.setTexture(islandHeightTexture);
		rocketBodyInstance.setTexture(islandHeightTexture);
		rocketHeadInstance.drawByInstance(instanceShadowShader, cone);
		rocketBodyInstance.drawByInstance(instanceShadowShader, cylinder);
	}
	else {
		rocketHeadInstance.drawByInstance(simpleInstanceObjectShader, cone);
		rocketBodyInstance.drawByInstance(simpleInstanceObjectShader, cylinder);
	}
	//if (smoke.size() > 0)
	//	drawSmoke(smoke);

	for (int i = 0; i < targets.size(); i++) {
		if (targets[i].state == 0) {
			glm::mat4 targetModel = MathHelper::getTransformMatrix(
				targets[i].pos.glmvec3(), targets[i].front.glmvec3(), targets[i].up.glmvec3(),
				glm::vec3(10, 10, 1));
			targetInstance.addModelMatrix(targetModel);
		}
	}
	if (tw->drawShadow->value()) {
		targetInstance.drawByInstance(simpleInstanceObjectShader, cylinder, false);
		targetInstance.setTexture(islandHeightTexture);
		targetInstance.drawByInstance(instanceShadowShader, cylinder);
	}
	else
		targetInstance.drawByInstance(simpleInstanceObjectShader, cylinder);
	for (int i = 0; i < targetFrags.size(); i++) {
		glm::mat4 targetFragModel = MathHelper::getTransformMatrix(
			targetFrags[i].pos.glmvec3(), targetFrags[i].front.glmvec3(), targetFrags[i].up.glmvec3(),
			glm::vec3(10, 10, 1));
		targetFragInstance.addModelMatrix(targetFragModel);
	}
	if (tw->drawShadow->value()) {
		targetFragInstance.drawByInstance(simpleInstanceObjectShader, sector, false);
		targetFragInstance.setTexture(islandHeightTexture);
		targetFragInstance.drawByInstance(instanceShadowShader, sector);
	}
	else
		targetFragInstance.drawByInstance(simpleInstanceObjectShader, sector);

	//draw axis
	if (!USE_MODEL) {
		glLineWidth(5);
		glBegin(GL_LINES);
		if (!doingShadows) {
			glColor3f(1, 0, 0);
		}
		glVertex3f(0, 0, 0);
		glVertex3f(20, 0, 0);
		if (!doingShadows) {
			glColor3f(0, 1, 0);
		}
		glVertex3f(0, 0, 0);
		glVertex3f(0, 20, 0);
		if (!doingShadows) {
			glColor3f(0, 0, 1);
		}
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 20);
		glEnd();
		glLineWidth(1);
	}

}

// 
//************************************************************************
//
// * this tries to see which control point is under the mouse
//	  (for when the mouse is clicked)
//		it uses OpenGL picking - which is always a trick
//########################################################################
// TODO: 
//		if you want to pick things other than control points, or you
//		changed how control points are drawn, you might need to change this
//########################################################################
//========================================================================
void TrainView::
doPick()
//========================================================================
{
	// since we'll need to do some GL stuff so we make this window as 
	// active window
	make_current();

	// where is the mouse?
	int mx = Fl::event_x();
	int my = Fl::event_y();

	// get the viewport - most reliable way to turn mouse coords into GL coords
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Set up the pick matrix on the stack - remember, FlTk is
	// upside down!
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPickMatrix((double)mx, (double)(viewport[3] - my),
		5, 5, viewport);

	// now set up the projection
	setProjection();

	// now draw the objects - but really only see what we hit
	GLuint buf[100];
	glSelectBuffer(100, buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	// draw the cubes, loading the names as we go
	for (size_t i = 0; i < m_pTrack->points.size(); ++i) {
		glLoadName((GLuint)(i + 1));
		m_pTrack->points[i].draw();
	}

	// go back to drawing mode, and see how picking did
	int hits = glRenderMode(GL_RENDER);
	if (hits) {
		// warning; this just grabs the first object hit - if there
		// are multiple objects, you really want to pick the closest
		// one - see the OpenGL manual 
		// remember: we load names that are one more than the index
		selectedCube = buf[3] - 1;
	}
	else // nothing hit, nothing selected
		selectedCube = -1;

	printf("Selected Cube %d\n", selectedCube);
}

Pnt3f TrainView::randUnitVector() {
	int range = 100;
	int a = -range + (rand() % (2 * range));
	int b = -range + (rand() % (2 * range));
	int c = -range + (rand() % (2 * range));
	Pnt3f v(a / 100.0f, b / 100.0f, c / 100.0f);
	v.normalize();
	return v;
}

void TrainView::addTarget()
{
	using namespace std;
	int rangeX = (140) - (-260);
	int rangeY = 100;
	int rangeZ = (350) - (-70);
	int x = -260 + (rand() % rangeX);
	int y = 5 + rand() % (rangeY);
	int z = -70 + (rand() % rangeZ);

	Pnt3f front = randUnitVector();
	front.normalize();
	targets.push_back(Entity(Pnt3f(x, y, z), front, front * Pnt3f(1, 0, 0)));
}

void TrainView::addMoreTarget()
{
	for (int i = 0; i < 10; i++) {
		addTarget();
	}
}

// Today is Friday in California
void TrainView::shoot()
{
	lastShootTime = tw->clock_time;
	lookingFront.normalize();
	lookingUp.normalize();
	Rocket rocket(trainPos + lookingFront * 10, lookingFront, lookingUp);
	if (USE_MODEL)
		rocket = Rocket(trainPos + trainFront * 4 + trainUp * 5 + lookingFront * 15, lookingFront, lookingUp);
	rocket.thrusterVelocity = lookingFront * 4;
	rockets.push_back(rocket);

	soundSource_RPGshot->Play(RPGshot);

	//if (animationFrame == 0) {
	//	animationFrame = 1;
	//}
}

// judge the sleeperDistance of target and rocket
void TrainView::collisionJudge()
{
	for (int targetID = 0; targetID < targets.size(); targetID++) {
		for (int rocketID = 0; rocketID < rockets.size(); rocketID++) {
			if (targets[targetID].state == 0 && rockets[rocketID].state == 0) {
				if (MathHelper::segmentIntersectCircle(
					rockets[rocketID].pos, rockets[rocketID].lastPos,
					targets[targetID].pos, targets[targetID].front, 5)) {

					targets[targetID].state = 1;
					rockets[rocketID].state = 1;

					lastExplodeTime = tw->clock_time;
					lastExplodePos = targets[targetID].pos;

					//target explode paricle effect
					//smoke
					ParticleGenerator& g1 = particleSystem.addParticleGenerator(particleShader);
					g1.setPosition(targets[targetID].pos.glmvec3());
					g1.setLife(2);
					g1.setColor(glm::vec3(0.078f, 0.078f, 0.078f), glm::vec3(0.273f, 0.273f, 0.273f), glm::vec3(0.273f, 0.273f, 0.273f), 0.7);
					g1.setParticleVelocity(3);
					g1.setParticleVelocityRandomOffset(1);
					g1.setFriction(0.85);
					g1.setParticleLife(65);
					g1.setParticleLifeRandomOffset(15);
					g1.setGenerateRate(80);
					g1.setGravity(-0.07);
					g1.setParticleSize(0.5);
					//outer fire
					ParticleGenerator& g2 = particleSystem.addParticleGenerator(particleShader);
					g2.setPosition(targets[targetID].pos.glmvec3());
					g2.setLife(2);
					g2.setColor(glm::vec3(0.98f, 0.99f, 0.039f), glm::vec3(0.98f, 0.99f, 0.039f), glm::vec3(0.98f, 0.99f, 0.039f), 0.5);
					g2.setParticleVelocity(7);
					g2.setParticleVelocityRandomOffset(2);
					g2.setFriction(0.95);
					g2.setParticleLife(100);
					g2.setGenerateRate(80);
					g2.setGravity(0.15);
					g2.setParticleSize(0.3);
					//inner fire
					ParticleGenerator& g3 = particleSystem.addParticleGenerator(particleShader);
					g3.setPosition(targets[targetID].pos.glmvec3());
					g3.setLife(2);
					g3.setColor(glm::vec3(1.0f, 0.105f, 0.039f), glm::vec3(1.0f, 0.621f, 0.0195f), glm::vec3(1.0f, 0.914f, 0.0195f), 0.7);
					g3.setParticleVelocity(3);
					g3.setParticleVelocityRandomOffset(1);
					g3.setFriction(0.85);
					g3.setParticleLife(40);
					g3.setParticleLifeRandomOffset(10);
					g3.setGenerateRate(80);
					g3.setGravity(0);
					g3.setParticleSize(0.7);

					soundSource_targetExplosion->Play(targetExplosion);
				}
			}
		}
	}
}

void TrainView::updateEntity() {
	if (tw->runButton->value()) {
		for (int rocketID = 0; rocketID < rockets.size(); rocketID++) {
			if (rockets[rocketID].state > 1 || rockets[rocketID].pos.len2() > 1000000 || rockets[rocketID].pos.y < -150) {
				rockets.erase(rockets.begin() + rocketID);
				rocketID--;
				continue;
			}
			else if (rockets[rocketID].state > 0) {
				// TODO: EXPLOSION!
				rockets[rocketID].state++;
			}
			else {
				// move it
				rockets[rocketID].advance();
			}
		}

		for (int targetID = 0; targetID < targets.size(); targetID++) {
			if (targets[targetID].state > 0) {
				// add its fragments
				for (int i = 0; i < 3; i++) {
					Pnt3f front = randUnitVector();
					PhysicalEntity frag(targets[targetID].pos + 2.5 * randUnitVector(), front, front * randUnitVector());
					frag.velocity = (frag.pos - targets[targetID].pos) * (0.5 + (rand() % 30) / 10.0);
					frag.angularVelocity = randUnitVector() * (rand() % 10);
					targetFrags.push_back(frag);
				}
				// delete this
				targets.erase(targets.begin() + targetID);
				targetID--;
				continue;
			}
		}
		for (int fragID = 0; fragID < targetFrags.size(); fragID++) {
			if (targetFrags[fragID].state < 1000 && targetFrags[fragID].pos.y>-5) {
				targetFrags[fragID].advance();
				targetFrags[fragID].state++;
			}
			else {
				// delete
				targetFrags.erase(targetFrags.begin() + fragID);
				fragID--;
				continue;
			}
		}
	}
}

void TrainView::gigaDrillBreak()
{
	using namespace MathHelper;


	Material trainMaterial = {
		glm::vec3(0.89225f, 0.19225f, 0.19225f),
		glm::vec3(0.80754f, 0.50754f, 0.50754f),
		glm::vec3(0.808273f, 0.508273f, 0.508273f),
		128.0f
	};
	InstanceDrawer trainInstance(trainMaterial);
	InstanceDrawer drillInstance(RenderDatabase::SLIVER_MATERIAL);
	InstanceDrawer blackLineInstance(RenderDatabase::SLIVER_MATERIAL);
	drillInstance.setTexture(this->getObjectTexture("drillImage"));

	static Pnt3f originalFront;
	static Pnt3f originalUp;
	static Pnt3f horizontalFront;
	static Pnt3f targetFront;
	static Pnt3f targetUp;
	static Pnt3f smallDrillPos[12];
	static bool exploded = false;
	glm::mat4 trainModel;
	glm::mat4 tankModel;
	glm::mat4 trainRotate;

	if (animationFrame == 1) {
		Pnt3f trainRight = trainFront * trainUp;
		originalUp = trainUp;
		originalFront = trainFront;
		horizontalFront = trainFront * cos(camRotateX) + trainRight * sin(camRotateX);
		horizontalFront.normalize();
		targetFront = horizontalFront * cos(camRotateY) + trainUp * sin(camRotateY);
		targetUp = horizontalFront * -sin(camRotateY) + trainUp * cos(camRotateY);
		targetFront.normalize();
		targetUp.normalize();
		for (int i = 0; i < 12; i++) {
			smallDrillPos[i] = randUnitVector() + Pnt3f(0, 0, (randomFloat() * 2 - 1));
		}
		exploded = false;
		soundSource_GDBEffect->Play(GDBEffect);
	}
	else if (animationFrame <= keyFrame[1]) {
		// turn horizontally
		float t = (animationFrame - keyFrame[0]) / (keyFrame[1] - keyFrame[0]);
		if (t < 0)
			t = 0;
		glm::vec3 front = lerpVec3(originalFront.glmvec3(), horizontalFront.glmvec3(), t);
		trainFront = Pnt3f(front);
		trainFront.normalize();
		trainUp.normalize();
		trainRotate = getTransformMatrix(trainPos.glmvec3(), front, trainUp.glmvec3(), glm::vec3(1, 1, 1));
		if (USE_MODEL) {
			//tankModel = MathHelper::getTransformMatrix(trainPos.glmvec3(), -trainFront.glmvec3(), trainUp.glmvec3(), glm::vec3(5, 5, 5));
			//modelShader->use();
			//modelShader->setMat4("model", tankModel);
			//tank->Draw(modelShader);
		}
		else {
			trainModel = getTransformMatrix(trainPos.glmvec3(), front, trainUp.glmvec3(), glm::vec3(6, 8, 10));
			trainInstance.addModelMatrix(trainModel);
		}
	}
	else if (animationFrame <= keyFrame[3]) {
		// turn to the sky
		float t = (animationFrame - keyFrame[2]) / (keyFrame[3] - keyFrame[2]);
		if (t < 0)
			t = 0;
		glm::vec3 up = lerpVec3(originalUp.glmvec3(), targetUp.glmvec3(), t);
		glm::vec3 front = lerpVec3(horizontalFront.glmvec3(), targetFront.glmvec3(), t);
		trainFront = Pnt3f(front);
		trainFront.normalize();
		trainUp = Pnt3f(up);
		trainUp.normalize();
		trainRotate = getTransformMatrix(trainPos.glmvec3(), front, up, glm::vec3(1, 1, 1));
		if (USE_MODEL) {
			//tankModel = MathHelper::getTransformMatrix(trainPos.glmvec3(), -trainFront.glmvec3(), trainUp.glmvec3(), glm::vec3(5, 5, 5));
			//modelShader->use();
			//modelShader->setMat4("model", tankModel);
			//tank->Draw(modelShader);
		}
		else {
			trainModel = getTransformMatrix(trainPos.glmvec3(), front, up, glm::vec3(6, 8, 10));
			trainInstance.addModelMatrix(trainModel);
		}
	}
	else {
		trainUp = Pnt3f(targetUp);
		trainFront = Pnt3f(targetFront);
		if (animationFrame > keyFrame[10]) {
			if (animationFrame < keyFrame[11] - 1) {
				float f = animationFrame - keyFrame[10];
				trainPos = trainPos + trainFront * f * 10;
			}
			else {
				float f = animationFrame - keyFrame[11] + 20;
				if (animationFrame >= 316)
					f -= (std::min(animationFrame - 316, 326.0 - 316.0));
				trainPos = trainPos + trainFront * f * 10;
			}
		}
		trainRotate = getTransformMatrix(trainPos.glmvec3(), targetFront.glmvec3(), targetUp.glmvec3(), glm::vec3(1, 1, 1));
		if (USE_MODEL) {
			//tankModel = MathHelper::getTransformMatrix(trainPos.glmvec3(), -trainFront.glmvec3(), trainUp.glmvec3(), glm::vec3(5, 5, 5));
			//modelShader->use();
			//modelShader->setMat4("model", tankModel);
			//tank->Draw(modelShader);
		}
		else {
			trainModel = getTransformMatrix(trainPos.glmvec3(), targetFront.glmvec3(), targetUp.glmvec3(), glm::vec3(6, 8, 10));
			trainInstance.addModelMatrix(trainModel);
		}
	}

	if (animationFrame < keyFrame[4]) {
		float t = (animationFrame - 24) / (32 - 24);
		if (t < 0)
			t = 0;
		else {
			if (t > 1)
				t = 1;
			for (int i = 0; i < 12; i++) {
				glm::mat4 smallDrillModel = getTransformMatrix(
					(smallDrillPos[i] * (2 + (lerp(0, 5, t)) * (staticSpiralPower / 3))).glmvec3(),
					smallDrillPos[i].glmvec3(), (smallDrillPos[i] * Pnt3f(1, 0, 0)).glmvec3(),
					glm::vec3(1, 1, lerp(0, 12, t)) * (staticSpiralPower / 3));
				drillInstance.addModelMatrix(trainRotate * smallDrillModel);
			}
		}
	}
	else if (animationFrame < keyFrame[5]) {
		float t = (animationFrame - keyFrame[4]) / (keyFrame[5] - keyFrame[4]);
		if (t < 0)
			t = 0;
		else {
			glm::mat4 drillModel = getTransformMatrix(
				(trainPos + trainFront * (7 + (lerp(0, 25, t)) * (staticSpiralPower / 3))).glmvec3(), trainFront.glmvec3(), trainUp.glmvec3(),
				glm::vec3(3, 3, lerp(0, 45, t)) * (staticSpiralPower / 3));
			drillInstance.addModelMatrix(drillModel);
			for (int i = 0; i < 12; i++) {
				glm::mat4 smallDrillModel = getTransformMatrix(
					(smallDrillPos[i] * (2 + (lerp(0, 5, 1 - t)) * (staticSpiralPower / 3))).glmvec3(),
					smallDrillPos[i].glmvec3(), (smallDrillPos[i] * Pnt3f(1, 0, 0)).glmvec3(),
					glm::vec3(1, 1, lerp(0, 12, 1 - t)) * (staticSpiralPower / 3));
				drillInstance.addModelMatrix(trainRotate * smallDrillModel);
			}
		}
	}
	else if (animationFrame < keyFrame[7]) {
		float t = (animationFrame - keyFrame[6]) / (keyFrame[7] - keyFrame[6]);
		if (t < 0)
			t = 0;
		// yeah, it is not a circle, so it will tramble when rotating!
		glm::mat4 drillModel = getTransformMatrix(
			(trainPos + trainFront * (7 + 25 * (staticSpiralPower / 3))).glmvec3(), trainFront.glmvec3(), trainUp.glmvec3(),
			glm::vec3(3 + lerp(0, 35, t), 3 + lerp(0, 34, t), 45) * (staticSpiralPower / 3));
		drillInstance.addModelMatrix(drillModel);
	}
	else if (animationFrame < keyFrame[13]) {
		float t = (animationFrame - keyFrame[8]);
		if (t < 0)
			t = 0;
		if (animationFrame >= 316 && animationFrame <= 326)
			t = 0;
		Pnt3f trainRight = trainFront * trainUp;
		trainRight.normalize();
		Pnt3f rotatingUp = trainUp * cos(t * 6.28 * 0.072 * 4) + trainRight * sin(t * 6.28 * 0.072 * 4);
		rotatingUp.normalize();
		glm::mat4 drillModel = getTransformMatrix(
			(trainPos + trainFront * (7 + 25 * (staticSpiralPower / 3))).glmvec3(), trainFront.glmvec3(), rotatingUp.glmvec3(),
			glm::vec3(38, 37, 45) * (staticSpiralPower / 3));
		drillInstance.addModelMatrix(drillModel);

		if (animationFrame > keyFrame[9]) {
			drillShader->use();
			drillShader->setFloat("z_rotation", t * 6.28 * 0.072);
			if (RenderDatabase::timeScale == RenderDatabase::BULLET_TIME_SCALE)
				drillShader->setBool("slow", true);
			else
				drillShader->setBool("slow", false);
			blackLineInstance.addModelMatrix(drillModel);
		}

		if (animationFrame >= 330 && !exploded) {
			targetChainExplosionStart(Entity(Pnt3f(0, 0, 0), Pnt3f(0, 0, -1), Pnt3f(0, 1, 0)));
			exploded = true;
		}
	}
	else {
		animationFrame = 0;
	}

	if (tw->drawShadow->value() && animationFrame < keyFrame[10]) {
		if (!USE_MODEL) {
			trainInstance.drawByInstance(simpleInstanceObjectShader, cube, false);
			trainInstance.setTexture(islandHeightTexture);
			trainInstance.drawByInstance(instanceShadowShader, cube);
		}
		drillInstance.drawByInstance(simpleInstanceObjectShader, cone, false);
		drillInstance.setTexture(islandHeightTexture);
		drillInstance.drawByInstance(instanceShadowShader, cone);
	}
	else {
		drillInstance.drawByInstance(simpleInstanceObjectShader, cone);
		if (!USE_MODEL) {
			trainInstance.drawByInstance(simpleInstanceObjectShader, cube);
		}
	}
	blackLineInstance.drawByInstance(drillShader, cone);
	glUseProgram(0);
}

//call by trainWindow every clock
void TrainView::updateParticleSystem() {
	particleSystem.update();
}

void TrainView::targetChainExplosionStart(Entity startTarget) {
	if (targetChainExplosionStartTime != INFINITY) return;

	//sort tartget pos
	for (int i = 0; i < targets.size(); i++) {
		for (int j = 0; j < targets.size() - 1 - i; j++) {
			if (MathHelper::distance(startTarget.pos, targets[j].pos) > MathHelper::distance(startTarget.pos, targets[j + 1].pos)) {
				//swap
				Entity temp = targets[j];
				targets[j] = targets[j + 1];
				targets[j + 1] = temp;
			}
		}
	}

	targetChainExplosionStartTime = tw->clock_time;
	targetChainExplosionFrameCount = 0;
}

void TrainView::targetChainExplosionUpdate() {
	if (targetChainExplosionStartTime == INFINITY) return;
	if (targets.size() == 0) {
		targetChainExplosionStartTime = INFINITY;
		return;
	}

	float animationTime = tw->clock_time - targetChainExplosionStartTime;
	targetChainExplosionFrameCount += RenderDatabase::timeScale;
	if (targetChainExplosionFrameCount < 1) return;
	targetChainExplosionFrameCount -= 1;

	int explosionNum = (int)(animationTime / 30.0f) + 1;
	for (int targetID = 0; targetID < explosionNum && targetID < targets.size(); targetID++) {
		targets[targetID].state = 1;
		//target explode paricle effect
		//smoke
		ParticleGenerator& g1 = particleSystem.addParticleGenerator(particleShader);
		g1.setPosition(targets[targetID].pos.glmvec3());
		g1.setLife(2);
		g1.setColor(glm::vec3(1.0f, 0.105f, 0.039f), glm::vec3(0.078f, 0.078f, 0.078f), glm::vec3(0.078f, 0.078f, 0.078f), 0.7);
		g1.setParticleVelocity(3);
		g1.setParticleVelocityRandomOffset(1);
		g1.setFriction(0.85);
		g1.setParticleLife(80);
		g1.setParticleLifeRandomOffset(15);
		g1.setGenerateRate(80);
		g1.setGravity(-0.07);
		g1.setParticleSize(0.5);
		//outer fire
		ParticleGenerator& g2 = particleSystem.addParticleGenerator(particleShader);
		g2.setPosition(targets[targetID].pos.glmvec3());
		g2.setLife(2);
		g2.setColor(glm::vec3(0.98f, 0.99f, 0.039f), glm::vec3(0.98f, 0.99f, 0.039f), glm::vec3(0.98f, 0.99f, 0.039f), 0.5);
		g2.setParticleVelocity(20);
		g2.setParticleVelocityRandomOffset(2);
		g2.setParticleLife(100);
		g2.setGenerateRate(80);
		g2.setGravity(0.15);
		g2.setParticleSize(0.3);
		//inner fire
		ParticleGenerator& g3 = particleSystem.addParticleGenerator(particleShader);
		g3.setPosition(targets[targetID].pos.glmvec3());
		g3.setLife(2);
		g3.setColor(glm::vec3(1.0f, 0.105f, 0.039f), glm::vec3(1.0f, 0.621f, 0.0195f), glm::vec3(1.0f, 0.914f, 0.0195f), 0.7);
		g3.setParticleVelocity(3);
		g3.setParticleVelocityRandomOffset(1);
		g3.setFriction(0.85);
		g3.setParticleLife(40);
		g3.setParticleLifeRandomOffset(10);
		g3.setGenerateRate(80);
		g3.setGravity(0);
		g3.setParticleSize(1.2);

		lastExplodeTime = tw->clock_time;
	}

	if (animationTime > keyFrame[13]) {
		targetChainExplosionStartTime = INFINITY;
	}
}

// 取得執行檔所在目錄，並將路徑分隔符號轉換為 '/'
std::string TrainView::getExecutableDir() {
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);           // 取得完整路徑
	std::string fullPath(buffer);

	// 將 '\' 轉換為 '/'
	for (char& c : fullPath) {
		if (c == '\\') {
			c = '/';
		}
	}

	// 取得目錄部分（去除檔案名稱）
	return fullPath.substr(0, fullPath.find_last_of("/")) + "/";
}