$input v_position

uniform vec4 front_color;
uniform vec4 back_color;
uniform vec4 ambiant_color;
uniform vec3 light_position;
uniform bool double_side;
uniform bool ghost_mode;

#include <bgfx_shader.sh>

void main()
{
	vec3 N = normalize(cross(dFdx(v_position), dFdy(v_position)));
	vec3 L = normalize(light_position - v_position);
	float lambert = dot(N, L);
	if (ghost_mode)
		lambert = 0.4 * pow(1.0 - lambert, 2);
	if (gl_FrontFacing)
		gl_FragColor = vec4(ambiant_color.rgb + lambert * front_color.rgb, front_color.a);
	else
		if (!double_side)
			discard;
		else gl_FragColor = vec4(ambiant_color.rgb + lambert * back_color.rgb, back_color.a);


}