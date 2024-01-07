$input a_position
$output v_color0

#include <bgfx_shader.sh>

uniform mat4 u_transform;

void main()
{
	gl_Position = u_transform * vec4(a_position, 1.0); //mul(u_transform, vec4(a_position, 1.0) );
	v_color0 = vec4(a_position, 1.0); //vec4(1.0, 0.0, 0.0, 1.0);
}