#version 120

uniform vec2 gScreenSize;
uniform sampler2D texture;
uniform float radius;

float threshold(float thr1,float thr2 ,float val) {
    if (val < thr1) { return 0.0; }
    if (val > thr2) { return 1.0; }
    return val;
}

// averaged pixel intensity from 3 color channels
float avg_intensity(vec4 pix) {
    return (pix.r + pix.g + pix.b)/3.0;
}
vec4 get_pixel(vec2 coords,float dx,float dy) {
    float value = texture2D(texture,coords + vec2(dx, dy)).r;
    return vec4(value);
}
float IsEdge(vec2 coords){
    float dxtex = 1.0 / gScreenSize.x;
    float dytex = 1.0 / gScreenSize.y;
    float pix[9];
    int k = -1;
    float delta;

    // read neighboring pixel intensities
    for (int i=-1; i<2; i++) {
        for(int j=-1; j<2; j++) {
            k++;
            pix[k] = avg_intensity(get_pixel(coords,float(i)*dxtex,float(j)*dytex));
        }
    }
    // average color differences around neighboring pixels
    delta = (abs(pix[1]-pix[7])+abs(pix[5]-pix[3]) +abs(pix[0]-pix[8])+abs(pix[2]-pix[6]))/2.0;
    return threshold(0.15,0.6,clamp(1.8*delta,0.0,1.0));
}
void main(){
    vec2 uv = gl_TexCoord[0].st;
    float edge = IsEdge(gl_TexCoord[0].xy);
    gl_FragColor = vec4(edge);
}