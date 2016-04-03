#ifndef LAGRANGE_H
#define LAGRANGE_H

#include "Object.h"

class Planet;
enum LAGRANGE_TYPE { LAGRANGE_TYPE_L1,LAGRANGE_TYPE_L2,LAGRANGE_TYPE_L3,LAGRANGE_TYPE_L4,LAGRANGE_TYPE_L5 };
class Lagrange: public ObjectBasic{
    private:
        boost::weak_ptr<Planet> m_Planet1;
        boost::weak_ptr<Planet> m_Planet2;
        bool m_Visible;
        void _calculateLagrangePosition(LAGRANGE_TYPE);
        void _init(std::string planet1,std::string planet2, LAGRANGE_TYPE type);

        LAGRANGE_TYPE m_Type;

        static float radius;
        static GLuint m_Buffer;
        static std::vector<glm::vec3> m_Vertices;

    public:
        Lagrange(std::string,
                 std::string,
                 LAGRANGE_TYPE = LAGRANGE_TYPE_L1,
                 std::string = "Lagrange Point",   //Object name
                 Scene* = nullptr
               );
        Lagrange(std::string,
                 std::string,
                 std::string = "L1",
                 std::string = "Lagrange Point",   //Object name
                 Scene* = nullptr
               );
        virtual ~Lagrange();

        const LAGRANGE_TYPE getType() const{ return m_Type; }

        glm::vec2 getGravityInfo(){ return glm::vec2(this->getRadius()*100,this->getRadius()*150); }

        void update(float);
        void render(GLuint=0, bool=false);
        void draw(GLuint=0,bool=false);
};

#endif