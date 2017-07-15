#include "Texture.h"

void Engine::Resources::Load::load(Texture* texture){ LoaderTexture::load(texture); }
void Engine::Resources::Load::unload(Texture* texture){ LoaderTexture::unload(texture); }
void Engine::Resources::Load::LoaderTexture::load(Texture* texture){
    texture->m_Name = "";
}
void Engine::Resources::Load::LoaderTexture::unload(Texture* texture){

}