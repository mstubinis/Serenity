#version 120

uniform vec2 gScreenSize;
uniform sampler2D texture;
uniform sampler2D texture1;

vec2 CalcTexCoord(){return gl_FragCoord.xy / gScreenSize;}

void main(){
    vec2 uv = CalcTexCoord();
    float glow = texture2D(texture,uv).r;
    vec4 diffuse = texture2D(texture1,uv);
    gl_FragColor = vec4(glow) * diffuse;
}