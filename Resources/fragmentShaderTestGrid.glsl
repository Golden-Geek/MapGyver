in vec3 Texcoord;
in vec3 Maskcoord;
in vec2 SurfacePosition;
out vec4 outColor;
uniform sampler2D tex;
uniform sampler2D mask;
uniform vec4 borderSoft;
uniform int invertMask;
uniform float ratio;

const float PI = 3.14159265359;

float map(float value, float min1, float max1, float min2, float max2) {
	return min2 + ((max2-min2)*(value-min1)/(max1-min1)); 
};

void main()
{
	outColor = textureProj(tex, Texcoord);
	vec3 test = Texcoord;
	test[0] /= test[2];
	test[1] /= test[2];
	test[2] = 0;
	test[0] -= 0.5;
	test[1] -= 0.5;
	test[0] *= ratio;

	float circle = length(test)*PI;
	circle = sin(circle * 4);
	circle = abs(circle);
	circle = 1-step(0.05, circle);

	float cross = abs(abs(test[0]) - abs(test[1]));
	cross = 1-step(0.005, cross);

	float gridX = abs(fract(test[0]*8));
	gridX = 1-step(0.04, gridX);

	float gridY = abs(fract(test[1]*8));
	gridY = 1-step(0.04, gridY);

	float chan = circle+cross+gridX+gridY;

	vec4 c = vec4(1-chan, 1-chan,1-chan, 1);
	outColor = c;


}