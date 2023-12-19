#version 430 core

layout( location = 0 ) in vec2 vs_in_uv;

// a pipeline-ban tovább adandó értékek
out vec3 vs_out_pos;
out vec3 vs_out_norm;
out vec2 vs_out_tex;

// shader külső paraméterei
uniform mat4 world;
uniform mat4 worldIT;
uniform mat4 viewProj;

uniform float ElapsedTimeInSec = 0.0;

vec3 GetPos(float u, float v)
{
	vec3 pos = vec3(-10.0, 0.0, 10.0) + vec3( 20.0, 0.0, -20.0) * vec3(u, 0.0, v);
	pos.y = sin( pos.z + ElapsedTimeInSec );

	return pos;
}

vec3 GetNorm(float u, float v)
{
	vec3 du = GetPos(u + 0.01, v) - GetPos(u - 0.01, v);
	vec3 dv = GetPos(u, v + 0.01) - GetPos(u, v - 0.01);

	return normalize(cross(du, dv));
}

void main()
{
	vec3 vs_in_pos = GetPos(vs_in_uv.x, vs_in_uv.y);
	gl_Position = viewProj * world * vec4( vs_in_pos, 1 );
	
	vs_out_pos = (world * vec4(vs_in_pos, 1)).xyz;
	vs_out_norm = (worldIT * vec4(GetNorm(vs_in_uv.x, vs_in_uv.y), 0)).xyz;
	vs_out_tex = vs_in_uv;
}