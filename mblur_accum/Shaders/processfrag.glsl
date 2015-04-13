 #version 150 core

 uniform sampler2D diffuseTex;
 uniform sampler2D tex;

 in Vertex{
 vec2 texCoord;
 } IN;

 out vec4 gl_FragColor;

 void main (void)
 {	
	vec4 a = texture2D(diffuseTex, IN.texCoord);
	//vec4 b = texture2D(tex, IN.texCoord);
	gl_FragColor = a;
 }
