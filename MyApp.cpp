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
	m_metaballProgramID = glCreateProgram();
	AssembleProgram(m_metaballProgramID, "Vert_Metaball.vert", "Frag_Metaball.frag");
}

void CMyApp::CleanShaders()
{
	glDeleteProgram(m_metaballProgramID);
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

void CMyApp::InitGeometry()
{
	MeshObject<glm::vec2> quadGPU;
	{
		MeshObject<Vertex> surfaceMeshCPU = GetParamSurfMesh(Param(), 1, 1);
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
	CleanOGLObject(m_quadGPU);
}

void CMyApp::InitTextures()
{
	InitSkyboxTextures();
}

void CMyApp::CleanTextures()
{
	CleanSkyboxTextures();
}

void CMyApp::InitSkyboxTextures()
{
	// skybox texture

	glGenTextures( 1, &m_skyboxTextureID );
	TextureFromFile( m_skyboxTextureID, "Assets/yokohama_xpos.jpg", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_X );
	TextureFromFile( m_skyboxTextureID, "Assets/yokohama_xneg.jpg", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_X );
	TextureFromFile( m_skyboxTextureID, "Assets/yokohama_ypos.jpg", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_Y );
	TextureFromFile( m_skyboxTextureID, "Assets/yokohama_yneg.jpg", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y );
	TextureFromFile( m_skyboxTextureID, "Assets/yokohama_zpos.jpg", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_Z );
	TextureFromFile( m_skyboxTextureID, "Assets/yokohama_zneg.jpg", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z );
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
	glClearColor(0, 0, 0, 1);

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
		glm::vec3(0.0, 0.0, -10.0),	// honnan nézzük a színteret	   - eye
		glm::vec3(0.0, 0.0, 0.0),   // a színtér melyik pontját nézzük - at
		glm::vec3(0.0, 1.0, 0.0));  // felfelé mutató irány a világban - up

	m_metaBalls.push_back(glm::vec4(2, 0, 1, 1));
	m_metaBalls.push_back(glm::vec4(-3, 2, 0, 2));
	m_metaBalls.push_back(glm::vec4(-1, -3, -2, 1.5));
	m_metaBalls.push_back(glm::vec4(1, -1, 0, 1.9));
	m_metaBalls.push_back(glm::vec4(1, -2, 3, 1.9));

	m_lights.AddLight(glm::vec3(1, 5, 2), glm::vec3(.5), glm::vec3(1, 0, 0));
	m_lights.AddLight(-glm::vec3(2, 5, 1), glm::vec3(.5), glm::vec3(0, 0, 1));

	return true;
}

void CMyApp::Clean()
{
	CleanShaders();
	CleanGeometry();
	CleanTextures();
}

void CMyApp::updateAnimation() {
	m_metaBallsAnimated.clear();
	for (int i = 0; i < m_metaBalls.size(); i++) {
		glm::vec4 ball = glm::vec4(m_metaBalls[i].x + 0.5f * (i % 2 == 0 ? cosf(m_ElapsedTimeInSec * 5 / (2 + i)) : (2 + i * 2) * sinf(m_ElapsedTimeInSec * 6 / (2 + i))),
			m_metaBalls[i].y + i % 2 == 0 ? 3 * sinf(m_ElapsedTimeInSec * 6 / (2 + i)) : 4* cosf(m_ElapsedTimeInSec * 4 / (2 + i)),
			m_metaBalls[i].z + i % 2 == 0 ? cosf(m_ElapsedTimeInSec * 7 / (3 + i)) : 1 - sinf(m_ElapsedTimeInSec * 3 / (2 + i)),
			(cosf(m_ElapsedTimeInSec) + 2) * .5 + m_metaBalls[i].w);

		m_metaBallsAnimated.push_back(ball);
	}
}

void CMyApp::Update( const SUpdateInfo& updateInfo )
{
	m_ElapsedTimeInSec = updateInfo.ElapsedTimeInSec;
	m_camera.Update(updateInfo.DeltaTimeInSec);
	if(!m_animationPaused)
		updateAnimation();
}

void CMyApp::Render()
{
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT)...
	// ... és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(m_quadGPU.vaoID);

	// - Textura
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTextureID);

	glUseProgram(m_metaballProgramID);

	glUniform1i(ul("cubeMap"), 0);

	glUniform1f(ul("time"), m_ElapsedTimeInSec);
	glUniform1f(ul("tr"), m_tr);

	GLsizei balls = static_cast<GLsizei>(m_metaBallsAnimated.size());

	if(balls > 0)
		glUniform4fv(ul("balls"), balls, glm::value_ptr(m_metaBallsAnimated[0]));
	glUniform1i(ul("ballCount"), balls);

	GLsizei lights = static_cast<GLsizei>(m_lights.GetSize());

	if(lights > 0) {
		glUniform3fv(ul("lightPoses"), lights, glm::value_ptr(*m_lights.GetLightPosesPointer()));
		glUniform3fv(ul("diffuseColors"), lights, glm::value_ptr(*m_lights.GetDiffuseColorPointer()));
		glUniform3fv(ul("specularColors"), lights, glm::value_ptr(*m_lights.GetSpecularColorPointer()));

	}
	glUniform1i(ul("lightCount"), lights);

	glUniform3fv(ul("eye"), 1, glm::value_ptr(m_camera.GetEye()));
	glUniform3fv(ul("at"), 1, glm::value_ptr(m_camera.GetAt()));
	glUniform3fv(ul("up"), 1, glm::value_ptr(m_camera.GetUp()));

	glUniform1f(ul("aspect"), m_camera.GetAspect());
	glUniform1f(ul("angle"), m_camera.GetAngle());
	glUniform1f(ul("near"), m_camera.GetZNear());
	glUniform1f(ul("far"), m_camera.GetZFar());

	glUniform1f(ul("windowSizeX"), m_camera.GetWindowSizeX());
	glUniform1f(ul("windowSizeY"), m_camera.GetWindowSizeY());

	glUniform2fv(ul("mousePos"), 1, glm::value_ptr(mousePos));

	glDrawElements(GL_TRIANGLES, m_quadGPU.count, GL_UNSIGNED_INT, nullptr);

	glUseProgram(0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	// VAO kikapcsolása
	glBindVertexArray(0);
}

void CMyApp::RenderGUI()
{
	if ( ImGui::Begin( "Metaballs" ) )
	{
		ImGui::SliderFloat("TR parameter", &m_tr, 0, 1.0f);
		ImGui::Checkbox("Animation Paused", &m_animationPaused);

		ImGui::SeparatorText("Metaballs Array");
		
		static int currentItem = -1;

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

				if ( is_seleceted )
                    ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox(); 
		}

		if (ImGui::Button("Add"))
		{
			if ( m_metaBalls.size() < 5 )
			{
				m_metaBalls.push_back( glm::vec4( 0,0,0,1));
				currentItem = static_cast<const int>( m_metaBalls.size() - 1 ); // Az új pontot állítjuk be aktuálisnak
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Delete") )
		{
			if ( !m_metaBalls.empty() && currentItem < m_metaBalls.size() && currentItem != -1 ) // currentItem valid index?
			{
				m_metaBalls.erase(m_metaBalls.begin() + currentItem ); // Iterátoron keresztül tudjuk törölni a kijelölt elemet
				currentItem = -1; // Törölve lett a kijelölés
			}
		}
	}
	ImGui::End();

	if (ImGui::Begin("Lights"))
	{
		static int currentItem = -1;

		if (ImGui::BeginListBox("Lights", ImVec2(0.0, glm::max(m_lights.GetMax(), (int)m_lights.GetSize()) * ImGui::GetTextLineHeightWithSpacing())))
		{
			for (int i = 0; i < static_cast<const int>(m_lights.GetSize()); ++i)
			{
				const bool is_seleceted = (currentItem == i); // épp ki van-e jelölve?
				if (ImGui::Selectable(std::to_string(i).c_str(), is_seleceted))
				{
					if (i == currentItem) currentItem = -1; // Ha rákattintottunk, akkor szedjük le a kijelölést
					else currentItem = i; // Különben jelöljük ki
				}

				if (is_seleceted)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox();
		}

		if (ImGui::Button("Add"))
		{
			if (m_lights.GetSize() < 5)
			{
				m_lights.AddLight(glm::vec3(0, 0, 0), glm::vec3(.5), glm::vec3(.5));
				currentItem = static_cast<const int>(m_lights.GetSize() - 1);
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Delete"))
		{
			if (m_lights.GetSize() != 0 && currentItem < m_lights.GetSize() && currentItem != -1) 
			{
				m_lights.RemoveLight(currentItem); 
				currentItem = -1;
			}
		}

		if (currentItem < m_lights.GetSize() && currentItem != -1)
		{
			ImGui::SliderFloat3("Coords", glm::value_ptr(*m_lights.GetLightAt(currentItem)), -10, 10);
			ImGui::ColorEdit3("Diffuse Color", glm::value_ptr(*m_lights.GetDiffuseAt(currentItem)));
			ImGui::ColorEdit3("Specular Color", glm::value_ptr(*m_lights.GetSpecularAt(currentItem)));
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
	if (mouse.state & SDL_BUTTON_LMASK)
		mousePos = glm::vec2(mousePos.x + mouse.xrel / 100.f, mousePos.y + mouse.yrel / 100.f);
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
