#version 150

uniform sampler2D u_Texture;
uniform sampler2D u_ShadowTexture; //testing shadow map
in vec2 fs_UV;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

void main()
{
    out_Col = texture(u_ShadowTexture, fs_UV);

}
