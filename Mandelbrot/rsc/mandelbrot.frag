//#version 400

uniform uvec2 size;
uniform dvec2 xRange;
uniform dvec2 yRange;
uniform int maxIters;

out vec4 outColor;

double map(double value, double inputMin, double inputMax, double outputMin, double outputMax)
{
	return outputMin + ((outputMax - outputMin) / (inputMax - inputMin)) * (value - inputMin);
}

int get_iterations()
{
    double x0 = map(gl_FragCoord.x, 0, size.x, xRange.x, xRange.y);
    double y0 = map(gl_FragCoord.y, 0, size.y, yRange.y, yRange.x);
 
    double x = 0;
    double y = 0;
    double x2 = 0;
    double y2 = 0;

    int i;
    for (i = 0; i < maxIters && x2 + y2 <= 4; i++)
    {
        y = 2 * x * y + y0;
        x = x2 - y2 + x0;
        x2 = x * x;
        y2 = y * y;
    }
    return i;
}

#define product(a, b) dvec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x)

int get_iterations_2()
{
    double x0 = map(gl_FragCoord.x, 0, size.x, xRange.x, xRange.y);
    double y0 = map(gl_FragCoord.y, 0, size.y, yRange.y, yRange.x);

    dvec2 c = dvec2(x0, y0);
    dvec2 z = dvec2(0, 0);

    int i;
    for (i = 0; i < maxIters && z.x*z.x + z.y*z.y <= 4; i++)
    {
        z = product(z, z) + c;
    }
    return i;
}


vec4 return_value()
{
    int iter = get_iterations();
    if (iter == maxIters)
    {
        gl_FragDepth = 0.0f;
        return vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
 
    return vec4(get_color(iter).xyz, 1.0f);
}
 
void main()
{
    outColor = return_value();
}