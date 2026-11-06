#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//camera position
uniform vec3 viewPos;

//matrices
uniform mat4 model;
uniform mat4 view;

//directional light (soarele) -> tip 1 de lumina
uniform vec3 lightDir;
uniform vec3 lightColor;

struct SpotLight{
    vec3 position;
    vec3 direction;
    float cutOff;   // unghiul conului interior
    float outerCutOff;   // unghiul conului exterior

    float constant;
    float linear;
    float quadratic;

    vec3 color;
};

//spot light (faruri si girofaruri) -> tip 2 de lumina
uniform SpotLight spotLight[4];

//textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//components for directional light
vec3 ambient;
vec3 diffuse;
vec3 specular;

// fog density uniform
uniform float fogDensity;

//illumination settings for directional light
float ambientStrength = 0.2f;
float specularStrength = 0.5f;
float shininess = 32.0f;

float computeShadow(){
    // perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5 + 0.5;
    
    if (normalizedCoords.z > 1.0f)
        return 0.0f;

    // Get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
    
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    
    // Check whether current frag pos is in shadow
    float bias = max(0.005f * (1.0f - dot(normalize(fNormal), normalize(lightDir))), 0.001f);
    float shadow = 0.0f;
    float texelSize = 1.0f / 8192.0f;

    // Percentage Closer Filtering => verificam mai multi pixeli din jurul umbrei 
    // nu doar un pixel si facem media pentru umbre mai smooth 
    for(int i=-2; i<=2; i++){
        for(int j=-2; j<=2; j++){   // facem media a 9 pixeli [-1,1]
            vec2 offset = vec2(i, j) * texelSize;   // offset pt fiecare vecin
            float pcfDepth = texture(shadowMap, normalizedCoords.xy + offset).r;    // citim depth ul din shadowMap
            shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;
        }
    }
    shadow /= 25.0f;

    return shadow;
}

vec3 computeSpotLight(SpotLight spotLight, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor){

    vec3 lightDir = normalize(spotLight.position - fragPos);

    // diffuse component
    float diff = max(dot(normal, lightDir), 0.0f);

    // specular component
    vec3 halfWayVector = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfWayVector), 0.0f), shininess);

    // attenuation
    float distance = length(spotLight.position - fragPos);
    float attenuation = 1.0f / (spotLight.constant + spotLight.linear * distance + spotLight.quadratic * (distance * distance));

    // spot light intensity
    float theta = dot(lightDir, normalize(-spotLight.direction));
    float epsilon = spotLight.cutOff - spotLight.outerCutOff;
    float intensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0f, 1.0f);

    vec3 diffuse = diff * spotLight.color * diffuseColor;
    vec3 specular = spec * specularStrength * spotLight.color * specularColor;

    return (diffuse * intensity + specular * intensity) * attenuation;

}

void main() 
{   
    vec4 colorTexture = texture(diffuseTexture, fTexCoords);
    vec3 diffuseColor = colorTexture.rgb;
    vec3 specularColor = texture(specularTexture, fTexCoords).rgb;

    // compute alpha testing for transparent background
    
    if(colorTexture.a < 0.1){
        discard;
    }

    vec3 normalEye = normalize(fNormal);
    vec3 viewDir = normalize(viewPos - fPosition);
    vec3 lightDirN = normalize(lightDir);

    // compute directional light (soarele)
    vec3 ambient = ambientStrength * lightColor * diffuseColor;

    float diff = max(dot(normalEye, lightDirN), 0.0f);

    vec3 diffuse = diff * lightColor * diffuseColor;

    vec3 halfWayVector = normalize(lightDirN + viewDir);

    float spec = pow(max(dot(normalEye, halfWayVector), 0.0f), shininess);
    vec3 specular = spec * specularStrength * lightColor * specularColor;

    // compute shadow
    float shadow = computeShadow();

    // compute final vertex color
    vec3 finalColor = (ambient + (1.0f - shadow * 1.1f) * (diffuse + specular));

    // compute the 4 spot lights to finalColor
    for(int k = 0; k < 4; k++){
        finalColor += computeSpotLight(spotLight[k], normalEye, fPosition, viewDir, diffuseColor, specularColor);
    }

    // compute fog
    float fogDistance = length(viewPos - fPosition); // distanta de la camera la texel
    float fogFactor = exp(-pow(fogDistance * fogDensity, 2));
    fogFactor = clamp(fogFactor, 0.0f, 1.0f);

    vec3 fogColor = vec3(0.5f, 0.5f, 0.5f); // culoare ceata
    vec3 colorWithFog = mix(fogColor, finalColor, fogFactor);   // combinam culorile pentru desenarea texelului

    fColor = vec4(colorWithFog, 1.0f);  // culoare finala
}
