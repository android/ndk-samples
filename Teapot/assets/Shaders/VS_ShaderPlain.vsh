//
//  ShaderPlain.vsh
//

#define USE_PHONG (1)

attribute highp vec3    myVertex;
attribute highp vec3    myNormal;
attribute mediump vec2  myUV;
attribute mediump vec4  myBone;

varying mediump vec2    texCoord;
varying lowp    vec4    colorDiffuse;

#if USE_PHONG
varying mediump vec3 position;
varying mediump vec3 normal;
#else
varying lowp    vec4    colorSpecular;
#endif

uniform highp mat4      uMVMatrix;
uniform highp mat4      uPMatrix;

uniform highp vec3      vLight0;

uniform lowp vec4       vMaterialDiffuse;
uniform lowp vec3       vMaterialAmbient;
uniform lowp vec4       vMaterialSpecular;

void main(void)
{
    highp vec4 p = vec4(myVertex,1);
    gl_Position = uPMatrix * p;

    texCoord = myUV;

    highp vec3 worldNormal = vec3(mat3(uMVMatrix[0].xyz, uMVMatrix[1].xyz, uMVMatrix[2].xyz) * myNormal);
    highp vec3 ecPosition = p.xyz;

    colorDiffuse = dot( worldNormal, normalize(-vLight0+ecPosition) ) * vMaterialDiffuse  + vec4( vMaterialAmbient, 1 );

#if USE_PHONG
    normal = worldNormal;
    position = ecPosition;
#else
    highp vec3 halfVector = normalize(ecPosition - vLight0);

    highp float NdotH = max(-dot(worldNormal, halfVector), 0.0);
    float fPower = vMaterialSpecular.w;
    highp float specular = min( pow(NdotH, fPower), 1.0);
    colorSpecular = vec4( vMaterialSpecular.xyz * specular, 1 );
#endif
}
