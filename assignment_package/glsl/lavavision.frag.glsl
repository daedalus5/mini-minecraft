#version 150

uniform sampler2D u_Texture;
uniform float u_Time;
in vec2 fs_UV;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

void main()
{
    vec2 uv = fs_UV;
    uv[1] = (fs_UV[1] / 2.f) - mod(u_Time / 10.f, 1.f/2.f);
    out_Col = texture(u_Texture, uv);
    out_Col[3] = 0.5;

}
