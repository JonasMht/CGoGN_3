$input v_position, v_normal

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
	vec3 N = v_normal;
	vec3 L = normalize(light_position - v_position);
	vec3 E = normalize(-v_position.xyz); 
    vec3 H = normalize(L + E);


	float Kd = max(dot(L, N), 0.0);
    vec3 diffuse = Kd * front_color.rgb;

	vec3 specular = vec3(0.0);
    if (Kd > 0.0f)
    {
        float Ks = max(0.0, dot(H, N));
        specular = pow(Ks, 10) * vec3(1.0, 1.0, 1.0);
    }
	

	float opacity = 1.0;
	if (ghost_mode){
		opacity = 0.5;

	}

	if (!gl_FrontFacing)
		opacity*=2;
	

	gl_FragColor = vec4(diffuse+specular, opacity);
	
}