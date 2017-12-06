#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform vec4 u_Color; // The color with which to render this instance of geometry.
uniform sampler2D u_Texture; // The texture to be read from by this shader
uniform vec4 u_Eye; // Camera position. Used for Blinn-Phong
uniform float u_Time; // Elapsed time since start of game

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_Col;
in vec4 fs_UV; // [u, v, blinn-phong exponent, flag for animation]
in vec4 fs_Pos;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

void main()
{
    vec4 pos2eye = u_Eye - fs_Pos;


    // Blinn-Phong
    vec3 H = normalize((vec3(pos2eye) + vec3(fs_LightVec)) / 2);
    vec3 N = normalize(vec3(fs_Nor));
    float exp = fs_UV[2];
    float specularIntensity = max(pow(dot(H, N), exp), 0);
    specularIntensity = clamp(specularIntensity, 0, 1);

    // Compute UVs for animated blocks
    vec2 uv = fs_UV.xy;
    if (fs_UV[3] == 1) {
        uv[0] = fs_UV[0] + mod(((u_Time / 10.f) * (1.f / 16.f)), (1.f/16.f));
    }

    // Lambert
    // Material base color (before shading)
    vec4 diffuseColor = texture(u_Texture, uv);
    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);
    float ambientTerm = 0.2;
    float lightIntensity = diffuseTerm + ambientTerm;   //Add a small float value to the color multiplier
                                                        //to simulate ambient lighting. This ensures that faces that are not
                                                        //lit by our point light are not completely black.

    vec4 color = vec4(diffuseColor.rgb * (0.8 * lightIntensity + 0.3 * specularIntensity), diffuseColor.a);

    // Compute fog
    float fog = 1.0 - pow(3,((-(length(pos2eye) - 110) * 0.05)));
    fog = clamp(fog, 0, 1);

    // Compute final shaded color
    out_Col = mix(color, vec4(0.8, 0.8, 0.8, 1), fog);
}
