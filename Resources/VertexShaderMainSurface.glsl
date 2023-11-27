in vec2 position;
in vec2 surfacePosition;
in vec3 texcoord;
in vec3 maskcoord;
out vec3 Texcoord;
out vec3 Maskcoord;
out vec2 SurfacePosition;
void main()
{
    Texcoord = texcoord;
    Maskcoord = maskcoord;
    SurfacePosition[0] = (surfacePosition[0]+1.0f)/2.0f;
    SurfacePosition[1] = (surfacePosition[1]+1.0f)/2.0f;
    gl_Position = vec4(position,0,1);
};
