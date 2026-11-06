#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <irrKlang.h>   // sound library
using namespace irrklang;
#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"
#include "stb_image.h"
#include <iostream>

// sound variables
ISoundEngine* soundEngine = createIrrKlangDevice();
ISound* sound = NULL;

// skybox variables
gps::SkyBox mySkyBox;
gps::Shader skyBoxShader;

// window
gps::Window myWindow;

// show cursor
bool showCursor = false;

// camera
gps::Camera myCamera(
    glm::vec3(-5.0f, 3.0f, 1.35f),
    glm::vec3(5.0f, 0.0f, 1.35f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.20f;

// projection planes
GLfloat NEAR_PLANE = 0.1f;
GLfloat FAR_PLANE = 250.0f;
GLfloat CUSTOM_FAR_PLANE = 45.0f;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// fog density parameter
GLfloat fogDensity = 0.0f;

// scene Tour variables
struct CameraFrame {
    glm::vec3 poz;
    glm::vec3 target;
};

std::vector<CameraFrame> sceneTour = {  // pozitia camerei si punctul catre care priveste pentru fiecare cadru al prezentarii scenei
    {{-13.7404f, 1.13901f, -0.368049f}, {-8.91589f, 0.723373f, -0.30424f}},
    {{-10.114f, 0.884436f, -1.26389f}, {-8.16562f, 0.706665f, -0.856134f}},
    {{-5.18243f, 0.884436f, -1.26389f}, {-0.034199f, 0.449664f, -0.015476f}},
    {{-0.322973f, 0.367062f, -0.358901f}, {2.15691f, -0.07034f, -0.359782f}},
    {{2.99951f, 0.367062f, -1.47037f}, {1.89858f, 0.488449f, -1.50314f}},
    {{2.99951f, 0.367062f, -2.30731f}, {1.89858f, 0.488449f, -1.50314f}},
    {{2.99951f, 0.367062f, 0.906953f}, {1.6421f, 0.491731f, 0.885227f}},
    {{2.99951f, 0.367062f, 1.82453f}, {1.6421f, 0.491731f, 0.885227f}},
    {{9.60067f, 3.42176f, 1.82453f}, {16.9919f, 4.82133f, 1.36926f}},
    {{12.824f, 3.95709f, 1.82453f}, {16.9919f, 4.82133f, 1.36926f}},
    {{15.1716f, 4.56669f, 1.82453f}, {31.1869f, 6.13755f, -0.329743f}},
    {{20.3089f, 5.5459f, 1.82453f}, {33.9987f, 7.17419f, -1.792f}},
    {{27.4179f, 6.41293f, 0.573422f}, {33.3404f, 6.58479f, -3.04912f}},
    {{30.9432f, 6.78148f, -2.23073f}, {33.7559f, 6.6819f, -3.39042f}},
    {{32.5771f, 6.78148f, -3.29499f}, {33.7559f, 6.6819f, -3.39042f}},
    {{34.2669f, 6.78148f, -4.26979f}, {129.113f, -0.070514f, -63.0556f}},
    {{62.6969f, 6.78148f, -25.4644f}, {83.2931f, 2.79013f, -52.4666f}},
    {{127.971f, 6.78148f, -37.7171f}, {330.416f, 1.31496f, 10.8837f}},
    {{168.07f, 6.78148f, 16.334f}, {355.152f, 1.36114f, -51.617f}},
    {{196.726f, 6.78148f, 42.9617f}, {555.664f, 4.00614f, -60.6836f}},
    {{220.517f, 3.53067f, 38.4063f}, {555.664f, 4.00614f, -60.6836f}},
    {{249.406f, 5.39103f, 31.4575f}, {372.547f, 2.68027f, 26.0585f}},
    {{272.782f, 5.39103f, 22.7136f}, {291.969f, 2.75923f, 17.8707f}},
    {{304.35f, 5.39103f, 10.6231f}, {373.157f, 2.68204f, 0.238113f}},
    {{319.701f, 5.39103f, 7.55612f}, {373.157f, 2.68204f, 0.238113f}},
    {{333.934f, 4.78068f, 6.31479f}, {373.157f, 2.68204f, 0.238113f}},
    {{343.123f, 4.78068f, 4.72741f}, {383.055f, 0.001597f, -1.07481f}},
    {{353.936f, 3.59596f, 3.66617f}, {390.128f, 0.009028f, -2.32893f}},
    {{360.61f, 2.20438f, 2.35143f}, {424.882f, 0.048415f, -5.67283f}},
    {{372.456f, 1.34864f, 0.162546f}, {555.595f, 0.997917f, -1.22154f}}
};

bool tourMode = true;
int currentFrame = 0;
float frameSmoothFactor = 0.08f;

void presentScene() {
    if (currentFrame >= sceneTour.size()) {
        tourMode = false;
        std::cout << "Prezentarea scenei finalizata ! Control manual activat !" << std::endl;
        return;
    }

    CameraFrame targetFrame = sceneTour[currentFrame];
    
    glm::vec3 cameraPoz = myCamera.getPosition();   // calculam pozitia si tinta curenta interpoland cu mix
    glm::vec3 cameraTarget = myCamera.getTarget();

    glm::vec3 smoothPoz = glm::mix(cameraPoz, targetFrame.poz, frameSmoothFactor);  // apropiem camera de pozitia si tinta dorita
    glm::vec3 smoothTarget = glm::mix(cameraPoz, targetFrame.target, frameSmoothFactor);

    myCamera.setPosition(smoothPoz);
    myCamera.setTarget(smoothTarget);

    if (glm::distance(cameraPoz, targetFrame.poz) < 0.2f) {  // daca suntem suficient de aproape de cadrul urmator atunci trecem la el
        currentFrame++;
    }

}

// shadow mapping
GLuint shadowMapFBO;
GLuint depthMapTexture;
const unsigned int SHADOW_WIDTH = 2 * 4096; // higher resolution = higher shadow quality but mor vRAM usage !
const unsigned int SHADOW_HEIGHT = 2 * 4096;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint fogDensityLoc;

// third-person-camera for NFS style
GLfloat camOffsetDistance = 6.0f; // distanta in spatele masinii
GLfloat camOffsetHeight = 2.2f;   // inaltime camera
GLfloat smoothFactor = 0.09f;   // fluiditate camera

bool NFSCameraON = false;

// mouse control
bool firstMouseMove = true;
GLfloat mouseSpeed = 0.1f;
GLfloat lastXPos = 0.0f;
GLfloat lastYPos = 0.0f;

GLboolean pressedKeys[1024];

// models
gps::Model3D bmwBody;
gps::Model3D bmwsf;
gps::Model3D bmwss;
gps::Model3D bmwdf;
gps::Model3D bmwds;
gps::Model3D porscheBody;
gps::Model3D porschesf;
gps::Model3D porschess;
gps::Model3D porschedf;
gps::Model3D porscheds;
gps::Model3D policeBody;
gps::Model3D policeLights;
gps::Model3D policesf;
gps::Model3D policess;
gps::Model3D policedf;
gps::Model3D policeds;
gps::Model3D road;
gps::Model3D surface;
gps::Model3D trees;
gps::Model3D lamps;
gps::Model3D buildings;
gps::Model3D finish_line;
gps::Model3D elicopter_elice_mica;
gps::Model3D elicopter_elice_mare;
gps::Model3D elicopter_body;

// animation variables
GLfloat bmw_angle = 0.0f;
GLfloat porsche_angle = 0.0f;
GLfloat police_angle = 0.0f;
GLfloat bmw_wheel_speed;    // viteza de rotatie a rotilor
GLfloat porsche_wheel_speed;
GLfloat police_wheel_speed;
GLfloat car_speed = 0.2f;  // viteza de translatie a masinii
GLfloat angle_helicopter = 0.0f;

// cars position in openGl scene
glm::vec3 bmwPoz = glm::vec3(-0.01f, 0.5f, -1.5f);
glm::vec3 porschePoz = glm::vec3(-0.01f, 0.465458f, 0.90f);
glm::vec3 policePoz = glm::vec3(-7.72989f, 0.465458f, -0.3f);
glm::vec3 elicopterPoz = glm::vec3(19.1998f, 12.0168f, 12.3837f);

enum SelectedCar {
    BMW = 1,
    PORSCHE = 2,
    POLICE = 3
};

SelectedCar currentCar;

void updateAudio() {
    if (!soundEngine) return; 

    static SelectedCar lastCar = (SelectedCar)0;
    if (currentCar != lastCar) {
        // oprim sirenele daca am schimbat masina
        if (sound) {
            sound->setVolume(0.0f);
            sound->stop();
            sound->drop();
            sound = NULL;
        }
        lastCar = currentCar;
    }
    if (currentCar == POLICE) {
        if (!sound || sound->isFinished()) {  // dam restart la sound daca nu canta nimic sau e gata
            sound = soundEngine->play2D("sounds/siren_sound.mp3", true, true, true);  // play2D(cale, loop, startPaused, track)
            if (sound) {
                sound->setVolume(0.3f); // volum => 30%
                sound->setIsPaused(false);
            }
        }
    }
    else if (currentCar == BMW) {
        if (!sound || sound->isFinished()) {  // dam restart la sound daca nu canta nimic sau e gata
            sound = soundEngine->play2D("sounds/inna.wav", true, true, true);  // play2D(cale, loop, startPaused, track)
            if (sound) {
                sound->setVolume(0.2f); // volum => 20%
                sound->setIsPaused(false);
            }
        }
    }
    else {
        if (!sound || sound->isFinished()) {  // dam restart la sound daca nu canta nimic sau e gata
            sound = soundEngine->play2D("sounds/nfs_music.mp3", true, true, true);  // play2D(cale, loop, startPaused, track)
            if (sound) {
                sound->setVolume(0.1f); // volum => 10%
                sound->setIsPaused(false);
            }
        }
    }
}

glm::vec3* getSelectedCarPos() {
    switch (currentCar) {
    case BMW:
        return &bmwPoz;
    case PORSCHE:
        return &porschePoz;
    case POLICE:
        return &policePoz;
    default:
        return &bmwPoz;
    };
}

GLfloat* getSelectedCarAngle() {
    switch (currentCar) {
    case BMW:
        return &bmw_angle;
    case PORSCHE:
        return &porsche_angle;
    case POLICE:
        return &police_angle;
    default:
        return &bmw_angle;
    };
}

GLfloat* getSelectedCarWheelSpeed() {
    switch (currentCar) {
    case BMW:
        return &bmw_wheel_speed;
    case PORSCHE:
        return &porsche_wheel_speed;
    case POLICE:
        return &police_wheel_speed;
    default:
        return &bmw_wheel_speed;
    };
}

// shaders
gps::Shader myBasicShader;
gps::Shader depthMapShader;

void initSkyBox() {

    std::vector<const GLchar*> faces;
    faces.push_back("skybox/right.png");
    faces.push_back("skybox/left.png");
    faces.push_back("skybox/top.png");
    faces.push_back("skybox/bottom.png");
    faces.push_back("skybox/back.png");
    faces.push_back("skybox/front.png");
    mySkyBox.Load(faces);

}

glm::vec3 bmwSize = glm::vec3(3.31f, 1.37935f, 0.90f);   // dimensiuni pt OBB collision
glm::vec3 porscheSize = glm::vec3(3.34f, 1.31151f, 0.92f);
glm::vec3 policeSize = glm::vec3(3.68f, 0.841439f, 1.31f);

struct OrientedBuildingBox {
    glm::vec3 position;
    glm::vec3 size;
    float rotation;
};

std::vector<OrientedBuildingBox> sceneBuildings = {
    // pozitie blender (x, z, -y) ; dimensiuni ; unghi rotatie cladire ;
    { glm::vec3(38.8132f,  6.89056f, 1.68289f),  glm::vec3(11.6472f, 14.2687f, 13.9621f), 29.3479f },
    { glm::vec3(69.3236f,  6.89056f, -50.9992f), glm::vec3(11.6472f, 14.2687f, 13.9621f), 29.3479f },
    { glm::vec3(186.467f,  6.89056f, -40.4885f), glm::vec3(23.4557f, 14.6169f, 13.9621f), 80.7434f },
    { glm::vec3(110.672f,  6.89056f, -4.99689f), glm::vec3(11.6472f, 14.2687f, 13.9621f), 577.938f },
    { glm::vec3(106.112f,  6.89056f, 17.8235f),  glm::vec3(11.6472f, 14.2687f, 13.9621f), 161.98f  },
    { glm::vec3(122.687f,  6.89056f, 20.6272f),  glm::vec3(11.6472f, 14.2687f, 13.9621f), -12.6726f },
    { glm::vec3(158.319f,  6.89056f, 48.9681f),  glm::vec3(34.4191f, 14.3871f, 13.9621f), -8.2043f  },
    { glm::vec3(218.565f,  6.89056f, 23.0567f),  glm::vec3(11.6472f, 14.2687f, 13.9621f), 11.4498f  },
    { glm::vec3(259.780f,  6.89056f, 38.1314f),  glm::vec3(11.6472f, 14.2687f, 13.9621f), 206.787f  },
    { glm::vec3(263.475f,  6.89056f, 13.0191f),  glm::vec3(11.6472f, 14.2687f, 13.9621f), 381.101f  },
    { glm::vec3(308.317f,  6.89056f, -7.26627f), glm::vec3(11.6472f, 14.2687f, 13.9621f), 544.391f  },
    { glm::vec3(322.709f,  6.89056f, -7.26627f), glm::vec3(11.6472f, 14.2687f, 13.9621f), 544.391f  },
    { glm::vec3(325.607f,  6.89056f, 17.2501f),  glm::vec3(11.6472f, 14.2687f, 13.9621f), 367.776f  }
};

bool checkBuildingCollision(glm::vec3 carPos, float carRadius, const OrientedBuildingBox& b) {
    // vectorul distanta de la pozitia cladirii la masina
    glm::vec3 delta = carPos - b.position;

    // rotatie inversa a masinii pt a fi aliniata catre cladire
    float angleRad = glm::radians(-b.rotation);
    float c = cos(angleRad);
    float s = sin(angleRad);

    float localX = delta.x * c + delta.z * s;	// BB min si max
    float localZ = -delta.x * s + delta.z * c;

    // cautam cel mai apropiat punct de pe marginea bounding box ului
    float padding = 0.2f;
    float halfX = (b.size.x / 2.0f) + padding;
    float halfZ = (b.size.z / 2.0f) + padding;

    float closestX = glm::clamp(localX, -halfX, halfX);
    float closestZ = glm::clamp(localZ, -halfZ, halfZ);

    // distante de la masina si cel mai apropiat punct
    float distX = localX - closestX;
    float distZ = localZ - closestZ;

    return (distX * distX + distZ * distZ) < (carRadius * carRadius);	// verificam distanta pt coliziune
}

bool checkBothCarAndBuildingCollision(glm:: vec3 nextPoz, SelectedCar currentCar) {
    float carToCarSafety = 0.50f;
    float buildingSafety = 0.75f;

    for (const auto& building : sceneBuildings) {   // verificam coliziunile pt masina curenta cu cladirile
        if (checkBuildingCollision(nextPoz, buildingSafety, building)) return true;
    }
       
    // verific coliziunile cu celelalte masini inactive folosindu le OBB ul
    if (currentCar != BMW) {    
        OrientedBuildingBox carOBB = { bmwPoz, bmwSize, bmw_angle };
        if (checkBuildingCollision(nextPoz, carToCarSafety, carOBB)) return true;
    }

    if (currentCar != PORSCHE) {
        OrientedBuildingBox carOBB = { porschePoz, porscheSize, porsche_angle };
        if (checkBuildingCollision(nextPoz, carToCarSafety, carOBB)) return true;
    }

    if (currentCar != POLICE) {
        OrientedBuildingBox carOBB = { policePoz, policeSize, police_angle };
        if (checkBuildingCollision(nextPoz, carToCarSafety, carOBB)) return true;
    }

    return false;
}

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);

    if (width == 0 || height == 0) return;

    // actualizam fereastra cu noile dimensiuni
    WindowDimensions newDimensions;
    newDimensions.width = width;
    newDimensions.height = height;
    myWindow.setWindowDimensions(newDimensions);

    // actualizam viewport ul
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    // actualizam matricea de proiectie
    projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, NEAR_PLANE, FAR_PLANE);

    // trimitem matricea de proiectie catre shader
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        NFSCameraON = !NFSCameraON;
        if (NFSCameraON) {
            std::cout << "Camera NFS : ON" << std::endl;
            std::cout << "Car: " << std::endl;
            switch (currentCar) {
                case BMW:
                    std::cout << "BMW M3 CAR" << std::endl;
                case PORSCHE:
                    std::cout << "PORSCHE 911 CAR" << std::endl;
                case POLICE:
                    std::cout << "POLICE CAR" << std::endl;
            }
        }
        else {
            std::cout << "Camera NFS : OFF" << std::endl;
        }
    }

    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        currentCar = BMW;
        std::cout << "Masina BMW M3 selectata !" << std::endl;
    }

    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        currentCar = PORSCHE;
        std::cout << "Masina PORSCHE 911 selectata !" << std::endl;
    }

    if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
        currentCar = POLICE;
        std::cout << "Masina Politie selectata !" << std::endl;
    }

    if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        std::cout << "Mod SOLID Activat !" << std::endl;
    }

    if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        std::cout << "Mod WIREFRAME Activat !" << std::endl;
    }

    if (key == GLFW_KEY_7 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        glPointSize(5.0f);
        std::cout << "Mod POLIGONAL Activat !" << std::endl;
    }
        
    // tasta M -> activam cursorul
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
        
    // tasta N -> dezactivam cursorul
    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        // resetam pentru a pastra pozitia precedenta
        firstMouseMove = true;
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }

    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        lightDir.y += 5.0f;
        std::cout << "Light Y: " << lightDir.y << std::endl;
    }

    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        lightDir.y -= 5.0f;
        std::cout << "Light Y: " << lightDir.y << std::endl;
    }

    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        lightDir.x += 5.0f;
        std::cout << "Light X: " << lightDir.x << std::endl;
    }

    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        lightDir.x -= 5.0f;
        std::cout << "Light X: " << lightDir.x << std::endl;
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    // daca e vizibil cursorul -> nu rotesc camera
    if (glfwGetInputMode(myWindow.getWindow(), GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {
        return;
    }

    if (NFSCameraON || tourMode) {  // nu rotim yaw si pitch daca e activata camera NFS
        return;
    }

    if (firstMouseMove == true) {
        lastXPos = xpos;
        lastYPos = ypos;
        firstMouseMove = false;
    }

    float offsetX = xpos - lastXPos;
    float offsetY = lastYPos - ypos;
    lastXPos = xpos;
    lastYPos = ypos;

    myCamera.rotate(offsetY * mouseSpeed, offsetX * mouseSpeed);
}

void moveCar(const glm::vec3& moveDirection, glm::vec3& carPoz) {

    // calculam noua pozitie
    glm::vec3 newPos = carPoz + moveDirection * car_speed;

    // verificam coliziunile cu cladirile si celelalte masini inactive
    if (!checkBothCarAndBuildingCollision(newPos, currentCar)) {
        carPoz = newPos;
        return;
    }

    // avem coliziune => incercam sa mergem pe axele X si Z
    glm::vec3 slideX = carPoz + glm::vec3(moveDirection.x * car_speed, 0.0f, 0.0f);
    if (!checkBothCarAndBuildingCollision(slideX, currentCar)) {
        carPoz.x = slideX.x;
    }

    glm::vec3 slideZ = carPoz + glm::vec3(0.0f, 0.0f, moveDirection.z * car_speed);
    if (!checkBothCarAndBuildingCollision(slideZ, currentCar)) {
        carPoz.z = slideZ.z;
    }
}

void updateNFSCamera() {

    // masina curenta folosita
    glm::vec3* currentCarPoz = getSelectedCarPos();
    // directia masinii
    GLfloat* currentAngle = getSelectedCarAngle();
    float carAngleRad = glm::radians(*currentAngle);
    // vectorul forward pt directie
    glm::vec3 forwardDir = glm::normalize(glm::vec3(cos(carAngleRad), 0.0f, -sin(carAngleRad)));

    // pozitia camerei
    glm::vec3 targetCamPos = *currentCarPoz - (forwardDir * camOffsetDistance);
    targetCamPos.y += camOffsetHeight;

    // interpolare liniara intre pozitia curenta si cea "ideala" pt a da un efect de smooth placing pt camera
    glm::vec3 currentPos = myCamera.getPosition();
    glm::vec3 smoothedPos = glm::mix(currentPos, targetCamPos, smoothFactor);

    // actualizam pozitia camerei
    myCamera.setPosition(smoothedPos);

    // punctul catre care priveste camera 
    glm::vec3 lookAtPoint = *currentCarPoz + (forwardDir * 3.0f) + glm::vec3(0.0f, 1.0f, 0.0f);
    myCamera.setTarget(lookAtPoint);
}

void processMovement() {

    angle_helicopter += 90.0f;
    if (tourMode) { // pornim prezentarea si intrerupem comenzile in timpul acesteia
        presentScene();
        return;
    }

    if (NFSCameraON) {
        updateNFSCamera();  // activam camera NFS
    }

    GLfloat* currentAngle = getSelectedCarAngle();
    glm::vec3* currentCarPos = getSelectedCarPos();
    GLfloat* currentWheelSpeed = getSelectedCarWheelSpeed();
    GLfloat angleRadians = glm::radians(*currentAngle);
    glm::vec3 forward = glm::normalize(glm::vec3(cos(angleRadians), 0.0f, -sin(angleRadians)));
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

	if (pressedKeys[GLFW_KEY_W]) {
        if (!NFSCameraON) myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        else {
            moveCar(forward, *currentCarPos);
            *currentWheelSpeed += 10.0f;
        }
	}

	if (pressedKeys[GLFW_KEY_S]) {
        if (!NFSCameraON) myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        else {
            moveCar(-forward, *currentCarPos);
            *currentWheelSpeed -= 10.0f;
        }
	}

	if (pressedKeys[GLFW_KEY_A]) {
        if (!NFSCameraON) myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        else {
            *currentAngle += 1.5f;
        }
	}

	if (pressedKeys[GLFW_KEY_D]) {
        if (!NFSCameraON) myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        else {
            *currentAngle -= 1.5f;
        }
	}

    if (pressedKeys[GLFW_KEY_8]) {
        fogDensity -= 0.001f;
        if (fogDensity < 0.0f) fogDensity = 0.0f;
    }

    if (pressedKeys[GLFW_KEY_9]) {
        fogDensity += 0.001f;
        if (fogDensity > 1.5f) fogDensity = 1.5f;
    }
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	//glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initObjects() {
    bmwBody.LoadModel("models/bmw_m3/bmw_body.obj");
    bmwsf.LoadModel("models/bmw_m3/bmw_sf.obj");
    bmwss.LoadModel("models/bmw_m3/bmw_ss.obj");
    bmwdf.LoadModel("models/bmw_m3/bmw_df.obj");
    bmwds.LoadModel("models/bmw_m3/bmw_ds.obj");
    porscheBody.LoadModel("models/porsche911/porsche_body.obj");
    porschesf.LoadModel("models/porsche911/porsche_sf.obj");
    porschess.LoadModel("models/porsche911/porsche_ss.obj");
    porschedf.LoadModel("models/porsche911/porsche_df.obj");
    porscheds.LoadModel("models/porsche911/porsche_ds.obj");
    policeBody.LoadModel("models/police/police_body.obj");
    policeLights.LoadModel("models/police/police_lights.obj");
    policesf.LoadModel("models/police/police_sf.obj");
    policess.LoadModel("models/police/police_ss.obj");
    policedf.LoadModel("models/police/police_df.obj");
    policeds.LoadModel("models/police/police_ds.obj");
    road.LoadModel("models/road/final_road.obj");
    surface.LoadModel("models/surface/surface.obj");
    trees.LoadModel("models/trees/tree_forest.obj");
    lamps.LoadModel("models/lamps/lamps.obj");
    buildings.LoadModel("models/buildings/buildings.obj");
    finish_line.LoadModel("models/finish_line/finish_line.obj");
    elicopter_elice_mica.LoadModel("models/helicopter/elicopter_elice_mica.obj");
    elicopter_elice_mare.LoadModel("models/helicopter/elicopter_elice_mare.obj");
    elicopter_body.LoadModel("models/helicopter/elicopter_body.obj");
}

void initShaders() {
	myBasicShader.loadShader("shaders/basic.vert","shaders/basic.frag");
    skyBoxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
}

void initUniforms() {
	myBasicShader.useShaderProgram();

	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // send fog density 
    fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");

    // send projection 
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               NEAR_PLANE, FAR_PLANE);
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light) and send it to shader
	lightDir = glm::vec3(50.0f, 30.0f, 15.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color and send it to shader
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    // setup spot lights
    glUniform3f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[0].color"), 1.0f, 1.0f, 0.9f); // culoare spot light -> galbena
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[0].constant"), 1.0f);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[0].linear"), 0.09f);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[0].quadratic"), 0.032f);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[0].cutOff"), glm::cos(glm::radians(20.5f)));
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[0].outerCutOff"), glm::cos(glm::radians(25.5f)));

    glUniform3f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[1].color"), 1.0f, 1.0f, 0.9f); // culoare spot light -> galbena
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[1].constant"), 1.0f);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[1].linear"), 0.09f);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[1].quadratic"), 0.032f);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[1].cutOff"), glm::cos(glm::radians(20.5f)));
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[1].outerCutOff"), glm::cos(glm::radians(25.5f)));

    glUniform3f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[2].color"), 1.0f, 1.0f, 0.9f); // culoare spot light -> galbena
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[2].constant"), 1.0f);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[2].linear"), 0.09f);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[2].quadratic"), 0.032f);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[2].cutOff"), glm::cos(glm::radians(45.0f)));
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[2].outerCutOff"), glm::cos(glm::radians(60.0f)));

    glUniform3f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[3].color"), 1.0f, 1.0f, 0.9f); // culoare spot light -> galbena
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[3].constant"), 1.0f);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[3].linear"), 0.09f);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[3].quadratic"), 0.032f);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[3].cutOff"), glm::cos(glm::radians(45.0f)));
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[3].outerCutOff"), glm::cos(glm::radians(60.0f)));
}

void initFBO() {

    glGenFramebuffers(1, &shadowMapFBO);
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {

    // pozitionam soarele uitandu se catre centru hartii (300, 0, 0)
    glm::vec3 lightPos = glm::vec3(270.0f, 0.0f, 0.0f) + glm::normalize(lightDir) * 150.0f;  // mutam soarele sa fie deasupra centrului cu 150
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(270.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    GLfloat xSize = 400.0f; // latimea zonei cu umbre
    GLfloat ySize = 400.0f; // lungimea zonei cu umbre
    GLfloat zNear = 1.0f;
    GLfloat zFar = 500.0f;  // adancimea

    // proiectia ortogonala a razelor de soare
    glm::mat4 lightProjection = glm::ortho(-xSize, xSize, -ySize, ySize, zNear, zFar);

    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

    return lightSpaceTrMatrix;
}

void drawObjects(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();
    
    // randare scena statica 
    model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // nu trimitem matricea normalelor daca suntem in depthMap
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    surface.Draw(shader);
    trees.Draw(shader);
    road.Draw(shader);
    lamps.Draw(shader);
    buildings.Draw(shader);
    finish_line.Draw(shader);

    // randare elicopter
    model = glm::mat4(1.0f);
    model = glm::translate(model, elicopterPoz);

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    elicopter_body.Draw(shader);

    // randare elice mare
    model = glm::mat4(1.0f);
    model = glm::translate(model, elicopterPoz);
    model = glm::translate(model, glm::vec3(-0.868489f, 1.18528f, -0.511178f));
    model = glm::rotate(model, angle_helicopter, glm::vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    elicopter_elice_mare.Draw(shader);

    // randare elice mica
    model = glm::mat4(1.0f);
    model = glm::translate(model, elicopterPoz);
    model = glm::rotate(model, glm::radians(-29.1383f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(4.00648f, -0.044867, 0.0f));
    model = glm::rotate(model, angle_helicopter, glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    elicopter_elice_mica.Draw(shader);

    // randare masina bmw
    model = glm::mat4(1.0f);
    model = glm::translate(model, bmwPoz);
    model = glm::rotate(model, glm::radians(bmw_angle), glm::vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    bmwBody.Draw(shader);

    // roata dreapta fata
    model = glm::mat4(1.0f);
    model = glm::translate(model, bmwPoz);
    model = glm::rotate(model, glm::radians(bmw_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(1.51251f, -0.320123f, 0.558421f));
    model = glm::rotate(model, glm::radians(-bmw_wheel_speed), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    bmwdf.Draw(shader);

    // roata dreapta spate
    model = glm::mat4(1.0f);
    model = glm::translate(model, bmwPoz);
    model = glm::rotate(model, glm::radians(bmw_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-0.474773f, -0.320054f, 0.561391f));
    model = glm::rotate(model, glm::radians(-bmw_wheel_speed), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    bmwds.Draw(shader);

    // roata stanga fata
    model = glm::mat4(1.0f);
    model = glm::translate(model, bmwPoz);
    model = glm::rotate(model, glm::radians(bmw_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(1.51668f, -0.322246f, -0.567954f));
    model = glm::rotate(model, glm::radians(-bmw_wheel_speed), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    bmwsf.Draw(shader);

    // roata stanga spate
    model = glm::mat4(1.0f);
    model = glm::translate(model, bmwPoz);
    model = glm::rotate(model, glm::radians(bmw_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-0.480258f, -0.322055f, -0.556924f));
    model = glm::rotate(model, glm::radians(-bmw_wheel_speed), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    bmwss.Draw(shader);

    // randare masina porsche
    model = glm::mat4(1.0f);
    model = glm::translate(model, porschePoz);
    model = glm::rotate(model, glm::radians(porsche_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    porscheBody.Draw(shader);

    // roata dreapta fata
    model = glm::mat4(1.0f);
    model = glm::translate(model, porschePoz);
    model = glm::rotate(model, glm::radians(porsche_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(1.13248f, -0.224158f, 0.560616f));
    model = glm::rotate(model, glm::radians(-porsche_wheel_speed), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    porschedf.Draw(shader);

    // roata dreapta spate
    model = glm::mat4(1.0f);
    model = glm::translate(model, porschePoz);
    model = glm::rotate(model, glm::radians(porsche_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-0.642618f, -0.218423f, 0.598751f));
    model = glm::rotate(model, glm::radians(-porsche_wheel_speed), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    porscheds.Draw(shader);

    // roata stanga fata
    model = glm::mat4(1.0f);
    model = glm::translate(model, porschePoz);
    model = glm::rotate(model, glm::radians(porsche_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(1.12805f, -0.232332f, -0.560707f));
    model = glm::rotate(model, glm::radians(-porsche_wheel_speed), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    porschesf.Draw(shader);

    // roata stanga spate
    model = glm::mat4(1.0f);
    model = glm::translate(model, porschePoz);
    model = glm::rotate(model, glm::radians(porsche_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-0.642438f, -0.224546f, -0.5989f));
    model = glm::rotate(model, glm::radians(-porsche_wheel_speed), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    porschess.Draw(shader);

    // randare masina politie
    model = glm::mat4(1.0f);
    model = glm::translate(model, policePoz);
    model = glm::rotate(model, glm::radians(police_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    policeBody.Draw(shader);

    // girofaruri
    model = glm::mat4(1.0f);
    model = glm::translate(model, policePoz);
    model = glm::rotate(model, glm::radians(police_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-0.188516f, 0.559468f, 0.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    policeLights.Draw(shader);

    // roata dreapta fata
    model = glm::mat4(1.0f);
    model = glm::translate(model, policePoz);
    model = glm::rotate(model, glm::radians(police_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(1.06544f, -0.210577f, 0.56442f));
    model = glm::rotate(model, glm::radians(-police_wheel_speed), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    policedf.Draw(shader);

    // roata dreapta spate
    model = glm::mat4(1.0f);
    model = glm::translate(model, policePoz);
    model = glm::rotate(model, glm::radians(police_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-0.955009f, -0.207657f, 0.58642f));
    model = glm::rotate(model, glm::radians(-police_wheel_speed), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    policeds.Draw(shader);

    // roata stanga fata
    model = glm::mat4(1.0f);
    model = glm::translate(model, policePoz);
    model = glm::rotate(model, glm::radians(police_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(1.061f, -0.220137f, -0.59814f));
    model = glm::rotate(model, glm::radians(-police_wheel_speed), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    policesf.Draw(shader);

    // roata stanga spate
    model = glm::mat4(1.0f);
    model = glm::translate(model, policePoz);
    model = glm::rotate(model, glm::radians(police_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-0.966838f, -0.219937f, -0.57214f));
    model = glm::rotate(model, glm::radians(-police_wheel_speed), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    policess.Draw(shader);
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    depthMapShader.useShaderProgram();

    // trimitem light matrix incontinuu
    glm::mat4 lightSpaceTrMatrix = computeLightSpaceTrMatrix();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceTrMatrix));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
 
    // desenam umbrele
    drawObjects(depthMapShader, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // randarea finala
    // resetam viewPort ul la dimensiunile ferestrei
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myBasicShader.useShaderProgram();
    // trimitem densitatea cetii incontinuu
    glUniform1f(fogDensityLoc, fogDensity);

    // trimitem matricele view si projection incontinuu
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // trimitem pozitia camerei pentru reflexiile speculare
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "viewPos"), 1, glm::value_ptr(myCamera.getPosition()));

    // trimitem matricea de umbre catre shader
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceTrMatrix));

    // activam multitexturing pentru a folosi umbrele din slotul 3, 0 si 1 sunt folosite de metodele Draw
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

    // actualizare pozitie si directie lumini spot light (faruri)
    GLfloat* currentAngle = getSelectedCarAngle();
    float carAngleRad = glm::radians(*currentAngle);
    glm::vec3* currentCarPoz = getSelectedCarPos();

    float distanceFront = 0.8f;
    float distanceLateral = 0.4f;
    float height = 0.5f;  

    // calculam vectorii fata si dreapta pentru precizia directiei farului
    glm::vec3 forwardDir = glm::normalize(glm::vec3(cos(carAngleRad), 0.0f, -sin(carAngleRad)));
    glm::vec3 rightDir = glm::normalize(glm::cross(forwardDir, glm::vec3(0.0f, 1.0f, 0.0f)));

    glm::vec3 headlightPosLeft = *currentCarPoz + (forwardDir * distanceFront) - (rightDir * distanceLateral);
    headlightPosLeft.y += height;

    glm::vec3 headlightPosRight = *currentCarPoz + (forwardDir * distanceFront) + (rightDir * distanceLateral);
    headlightPosRight.y += height;

    glm::vec3 headlightDir = glm::normalize(glm::vec3(forwardDir.x, -0.2f, forwardDir.z));

    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[0].position"), 1, glm::value_ptr(headlightPosLeft));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[0].direction"), 1, glm::value_ptr(headlightDir));

    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[1].position"), 1, glm::value_ptr(headlightPosRight));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[1].direction"), 1, glm::value_ptr(headlightDir));

    // trimitem continuu culorile rosu-albastru pt girofaruri politie atunci cand este condusa
    glm::vec3 light0, light1;
    glm::vec3 redLight = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 blueLight = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 lightsOffset = glm::vec3(-0.188516f, 0.559468f, 0.0f);

    float speed = 10.0f;    // viteza blink lumina
    bool toggle = (int)(glfwGetTime() * speed) % 2 == 0;    // timer pt lumini girofar

    glm::vec3 lightPoz0 = *currentCarPoz + (forwardDir * lightsOffset.x) - (rightDir * 0.25f) + glm::vec3(0.0f, lightsOffset.y, 0.0f);   // pozitie girofaruri
    glm::vec3 lightPoz1 = *currentCarPoz + (forwardDir * lightsOffset.x) + (rightDir * 0.25f) + glm::vec3(0.0f, lightsOffset.y, 0.0f);
        
    if (currentCar == POLICE) {
        light0 = toggle ? redLight : blueLight;
        light1 = toggle ? blueLight : redLight;
    } else {
        light0 = light1 = glm::vec3(0.0f);  // oprite in rest
    }

    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[2].position"), 1, glm::value_ptr(lightPoz0));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[2].direction"), 1, glm::value_ptr(forwardDir));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[2].color"), 1, glm::value_ptr(light0));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[3].position"), 1, glm::value_ptr(lightPoz1));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[3].direction"), 1, glm::value_ptr(forwardDir));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight[3].color"), 1, glm::value_ptr(light1));

    // desenam scena finala
    drawObjects(myBasicShader, false);

    // desenam skybox ul la final dupa randarea tuturor obiectelor
    mySkyBox.Draw(skyBoxShader, view, projection);
}

void cleanup() {
    myWindow.Delete();
    glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
    if (sound) sound->drop(); // curatare sound library
    if (soundEngine) soundEngine->drop();
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initObjects();
	initShaders();
    initUniforms();
    initFBO();
    initSkyBox();
    setWindowCallbacks();
    
	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();
        updateAudio();
		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}