$input a_position, a_color0, a_normal
$output v_color0, v_normal, v_view

uniform mat4 u_transform;
uniform mat4 u_ModelView;
uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat4 u_Projection;
uniform vec4 u_LightPos;
uniform vec4 u_CameraPos;


#include <bgfx_shader.sh>


void main()
{
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
    vec3 normal = a_normal * 2.0 - 1.0;
    v_normal = mul(u_modelView, vec4(normal.xyz, 0.0) ).xyz;
    v_view = mul(u_modelView, vec4(a_position, 1.0) ).xyz;
    gl_Position = u_transform * vec4(a_position, 1.0);    
    v_color0 = a_color0;
}