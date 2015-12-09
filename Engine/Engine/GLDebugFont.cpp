#include "GLDebugFont.h"

#ifdef _WIN32//for glut.h
#include <windows.h>
#endif

//think different
#if defined(__APPLE__) && !defined (VMDMESA)
#include <TargetConditionals.h>
#if (defined (TARGET_OS_IPHONE) && TARGET_OS_IPHONE) || (defined (TARGET_IPHONE_SIMULATOR) && TARGET_IPHONE_SIMULATOR)
#import <OpenGLES/ES1/gl.h>
#define glOrtho glOrthof
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#endif
#else

#ifdef _WINDOWS
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif
#endif

#include <stdio.h>
#include <string.h> //for memset

void GLDebugResetFont(int screenWidth,int screenHeight){	

}
#define USE_ARRAYS 1
void GLDebugDrawStringInternal(int x,int y,const char* string, const btVector3& rgb){ 
	GLDebugDrawStringInternal(x,y,string,rgb,true,10); 
}
void GLDebugDrawStringInternal(int x,int y,const char* string, const btVector3& rgb, bool enableBlend, int spacing){

}
void GLDebugDrawString(int x,int y,const char* string){

}