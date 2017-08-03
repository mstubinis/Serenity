#include "Engine_AnimationProcessor.h"
#include "Engine_Renderer.h"
#include "Engine_Resources.h"
#include "MeshInstance.h"
#include "Mesh.h"

using namespace Engine;
using namespace std;

AnimationProcessor::AnimationProcessor(){
}
AnimationProcessor::~AnimationProcessor(){
}
void AnimationProcessor::process(MeshInstance* item,vector<MeshInstanceAnimation>& queue,vector<glm::mat4>& transforms){
    for(uint j = 0; j < queue.size(); j++){
        MeshInstanceAnimation& a = queue.at(j);
        if(a.mesh == item->mesh()){
            a.currentTime += Resources::dt();
            if(transforms.size() == 0){
                transforms.resize(a.mesh->m_Skeleton->m_NumBones,glm::mat4(1));
            }
            a.mesh->playAnimation(transforms,a.animName,a.currentTime);
            if(a.currentTime >= a.endTime){
                a.currentTime = 0;
                a.currentLoops++;
            }
        }
    }
    Renderer::sendUniform1iSafe("AnimationPlaying",1);
    Renderer::sendUniformMatrix4fvSafe("gBones[0]",transforms,transforms.size());

    _cleanupQueue(queue);
}
void AnimationProcessor::_cleanupQueue(vector<MeshInstanceAnimation>& queue){
    for (auto it = queue.cbegin(); it != queue.cend();){ //replace with remove/erase eventaully...
        if (it->requestedLoops > 0 && (it->currentLoops >= it->requestedLoops)){
            it = queue.erase(it);
        }
        else{
            ++it;
        }
    }
}
