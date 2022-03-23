#include <serenity/renderer/particles/ParticleRendererOpenGL.h>
#include <serenity/system/Engine.h>
#include <serenity/resources/mesh/Mesh.h>

#include <serenity/renderer/particles/ParticleContainer.h>
#include <serenity/renderer/particles/ParticleIncludes.h>

void Engine::priv::ParticleRendererOpenGL::init() {
    //particle instancing
    Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getParticleMesh().get<Mesh>()->getVertexData().bind();
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STREAM_DRAW);

    auto sizeofOne = sizeof(ParticleFloatType) * 4;
    auto sizeofTwo = sizeof(ParticleFloatType) * 2;

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, PARTICLE_FLOAT_TYPE, GL_FALSE, sizeof(Engine::priv::ParticleDOD), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, PARTICLE_FLOAT_TYPE, GL_FALSE, sizeof(Engine::priv::ParticleDOD), (void*)sizeofOne);
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 2, GL_UNSIGNED_SHORT, sizeof(Engine::priv::ParticleDOD), (void*)(sizeofOne + sizeofTwo));

    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);

    Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getParticleMesh().get<Mesh>()->getVertexData().unbind();
}
void Engine::priv::ParticleRendererOpenGL::render(Engine::priv::Renderer& renderer, Engine::priv::ParticleContainer& particleContainer) {
    const auto particle_count = particleContainer.getNumParticles();
    /*
    const size_t particle_count = system.ParticlesDOD.size();
    if (particle_count > 0) {
        m_Renderer.bind(program.get<ShaderProgram>());
        for (auto it = system.Bimap.rbegin(); it != system.Bimap.rend(); ++it) {
            system.MaterialIDToIndex.try_emplace(it->first, (uint32_t)system.MaterialIDToIndex.size());
        }
        for (auto& particlePOD : system.ParticlesDOD) {
            particlePOD.MatID = system.MaterialIDToIndex.at(particlePOD.MatID);
        }
        for (const auto& [id, index] : system.MaterialIDToIndex) {
            Material* mat              = system.Bimap.at(id);
            Texture& texture           = *mat->getComponent(MaterialComponentType::Diffuse).getTexture(0).get<Texture>();
            const std::string location = "Tex" + std::to_string(index);
            Engine::Renderer::sendTextureSafe(location.c_str(), texture, index);
        }
        const auto maxTextures = getMaxNumTextureUnits() - 1U;
        Engine::Renderer::sendTextureSafe("gDepthMap", m_GBuffer.getTexture(GBufferType::Depth), maxTextures);

        glBindBuffer(GL_ARRAY_BUFFER, m_Particle_Instance_VBO);
        glBufferData(GL_ARRAY_BUFFER, particle_count * sizeof(Engine::priv::ParticleDOD), system.ParticlesDOD.data(), GL_STREAM_DRAW);

        auto& particleMesh = *Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getParticleMesh().get<Mesh>();
        renderer.bind(&particleMesh);
        glDrawElementsInstanced(GL_TRIANGLES, GLsizei(particleMesh.getVertexData().m_Indices.size()), GL_UNSIGNED_INT, 0, GLsizei(particle_count));
        renderer.unbind(&particleMesh);
    }
    */
}