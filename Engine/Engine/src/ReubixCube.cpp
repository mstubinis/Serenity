#include "ReubixCube.h"



class ReubixCubeObject::impl{
    public:
        glm::mat3x3 m_Up1;  glm::mat3x3 m_Up2;  glm::mat3x3 m_Up3;
        glm::mat3x3 m_Left1;  glm::mat3x3 m_Left2;  glm::mat3x3 m_Left3;
        glm::mat3x3 m_Front1;  glm::mat3x3 m_Front2;  glm::mat3x3 m_Front3;

        //create 7 anchor points to use as parents - top,center,bottom,front,back,left,right


        void _init(){
            //init the default indices for each rotateable side
            m_Up1 = glm::mat3x3(0,1,2,3,4,5,6,7,8);
            m_Up2 = glm::mat3x3(9,10,11,12,13,14,15,16,17);
            m_Up3 = glm::mat3x3(18,19,20,21,22,23,24,25,26);

            m_Left1 = glm::mat3x3(0,3,6,9,12,15,18,21,24);
            m_Left2 = glm::mat3x3(1,4,7,10,13,16,19,22,25);
            m_Left3 = glm::mat3x3(2,5,8,11,14,17,20,23,26);

            m_Front1 = glm::mat3x3(0,1,2,9,10,11,18,19,20);
            m_Front2 = glm::mat3x3(3,4,5,12,13,14,21,22,23);
            m_Front3 = glm::mat3x3(6,7,8,15,16,17,24,25,26);
        }
        void _rotateMatrix(glm::mat3& _matrix,bool& left){
            _matrix = glm::transpose(_matrix); glm::mat3 copy(_matrix);
            if(left){
                //make row 1 row 3 ||| make row 3 row 1
                //row 1 to row 3
                _matrix[0] = copy[6]; _matrix[1] = copy[7]; _matrix[2] = copy[8];
                //row 3 to row 1
                _matrix[6] = copy[0]; _matrix[7] = copy[1]; _matrix[8] = copy[2];
            }
            else{
                //make col 3 col 1 ||| make col 1 col 3
                //col 1 to col 3
                _matrix[0] = copy[2]; _matrix[3] = copy[5]; _matrix[6] = copy[8];
                //col 3 to col 1
                _matrix[2] = copy[0]; _matrix[5] = copy[3]; _matrix[8] = copy[6];
            }
        }
};


ReubixCubeObject::ReubixCubeObject():m_i(new impl){
    m_i->_init();
}
ReubixCubeObject::~ReubixCubeObject(){
}
void ReubixCubeObject::rotateUp1(bool left){
    m_i->_rotateMatrix(m_i->m_Up1,left);

    //all change except up2 and up3
    //front1 - [0,1,2] become up1 [0,1,2]
    m_i->m_Front1[0] = m_i->m_Up1[0];
    m_i->m_Front1[1] = m_i->m_Up1[1];
    m_i->m_Front1[2] = m_i->m_Up1[2];

    //front2 - [0,1,2] become up1 [3,4,5]
    m_i->m_Front2[0] = m_i->m_Up1[3];
    m_i->m_Front2[1] = m_i->m_Up1[4];
    m_i->m_Front2[2] = m_i->m_Up1[5];

    //front3 - [0,1,2] become up1 [6,7,8]
    m_i->m_Front3[0] = m_i->m_Up1[6];
    m_i->m_Front3[1] = m_i->m_Up1[7];
    m_i->m_Front3[2] = m_i->m_Up1[8];

    //left1 - [0,1,2] becomes up1 [0,3,6]
    m_i->m_Left1[0] = m_i->m_Up1[0];
    m_i->m_Left1[1] = m_i->m_Up1[3];
    m_i->m_Left1[2] = m_i->m_Up1[6];

    //left2 - [0,1,2] becomes up1 [1,4,7]
    m_i->m_Left2[0] = m_i->m_Up1[1];
    m_i->m_Left2[1] = m_i->m_Up1[4];
    m_i->m_Left2[2] = m_i->m_Up1[7];

    //left3 - [0,1,2] becomes up1 [2,5,8]
    m_i->m_Left3[0] = m_i->m_Up1[2];
    m_i->m_Left3[1] = m_i->m_Up1[5];
    m_i->m_Left3[2] = m_i->m_Up1[8];
}
void ReubixCubeObject::rotateUp2(bool left){
    m_i->_rotateMatrix(m_i->m_Up2,left);

    //all change except up1 and up3

    //front1 [3,4,5] become up2 [0,1,2]
    m_i->m_Front1[3] = m_i->m_Up2[0];
    m_i->m_Front1[4] = m_i->m_Up2[1];
    m_i->m_Front1[5] = m_i->m_Up2[2];

    //front2 [3,4,5] become up2 [3,4,5]
    m_i->m_Front2[3] = m_i->m_Up2[3];
    m_i->m_Front2[4] = m_i->m_Up2[4];
    m_i->m_Front2[5] = m_i->m_Up2[5];

    //front3 [3,4,5] become up2 [6,7,8]
    m_i->m_Front3[3] = m_i->m_Up2[6];
    m_i->m_Front3[4] = m_i->m_Up2[7];
    m_i->m_Front3[5] = m_i->m_Up2[8];

    //left1 [3,4,5] become up2 [0,3,6]
    m_i->m_Left1[3] = m_i->m_Up2[0];
    m_i->m_Left1[4] = m_i->m_Up2[3];
    m_i->m_Left1[5] = m_i->m_Up2[6];

    //left2 [3,4,5] become up2 [1,4,7]
    m_i->m_Left2[3] = m_i->m_Up2[1];
    m_i->m_Left2[4] = m_i->m_Up2[4];
    m_i->m_Left2[5] = m_i->m_Up2[7];

    //left3 [3,4,5] become up2 [2,5,8]
    m_i->m_Left3[3] = m_i->m_Up2[2];
    m_i->m_Left3[4] = m_i->m_Up2[5];
    m_i->m_Left3[5] = m_i->m_Up2[8];

}
void ReubixCubeObject::rotateUp3(bool left){
    m_i->_rotateMatrix(m_i->m_Up3,left);

    //all change except up1 and up2

    //front1 [6,7,8] become up3 [0,1,2]
    m_i->m_Front1[3] = m_i->m_Up3[0];
    m_i->m_Front1[4] = m_i->m_Up3[1];
    m_i->m_Front1[5] = m_i->m_Up3[2];

    //front2 [6,7,8] become up3 [3,4,5]
    m_i->m_Front2[3] = m_i->m_Up3[3];
    m_i->m_Front2[4] = m_i->m_Up3[4];
    m_i->m_Front2[5] = m_i->m_Up3[5];

    //front3 [6,7,8] become up3 [6,7,8]
    m_i->m_Front3[3] = m_i->m_Up3[6];
    m_i->m_Front3[4] = m_i->m_Up3[7];
    m_i->m_Front3[5] = m_i->m_Up3[8];

    //left1 [6,7,8] become up3 [0,3,6]
    m_i->m_Left1[3] = m_i->m_Up3[0];
    m_i->m_Left1[4] = m_i->m_Up3[3];
    m_i->m_Left1[5] = m_i->m_Up3[6];

    //left2 [6,7,8] become up3 [1,4,7]
    m_i->m_Left2[3] = m_i->m_Up3[1];
    m_i->m_Left2[4] = m_i->m_Up3[4];
    m_i->m_Left2[5] = m_i->m_Up3[7];

    //left3 [6,7,8] become up3 [2,5,8]
    m_i->m_Left3[3] = m_i->m_Up3[2];
    m_i->m_Left3[4] = m_i->m_Up3[5];
    m_i->m_Left3[5] = m_i->m_Up3[8];
}

void ReubixCubeObject::rotateLeft1(bool left){
    m_i->_rotateMatrix(m_i->m_Left1,left);

    //all change except Left2 and Left3
}
void ReubixCubeObject::rotateLeft2(bool left){
    m_i->_rotateMatrix(m_i->m_Left2,left);

    //all change except Left1 and Left3

}
void ReubixCubeObject::rotateLeft3(bool left){
    m_i->_rotateMatrix(m_i->m_Left3,left);

    //all change except Left2 and Left1

}

void ReubixCubeObject::rotateFront1(bool left){
    m_i->_rotateMatrix(m_i->m_Front1,left);

    //all change except Front2 and Front3

}
void ReubixCubeObject::rotateFront2(bool left){
    m_i->_rotateMatrix(m_i->m_Front2,left);

    //all change except Front1 and Front3

}
void ReubixCubeObject::rotateFront3(bool left){
    m_i->_rotateMatrix(m_i->m_Front3,left);

    //all change except Front2 and Front1

}