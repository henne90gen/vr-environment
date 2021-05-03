const float PI = 3.14159265358979323846;
#if 0
const vec3 _bR = vec3(1.95e-4, 1.1e-3, 2.94e-3);
const float _bM = 4e-5;
const float _g = 0.93F;
#elif 1
const vec3 _bR = vec3(5.5e-6, 13.0e-6, 22.4e-6);
const float _bM = 2.1e-5;
const float _g = 0.75f;
#else
const vec3 _bR = vec3(5.8e-6, 13.5e-6, 33.1e-6);
const float _bM = 210.0e-5;
const float _g = 0.76F;
#endif

// http://developer.amd.com/wordpress/media/2012/10/GDC_02_HoffmanPreetham.pdf
// https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/simulating-sky/simulating-colors-of-the-sky
vec4 calcScattering(vec3 cameraPosition, vec3 modelPosition, vec3 lightDirection, vec3 lightColor, float lightPower, vec3 atmosphere,
vec4 color) {
    vec3 bR = _bR * atmosphere.x;
    float bM = _bM * atmosphere.y;
    float g = _g * atmosphere.z;

    vec3 modelToCamera = cameraPosition - modelPosition;
    float s = length(modelToCamera);
    vec3 extinction = exp(-(bR + bM) * s);

    float cosTheta = dot(modelToCamera, lightDirection) / (length(modelToCamera) * length(lightDirection));
    float cR = 3 / (16*PI);
    float cM = 1 / (4*PI);
    vec3 bRTheta = cR * bR * (1 + cosTheta*cosTheta);
    float oneMinusGSq = (1-g) * (1-g);
    float bMTheta = cM * bM * (oneMinusGSq / pow(1 + g*g - 2*g*cosTheta, 3/2));
    vec3 t1 = (bRTheta + bMTheta) / (bR + bM);
    vec3 t2 = 1 - extinction;
    vec3 inScatter = t1 * lightColor * lightPower * t2;

    return color * vec4(extinction, 1.0F) + vec4(inScatter, 1.0F);
}


