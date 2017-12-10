#version 330

// Ouput data
//layout(location = 0) out float fragmentdepth;
//out float fragmentdepth;
//out vec4 out_Col;
layout(location = 0) out vec4 out_Col;

void main()
{
    //fragmentdepth = gl_FragCoord.z;
    out_Col = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1);
    //out_Col = vec4(0, 0, gl_FragCoord.z, 1);
}
