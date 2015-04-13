 #pragma once

 #include "../../nclgl/OGLRenderer.h"
 #include "../../nclgl/HeightMap.h"
 #include "../../nclgl/Camera.h"
 #include "../../nclgl/OBJMesh.h"
 #define POST_PASSES 10

 class Renderer : public OGLRenderer {
 public :
 Renderer ( Window & parent );
 virtual ~ Renderer ( void );

 virtual void RenderScene ();
 virtual void UpdateScene ( float msec );

 protected :
 void PresentScene ();
 void DrawPostProcess ();
 void DrawScene ();
 void GenerateScreenTexture(GLuint &into, bool depth = false);
 Matrix4 preView;
 Matrix4 preModel;
 bool   isfirst;
 float posx;
 float velocity;


 Shader * sceneShader ;
 Shader * processShader;
 Shader * combineShader ;

 Camera * camera ;

 Mesh * quad ;
 Mesh * quad2;
 Mesh * sphere;
 HeightMap * heightMap ;

 GLuint bufferFBO ;
 GLuint processFBO ;
 GLuint bufferColourTex[2];
 GLuint bufferDepthTex[2];

 int count;
 int num;
 bool closeBlur;
 };