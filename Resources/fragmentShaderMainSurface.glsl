in vec3 Texcoord;
in vec3 Maskcoord;
in vec2 SurfacePosition;
out vec4 outColor;
uniform sampler2D tex;
uniform sampler2D mask;
uniform vec4 borderSoft;
uniform int invertMask;

float map(float value, float min1, float max1, float min2, float max2) {
	return min2 + ((max2-min2)*(value-min1)/(max1-min1)); 
};

void main()
{
	outColor = textureProj(tex, Texcoord);
    vec2 tex2D = Texcoord.xy / Texcoord.z;
    if (tex2D.x>1 || tex2D.x<0 ||tex2D.y>1 || tex2D.y<0) 
    { outColor = vec4(0,0,0,0); }
   	vec4 maskColor = textureProj(mask, Maskcoord);
   	float alpha = 1.0f;
   	if (SurfacePosition[1] > 1-borderSoft[0])    {alpha *= map(SurfacePosition[1],1.0f,1-borderSoft[0],0.0f,1.0f);} // top
   	if (SurfacePosition[0] > 1.0f-borderSoft[1]) {alpha *= map(SurfacePosition[0], 1.0f, 1 - borderSoft[1], 0.0f, 1.0f); } // right
   	if (SurfacePosition[1] < borderSoft[2])      {alpha *= map(SurfacePosition[1],0.0f,borderSoft[2],0.0f,1.0f);} // bottom
   	if (SurfacePosition[0] < borderSoft[3])      {alpha *= map(SurfacePosition[0],0.0f,borderSoft[3],0.0f,1.0f);} // left
	float maskValue = invertMask == 0 ? maskColor[1] : 1-maskColor[1];
   	alpha *= maskValue; 
   	outColor[3] = alpha;
};
