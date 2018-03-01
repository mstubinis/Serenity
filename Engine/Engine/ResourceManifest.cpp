#include "ResourceManifest.h"
#include "Engine_Resources.h"

using namespace Engine;

Handle ResourceManifest::skyFromSpace;
Handle ResourceManifest::skyFromAtmosphere;
Handle ResourceManifest::groundFromSpace;

void ResourceManifest::init(){
	Handle skyFromSpaceVert = Resources::addShader("AS_SkyFromSpace_Vert","data/Shaders/AS_skyFromSpace_vert.glsl",ShaderType::Vertex);
	Handle skyFromSpaceFrag = Resources::addShader("AS_SkyFromSpace_Frag","data/Shaders/AS_skyFromSpace_frag.glsl",ShaderType::Fragment);
    skyFromSpace = Resources::addShaderProgram("AS_SkyFromSpace",skyFromSpaceVert,skyFromSpaceFrag,ShaderRenderPass::Geometry);

	Handle skyFromAtVert = Resources::addShader("AS_SkyFromAtmosphere_Vert","data/Shaders/AS_skyFromAtmosphere_vert.glsl",ShaderType::Vertex);
	Handle skyFromAtFrag = Resources::addShader("AS_SkyFromAtmosphere_Frag","data/Shaders/AS_skyFromAtmosphere_frag.glsl",ShaderType::Fragment);
    skyFromAtmosphere = Resources::addShaderProgram("AS_SkyFromAtmosphere",skyFromAtVert,skyFromAtFrag,ShaderRenderPass::Geometry);

	Handle groundFromSpaceVert = Resources::addShader("AS_GroundFromSpace_Vert","data/Shaders/AS_groundFromSpace_vert.glsl",ShaderType::Vertex);
	Handle groundFromSpaceFrag = Resources::addShader("AS_GroundFromSpace_Frag","data/Shaders/AS_groundFromSpace_frag.glsl",ShaderType::Fragment);
    groundFromSpace = Resources::addShaderProgram("AS_GroundFromSpace",groundFromSpaceVert,groundFromSpaceFrag,ShaderRenderPass::Geometry);
}