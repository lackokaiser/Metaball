#version 430

layout(location = 0) in vec2 uv_coords;

out vec2 vs_uv;

void main(){
	gl_Position = vec4(uv_coords, 0, 1);
	vs_uv = uv_coords;
}