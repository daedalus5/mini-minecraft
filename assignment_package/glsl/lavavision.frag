#version 150

in vec2 fs_UV;

out vec3 color;

uniform sampler2D u_RenderedTexture;

void main()
{
    // TODO Homework 5
    vec3 initColor = texture(u_RenderedTexture,fs_UV).rgb;

    float c1 = 0.77*initColor.r + 0.21*initColor.g + 0.07*initColor.b; //Calculates GreyScale value with current texture pixel's rgb values
    vec3 lavaColor = vec3(c1,c1,c1);
                                         // stores the coordinates of the center of the screen, i.e (0.5, 0.5) since UV coordinates are in [0,1] range


    color = lavaColor*0.23;

}
