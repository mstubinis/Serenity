#version 120

uniform sampler2D DiffuseTexture;
uniform int DiffuseTextureEnabled;
uniform vec4 Object_Color;

varying vec2 UV;

void main(void){
    gl_FragData[0] = Object_Color;
    
    gl_FragData[0] = mix(Object_Color, Object_Color * texture2D(DiffuseTexture, UV), (DiffuseTextureEnabled == 1));
    
    //if(DiffuseTextureEnabled == 1){
        //gl_FragData[0] *= texture2D(DiffuseTexture, UV);
    //}
    gl_FragData[1] = vec4(1.0);
    gl_FragData[2].r = 0.0;
    gl_FragData[2].b = 0.0;
}
