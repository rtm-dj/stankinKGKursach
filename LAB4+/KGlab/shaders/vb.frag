#version 120
uniform sampler2D tex_stankin;
uniform sampler2D tex_vb;
uniform float time;
varying vec2 texCoord;

void main(void)
{
    float x = time;
    float faktor = clamp(pow(cos(x+0.8),50)
                             +0.1*cos(x*30+1)
                             +0.05*cos(x*50+2),
                          0,1);
    //faktor=clamp(cos(x),0,1);
    gl_FragColor = vec4(texture2D(tex_stankin,texCoord).rgb * (1-faktor) + texture2D(tex_vb,texCoord).rgb *(faktor) ,1.0); 
}
