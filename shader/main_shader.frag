#version 150 core
in vec3 f_color;
in vec3 f_normal;
in vec3 f_position;
in vec4 f_position_lightspace;
out vec4 outColor;
uniform vec3 lightPos;
uniform vec3 lightcolor;
uniform vec3 viewPos;
uniform bool if_uni_color;
uniform bool if_reflection;
uniform bool if_refraction;
uniform bool shadow_red;
uniform vec3 uni_color;
uniform samplerCube skyboxmap;
uniform sampler2D shadowmap;


float ShadowCalc(vec4 fragPosLightSpace, float bias)
{
    vec3 projcoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projcoords = projcoords * 0.5 + 0.5;
    float closestdepth = texture(shadowmap, projcoords.xy).r;
    float curdepth = projcoords.z;
    float shadow = (curdepth - bias) > closestdepth ? 1.0 : 0.0;
    return shadow;
}

void main()
{
    float ambientStrength = 0.1;
    float specularStrength = 0.5;
    vec3 norm = normalize(f_normal);
    //ambient
    vec3 ambient = ambientStrength * lightcolor;
    //diffuse
    vec3 lightdir = normalize(lightPos - f_position);
    float diff = max(dot(norm, lightdir), 0.0);
    vec3 diffuse = diff * lightcolor;
    //specular
    vec3 viewdir = normalize(viewPos - f_position);
    vec3 reflectdir = reflect(-lightdir, norm);
    float spec = pow(max(dot(viewdir,reflectdir), 0.0), 30);
    vec3 specular = specularStrength * spec * lightcolor;
    //shadow
    float bias = max(0.05 * (1.0 - dot(norm, lightdir)), 0.005);
    float shadow = ShadowCalc(f_position_lightspace, bias);
    //reflection
    vec3 reflection_dir = reflect(-viewdir, norm);
    //refraction
    vec3 refraction_dir = refract(-viewdir, norm, 1.00/1.52);
    //results
    vec3 shadow_color = (ambient + (1.0 - shadow) * (diffuse + specular)) * f_color;
    vec4 reflection_color = (shadow == 1.0) ? vec4(shadow_color, 1.0) : mix(vec4(shadow_color,1.0), vec4(texture(skyboxmap, reflection_dir).rgb, 1.0), 0.8);
    vec4 refraction_color = (shadow == 1.0) ? vec4(shadow_color, 1.0) : mix(vec4(shadow_color,1.0), vec4(texture(skyboxmap, refraction_dir).rgb, 1.0), 0.8);
 
    if(if_uni_color)
        outColor = vec4(uni_color, 1.0);
    else if(shadow_red && shadow == 1.0)
        outColor = vec4(1.0, 0.0, 0.0, 1.0);
    else if(if_reflection)
        outColor = reflection_color;
    else if(if_refraction)
        outColor = refraction_color;
    else
        outColor = vec4(shadow_color, 1.0); 
}