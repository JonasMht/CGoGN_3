$input a_position
$output v_position

#include <bgfx_shader.sh>

void main() {
	vec4 position = u_modelView * vec4(a_position, 1.0);
	v_position = position.xyz;
	gl_Position = u_proj * position;
}
