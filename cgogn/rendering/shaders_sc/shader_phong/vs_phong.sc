$input a_position, a_normal
$output v_position, v_normal
#include "common.sh"

void main() {
	vec4 position = u_modelView * vec4(a_position, 1.0);
	v_position = position.xyz;
	
	v_normal = mul(u_modelView, vec4(a_normal, 0.0) ).xyz;
	gl_Position = u_proj * position;
}
