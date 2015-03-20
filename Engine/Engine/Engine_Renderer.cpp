#include "Engine_Renderer.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"

Renderer::Renderer(RENDER_TYPE type){

	Resources->Load_Texture_Into_GLuint(RandomMapSSAO,"Textures\\SSAONormal.png");

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	Set_Render_Type(type);
	m_EnableLighting = true;
	m_EnableNormalMapping = true;
	m_EnableLightmapping = true;

	m_gBuffer = new GBuffer(Window->getSize().x,Window->getSize().y);
}
Renderer::~Renderer(){
	delete m_gBuffer;
}
void Renderer::Geometry_Pass(bool debug){
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0,0,0,1);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
	for(auto object:Resources->Objects){
		object->Render(m_Type);
	}
	if(debug){
		GLuint shaderProgram = Resources->Get_Shader_Program("Deferred")->Get_Shader_Program();
		glUseProgram(shaderProgram);
		for(auto pointLight:Resources->Lights_Points){

			glm::mat4 model = glm::mat4(1);
			model = glm::translate(model, pointLight->Position);
			float scl = CalcPointLightBSphere(pointLight);
			model = glm::scale(model,glm::vec3(scl,scl,scl));

			glm::mat4 MVP = Resources->Current_Camera()->Calculate_Projection(model);

			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP" ), 1, GL_FALSE, glm::value_ptr(MVP));
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "World" ), 1, GL_FALSE, glm::value_ptr(model));
			glUniform3f(glGetUniformLocation(shaderProgram, "Object_Color"),1.0f,1.0f,1.0f);
			Resources->Get_Mesh("DEBUGLight")->Render();
		}
		glUseProgram(0);
	}

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
}
void Renderer::Lighting_Pass(){

	glEnable(GL_BLEND);
   	glBlendEquation(GL_FUNC_ADD);
   	glBlendFunc(GL_ONE, GL_ONE);
    glClear(GL_COLOR_BUFFER_BIT);

	this->Pass_Light_Point();
	//this->Pass_Light_Dir();
	//this->Pass_Light_Spot();
}
void Renderer::Set_Render_Type(RENDER_TYPE type){ m_Type = type; }
void Renderer::Render(bool debug){

	switch(m_Type){
		case RENDER_TYPE_FORWARD:
			glDepthMask(GL_TRUE);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(0,0,0,1);
			glEnable(GL_DEPTH_TEST);
			for(auto object:Resources->Objects){
				object->Render();
			}
			break;
		case RENDER_TYPE_DEFERRED:

			m_gBuffer->Start(BUFFER_TYPE_DIFFUSE,BUFFER_TYPE_NORMAL,BUFFER_TYPE_BLOOM);

			this->Geometry_Pass(debug);
			m_gBuffer->Stop();
			m_gBuffer->Start(BUFFER_TYPE_LIGHTING);
			this->Lighting_Pass();
			m_gBuffer->Stop();
			m_gBuffer->Start(BUFFER_TYPE_SSAO);
			this->Pass_SSAO();

			m_gBuffer->Stop();
			m_gBuffer->Start(BUFFER_TYPE_FREE1);
			this->Pass_Blur_Horizontal(m_gBuffer->Texture(BUFFER_TYPE_SSAO));
			m_gBuffer->Stop();
			m_gBuffer->Start(BUFFER_TYPE_SSAO);
			this->Pass_Blur_Vertical(m_gBuffer->Texture(BUFFER_TYPE_FREE1));

			

			m_gBuffer->Stop();
			this->Pass_Final();

			break;
		default:
			break;
	}
}
float Renderer::CalcPointLightBSphere(const PointLight* Light){
   	float MaxChannel = std::max(std::max(Light->Color.x, Light->Color.y), Light->Color.z);
   	float ret = (-Light->Attenuation.Linear + sqrtf(Light->Attenuation.Linear * Light->Attenuation.Linear -
       	4 * Light->Attenuation.Exp * (Light->Attenuation.Exp - 256 * MaxChannel * Light->DiffuseIntensity))) 
   	   	   	/
   	   	2 * Light->Attenuation.Exp;
   	return ret;
}
void Renderer::Pass_Light_Point(){
	GLuint shader = Resources->Get_Shader_Program("Deferred_Light_Point")->Get_Shader_Program();
	glm::vec3 camPos = Resources->Current_Camera()->Position();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Window->getSize().x),static_cast<float>(Window->getSize().y));
	glUniform3f(glGetUniformLocation(shader,"gEyeWorldPos"), camPos.x, camPos.y, camPos.z);
	glUniformMatrix4fv(glGetUniformLocation(shader, "VPInverse" ), 1, GL_FALSE, glm::value_ptr(Resources->Current_Camera()->Calculate_ViewProjInverted()));

	GLuint m_textureID[2];
	m_textureID[0] = glGetUniformLocation(shader,"gNormalMap");
	m_textureID[1] = glGetUniformLocation(shader,"gPositionMap");

		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer->Texture(BUFFER_TYPE_NORMAL));
		glUniform1i( m_textureID[0], 0 );

		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer->Texture(BUFFER_TYPE_DEPTH));
		glUniform1i( m_textureID[1], 1 );

	struct {
		GLuint Color;
		GLuint AmbientIntensity;
		GLuint DiffuseIntensity;
		GLuint Position;
		struct {
			GLuint Constant;
			GLuint Linear;
			GLuint Exp;
		} Atten;
	} m_pointLightLocation;
	for (auto light:Resources->Lights_Points) {

		m_pointLightLocation.Color = glGetUniformLocation(shader,"gPointLight.Base.Color");
		m_pointLightLocation.AmbientIntensity = glGetUniformLocation(shader,"gPointLight.Base.AmbientIntensity");
		m_pointLightLocation.Position = glGetUniformLocation(shader,"gPointLight.Position");
		m_pointLightLocation.DiffuseIntensity = glGetUniformLocation(shader,"gPointLight.Base.DiffuseIntensity");
		m_pointLightLocation.Atten.Constant = glGetUniformLocation(shader,"gPointLight.Atten.Constant");
		m_pointLightLocation.Atten.Linear = glGetUniformLocation(shader,"gPointLight.Atten.Linear");
        m_pointLightLocation.Atten.Exp = glGetUniformLocation(shader,"gPointLight.Atten.Exp");

		glUniform3f(m_pointLightLocation.Color, light->Color.x, light->Color.y,light->Color.z);
        glUniform1f(m_pointLightLocation.AmbientIntensity, light->AmbientIntensity);
        glUniform1f(m_pointLightLocation.DiffuseIntensity, light->DiffuseIntensity);
        glUniform3f(m_pointLightLocation.Position, light->Position.x, light->Position.y, light->Position.z);
        glUniform1f(m_pointLightLocation.Atten.Constant, light->Attenuation.Constant);
        glUniform1f(m_pointLightLocation.Atten.Linear, light->Attenuation.Linear);
        glUniform1f(m_pointLightLocation.Atten.Exp, light->Attenuation.Exp);

        glUniform1f(glGetUniformLocation(shader,"gMatSpecularIntensity"), 0.6f);
        glUniform1f(glGetUniformLocation(shader,"gSpecularPower"), 0.6f);

		Init_Quad();
   	}

	// Reset OpenGL state
	for(unsigned int i = 0; i < 2; i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);
}
void Renderer::Pass_Light_Dir(){
	GLuint shader = Resources->Get_Shader_Program("Deferred_Light_Dir")->Get_Shader_Program();
	glm::vec3 camPos = Resources->Current_Camera()->Position();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Window->getSize().x),static_cast<float>(Window->getSize().y));
	glUniform3f(glGetUniformLocation(shader,"gEyeWorldPos"), camPos.x, camPos.y, camPos.z);
	glUniformMatrix4fv(glGetUniformLocation(shader, "VPInverse" ), 1, GL_FALSE, glm::value_ptr(Resources->Current_Camera()->Calculate_ViewProjInverted()));

	GLuint m_textureID[2];
	m_textureID[0] = glGetUniformLocation(shader,"gNormalMap");
	m_textureID[1] = glGetUniformLocation(shader,"gPositionMap");

		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer->Texture(BUFFER_TYPE_NORMAL));
		glUniform1i( m_textureID[0], 0 );

		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer->Texture(BUFFER_TYPE_DEPTH));
		glUniform1i( m_textureID[1], 1);

	struct {
		GLuint Color;
		GLuint AmbientIntensity;
		GLuint DiffuseIntensity;
		GLuint Direction;
	} m_DirLightLocation;
	for (auto light:Resources->Lights_Directional) {

		m_DirLightLocation.Color = glGetUniformLocation(shader,"gDirectionalLight.Base.Color");
		m_DirLightLocation.AmbientIntensity = glGetUniformLocation(shader,"gDirectionalLight.Base.AmbientIntensity");
		m_DirLightLocation.DiffuseIntensity = glGetUniformLocation(shader,"gDirectionalLight.Base.DiffuseIntensity");
		m_DirLightLocation.Direction = glGetUniformLocation(shader,"gDirectionalLight.Direction");

		glUniform3f(m_DirLightLocation.Color, light->Color.x, light->Color.y,light->Color.z);
		glUniform3f(m_DirLightLocation.Direction, light->Direction.x, light->Direction.y,light->Direction.z);
        glUniform1f(m_DirLightLocation.AmbientIntensity, light->AmbientIntensity);
        glUniform1f(m_DirLightLocation.DiffuseIntensity, light->DiffuseIntensity);

        glUniform1f(glGetUniformLocation(shader,"gMatSpecularIntensity"), 0.6f);
        glUniform1f(glGetUniformLocation(shader,"gSpecularPower"), 0.6f);

		Init_Quad();

   	}
	// Reset OpenGL state
	for(unsigned int i = 0; i < 2; i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);
}
void Renderer::Pass_Light_Spot()
{
}
void Renderer::Pass_SSAO(){
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint shader = Resources->Get_Shader_Program("Deferred_SSAO")->Get_Shader_Program();
	glUseProgram(shader);

	glUniformMatrix4fv(glGetUniformLocation(shader, "VPInverse" ), 1, GL_FALSE, glm::value_ptr(Resources->Current_Camera()->Calculate_ViewProjInverted()));

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Window->getSize().x),static_cast<float>(Window->getSize().y));
	glUniform1f(glGetUniformLocation(shader,"gIntensity"), 2.9f);
	glUniform1f(glGetUniformLocation(shader,"gBias"), 0.01f);
	glUniform1f(glGetUniformLocation(shader,"gRadius"), 0.7f);
	glUniform1f(glGetUniformLocation(shader,"gScale"), 1.2f);

	GLuint m_textureID[3];
	m_textureID[0] = glGetUniformLocation(shader,"gNormalMap");
	m_textureID[1] = glGetUniformLocation(shader,"gPositionMap");
	m_textureID[2] = glGetUniformLocation(shader,"gRandomMap");

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->Texture(BUFFER_TYPE_NORMAL));
	glUniform1i( m_textureID[0], 0 );

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->Texture(BUFFER_TYPE_DEPTH));
	glUniform1i( m_textureID[1], 1 );

	glActiveTexture(GL_TEXTURE2);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,RandomMapSSAO);
	glUniform1i( m_textureID[2], 2 );

	Init_Quad();

	for(unsigned int i = 0; i < 3; i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);
}
void Renderer::Pass_Blur_Horizontal(GLuint texture){
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint shader = Resources->Get_Shader_Program("Deferred_Blur_Horizontal")->Get_Shader_Program();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Window->getSize().x),static_cast<float>(Window->getSize().y));


	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(shader,"texture"), 0 );

	Init_Quad();

	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
}
void Renderer::Pass_Blur_Vertical(GLuint texture){
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint shader = Resources->Get_Shader_Program("Deferred_Blur_Vertical")->Get_Shader_Program();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Window->getSize().x),static_cast<float>(Window->getSize().y));

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(shader,"texture"), 0 );

	Init_Quad();

	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
}
void Renderer::Pass_Final(){
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint shader = Resources->Get_Shader_Program("Deferred_Final")->Get_Shader_Program();
	glUseProgram(shader);

	glUniform2f(glGetUniformLocation(shader,"gScreenSize"), static_cast<float>(Window->getSize().x),static_cast<float>(Window->getSize().y));

	GLuint m_textureID[4];
	m_textureID[0] = glGetUniformLocation(shader,"gColorMap");
	m_textureID[1] = glGetUniformLocation(shader,"gLightMap");
	m_textureID[2] = glGetUniformLocation(shader,"gSSAOMap");
	m_textureID[3] = glGetUniformLocation(shader,"gGlowMap");

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->Texture(BUFFER_TYPE_DIFFUSE));
	glUniform1i( m_textureID[0], 0 );

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->Texture(BUFFER_TYPE_LIGHTING));
	glUniform1i( m_textureID[1], 1 );

	glActiveTexture(GL_TEXTURE2);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->Texture(BUFFER_TYPE_SSAO));
	glUniform1i( m_textureID[2], 2 );

	glActiveTexture(GL_TEXTURE3);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->Texture(BUFFER_TYPE_BLOOM));
	glUniform1i( m_textureID[3], 3 );

	Init_Quad();

	for(unsigned int i = 0; i < 4; i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);
}
void Renderer::Init_Quad(){
	//Projection setup
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0,Window->getSize().x,0,Window->getSize().y,0.1f,2);	
	
	//Model setup
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// Render the quad
	glLoadIdentity();
	glColor3f(1,1,1);
	glTranslatef(0,0,-1.0);
	
	glBegin(GL_QUADS);
	glTexCoord2f( 0, 0 );
	glVertex3f(    0.0f, 0.0f, 0.0f);
	glTexCoord2f( 1, 0 );
	glVertex3f(   (float) Window->getSize().x, 0.0f, 0.0f);
	glTexCoord2f( 1, 1 );
	glVertex3f(   (float) Window->getSize().x, (float) Window->getSize().y, 0.0f);
	glTexCoord2f( 0, 1 );
	glVertex3f(    0.0f,  (float) Window->getSize().y, 0.0f);
	glEnd();

	//Reset the matrices	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}