#version 330

uniform sampler2D texture;
uniform vec2 gScreenSize;


layout(location=0)out vec4 BlurOut;

varying vec2 offset[14];

vec2 CalcTexCoord(){return gl_FragCoord.xy / gScreenSize;}
void main(void){
   vec4 sum = vec4(0);
   vec2 uv = CalcTexCoord();
   vec4 color = texture2D(texture, uv );

	sum += texture2D(texture, uv+offset[ 0])*0.0044;
	sum += texture2D(texture, uv+offset[ 1])*0.009;
	sum += texture2D(texture, uv+offset[ 2])*0.0216;
	sum += texture2D(texture, uv+offset[ 3])*0.044;
	sum += texture2D(texture, uv+offset[ 4])*0.078;
	sum += texture2D(texture, uv+offset[ 5])*0.116;
	sum += texture2D(texture, uv+offset[ 6])*0.147;
	sum += texture2D(texture, uv )*0.16;
	sum += texture2D(texture, uv+offset[ 7])*0.147;
	sum += texture2D(texture, uv+offset[ 8])*0.116;
	sum += texture2D(texture, uv+offset[ 9])*0.078;
	sum += texture2D(texture, uv+offset[10])*0.044;
	sum += texture2D(texture, uv+offset[11])*0.0216;
	sum += texture2D(texture, uv+offset[12])*0.009;
	sum += texture2D(texture, uv+offset[13])*0.0044;

   BlurOut = sum;
}