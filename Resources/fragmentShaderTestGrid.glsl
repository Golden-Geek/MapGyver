#define     TAU 6.28318530717958647 // pi * 2
#define     PI  3.14159265358979323 // pi

// hsv to rgb
vec3 hsv2rgb(vec3 c) {
  vec3 rgb = clamp(abs(mod(c.x*6.+vec3(0.,4.,2.),6.)-3.)-1.,0.,1.);
  rgb = rgb * rgb * (3. - 2. * rgb);
  return c.z * mix(vec3(1.), rgb, c.y);
}

// sd polygon
const int N = 8;
float sdPolygon( in vec2 p, in vec2[N] v ) 
{
    int num = v.length();
    float d = dot(p-v[0],p-v[0]);
    float s = 1.0;
    for( int i=0, j=num-1; i<num; j=i, i++ )
    {
        // distance
        vec2 e = v[j] - v[i];
        vec2 w =    p - v[i];
        vec2 b = w - e*clamp( dot(w,e)/dot(e,e), 0.0, 1.0 );
        d = min( d, dot(b,b) );

        // winding number from http://geomalgorithms.com/a03-_inclusion.html
        bvec3 cond = bvec3( p.y>=v[i].y, 
                            p.y <v[j].y, 
                            e.x*w.y>e.y*w.x );
        if( all(cond) || all(not(cond)) ) s=-s;  
    }
    
    return s*sqrt(d);
}

// sd segment
float sdSegment( in vec2 p, in vec2 a, in vec2 b )
{
    vec2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}

// sd box
float sdBox( vec2 p, vec2 b, vec2 o)
{
    p -= o;
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

// sd circle
float sdCircle( vec2 p, float r )
{
    return length(p) - r;
}

float ndot(vec2 a, vec2 b ) { return a.x*b.x - a.y*b.y; }

// sd rhombus
float sdRhombus( in vec2 p, in vec2 b ) 
{
    p = abs(p);

    float h = clamp( ndot(b-2.0*p,b)/dot(b,b), -1.0, 1.0 );
    float d = length( p-0.5*b*vec2(1.0-h,1.0+h) );

	return d * sign( p.x*b.y + p.y*b.x - b.x*b.y );
}

// sd distorted pill
float sdDistortedPill(vec2 p, vec2 start, vec2 end, float radius, float power, float depth)
{
    p = -p;
    vec2 dir = normalize(end - start);

    vec2 localP = p - start;
    float proj = dot(localP, dir);
    float perpProj = dot(localP, vec2(-dir.y, dir.x));

    float alongLine = clamp(proj, 0.0, length(end - start));
    vec2 closestPoint = start + dir * alongLine;

    float d = length(p - closestPoint) - radius;

    float frequency = PI / length(end - start);
    float sind = sin(frequency * alongLine);
    sind = pow(sind, power);
    sind *= depth*radius;
    
    return d + sind;
}


// stroke edge
void stroke(float d, vec4 c, inout vec4 fragColor, float w)
{
    float m = 1.0-step(.5*w, abs(d));
    fragColor = mix(fragColor, c, m*c.a);
}

// stroke edge with anti-aliasing
void strokeAA(float d, vec4 c, inout vec4 fragColor, float w, float aa)
{
    float m = smoothstep(0.5 * (w + aa), 0.5 * (w - aa), abs(d));
    fragColor = mix(fragColor, c, m*c.a);
}

// background color
#define bg_c vec4(0.0, 0.0, 0.0, 1.0)

// grid color
#define grid_c vec4(.5, .5, .5, 1.0)

// grid line width
#define grid_w 2.0

// grid vertical divisions
// grid cross
#define grid_s vec2(9.0, 0.0)                     

// circlegrid color
#define circlegrid_c vec4(0.5)

// circlegrid line width
// circlegrid size
// circlegrid offset x
// circle grid offset y
#define circlegrid_s vec4(1.0, .25, 0.0, 0.0)

// circle color
#define circle_c vec4(1.0)

// circle line width
#define circle_w 2.0

// TL to BR diagonal color
#define diag1_c vec4(.5, 1.0, .75, 1.0)

// BL to TR diagonal color
#define diag2_c vec4(1.0, .5, .666, 1.0)

// diagonal line width
#define diag_w 2.0

// horizontal centerline color
#define centerh_c vec4(1.0, 1.0, .5, 1.0)

// vertical centerline color
#define centerv_c vec4(1.0, 1.0, .5, 1.0)

// center line width
#define center_w 2.0

// rainbow line width
// rainbow alpha
// rainbow period
// rainbow offset
#define rainbow_s vec4(3.0, 1.0, 2.0, .17)

// border color
#define border_c vec4(.5, 1.0, .5, 1.0)

// border line width
#define border_w 2.0

// logo color 1
#define logo_c1 vec4(0.0, 0.0, 0.0, 1.0)

// logo color 2
#define logo_c2 vec4(1.0, 1.0, 1.0, 1.0)

// logo color 3
#define logo_c3 vec4(.5, 0.5, 0.5, 1.0)

// logo type
// logo size
#define logo_s vec2(0.0, .1)

// grid
void grid(vec2 p, float px_size, inout vec4 fragColor)
{
    float gc = (1.0 - grid_s.y)*.5;
    vec2 grid = p - round(p*grid_s.x)/grid_s.x;
    grid *= grid_s.x;
    float d = min(abs(grid.x), abs(grid.y));
    vec2 cross = vec2(step(gc, abs(grid.x)), step(gc, abs(grid.y)));
    d += max(cross.x, cross.y);
    stroke(d, grid_c, fragColor, grid_s.x*grid_w*px_size);
}

// circle grid
void circleGrid(vec2 p, float px_size, inout vec4 fragColor)
{
    p -= circlegrid_s.zw/grid_s.x;
    vec2 cell = mod(p * vec2(grid_s.x), 1.0);
    vec2 center = mod(vec2(.5), 1.0);
    float radius = circlegrid_s.y;
    vec2 dist = cell - center;
    float d = length(dist) - radius;
    strokeAA(d, circlegrid_c, fragColor, circlegrid_s.x*grid_s.x * px_size, length(fwidth(p*grid_s.x)));
}

// circle
void circle(vec2 p, vec2 center, float radius, vec4 c, float px_size, inout vec4 fragColor)
{
    center = center + round(p);
    
    float d = length(p - center) - radius;
    strokeAA(d, c, fragColor, circle_w * px_size, length(fwidth(p)));
}

// rainbow circle
void rainbowCircle(vec2 p, vec2 center, float radius, vec4 c, float px_size, inout vec4 fragColor)
{
	vec2 pq = vec2(atan(p.x, p.y) / TAU*rainbow_s.z + rainbow_s.w, length(p));
	float d = length(p - center) - radius;
	c = vec4(hsv2rgb(vec3(pq.x, 1., 1.)), 1.0);
	strokeAA(d, c, fragColor, rainbow_s.x * px_size, length(fwidth(p)));
}

// horizontal center line
void centerH(vec2 p, float px_size, inout vec4 fragColor)
{
    stroke(abs(p.x), centerh_c, fragColor, center_w*px_size);
    p = mod(p, 1.0)-.5;
    stroke(abs(p.x), centerh_c, fragColor, center_w*px_size);
}

// vertical center line
void centerV(vec2 p, float px_size, inout vec4 fragColor)
{
    stroke(abs(p.y), centerv_c, fragColor, center_w*px_size);
    p = mod(p, 1.0)-.5;
    stroke(abs(p.y), centerh_c, fragColor, center_w*px_size);
}

// diagonal lines
void diagonals(vec2 p, float px_size, float aspect, inout vec4 fragColor)
{
	p += .5;
	p = mod(p, 1.0);
	//p = p - floor(p);

	float dist = abs(p.x - p.y);
	strokeAA(dist, diag1_c, fragColor, diag_w*px_size, length(fwidth(p)));
	
	dist = abs(p.x + p.y)-1.0;
	strokeAA(dist, diag2_c, fragColor, diag_w*px_size, length(fwidth(p)));
	//fragColor = vec4(vec3(dist), 1.0);
}

// render border
void border(vec2 p, vec2 res, inout vec4 fragColor)
{
	float d = min(min(p.x, 1.0-p.x)*res.x, min(p.y, 1.0-p.y)*res.y);
	fragColor = mix(fragColor, border_c, 1.0-step(border_w, d));
}


// mw logo
void mwLogo(vec2 p, vec2 offset, float scale, float aspect, float px_size, inout vec4 fragColor)
{
	p -= offset;
	p.y *= .6;
	p /= scale;
	
	
	float w = 20.0 * px_size;
	
	vec2 ra =vec2(0.5,0.5);
	float d1 = sdRhombus( vec2(p.x + .5, p.y), ra );
	float d2 = sdRhombus( vec2(p.x - .5, p.y), ra );
	fragColor = mix(fragColor, vec4(vec3(pow(min(d1+.95, d2+.95), 8.0)), 1.0)*logo_c3, (1.0-step(0.0, min(d1, d2)))*logo_c3.a);
	strokeAA(min(d1, d2), logo_c2, fragColor, w, length(fwidth(p)));
	strokeAA(min(d1+.1, d2+.1), logo_c2 * vec4(0.8), fragColor, w, length(fwidth(p)));
	strokeAA(min(d1+.2, d2+.2), logo_c2 * vec4(0.6), fragColor, w, length(fwidth(p)));
	strokeAA(min(d1+.3, d2+.3), logo_c2 * vec4(0.4), fragColor, w, length(fwidth(p)));
	
	float d3 = sdSegment(p, vec2(1.0, 0.0), vec2(-1.0, 0.0));
	strokeAA(d3, logo_c2, fragColor, w, length(fwidth(p)));
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // normalized pixel coordinates
    vec2 vUV = fragCoord/iResolution.xy;
    
    
    // resolution
    vec2 res = iResolution.xy;
    
    // aspect ratio
    float aspect = res.x / res.y;
	
	// -.5 to .5 coordinate space
	vec2 p = vUV.st-.5;
	
	vec2 logo_offset = vec2(0.0, 0.0);
	
	// pixel size
	float px_size = 0.0;
	
	// alignment for different aspect ratios
	if (aspect > 1.0)
	{
		p.x *= aspect;
		logo_offset.x = (aspect>1.0+logo_s.y*2.0)?.5 : .25;
		px_size = 1.0/res.y;
	} else {
		p.y /= aspect;
		logo_offset.y = (aspect<1.0-logo_s.y*2.0)?.5 : .25;
		px_size = 1.0/res.x;
	} 
	

	
	// background color
	fragColor = bg_c;
	
	// square grid
	grid(p, px_size, fragColor);
	
	// circle grid
	circleGrid(p, px_size, fragColor);
	
	// big circles
    circle(p, vec2(0), 0.5, circle_c, px_size, fragColor);
    
    // diagonal lines
    diagonals(p, px_size, aspect, fragColor);
    
    // vertical center lines
	centerV(p, px_size, fragColor);
	
	// horizontal center lines
	centerH(p, px_size, fragColor);
	
	// rainbow circle
    rainbowCircle(p, vec2(0), .333, vec4(1.0), px_size, fragColor);

	// border lines
    //vec2 b_pos = vec2(vUV.x * aspect, vUV.y);
    //b_pos = vUV.xy * res;
    border(vUV.st, res, fragColor);
    

    // mw logo
	mwLogo(p, logo_offset, logo_s.y, aspect, px_size, fragColor);
	mwLogo(p, -logo_offset, logo_s.y, aspect, px_size, fragColor);

	
	//pxGrid(p, px_size, fragColor);
}