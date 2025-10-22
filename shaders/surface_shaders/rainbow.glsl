float _hue2rgb(float p, float q, float t) {
    if (t < 0.0) t += 1.0;
    if (t > 1.0) t -= 1.0;
    if (t < 1.0/6.0) return p + (q - p) * 6.0 * t;
    if (t < 0.5) return q;
    if (t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6.0;
    return p;
}

vec3 hsl2rgb(vec3 hsl) {
    float H = fract(hsl.x);
    float S = clamp(hsl.y, 0.0, 1.0);
    float L = clamp(hsl.z, 0.0, 1.0);
    if (S == 0.0) return vec3(L); // achromatic
    float q = (L < 0.5) ? (L * (1.0 + S)) : (L + S - L * S);
    float p = 2.0 * L - q;
    return vec3(
        _hue2rgb(p, q, H + 1.0/3.0),
        _hue2rgb(p, q, H),
        _hue2rgb(p, q, H - 1.0/3.0)
    );
}

SurfaceShaderResult surfaceShader_Rainbow(SurfaceShaderData data) {
    SurfaceShaderResult result;
    // We render a solid emissive sweep, so disable non-emissive contribution.
    result.color = vec3(0.0);
    result.emissive = vec3(0.0);
    result.emissiveStrength = data.emissiveStrength;
    result.ambient = 0.0;
    result.diffuse = 0.0;
    result.specular = 0.0;
    result.shininess = data.shininess;

    // Parameters you can tweak:
    vec3 sweepDir = normalize(vec3(1.0, 0.5, 0.25)); // sweep direction in world space
    float freq = 2.0;    // spatial frequency of the sweep
    float speed = 0.002; // time speed of the sweep

    // Cosine-driven phase -> map to [0,1] for hue
    float phase = dot(data.hitPoint, sweepDir) * freq + data.time * speed;
    float cosVal = cos(phase);
    float hue = fract(0.5 + 0.5 * cosVal); // smooth circular mapping with cosine

    // Solid vivid colors: full saturation, mid lightness
    float saturation = 1.0;
    float lightness = 0.5;
    vec3 emissiveColor = hsl2rgb(vec3(hue, saturation, lightness));

    // Apply emissive strength from material data
    result.emissive = emissiveColor * result.emissiveStrength;

    return result;
}
