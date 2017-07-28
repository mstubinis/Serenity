#include "Math.h"
#include "Engine_Resources.h"
#include "Camera.h"
#include "Scene.h"
#include "Object.h"

using namespace Engine;

static std::list<glm::vec3> Game::Pathfinding::getPathFrom(Object* start, Object* finish){
    std::list<glm::vec3> path;

    //first generate the final end point
    glm::vec3 end = finish->getPosition() - start->getPosition();
    end = glm::normalize(end);
    end = finish->getPosition() - end * finish->getRadius()*2.0f;

    //now add the start & end points to the list
    path.push_front(start->getPosition());
    path.push_back(end);

    //now the while loop, first check if the start and end points have anything in the way
    bool objInWay = true;
    while(objInWay == true){
        objInWay = false;
        for(auto obj:Resources::getCurrentScene()->objects()){
            if(Resources::getActiveCamera()->sphereIntersectTest(finish->getPosition(),finish->getRadius()*2.0f) == true){
                objInWay = true;

                //now calculate the point perpendicular to the line from start to goal and add it to the path

            }
        }
    }
    return path;
}
