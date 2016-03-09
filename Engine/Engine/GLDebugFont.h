#ifndef BT_DEBUG_FONT_H
#define BT_DEBUG_FONT_H

class btVector3;
void GLDebugDrawStringInternal(int x,int y,const char* string,const btVector3& rgb, bool enableBlend, int spacing);
void GLDebugDrawStringInternal(int x,int y,const char* string,const btVector3& rgb);
void GLDebugDrawString(int x,int y,const char* string);
void GLDebugResetFont(int screenWidth,int screenHeight);

#endif