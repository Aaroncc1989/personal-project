#include "Renderer.h"

Renderer::Renderer(Window & parent) : OGLRenderer(parent) {
	camera = new Camera(-20.0f, 180.0f, Vector3(2000, 500, 0));
	quad = Mesh::GenerateQuad();
	//sphere = new OBJMesh("../../Meshes/sphere.obj");
	sphere = Mesh::GenerateQuad();
	posx = 2200.f;
	velocity = 0.5f;

	heightMap = new HeightMap("../../Textures/terrain.raw");
	heightMap->SetTexture(
		SOIL_load_OGL_texture("../../Textures/ground.jpg",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	sphere->SetTexture(
		SOIL_load_OGL_texture("../../Textures/ball.jpg", 
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0));

	sceneShader = new Shader("../../Shaders/TexturedVertex.glsl",
		"../../Shaders/TexturedFragment.glsl");
	processShader = new Shader("../../Shaders/processVertex.glsl",
		"../../Shaders/processfrag.glsl");

	if (!processShader->LinkProgram() || !sceneShader->LinkProgram() ||
		!heightMap->GetTexture()) {
		return;
	}

	SetTextureRepeating(heightMap->GetTexture(), true);
	SetTextureRepeating(sphere->GetTexture(), true);

	GenerateScreenTexture(bufferColourTex[0]);
	GenerateScreenTexture(bufferColourTex[1]);
	GenerateScreenTexture(bufferDepthTex[0], true);
	GenerateScreenTexture(bufferDepthTex[1], true);
	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &processFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, bufferDepthTex[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, bufferColourTex[0], 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, bufferDepthTex[1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, bufferColourTex[1],0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_DEPTH_TEST);
	init = true;
	isfirst = true;
	count = 0;
	num = 50;

	closeBlur = false;
}


Renderer ::~Renderer(void){
	delete sceneShader;
	delete processShader;
	currentShader = NULL;

	delete heightMap;
	delete quad;
	delete camera;
	delete sphere;

	glDeleteTextures(2, bufferColourTex);
	glDeleteTextures(2, bufferDepthTex);
	glDeleteFramebuffers(1, &bufferFBO);
}


void Renderer::UpdateScene(float msec) {
	camera->UpdateCamera(msec);
	viewMatrix = camera->BuildViewMatrix();
	if (posx < 2000 || posx>3000)
	{
		velocity = -velocity;
	}
	posx += velocity;
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_Q))
	{
		closeBlur = !closeBlur;
	}
}

void Renderer::RenderScene() {
	DrawScene();
	//DrawPostProcess();
	//PresentScene();
	if (!closeBlur){
		if (count == 0)
		{
			glAccum(GL_LOAD, 1.0 / num);
		}
		else
		{
			glAccum(GL_ACCUM, 1.0 / num);
		}

		count++;

		if (count >= num)
		{
			count = 0;
			glAccum(GL_RETURN, 1.0);
			SwapBuffers();
		}
	}
	else
	{
		SwapBuffers();
	}

}
void Renderer::DrawScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	SetCurrentShader(sceneShader);
	glEnable(GL_DEPTH_TEST);
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f,
		(float)width / (float)height, 45.0f);

	modelMatrix.SetScalingVector(Vector3(100, 100, 100));
	modelMatrix.SetPositionVector(Vector3(posx, 500, 1000));
	UpdateShaderMatrices();
	sphere->Draw();
	glUseProgram(0);
}

void Renderer::DrawPostProcess()
{
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	SetCurrentShader(sceneShader);
	Matrix4 tmpmod = modelMatrix;
	Matrix4 tmpview = viewMatrix;
	modelMatrix = preModel;
	viewMatrix = preView;

	UpdateShaderMatrices();
	sphere->Draw();
	preModel = tmpmod;
	preView = tmpview;
	glUseProgram(0);
}

void Renderer::PresentScene()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	SetCurrentShader(processShader);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "tex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex[1]);

	projMatrix = Matrix4::Orthographic(-1, 1, 1, -1, -1, 1);
	viewMatrix.ToIdentity();
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();
	quad->SetTexture(bufferColourTex[0]);
	quad->Draw();
	glUseProgram(0);
}


void Renderer::GenerateScreenTexture(GLuint & into, bool depth) {
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0,
		depth ? GL_DEPTH_COMPONENT24 : GL_RGBA8,
		width, height, 0,
		depth ? GL_DEPTH_COMPONENT : GL_RGBA,
		GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
}