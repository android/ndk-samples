#version 300 es
precision mediump float;

uniform mediump vec4  vMaterialSpecular;
uniform highp vec3 vLight0;

in lowp vec4 colorDiffuse;
in vec3 position;
in vec3 normal;
out vec4 outColor;

void main()
{
    mediump vec3 halfVector = normalize(-vLight0 + position);
    mediump float NdotH = max(dot(normalize(normal), halfVector), 0.0);
    mediump float fPower = vMaterialSpecular.w;
    mediump float specular = pow(NdotH, fPower);

    lowp vec4 colorSpecular = vec4( vMaterialSpecular.xyz * specular, 1 );
    outColor = colorDiffuse + colorSpecular;
}
