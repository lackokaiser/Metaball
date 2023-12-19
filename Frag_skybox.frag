#version 430

// pipeline-ból bejövő per-fragment attribútumok
in vec3 vs_out_pos;

out vec4 fs_out_col;

// procedurális színek
//version 1
uniform vec3 groundColor = vec3( 0.2, 0.2, 0.2 );
uniform vec3 skyColor = vec3( 0.2, 0.2, 0.5 );
//version 2
uniform vec3 groundColor0 = vec3( 0.2, 0.2, 0.2 );
uniform vec3 groundColor1 = vec3( 0.4, 0.4, 0.4 );
uniform vec3 skyColor0 = vec3( 0.2, 0.2, 0.5 );
uniform vec3 skyColor1 = vec3( 0.4, 0.4, 0.7 );

// skybox textúra
uniform samplerCube skyboxTexture;

void main()
{
	//version 1
	//fs_out_col = vec4( mix(groundColor, skyColor, normalize(vs_out_pos).y * 0.5 + 0.5 ), 1.0);

	//version 2
	//if ( vs_out_pos.y < 0.0 )
	//{
	//	fs_out_col = vec4( mix(groundColor0, groundColor1, normalize(vs_out_pos).y + 1.0 ), 1.0);
	//}
	//else
	//{
	//	fs_out_col = vec4( mix(skyColor0, skyColor1, normalize(vs_out_pos).y ), 1.0);
	//}

	// skybox textúra
	fs_out_col = texture( skyboxTexture, vs_out_pos );
}