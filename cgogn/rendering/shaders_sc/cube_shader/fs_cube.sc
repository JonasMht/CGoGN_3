$input v_color0, v_normal, v_view

uniform vec4 u_Ia;
uniform vec4 u_Id;
uniform vec4 u_Ka;
uniform vec4 u_Kd;
uniform vec4 u_LightPos;


#define Ia u_Ia.xyz
#define Ka u_Ka.x
#define Id u_Id.xyz
#define Kd u_Kd.x
#define LightPos u_LightPos.xyz

#include <common.sh>

void main()
{
    vec3 toLight = LightPos - v_view;
    vec3 lightDir = normalize(toLight);
    
    vec3 ambientLighting = Ia * Ka;
    float dotNormal = max(dot(v_normal, lightDir),0.0);
    vec3 diffuseLighting = Id * Kd * dotNormal;
    vec3 totalLighting = diffuseLighting + ambientLighting;
    gl_FragColor = vec4(totalLighting, 1.0) * v_color0;
}