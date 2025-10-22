SurfaceShaderResult surfaceShader_Toon(SurfaceShaderData data) {
    SurfaceShaderResult result;
    result.color = vec3(0.0);
    result.emissive = vec3(0.0);
    result.emissiveStrength = data.emissiveStrength;
    result.ambient = data.ambient;
    result.diffuse = data.diffuse;
    result.specular = data.specular;
    result.shininess = data.shininess;
    
    vec3 finalColor = data.ambient * data.surfaceColor;
    
    // Toon shading with discrete light levels
    for (int i = 0; i < pushConst.numLights; i++) {
        vec4 lightData = lightBuffer.lightData[i];
        vec3 lightPos = lightData.xyz;
        float intensity = lightData.w;
        
        float shadowFactor = getShadowFactor(data.hitPoint, lightPos);
        if (shadowFactor < 0.5) continue;
        
        vec3 lightVec = lightPos - data.hitPoint;
        float distance = length(lightVec);
        vec3 lightDir = lightVec / distance;
        
        float attenuation = intensity / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        
        // Quantize diffuse lighting into discrete bands
        float diffuseRaw = max(0.0, dot(data.normal, lightDir));
        float diffuseQuantized = diffuseRaw > 0.6 ? 1.0 : (diffuseRaw > 0.3 ? 0.5 : 0.0);
        vec3 diffuse = data.diffuse * diffuseQuantized * data.surfaceColor * attenuation;
        
        // Hard edge specular
        vec3 halfDir = normalize(lightDir + data.viewDir);
        float specularRaw = pow(max(0.0, dot(data.normal, halfDir)), data.shininess);
        float specularQuantized = specularRaw > 0.8 ? 1.0 : 0.0;
        vec3 specular = data.surfaceColor * data.specular * specularQuantized * attenuation;
        
        finalColor += (diffuse + specular);
    }
    
    result.color = finalColor;
    
    // Add emissive if material has it
    if (data.emissiveStrength > 0.0) {
        result.emissive = data.surfaceColor * data.emissiveStrength;
    }
    
    return result;
}
