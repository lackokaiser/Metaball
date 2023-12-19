#include "MyApp.h"
#include "SDL_GLDebugMessageCallback.h"
#include "ObjParser.h"
#include "ParametricSurfaceMesh.hpp"

#include <imgui.h>

#include <string>

CMyApp::CMyApp()
{
}

CMyApp::~CMyApp()
{
}

void CMyApp::SetupDebugCallback()
{
	// engedélyezzük és állítsuk be a debug callback függvényt ha debug context-ben vagyunk 
	GLint context_flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
	if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(SDL_GLDebugMessageCallback, nullptr);
	}
}

void CMyApp::InitShaders()
{
	/*m_programID = glCreateProgram();
	AssembleProgram( m_programID, "Vert_PosNormTex.vert", "Frag_Lighting.frag" );
	m_programWaterID = glCreateProgram();
	AssembleProgram( m_programWaterID, "Vert_Water.vert", "Frag_Lighting.frag" );

	m_programAxesID = glCreateProgram();
	AssembleProgram(m_programAxesID, "Vert_axes.vert", "Frag_PosCol.frag");

	m_trajectoryID = glCreateProgram();
	AssembleProgram(m_trajectoryID, "Vert_traj.vert", "Frag_PosCol.frag");*/

	m_metaballProgramID = glCreateProgram();
	AssembleProgram(m_metaballProgramID, "Vert_Metaball.vert", "Frag_Metaball.frag");


	//InitSkyboxShaders();
}

void CMyApp::InitSkyboxShaders()
{
	m_programSkyboxID = glCreateProgram();
	AssembleProgram( m_programSkyboxID, "Vert_skybox.vert", "Frag_skybox.frag" );
}

void CMyApp::CleanShaders()
{
	/*glDeleteProgram( m_programID );
	glDeleteProgram( m_programWaterID );
	glDeleteProgram(m_programAxesID);
	glDeleteShader(m_trajectoryID);*/
	glDeleteProgram(m_metaballProgramID);
	//CleanSkyboxShaders();
}

void CMyApp::CleanSkyboxShaders()
{
	glDeleteProgram( m_programSkyboxID );
}

struct Param
{
	glm::vec3 GetPos(float u, float v) const noexcept
	{
        return glm::vec3(u, v, 0.0);
    }

	glm::vec3 GetNorm(float u, float v) const noexcept
	{
        return glm::vec3(0.0, 0.0, 1.0);
    }

	glm::vec2 GetTex( float u, float v ) const noexcept
	{
        return glm::vec2(u, v);
    }
};

struct Water
{
	glm::vec3 GetPos(float u, float v) const noexcept
	{
		glm::vec3 pos = glm::vec3(-10.0, 0.0, 10.0) + glm::vec3( 20.0, 0.0, -20.0) * glm::vec3(u, 0.0, v);
		pos.y = sinf(pos.z);

		return pos;
	}

	glm::vec3 GetNorm(float u, float v) const noexcept
	{
		glm::vec3 du = GetPos(u + 0.01f, v) - GetPos(u - 0.01f, v);
		glm::vec3 dv = GetPos(u, v + 0.01f) - GetPos(u, v - 0.01f);

		return glm::normalize(glm::cross(du, dv));
	}

	glm::vec2 GetTex( float u, float v ) const noexcept
	{
        return glm::vec2(u, v);
    }
};

void CMyApp::InitGeometry()
{

	const std::initializer_list<VertexAttributeDescriptor> vertexAttribList =
	{
		{ 0, offsetof( glm::vec2, x ), 2, GL_FLOAT },
	};

	// Suzanne

	/*MeshObject<Vertex> suzanneMeshCPU = ObjParser::parse("Assets/Suzanne.obj");

	m_SuzanneGPU = CreateGLObjectFromMesh( suzanneMeshCPU, vertexAttribList );*/

	// Skybox
	InitSkyboxGeometry();

	// Water
	/*MeshObject<glm::vec2> waterCPU;
	{
		MeshObject<Vertex> surfaceMeshCPU = GetParamSurfMesh( Param(), 160, 80 );
		for ( const Vertex& v : surfaceMeshCPU.vertexArray )
		{
			waterCPU.vertexArray.emplace_back( glm::vec2( v.position.x, v.position.y ) );
		}
		waterCPU.indexArray = surfaceMeshCPU.indexArray;
	}
	m_waterGPU = CreateGLObjectFromMesh( waterCPU, { { 0, offsetof( glm::vec2,x), 2, GL_FLOAT}});*/

	// quad

	/*MeshObject<glm::vec2> quadMeshCPU;

	quadMeshCPU.vertexArray = {
		glm::vec2(-1, -1),
		glm::vec2(1, -1),
		glm::vec2(1, 1),
		glm::vec2(-1, 1),
	};

	quadMeshCPU.indexArray = {
		0, 1, 2,
		2,3,0
	};*/

	MeshObject<glm::vec2> quadGPU;
	{
		MeshObject<Vertex> surfaceMeshCPU = GetParamSurfMesh(Param(), 160, 80);
		for (const Vertex& v : surfaceMeshCPU.vertexArray)
		{
			quadGPU.vertexArray.push_back(glm::vec2(2 * v.position.x - 1, 2 * v.position.y - 1));
		}
		quadGPU.indexArray = surfaceMeshCPU.indexArray;
	}
	m_quadGPU = CreateGLObjectFromMesh(quadGPU, { { 0, offsetof(glm::vec2,x), 2, GL_FLOAT} });
}

void CMyApp::CleanGeometry()
{
	CleanOGLObject( m_SuzanneGPU );
	CleanOGLObject(m_quadGPU);
	CleanSkyboxGeometry();
}

void CMyApp::InitSkyboxGeometry()
{
	// skybox geo
	MeshObject<glm::vec3> skyboxCPU =
	{
		std::vector<glm::vec3>
		{
			// hátsó lap
			glm::vec3(-1, -1, -1),
			glm::vec3( 1, -1, -1),
			glm::vec3( 1,  1, -1),
			glm::vec3(-1,  1, -1),
			// elülső lap
			glm::vec3(-1, -1, 1),
			glm::vec3( 1, -1, 1),
			glm::vec3( 1,  1, 1),
			glm::vec3(-1,  1, 1),
		},

		std::vector<GLuint>
		{
			// hátsó lap
			0, 1, 2,
			2, 3, 0,
			// elülső lap
			4, 6, 5,
			6, 4, 7,
			// bal
			0, 3, 4,
			4, 3, 7,
			// jobb
			1, 5, 2,
			5, 6, 2,
			// alsó
			1, 0, 4,
			1, 4, 5,
			// felső
			3, 2, 6,
			3, 6, 7,
		}
	};

	m_SkyboxGPU = CreateGLObjectFromMesh( skyboxCPU, { { 0, offsetof( glm::vec3,x ), 3, GL_FLOAT } } );
}

void CMyApp::CleanSkyboxGeometry()
{
	CleanOGLObject( m_SkyboxGPU );
}

void CMyApp::InitTextures()
{
	// diffuse texture

	glGenTextures( 1, &m_SuzanneTextureID );
	TextureFromFile( m_SuzanneTextureID, "Assets/wood.jpg" );
	SetupTextureSampling( GL_TEXTURE_2D, m_SuzanneTextureID );

	glGenTextures( 1, &m_waterTextureID );
	TextureFromFile( m_waterTextureID, "Assets/water_texture.jpg" );
	SetupTextureSampling( GL_TEXTURE_2D, m_waterTextureID );

	glGenTextures(1, &m_glassTextureID );
	TextureFromFile(m_glassTextureID, "Assets/danger_glass.png");
	SetupTextureSampling(GL_TEXTURE_2D, m_glassTextureID);

	InitSkyboxTextures();
}

void CMyApp::CleanTextures()
{
	glDeleteTextures( 1, &m_SuzanneTextureID );
	glDeleteTextures( 1, &m_waterTextureID );
	glDeleteTextures(1, &m_glassTextureID);

	CleanSkyboxTextures();
}

void CMyApp::InitSkyboxTextures()
{
	// skybox texture

	glGenTextures( 1, &m_skyboxTextureID );
	TextureFromFile( m_skyboxTextureID, "Assets/lab_xpos.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_X );
	TextureFromFile( m_skyboxTextureID, "Assets/lab_xneg.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_X );
	TextureFromFile( m_skyboxTextureID, "Assets/lab_ypos.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_Y );
	TextureFromFile( m_skyboxTextureID, "Assets/lab_yneg.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y );
	TextureFromFile( m_skyboxTextureID, "Assets/lab_zpos.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_Z );
	TextureFromFile( m_skyboxTextureID, "Assets/lab_zneg.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z );
	SetupTextureSampling( GL_TEXTURE_CUBE_MAP, m_skyboxTextureID, false );

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void CMyApp::CleanSkyboxTextures()
{
	glDeleteTextures( 1, &m_skyboxTextureID );
}

bool CMyApp::Init()
{
	SetupDebugCallback();

	// törlési szín legyen kékes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);
	
	// Nem minden driver támogatja a vonalak és pontok vastagabb megjelenítését, ezért
	// lekérdezzük, hogy támogatott-e a GL_LINE_WIDTH_RANGE és GL_POINT_SIZE_RANGE tokenek.
	{
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glPointSize.xhtml
        GLfloat pointSizeRange[2] = { 0.0f, 0.0f };
        glGetFloatv(GL_POINT_SIZE_RANGE, pointSizeRange); // lekérdezzük a támogatott pontméretek tartományát
		glPointSize( std::min( 16.0f, pointSizeRange[ 1 ] ) ); // nagyobb pontok
    }

    {
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glLineWidth.xhtml
        GLfloat lineWidthRange[2] = { 0.0f, 0.0f };
        glGetFloatv(GL_LINE_WIDTH_RANGE, lineWidthRange); // lekérdezzük a támogatott vonalvastagság tartományát
		//glLineWidth( std::min( 4.0f, lineWidthRange[ 1 ] ) ); // vastagabb vonalak
	}

	InitShaders();
	InitGeometry();
	InitTextures();

	//
	// egyéb inicializálás
	//

	glEnable(GL_CULL_FACE); // kapcsoljuk be a hátrafelé néző lapok eldobását
	glCullFace(GL_BACK);    // GL_BACK: a kamerától "elfelé" néző lapok, GL_FRONT: a kamera felé néző lapok

	glEnable(GL_DEPTH_TEST); // mélységi teszt bekapcsolása (takarás)

	// kamera
	m_camera.SetView(
		glm::vec3(0.0, 7.0, 7.0),	// honnan nézzük a színteret	   - eye
		glm::vec3(0.0, 0.0, 0.0),   // a színtér melyik pontját nézzük - at
		glm::vec3(0.0, 1.0, 0.0));  // felfelé mutató irány a világban - up

	m_metaBalls.push_back(glm::vec4(1, 0, 0, 1));
	m_metaBalls.push_back(glm::vec4(-1, 0, 0, 1));

	return true;
}

void CMyApp::Clean()
{
	CleanShaders();
	CleanGeometry();
	CleanTextures();
}

void CMyApp::Update( const SUpdateInfo& updateInfo )
{
	m_ElapsedTimeInSec = updateInfo.ElapsedTimeInSec;
	m_camera.Update(updateInfo.DeltaTimeInSec);
}

void CMyApp::Render()
{
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT)...
	// ... és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(m_quadGPU.vaoID);
	glUseProgram(m_metaballProgramID);

	glUniform1f(ul("time"), m_ElapsedTimeInSec);
	glUniform1f(ul("tr"), m_tr);

	GLsizei balls = static_cast<GLsizei>(m_metaBalls.size());

	glUniform4fv(ul("balls"), balls, glm::value_ptr(m_metaBalls[0]));
	glUniform1i(ul("ballCount"), balls);

	glDrawElements(GL_TRIANGLES, m_quadGPU.count, GL_UNSIGNED_INT, nullptr);

	// shader kikapcsolasa
	glUseProgram(0);

	// - Textúrák kikapcsolása, minden egységre külön
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	// VAO kikapcsolása
	glBindVertexArray(0);

	//// Suzanne

	//glBindVertexArray( m_SuzanneGPU.vaoID );

	//// - Textúrák beállítása, minden egységre külön
	//glActiveTexture( GL_TEXTURE0 );
	//glBindTexture( GL_TEXTURE_2D, m_SuzanneTextureID );


	//glUseProgram( m_programID );

	//// - uniform parameterek beállítása
	//glm::vec3 suzanneForward = EvaluatePathTangent(); // Merre nézzen a Suzanne?
	//glm::vec3 suzanneWorldUp = glm::vec3(0, 1, 0);
	//if (fabsf(suzanneForward.y) > 0.99) {
	//	suzanneWorldUp = glm::vec3(1, 0, 0);
	//}
	//glm::vec3 suzanneRight = glm::normalize(glm::cross(suzanneForward, suzanneWorldUp)); // Jobbra nézése
	//glm::vec3 suzanneUp = glm::cross(suzanneRight, suzanneForward); // Felfelé nézése

	//// A három vektorból álló bázisvektorokat egy mátrixba rendezzük, hogy tudjuk velük forgatni a Suzanne-t
	//glm::mat4 suzanneRot(1.0f);
	//suzanneRot[0] = glm::vec4(suzanneForward, 0.0f);
	//suzanneRot[1] = glm::vec4(     suzanneUp, 0.0f);
	//suzanneRot[2] = glm::vec4(  suzanneRight, 0.0f);

	//// A Suzanne alapállásban a Z tengelyre néz, de nekünk az X tengelyre kell, ezért elforgatjuk
	//static const glm::mat4 suzanneTowardX = glm::rotate(glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));

	//glm::mat4 matWorld = glm::translate(EvaluatePathPosition()) * suzanneRot * suzanneTowardX;

	//glUniformMatrix4fv( ul( "world" ),    1, GL_FALSE, glm::value_ptr( matWorld ) );
	//glUniformMatrix4fv( ul( "worldIT" ),  1, GL_FALSE, glm::value_ptr( glm::transpose( glm::inverse( matWorld ) ) ) );

	//glUniformMatrix4fv( ul( "viewProj" ), 1, GL_FALSE, glm::value_ptr( m_camera.GetViewProj() ) );

	//// - Fényforrások beállítása
	//glUniform3fv( ul( "cameraPos" ), 1, glm::value_ptr( m_camera.GetEye() ) );
	//glUniform4fv( ul( "lightPos" ),  1, glm::value_ptr( m_lightPos ) );

	//glUniform3fv( ul( "La" ),		 1, glm::value_ptr( m_La ) );
	//glUniform3fv( ul( "Ld" ),		 1, glm::value_ptr( m_Ld ) );
	//glUniform3fv( ul( "Ls" ),		 1, glm::value_ptr( m_Ls ) );

	//glUniform1f( ul( "lightConstantAttenuation"	 ), m_lightConstantAttenuation );
	//glUniform1f( ul( "lightLinearAttenuation"	 ), m_lightLinearAttenuation   );
	//glUniform1f( ul( "lightQuadraticAttenuation" ), m_lightQuadraticAttenuation);

	//// - Anyagjellemzők beállítása
	//glUniform3fv( ul( "Ka" ),		 1, glm::value_ptr( m_Ka ) );
	//glUniform3fv( ul( "Kd" ),		 1, glm::value_ptr( m_Kd ) );
	//glUniform3fv( ul( "Ks" ),		 1, glm::value_ptr( m_Ks ) );

	//glUniform1f( ul( "Shininess" ),	m_Shininess );


	//// - textúraegységek beállítása
	//glUniform1i( ul( "texImage" ), 0 );

	//glDrawElements( GL_TRIANGLES,    
	//				m_SuzanneGPU.count,			 
	//				GL_UNSIGNED_INT,
	//				nullptr );

	//
	//// Viz

	//glBindVertexArray( m_waterGPU.vaoID );

	//// - Textúrák beállítása, minden egységre külön
	//glActiveTexture( GL_TEXTURE0 );
	//glBindTexture( GL_TEXTURE_2D, m_waterTextureID );

	//glUseProgram( m_programWaterID );
	//
	//matWorld = glm::translate(glm::vec3(0, -2, 0));

	//// Mivel másik shader-t használunk, ezért újra be kell állítani a uniform paramétereket
	//glUniformMatrix4fv( ul( "world" ),    1, GL_FALSE, glm::value_ptr( matWorld ) );
	//glUniformMatrix4fv( ul( "worldIT" ),  1, GL_FALSE, glm::value_ptr( glm::transpose( glm::inverse( matWorld ) ) ) );

	//glUniformMatrix4fv( ul( "viewProj" ), 1, GL_FALSE, glm::value_ptr( m_camera.GetViewProj() ) );

	//// - Fényforrások beállítása
	//glUniform3fv( ul( "cameraPos" ), 1, glm::value_ptr( m_camera.GetEye() ) );
	//glUniform4fv( ul( "lightPos" ),  1, glm::value_ptr( m_lightPos ) );

	//glUniform3fv( ul( "La" ),		 1, glm::value_ptr( m_La ) );
	//glUniform3fv( ul( "Ld" ),		 1, glm::value_ptr( m_Ld ) );
	//glUniform3fv( ul( "Ls" ),		 1, glm::value_ptr( m_Ls ) );

	//glUniform1f( ul( "lightConstantAttenuation"	 ), m_lightConstantAttenuation );
	//glUniform1f( ul( "lightLinearAttenuation"	 ), m_lightLinearAttenuation   );
	//glUniform1f( ul( "lightQuadraticAttenuation" ), m_lightQuadraticAttenuation);

	//// - Anyagjellemzők beállítása
	//glUniform3fv( ul( "Ka" ),		 1, glm::value_ptr( m_Ka ) );
	//glUniform3fv( ul( "Kd" ),		 1, glm::value_ptr( m_Kd ) );
	//glUniform3fv( ul( "Ks" ),		 1, glm::value_ptr( m_Ks ) );

	//glUniform1f( ul( "Shininess" ),	m_Shininess );

	//glUniform1f( ul( "ElapsedTimeInSec" ),	m_ElapsedTimeInSec );

	//glDrawElements( GL_TRIANGLES,    
	//				m_waterGPU.count,			 
	//				GL_UNSIGNED_INT,
	//				nullptr );

	//// Tengely

	//glBindVertexArray(0);

	//glUseProgram(m_programAxesID);

	//glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(glm::identity<glm::mat4>()));
	//glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));

	//glDrawArrays(GL_LINES, 0, 6);


	//// trajectory

	//glBindVertexArray(0);
	//glUseProgram(m_trajectoryID);

	//glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(glm::identity<glm::mat4>()));
	//glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));

	//glUniform3fv(ul("color"), 1, glm::value_ptr(glm::vec3(1, 0, 0)));

	//GLsizei controlPointCount = static_cast<GLsizei>(m_controlPoints.size());

	//glUniform3fv(ul("positions"), controlPointCount, glm::value_ptr(m_controlPoints[0]));

	//glDrawArrays(GL_LINE_STRIP, 0, controlPointCount);

	//glUniform3fv(ul("color"), 1, glm::value_ptr(glm::vec3(1, 0, 1)));

	//glDrawArrays(GL_POINTS, 0, controlPointCount);
	////
	//// skybox
	////

	//// - VAO
	//glBindVertexArray( m_SkyboxGPU.vaoID );

	//// - Textura
	//glActiveTexture( GL_TEXTURE0 );
	//glBindTexture( GL_TEXTURE_CUBE_MAP, m_skyboxTextureID );

	//// - Program
	//glUseProgram( m_programSkyboxID );

	//// - uniform parameterek
	//glUniformMatrix4fv( ul("world"),    1, GL_FALSE, glm::value_ptr( glm::translate( m_camera.GetEye() ) ) );
	//glUniformMatrix4fv( ul("viewProj"), 1, GL_FALSE, glm::value_ptr( m_camera.GetViewProj() ) );

	//// - textúraegységek beállítása
	//glUniform1i( ul( "skyboxTexture" ), 0 );

	//// mentsük el az előző Z-test eredményt, azaz azt a relációt, ami alapján update-eljük a pixelt.
	//GLint prevDepthFnc;
	//glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFnc);

	//// most kisebb-egyenlőt használjunk, mert mindent kitolunk a távoli vágósíkokra
	//glDepthFunc(GL_LEQUAL);

	//// - Rajzolas
	//glDrawElements( GL_TRIANGLES, m_SkyboxGPU.count, GL_UNSIGNED_INT, nullptr );

	//glDepthFunc(prevDepthFnc);

	//// üveg

	//glBindVertexArray(m_quadGPU.vaoID);

	//glUseProgram(m_programID);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_glassTextureID);

	//matWorld = glm::translate(glm::vec3(0, 0, 15)) * glm::scale(glm::vec3(15, 7.5, 15));

	//glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));

	//glDisable(GL_CULL_FACE);
	//glEnable(GL_BLEND);

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDrawElements(GL_TRIANGLES, m_quadGPU.count, GL_UNSIGNED_INT, nullptr);

	//glDisable(GL_BLEND);
	//glEnable(GL_CULL_FACE);

	
}

void CMyApp::RenderGUI()
{
	//ImGui::ShowDemoWindow();
	//if ( ImGui::Begin( "Lighting settings" ) )
	//{		
	//	ImGui::InputFloat("Shininess", &m_Shininess, 0.1f, 1.0f, "%.1f" );
	//	static float Kaf = 1.0f;
	//	static float Kdf = 1.0f;
	//	static float Ksf = 1.0f;
	//	if ( ImGui::SliderFloat( "Ka", &Kaf, 0.0f, 1.0f ) )
	//	{
	//		m_Ka = glm::vec3( Kaf );
	//	}
	//	if ( ImGui::SliderFloat( "Kd", &Kdf, 0.0f, 1.0f ) )
	//	{
	//		m_Kd = glm::vec3( Kdf );
	//	}
	//	if ( ImGui::SliderFloat( "Ks", &Ksf, 0.0f, 1.0f ) )
	//	{
	//		m_Ks = glm::vec3( Ksf );
	//	}

	//	{
	//		static glm::vec2 lightPosXZ = glm::vec2( 0.0f );
	//		lightPosXZ = glm::vec2( m_lightPos.x, m_lightPos.z );
	//		if ( ImGui::SliderFloat2( "Light Position XZ", glm::value_ptr( lightPosXZ ), -1.0f, 1.0f ) )
	//		{
	//			float lightPosL2 = lightPosXZ.x * lightPosXZ.x + lightPosXZ.y * lightPosXZ.y;
	//			if ( lightPosL2 > 1.0f ) // Ha kívülre esne a körön, akkor normalizáljuk
	//			{
	//				lightPosXZ /= sqrtf( lightPosL2 );
	//				lightPosL2 = 1.0f;
	//			}

	//			m_lightPos.x = lightPosXZ.x;
	//			m_lightPos.z = lightPosXZ.y;
	//			m_lightPos.y = sqrtf( 1.0f - lightPosL2 );
	//		}
	//		ImGui::LabelText( "Light Position Y", "%f", m_lightPos.y );
	//	}
	//}
	//ImGui::End();

	if ( ImGui::Begin( "Metaballs" ) )
	{
		// A paramétert szabályozó csúszka
		ImGui::SliderFloat("TR parameter", &m_tr, 0, 1.0f);

		ImGui::SeparatorText("Metaballs Array");
		
		// A kijelölt pont indexe
		// Lehetne a CMyApp tagváltozója is, de mivel csak a GUI-hoz kell, ezért elégséges lokális, de statikus változónak lennie
		static int currentItem = -1;

		// A listboxban megjelenítjük a pontokat
		// Legyen a magasssága annyi, hogy MAX_POINT_COUNT elem férjen bele
		// ImGui::GetTextLineHeightWithSpacing segítségével lekérhető egy sor magassága
		if (ImGui::BeginListBox("Metaballs Array", ImVec2(0.0, glm::max(5, (int) m_metaBalls.size()) * ImGui::GetTextLineHeightWithSpacing())))
		{
			for ( int i = 0; i < static_cast<const int>( m_metaBalls.size() ); ++i )
			{
				const bool is_seleceted = ( currentItem == i ); // épp ki van-e jelölve?
				if ( ImGui::Selectable( std::to_string( i ).c_str(), is_seleceted ) )
				{
					if ( i == currentItem ) currentItem = -1; // Ha rákattintottunk, akkor szedjük le a kijelölést
					else currentItem = i; // Különben jelöljük ki
				}

				// technikai apróság, nem baj ha lemarad.
				if ( is_seleceted )
                    ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox(); 
		}

		// Gombnyomásra új pontot adunk a végére
		if (ImGui::Button("Add")) // Akkor tér vissza true-val, ha rákattintottunk
		{
			if ( m_metaBalls.size() < 5 )
			{
				m_metaBalls.push_back( glm::vec4( 0,0,0,1));
				currentItem = static_cast<const int>( m_metaBalls.size() - 1 ); // Az új pontot állítjuk be aktuálisnak
			}
		}

		ImGui::SameLine();

		// Gombnyomásra töröljük a kijelölt pontot
		if (ImGui::Button("Delete") )
		{
			if ( !m_metaBalls.empty() && currentItem < m_metaBalls.size() && currentItem != -1 ) // currentItem valid index?
			{
				m_metaBalls.erase(m_metaBalls.begin() + currentItem ); // Iterátoron keresztül tudjuk törölni a kijelölt elemet
				currentItem = -1; // Törölve lett a kijelölés
			}
		}

		// Ha van kijelölt elem, akkor jelenítsük meg a koordinátáit
		// és lehessen szerkeszteni
		if ( currentItem < m_metaBalls.size() && currentItem != -1 ) // currentItem valid index?
		{
			ImGui::SliderFloat3("Coords", glm::value_ptr(m_metaBalls[currentItem]), -10, 10);
		}
	}
	ImGui::End();
}

GLint CMyApp::ul( const char* uniformName ) noexcept
{
	GLuint programID = 0;

	// Kérdezzük le az aktuális programot!
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
	glGetIntegerv( GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>( &programID ) );
	// A program és a uniform név ismeretében kérdezzük le a location-t!
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetUniformLocation.xhtml
	return glGetUniformLocation( programID, uniformName );
}

// https://wiki.libsdl.org/SDL2/SDL_KeyboardEvent
// https://wiki.libsdl.org/SDL2/SDL_Keysym
// https://wiki.libsdl.org/SDL2/SDL_Keycode
// https://wiki.libsdl.org/SDL2/SDL_Keymod

void CMyApp::KeyboardDown(const SDL_KeyboardEvent& key)
{	
	if ( key.repeat == 0 ) // Először lett megnyomva
	{
		if ( key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_CTRL )
		{
			CleanShaders();
			InitShaders();
		}
		if ( key.keysym.sym == SDLK_F1 )
		{
			GLint polygonModeFrontAndBack[ 2 ] = {};
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
			glGetIntegerv( GL_POLYGON_MODE, polygonModeFrontAndBack ); // Kérdezzük le a jelenlegi polygon módot! Külön adja a front és back módokat.
			GLenum polygonMode = ( polygonModeFrontAndBack[ 0 ] != GL_FILL ? GL_FILL : GL_LINE ); // Váltogassuk FILL és LINE között!
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPolygonMode.xhtml
			glPolygonMode( GL_FRONT_AND_BACK, polygonMode ); // Állítsuk be az újat!
		}
	}
	m_camera.KeyboardDown( key );
}

void CMyApp::KeyboardUp(const SDL_KeyboardEvent& key)
{
	m_camera.KeyboardUp( key );
}

// https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent

void CMyApp::MouseMove(const SDL_MouseMotionEvent& mouse)
{
	m_camera.MouseMove( mouse );
}

// https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent

void CMyApp::MouseDown(const SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(const SDL_MouseButtonEvent& mouse)
{
}

// https://wiki.libsdl.org/SDL2/SDL_MouseWheelEvent

void CMyApp::MouseWheel(const SDL_MouseWheelEvent& wheel)
{
	m_camera.MouseWheel( wheel );
}


// a két paraméterben az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);
	m_camera.Resize( _w, _h );
}

// Pozíció kiszámítása a kontrollpontok alapján
glm::vec3 CMyApp::EvaluatePathPosition() const
{
	if (m_controlPoints.size() == 0) // Ha nincs pont, akkor visszaadjuk az origót
		return glm::vec3(0);

	const int interval = (const int)m_currentParam; // Melyik két pont között vagyunk?

	if (interval < 0) // Ha a paraméter negatív, akkor a kezdőpontot adjuk vissza
		return m_controlPoints[0];

	if (interval >= m_controlPoints.size() - 1) // Ha a paraméter nagyobb, mint a pontok száma, akkor az utolsó pontot adjuk vissza
		return m_controlPoints[m_controlPoints.size() - 1];

	float localT = m_currentParam - interval; // A paramétert normalizáljuk az aktuális intervallumra
	
	return glm::mix( m_controlPoints[interval], m_controlPoints[interval + 1], localT ); // Lineárisan interpolálunk a két kontrollpont között
}

// Tangens kiszámítása a kontrollpontok alapján
glm::vec3 CMyApp::EvaluatePathTangent() const
{
	if (m_controlPoints.size() < 2) // Ha nincs elég pont az interpolációhoy, akkor visszaadjuk az x tengelyt
		return glm::vec3(1.0,0.0,0.0);

	int interval = (int)m_currentParam; // Melyik két pont között vagyunk?

	if (interval < 0) // Ha a paraméter negatív, akkor a kezdő intervallumot adjuk vissza
		interval = 0;

	if (interval >= m_controlPoints.size() - 1) // Ha a paraméter nagyobb, mint az intervallumok száma, akkor az utolsót adjuk vissza
		interval = static_cast<int>( m_controlPoints.size() - 2 );

	return glm::normalize(m_controlPoints[interval + 1] - m_controlPoints[interval]);
}
