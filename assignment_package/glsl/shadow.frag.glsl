#version 330

// Ouput data
//layout(location = 0) out float fragmentdepth;
//out float fragmentdepth;
//layout(location = 0) out vec3 out_Col;
layout(location = 0) out vec3 out_Col;

void main()
{
    //fragmentdepth = gl_FragCoord.z;
    out_Col = vec3(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z);
}
