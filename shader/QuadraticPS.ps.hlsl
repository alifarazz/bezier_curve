float4 QuadraticPS(float2 p
                   : TEXCOORD0, float4 color
                   : COLOR0)
    : COLOR
{ // Gradients
    float2 px = ddx(p);
    float2 py = ddy(p); // Chain rule
    float fx = (2 * p.x) * px.x - px.y;
    float fy = (2 * p.x) * py.x - py.y;
    // Signed distance
    float sd = (p.x * p.x - p.y) / sqrt(fx * fx + fy * fy);
    // Linear alpha
    float alpha = 0.5 - sd;
    if (alpha > 1)
        // Inside
        color.a = 1;
    else if (alpha < 0)
        // Outside
        clip(-1);
    else
        // Near boundary
        color.a = alpha;
    return color;
}