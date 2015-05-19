//
//  ShaderPlain.fsh
//

#define USE_PHONG (1)

uniform lowp vec3       vMaterialAmbient;
uniform mediump vec4       vMaterialSpecular;

varying lowp vec4 colorDiffuse;

#if USE_PHONG
uniform highp vec3      vLight0;
varying mediump vec3 position;
varying mediump vec3 normal;
#else
varying lowp vec4 colorSpecular;
#endif

void main()
{
#if USE_PHONG
    mediump vec3 halfVector = normalize(-vLight0 + position);
    mediump float NdotH = max(dot(normalize(normal), halfVector), 0.0);
    mediump float fPower = vMaterialSpecular.w;
    mediump float specular = pow(NdotH, fPower);

    lowp vec4 colorSpecular = vec4( vMaterialSpecular.xyz * specular, 1 );
    gl_FragColor = colorDiffuse + colorSpecular;
#else
    gl_FragColor = colorDiffuse + colorSpecular;
#endif
}
