//#version 450 core

uniform uvec2 size;
uniform highp dvec2 xRange;
uniform dvec2 yRange;
uniform int maxIters;
uniform int frame;

out vec4 outColor;

double map(double value, double inputMin, double inputMax, double outputMin, double outputMax)
{
    return outputMin + ((outputMax - outputMin) / (inputMax - inputMin)) * (value - inputMin);
}

double rand(float s) {
    return fract(sin(s * 12.9898) * 43758.5453);
}

int get_iterations()
{
    dvec2 screen_pos = gl_FragCoord.xy;
    dvec2 pos = screen_pos + dvec2(rand(frame), rand(frame));

    double x0 = map(pos.x, 0, size.x, xRange.x, xRange.y);
    double y0 = map(pos.y, 0, size.y, yRange.x, yRange.y);

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

vec4 return_value()
{
    int iter = get_iterations();
    if (iter == maxIters)
    {
        gl_FragDepth = 0.0f;
        return vec4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    return vec4(get_color(iter).xyz, 1.0 / (frame + 1.0));
}

void main()
{
    if (frame < 100)
        outColor = return_value();
}