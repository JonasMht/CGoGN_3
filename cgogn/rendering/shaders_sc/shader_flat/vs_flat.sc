$input a_position
$output v_position

uniform mat4 projection_matrix;
uniform mat4 model_view_matrix;

void main() {
	vec4 position = model_view_matrix * vec4(a_position, 1.0);
	v_position = position.xyz;
	gl_Position = projection_matrix * position;
}
