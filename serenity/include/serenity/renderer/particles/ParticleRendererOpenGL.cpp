#include <serenity/renderer/particles/ParticleRendererOpenGL.h>
#include <serenity/system/Engine.h>
#include <serenity/resources/mesh/Mesh.h>

#include <serenity/renderer/particles/ParticleContainer.h>
#include <serenity/renderer/particles/ParticleIncludes.h>
#include <serenity/renderer/GBuffer.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/resources/material/Material.h>

Engine::priv::ParticleRendererOpenGL::ParticleRendererOpenGL() {
    //particle instancing
    /*
    Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getParticleMesh().get<Mesh>()->getVertexData().bind(); //this binds attributes 0 and 1

    m_VBOs.reserve(5);
    for (size_t i = 0; i < m_VBOs.capacity(); ++i) {
        uint32_t& vbo = m_VBOs.emplace_back();
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
    }
    */

    //auto sizeofOne = sizeof(ParticleFloatType) * 4;
    //auto sizeofTwo = sizeof(ParticleFloatType) * 2;
    //0 = vec3 vertex position
    //1 = vec2 vertex uv
    /*
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, PARTICLE_FLOAT_TYPE, GL_FALSE, sizeof(Engine::priv::ParticleDOD), (void*)0); //positons and scale.x
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, PARTICLE_FLOAT_TYPE, GL_FALSE, sizeof(Engine::priv::ParticleDOD), (void*)sizeofOne); //scale.y and angularRotation
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 2, GL_UNSIGNED_SHORT, sizeof(Engine::priv::ParticleDOD), (void*)(sizeofOne + sizeofTwo)); //matIndex and packedColor
    */

    /*
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);

    Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getParticleMesh().get<Mesh>()->getVertexData().unbind();
    */
}
Engine::priv::ParticleRendererOpenGL::~ParticleRendererOpenGL() {
    glDeleteBuffers(GLsizei(m_VBOs.size()), m_VBOs.data());
}
void Engine::priv::ParticleRendererOpenGL::render(Engine::priv::RenderModule& renderModule, Engine::priv::ParticleContainer& particleContainer) {
    const auto particle_count = particleContainer.getNumParticles();
}
void Engine::priv::ParticleRendererOpenGL::render(Engine::priv::RenderModule& renderModule, Engine::priv::ParticleContainer& particleContainer, size_t particle_count, Handle program, Engine::priv::GBuffer& gBuffer, size_t maxTextureUnits, Engine::unordered_bimap<Material*, uint32_t>& bimap, std::unordered_map<uint32_t, uint32_t>& materialIDToIndex) {
    if (particle_count > 0) {
        
        for (auto it = bimap.rbegin(); it != bimap.rend(); ++it) {
            materialIDToIndex.try_emplace(it->first, uint32_t(materialIDToIndex.size()));
        }
        for (size_t i = 0; i < particle_count; ++i) {
            ParticleMaterialIDType matID = particleContainer.getMatID(i);
            matID = materialIDToIndex.at(uint32_t(matID));
        }    

        /*ultimate goal: materialIndex (0 to 10 or whatever) => Material*. Make sure materialIndex (0 to 10 or whatever) is available as vertex information at the shader stage
        * 
        * try:
        *     if(matID) does not exist : 
                  map it to matID => containerSize
        *         containerSize => material*
        *         ++containerSize
        *      matID = containerSize[matID]
        */

        renderModule.bind(program.get<ShaderProgram>());
#if 0
        for (const auto& [id, index] : materialIDToIndex) {
            Material* mat              = bimap.at(id);
            Texture& texture           = *mat->getComponent(MaterialComponentType::Diffuse).getTexture(0).get<Texture>();
            const std::string location = "Tex" + std::to_string(index);
            Engine::Renderer::sendTextureSafe(location.c_str(), texture, index);
        }
        const int maxTextures = int(maxTextureUnits) - 1;
        Engine::Renderer::sendTextureSafe("gDepthMap", gBuffer.getTexture(GBufferType::Depth), maxTextures);

        auto& particleMesh = *Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getParticleMesh().get<Mesh>();
        renderModule.bind(&particleMesh);


        size_t accumulator = 0;
        auto bind_data = [](size_t index, size_t numComponents, GLuint vbo, size_t glType, size_t& inAccumulator, size_t particleCount, size_t byteSize, void* data) {
            glEnableVertexAttribArray(GLuint(index));
            glVertexAttribPointer(GLuint(index), GLint(numComponents), GLenum(glType), GL_FALSE, 0 /*byteSize*/ /* TODO: should byteSize be 0? */, reinterpret_cast<void*>(inAccumulator));
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferSubData(GL_ARRAY_BUFFER, GLintptr(inAccumulator), GLsizeiptr(byteSize * particleCount), &data);
            inAccumulator += (byteSize * particleCount);
        };

        bind_data(2, 3, m_VBOs[0], PARTICLE_FLOAT_TYPE, accumulator, particle_count, (sizeof(ParticleFloatType) * 3), particleContainer.getDataPtr<ParticleMember::Position>());
        bind_data(3, 2, m_VBOs[1], PARTICLE_FLOAT_TYPE, accumulator, particle_count, (sizeof(ParticleFloatType) * 2), particleContainer.getDataPtr<ParticleMember::Scale>());
        bind_data(4, 1, m_VBOs[2], PARTICLE_FLOAT_TYPE, accumulator, particle_count, sizeof(ParticleFloatType), particleContainer.getDataPtr<ParticleMember::Angle>());
        bind_data(5, 1, m_VBOs[3], GL_UNSIGNED_SHORT,   accumulator, particle_count, sizeof(ParticleIDType), particleContainer.getDataPtr<ParticleMember::MaterialID>());
        bind_data(6, 1, m_VBOs[4], GL_UNSIGNED_SHORT,   accumulator, particle_count, sizeof(ParticleIDType), particleContainer.getDataPtr<ParticleMember::Color>());

        glDrawElementsInstanced(GL_TRIANGLES, GLsizei(particleMesh.getVertexData().m_Indices.size()), GL_UNSIGNED_INT, 0, GLsizei(particle_count));
        renderModule.unbind(&particleMesh);
#endif
    }
}