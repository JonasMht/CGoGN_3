$input v_position

uniform vec4 front_color;
uniform vec4 back_color;
uniform vec4 ambiant_color;
uniform vec4 light_position_;
uniform vec4 params;

#define light_position light_position_.xyz
#define double_side_ params.x
#define ghost_mode_ params.y

#include "common.sh"


void main()
{
	bool double_side = double_side_ > 0.5;	
	bool ghost_mode = ghost_mode_ > 0.5;
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