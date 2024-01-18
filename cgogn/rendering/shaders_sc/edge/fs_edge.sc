$input v_position

uniform vec4 color_;
uniform vec4 ambiant_color_;
uniform vec4 light_position_;
uniform vec4 param1_;

#define point_size param1_.x

#include "common.sh"


void main()
{

	gl_FragColor = color_;
}