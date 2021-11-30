// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "Helpers.h"
#include "MeshObject.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#else
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#endif

// OpenGL Mathematics Library
#include <glm/glm.hpp> // glm::vec3
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <chrono>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

// VertexBufferObject wrapper
VertexBufferObject VBO;
VertexBufferObject CBO;
VertexBufferObject NBO;

// Contains the vertex positions
// The default 6 vertices are used to show axis
std::vector<glm::vec3> V(48);
std::vector<glm::vec3> C(48);
std::vector<glm::vec3> N_v(48);
std::vector<glm::vec3> N_f(48);

// Constants
const glm::mat4 UnitMatrix(
    glm::vec4(1, 0, 0, 0),
    glm::vec4(0, 1, 0, 0),
    glm::vec4(0, 0, 1, 0),
    glm::vec4(0, 0, 0, 1)
);

// Object List
std::vector<MeshObject> ObjectList;
unsigned int OBJECT_SELECTED = -1;

// View Matrix constructors
glm::vec3 CamaraPosition(3,3,3);
glm::vec3 CamaraUp(0.0f,1.0f,0.0f);

// MVP Matrices and Perspective-mode-flag
glm::mat4 View(UnitMatrix);
glm::mat4 Perspective(UnitMatrix);
bool IF_PERSPECTIVE;

// Lightspace MVP Matrices
glm::vec3 LightPos(3.0f,0.0f,0.0f);
glm::vec3 LightTransVec(0.0f,5.0f,0.0f);
float LightRotate = 0.0f;
glm::mat4 LightView(UnitMatrix);
glm::mat4 LightPerspective(UnitMatrix);

// Trackball coordinates
bool IF_TRACKBALL;
double R, THETA, PHI;

// Transfromation modes
enum mode {
    TRANSLATION_MODE = 0,
    ROTATION_MODE = 1,
};
mode Operation_mode = TRANSLATION_MODE;

// shadow color
bool SHADOW_RED = false;

// Skybox sources
std::vector<std::string> SkyBox
{
    "/home/kurisute/Desktop/CG/assignments/assignment-4/data/night_posx.png",
    "/home/kurisute/Desktop/CG/assignments/assignment-4/data/night_negx.png",
    "/home/kurisute/Desktop/CG/assignments/assignment-4/data/night_posy.png",
    "/home/kurisute/Desktop/CG/assignments/assignment-4/data/night_negy.png",
    "/home/kurisute/Desktop/CG/assignments/assignment-4/data/night_posz.png",
    "/home/kurisute/Desktop/CG/assignments/assignment-4/data/night_negz.png"
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


glm::vec3 cursor_pos_in_window(GLFWwindow* window){
    // Get the position of the mouse in the window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert screen position to world coordinates
    glm::vec4 p_screen(xpos,height-1-ypos,0,1);
    glm::vec4 p_canonical_1((p_screen.x/width)*2-1,(p_screen.y/height)*2-1,0,1);
    glm::vec4 p_canonical_2((p_screen.x/width)*2-1,(p_screen.y/height)*2-1,1,1);
    glm::vec4 p_world_1 = glm::inverse(View) * glm::inverse(Perspective) * p_canonical_1;
    glm::vec4 p_world_2 = glm::inverse(View) * glm::inverse(Perspective) * p_canonical_2;
    // std::cout << p_world.x << p_world.y << std::endl;
    
    return glm::vec3(p_world_1 - p_world_2);
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Get the position of the mouse in the window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert screen position to world coordinates
    double xworld = ((xpos/double(width))*2)-1;
    double yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw

    // Update the position of the first vertex if the left button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        glReadPixels(xpos, height - ypos - 1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &OBJECT_SELECTED);
        std::cout << "selected:" << OBJECT_SELECTED << std::endl; 
    }

    // Upload the change to the GPU
    VBO.update(V);
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS){
        if(Operation_mode == ROTATION_MODE){
            switch(key)
            {
                case GLFW_KEY_W:
                    ObjectList[OBJECT_SELECTED].RotateVector.x += 5;
                    break;
                case GLFW_KEY_S:
                    ObjectList[OBJECT_SELECTED].RotateVector.x -= 5;
                    break;
                case GLFW_KEY_A:
                    ObjectList[OBJECT_SELECTED].RotateVector.y += 5;
                    break;
                case GLFW_KEY_D:
                    ObjectList[OBJECT_SELECTED].RotateVector.y -= 5;
                    break;
                case GLFW_KEY_F:
                    ObjectList[OBJECT_SELECTED].RotateVector.z += 5;
                    break;
                case GLFW_KEY_G:
                    ObjectList[OBJECT_SELECTED].RotateVector.z -= 5;
                    break;
            }
        }

        if(Operation_mode == TRANSLATION_MODE){
            switch(key)
            {
                case GLFW_KEY_W:
                    ObjectList[OBJECT_SELECTED].TranslateVector.y += 0.05;
                    break;
                case GLFW_KEY_S:
                    ObjectList[OBJECT_SELECTED].TranslateVector.y -= 0.05;
                    break;
                case GLFW_KEY_A:
                    ObjectList[OBJECT_SELECTED].TranslateVector.x -= 0.05;
                    break;
                case GLFW_KEY_D:
                    ObjectList[OBJECT_SELECTED].TranslateVector.x += 0.05;
                    break;
                case GLFW_KEY_F:
                    ObjectList[OBJECT_SELECTED].TranslateVector.z += 0.05;
                    break;
                case GLFW_KEY_G:
                    ObjectList[OBJECT_SELECTED].TranslateVector.z -= 0.05;
                    break;
            }
        }

        // Change Camara
        if(IF_TRACKBALL){
            std::cout << R << THETA << PHI << std::endl;
            switch(key){
                case GLFW_KEY_UP:
                    THETA -= 0.02;
                    break;
                case GLFW_KEY_DOWN:
                    THETA += 0.02;
                    break;
                case GLFW_KEY_LEFT:
                    PHI += 0.02;
                    break;
                case GLFW_KEY_RIGHT:
                    PHI -= 0.02;
                    break;
                case GLFW_KEY_EQUAL:
                    R -= 0.05;
                    break;
                case GLFW_KEY_MINUS:
                    R += 0.05;
                    break;
            }
            CamaraPosition.x = R * sin(THETA) * cos(PHI);
            CamaraPosition.y = R * cos(THETA);
            CamaraPosition.z = R * sin(THETA) * sin(PHI);
            glm::vec3 look = glm::normalize(CamaraPosition);
            glm::vec3 worldUp(0.0f,0.1f,0.0f);
            glm::vec3 right = glm::cross(look,worldUp);
            CamaraUp = -glm::cross(look,right);
        }
        else{
            switch(key){
                case GLFW_KEY_UP:
                    CamaraPosition.y += 0.05;
                    break;
                case GLFW_KEY_DOWN:
                    CamaraPosition.y -= 0.05;
                    break;
                case GLFW_KEY_LEFT:
                    CamaraPosition.x -= 0.05;
                    break;
                case GLFW_KEY_RIGHT:
                    CamaraPosition.x += 0.05;
                    break;
                case GLFW_KEY_EQUAL:
                    CamaraPosition.z -= 0.05;
                    break;
                case GLFW_KEY_MINUS:
                    CamaraPosition.z += 0.05;
                    break;
            }
        }
        
        int delete_size = 0;
        switch(key)
        {
            case  GLFW_KEY_1:
                ObjectList.push_back(MeshObject("/home/kurisute/Desktop/CG/assignments/assignment-3/data/cube.off",V.size()));
                std::cout << "cube object loaded!" << std::endl;
                V.insert(V.end(),ObjectList[ObjectList.size()-1].V.begin(),ObjectList[ObjectList.size()-1].V.end());
                C.insert(C.end(),ObjectList[ObjectList.size()-1].C.begin(),ObjectList[ObjectList.size()-1].C.end());
                N_f.insert(N_f.end(),ObjectList[ObjectList.size()-1].N_f.begin(),ObjectList[ObjectList.size()-1].N_f.end());
                N_v.insert(N_v.end(),ObjectList[ObjectList.size()-1].N_v.begin(),ObjectList[ObjectList.size()-1].N_v.end());
                std::cout << V.size() << std::endl;
                OBJECT_SELECTED = ObjectList.size()-1;
                break;
            case GLFW_KEY_2:
                ObjectList.push_back(MeshObject("/home/kurisute/Desktop/CG/assignments/assignment-3/data/bumpy_cube.off",V.size()));
                V.insert(V.end(),ObjectList[ObjectList.size()-1].V.begin(),ObjectList[ObjectList.size()-1].V.end());
                C.insert(C.end(),ObjectList[ObjectList.size()-1].C.begin(),ObjectList[ObjectList.size()-1].C.end());
                N_f.insert(N_f.end(),ObjectList[ObjectList.size()-1].N_f.begin(),ObjectList[ObjectList.size()-1].N_f.end());
                N_v.insert(N_v.end(),ObjectList[ObjectList.size()-1].N_v.begin(),ObjectList[ObjectList.size()-1].N_v.end());
                OBJECT_SELECTED = ObjectList.size()-1;
                break;
            case  GLFW_KEY_3:
                ObjectList.push_back(MeshObject("/home/kurisute/Desktop/CG/assignments/assignment-3/data/bunny.off",V.size()));
                V.insert(V.end(),ObjectList[ObjectList.size()-1].V.begin(),ObjectList[ObjectList.size()-1].V.end());
                C.insert(C.end(),ObjectList[ObjectList.size()-1].C.begin(),ObjectList[ObjectList.size()-1].C.end());
                N_f.insert(N_f.end(),ObjectList[ObjectList.size()-1].N_f.begin(),ObjectList[ObjectList.size()-1].N_f.end());
                N_v.insert(N_v.end(),ObjectList[ObjectList.size()-1].N_v.begin(),ObjectList[ObjectList.size()-1].N_v.end());
                OBJECT_SELECTED = ObjectList.size()-1;
                break;

            // Change Transformation Modes
            case GLFW_KEY_R:
                Operation_mode = ROTATION_MODE;
                break;
            case GLFW_KEY_T:
                Operation_mode = TRANSLATION_MODE;
                break;
            
            // Scale
            case GLFW_KEY_Q:
                ObjectList[OBJECT_SELECTED].ScaleVector += glm::vec3(0.05,0.05,0.05);
                break;
            case GLFW_KEY_E:
                ObjectList[OBJECT_SELECTED].ScaleVector -= glm::vec3(0.05,0.05,0.05);
                break;

            // Delete Object
            case GLFW_KEY_DELETE:
                std::cout << "OBJSIZE:"<< ObjectList.size() << std::endl;
                if(OBJECT_SELECTED == -1 || OBJECT_SELECTED >= ObjectList.size())
                    break;
                delete_size = ObjectList[OBJECT_SELECTED].V.size();
                V.erase(V.begin()+ObjectList[OBJECT_SELECTED].VBO_Pos,V.begin()+ObjectList[OBJECT_SELECTED].VBO_Pos + ObjectList[OBJECT_SELECTED].V.size());
                C.erase(C.begin()+ObjectList[OBJECT_SELECTED].VBO_Pos,C.begin()+ObjectList[OBJECT_SELECTED].VBO_Pos + ObjectList[OBJECT_SELECTED].C.size());
                N_f.erase(N_f.begin()+ObjectList[OBJECT_SELECTED].VBO_Pos,N_f.begin()+ObjectList[OBJECT_SELECTED].VBO_Pos + ObjectList[OBJECT_SELECTED].N_f.size());
                N_v.erase(N_v.begin()+ObjectList[OBJECT_SELECTED].VBO_Pos,N_v.begin()+ObjectList[OBJECT_SELECTED].VBO_Pos + ObjectList[OBJECT_SELECTED].N_v.size());
                ObjectList.erase(ObjectList.begin() + OBJECT_SELECTED);
                for(int i = OBJECT_SELECTED;i < ObjectList.size();i++){
                    ObjectList[i].VBO_Pos -= delete_size;
                }
                break;
                 
            // Change Light Positionx
            case GLFW_KEY_Y:
                LightRotate += 2.0f;
                break;
            case GLFW_KEY_U:
                LightRotate -= 2.0f;
                break;

            // Change Persepctive Mode
            case GLFW_KEY_P:
                IF_PERSPECTIVE = true;
                break;
            case GLFW_KEY_O:
                IF_PERSPECTIVE = false;
                break;
            
            // Change Trackball Mode
            case GLFW_KEY_N:
                IF_TRACKBALL = false;
                CamaraUp = glm::vec3(0,1,0);
                break;
            case GLFW_KEY_M:
                IF_TRACKBALL = true;
                R = sqrt(pow(CamaraPosition.x,2)+pow(CamaraPosition.y,2)+pow(CamaraPosition.z,2));
                THETA = acos(CamaraPosition.y/R);
                PHI = atan(CamaraPosition.z/CamaraPosition.x);
                break;
            
            // Change Rendering Mode to Selected Object
            case GLFW_KEY_X:
                ObjectList[OBJECT_SELECTED].Rmode = REFLECTION;
                break;
            case GLFW_KEY_C:
                ObjectList[OBJECT_SELECTED].Rmode = REFRACTION;
                break;
            case GLFW_KEY_Z:
                ObjectList[OBJECT_SELECTED].Rmode = PHONG;
                break;
            
            // Change shadow color
            case GLFW_KEY_B:
                SHADOW_RED = !SHADOW_RED;
            
            default:
                break;
        }
    }
    // Upload the change to the GPU
    VBO.update(V);
}


std::string LoadShader(const GLchar* path){
    std::string content;
    std::ifstream file(path, std::ios::in);

    if(!file.is_open()) {
        std::cerr << "Could not read file " << path << ". File does not exist." << std::endl;
        return "";
    }
    file.open(path);
    std::stringstream filestream;
    filestream << file.rdbuf();
    file.close();
    content = filestream.str();
    return content;
}


unsigned int LoadSkybox(std::vector<std::string> skybox){
    unsigned int textureID;
    glGenTextures(1,&textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, channels;
    for(int i = 0; i < skybox.size(); i++){
        unsigned char *data = stbi_load(skybox[i].c_str(), &width, &height, &channels, 0);
        if(data){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else{
            std::cout << "SkyBox Read Error at" << skybox[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    return textureID;
}


int main(void)
{
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(1280, 960, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    #ifndef __APPLE__
      glewExperimental = true;
      GLenum err = glewInit();
      if(GLEW_OK != err)
      {
        /* Problem: glewInit failed, something is seriously wrong. */
       fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
      }
      glGetError(); // pull and savely ignonre unhandled errors like GL_INVALID_ENUM
      fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
    #endif

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
    printf("Supported OpenGL is %s\n", (const char*)glGetString(GL_VERSION));
    printf("Supported GLSL is %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Initialize the VAO
    // A Vertex Array Object (or VAO) is an object that describes how the vertex
    // attributes are stored in a Vertex Buffer Object (or VBO). This means that
    // the VAO is not the actual object storing the vertex data,
    // but the descriptor of the vertex data.
    VertexArrayObject VAO;
    VAO.init();
    VAO.bind();

    // Initialize the VBO with the vertices data
    // A VBO is a data container that lives in the GPU memory
    VBO.init();
    CBO.init();
    NBO.init();

    // Add Axis to illustrate the result better (from V[0] to V[5])
    {
        V[0] = glm::vec3(1e+6,0,0);
        V[1] = glm::vec3(-1e+6,0,0);
        V[2] = glm::vec3(0,1e+6,0);
        V[3] = glm::vec3(0,-1e+6,0);
        V[4] = glm::vec3(0,0,1e+6);
        V[5] = glm::vec3(0,0,-1e+6);

        C[0] = glm::vec3(1,0,0);
        C[1] = glm::vec3(1,0,0);
        C[2] = glm::vec3(0,1,0);
        C[3] = glm::vec3(0,1,0);
        C[4] = glm::vec3(0,0,1);
        C[5] = glm::vec3(0,0,1);

        N_f[0] = glm::vec3(1,0,0);
        N_f[1] = glm::vec3(1,0,0);
        N_f[2] = glm::vec3(0,1,0);
        N_f[3] = glm::vec3(0,1,0);
        N_f[4] = glm::vec3(0,0,1);
        N_f[5] = glm::vec3(0,0,1);
    }

    // Add Platform to illustrate shadows better (from V[6] to V[11])
    {
        V[6] = glm::vec3(3.0,-1.0,3.0);
        V[7] = glm::vec3(3.0,-1.0,-3.0);
        V[8] = glm::vec3(-3.0,-1.0,3.0);
        V[9] = glm::vec3(-3.0,-1.0,3.0);
        V[10] = glm::vec3(3.0,-1.0,-3.0);
        V[11] = glm::vec3(-3.0,-1.0,-3.0);

        C[6] = glm::vec3(0.3,0.3,0.3);
        C[7] = glm::vec3(0.3,0.3,0.3);
        C[8] = glm::vec3(0.3,0.3,0.3);
        C[9] = glm::vec3(0.3,0.3,0.3);
        C[10] = glm::vec3(0.3,0.3,0.3);
        C[11] = glm::vec3(0.3,0.3,0.3);

        N_f[6] = glm::vec3(0,1,0);
        N_f[7] = glm::vec3(0,1,0);
        N_f[8] = glm::vec3(0,1,0);
        N_f[9] = glm::vec3(0,1,0);
        N_f[10] = glm::vec3(0,1,0);
        N_f[11] = glm::vec3(0,1,0);
    }
    
    for(int i = 0;i < 12;i++){
        N_v[i] = glm::vec3(0,1,0);
    }
    // Add the skybox (from V[12] to V[47])
    {
        //zneg
        V[12] = glm::vec3(-100.0,100.0,-100.0);
        V[13] = glm::vec3(-100.0,-100.0,-100.0);
        V[14] = glm::vec3(100.0,-100.0,-100.0);
        V[15] = glm::vec3(100.0,-100.0,-100.0);
        V[16] = glm::vec3(100.0,100.0,-100.0);
        V[17] = glm::vec3(-100.0,100.0,-100.0);
        for(int i = 12; i < 18; i++){
            N_f[i] = glm::vec3(0.0,0.0,1.0);
            N_v[i] = glm::vec3(0.0,0.0,1.0);
        }

        //xneg
        V[18] = glm::vec3(-100.0,-100.0,100.0);
        V[19] = glm::vec3(-100.0,-100.0,-100.0);
        V[20] = glm::vec3(-100.0,100.0,-100.0);
        V[21] = glm::vec3(-100.0,100.0,-100.0);
        V[22] = glm::vec3(-100.0,100.0,100.0);
        V[23] = glm::vec3(-100.0,-100.0,100.0);
        for(int i = 18; i < 24; i++){
            N_f[i] = glm::vec3(1.0,0.0,0.0);
            N_v[i] = glm::vec3(1.0,0.0,0.0);
        }

        //xpos
        V[24] = glm::vec3(100.0,-100.0,-100.0);
        V[25] = glm::vec3(100.0,-100.0,100.0);
        V[26] = glm::vec3(100.0,100.0,100.0);
        V[27] = glm::vec3(100.0,100.0,100.0);
        V[28] = glm::vec3(100.0,100.0,-100.0);
        V[29] = glm::vec3(100.0,-100.0,-100.0);
        for(int i = 24; i < 30; i++){
            N_f[i] = glm::vec3(-1.0,0.0,0.0);
            N_v[i] = glm::vec3(-1.0,0.0,0.0);
        }

        //zpos
        V[30] = glm::vec3(-100.0,-100.0,100.0);
        V[31] = glm::vec3(-100.0,100.0,100.0);
        V[32] = glm::vec3(100.0,100.0,100.0);
        V[33] = glm::vec3(100.0,100.0,100.0);
        V[34] = glm::vec3(100.0,-100.0,100.0);
        V[35] = glm::vec3(-100.0,-100.0,100.0);
        for(int i = 30; i < 36; i++){
            N_f[i] = glm::vec3(0.0,0.0,-1.0);
            N_v[i] = glm::vec3(0.0,0.0,-1.0);
        }

        //ypos
        V[36] = glm::vec3(-100.0,100.0,-100.0);
        V[37] = glm::vec3(100.0,100.0,-100.0);
        V[38] = glm::vec3(100.0,100.0,100.0);
        V[39] = glm::vec3(100.0,100.0,100.0);
        V[40] = glm::vec3(-100.0,100.0,100.0);
        V[41] = glm::vec3(-100.0,100.0,-100.0);
        for(int i = 36; i < 42; i++){
            N_f[i] = glm::vec3(0.0,-1.0,0.0);
            N_v[i] = glm::vec3(0.0,-1.0,0.0);
        }

        //yneg
        V[42] = glm::vec3(-100.0,-100.0,-100.0);
        V[43] = glm::vec3(-100.0,-100.0,100.0);
        V[44] = glm::vec3(100.0,-100.0,-100.0);
        V[45] = glm::vec3(100.0,-100.0,-100.0);
        V[46] = glm::vec3(-100.0,-100.0,100.0);
        V[47] = glm::vec3(100.0,-100.0,100.0);
        for(int i = 42; i < 48; i++){
            N_f[i] = glm::vec3(0.0,1.0,0.0);
            N_v[i] = glm::vec3(0.0,1.0,0.0);
        }

        for(int i = 12; i < 48; i++){
            C[i] = glm::vec3(0.0,0.0,0.0);
        }  
    }

    VBO.update(V);
    CBO.update(C);
    NBO.update(N_v);

    IF_PERSPECTIVE = true;
    IF_TRACKBALL = false;

    //Add Lightsource
    // ObjectList.push_back(MeshObject("/home/kurisute/Desktop/CG/assignments/assignment-3/data/lightcube.off",V.size()));
    // V.insert(V.end(),ObjectList[ObjectList.size()-1].V.begin(),ObjectList[ObjectList.size()-1].V.end());
    // C.insert(C.end(),ObjectList[ObjectList.size()-1].C.begin(),ObjectList[ObjectList.size()-1].C.end());
    // N_f.insert(N_f.end(),ObjectList[ObjectList.size()-1].N_f.begin(),ObjectList[ObjectList.size()-1].N_f.end());
    // N_v.insert(N_v.end(),ObjectList[ObjectList.size()-1].N_v.begin(),ObjectList[ObjectList.size()-1].N_v.end());
    // OBJECT_SELECTED = ObjectList.size()-1;
    // ObjectList[OBJECT_SELECTED].UnitScale = glm::vec3(1,1,1);

    //Initialize shadow depth
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    Program shadow_program;
    Program skybox_program;
    std::string vertex_shader = LoadShader("/home/kurisute/Desktop/CG/assignments/assignment-4/shader/main_shader.vert");
    std::string fragment_shader = LoadShader("/home/kurisute/Desktop/CG/assignments/assignment-4/shader/main_shader.frag");
    std::string shadow_vertex_shader = LoadShader("/home/kurisute/Desktop/CG/assignments/assignment-4/shader/shadow_shader.vert");
    std::string shadow_frag_shader = LoadShader("/home/kurisute/Desktop/CG/assignments/assignment-4/shader/shadow_shader.frag");
    std::string skybox_vertex_shader = LoadShader("/home/kurisute/Desktop/CG/assignments/assignment-4/shader/skybox_shader.vert");
    std::string skybox_frag_shader = LoadShader("/home/kurisute/Desktop/CG/assignments/assignment-4/shader/skybox_shader.frag");

    // Compile the two shaders and upload the binary to the GPU
    // Note that we have to explicitly specify that the output "slot" called outColor
    // is the one that we want in the fragment buffer (and thus on screen)
    program.init(vertex_shader,fragment_shader,"outColor");
    shadow_program.init(shadow_vertex_shader,shadow_frag_shader,"");
    skybox_program.init(skybox_vertex_shader,skybox_frag_shader,"outColor");
    
    program.bind();
    // The vertex shader wants the position of the vertices as an input.
    // The following line connects the VBO we defined above with the position "slot"
    // in the vertex shader
    program.bindVertexAttribArray("position",VBO);
    program.bindVertexAttribArray("color",CBO);
    program.bindVertexAttribArray("normal",NBO);
    shadow_program.bindVertexAttribArray("position",VBO);
    skybox_program.bindVertexAttribArray("position",VBO);


    // Save the current time --- it will be used to dynamically change the triangle color
    auto t_start = std::chrono::high_resolution_clock::now();

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // Register the mouse callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Update viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load skybox
    unsigned int skyboxTexture = LoadSkybox(SkyBox);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Bind your VAO (not necessary if you have only one)
        VAO.bind();

        // Bind your program
        program.bind();

        // Clear the framebuffer
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClearStencil(-1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        
        VBO.update(V);
        CBO.update(C);
        NBO.update(N_v);
        
        glm::vec3 light_pos = glm::vec3(glm::translate(UnitMatrix,LightTransVec)
                            * glm::rotate(UnitMatrix,glm::radians(LightRotate),glm::vec3(0.0f,1.0f,0.0f))
                            * glm::vec4(LightPos,1.0f));
        glUniform3f(program.uniform("lightPos"),light_pos.x,light_pos.y,light_pos.z);
        glUniform3f(program.uniform("lightcolor"),1.0f,1.0f,1.0f);

        // View Matrix
        View = glm::lookAt(CamaraPosition,glm::vec3(0,0,0),CamaraUp);
        glUniformMatrix4fv(program.uniform("view"),1,GL_FALSE,glm::value_ptr(View));
        glUniform3f(program.uniform("viewPos"),CamaraPosition.x,CamaraPosition.y,CamaraPosition.z);

        // Perspective Matrix
        int width,height;
        glfwGetWindowSize(window, &width, &height);
        float ratio = 1.0f*width/height;
        if(IF_PERSPECTIVE)
            Perspective = glm::perspective(glm::radians(70.0f),ratio,0.1f,500.0f);
        else
            Perspective = glm::ortho(-1.0f*ratio,1.0f*ratio,-1.0f,1.0f,0.1f,500.0f);
        glUniformMatrix4fv(program.uniform("perspective"),1,GL_FALSE,glm::value_ptr(Perspective));

        // LightSpace Matrix
        LightView = glm::lookAt(light_pos,glm::vec3(0.0f,-1.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f));
        //LightPerspective = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,1.0f,100.0f);
        LightPerspective = glm::perspective(glm::radians(90.0f),ratio,1.0f,100.0f);
        
        // Shadow Calculation
        shadow_program.bind();
        glViewport(0,0,1024,1024);
        // glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glUniformMatrix4fv(shadow_program.uniform("light_perspective"),1,GL_FALSE,glm::value_ptr(LightPerspective));
        glUniformMatrix4fv(shadow_program.uniform("light_view"),1,GL_FALSE,glm::value_ptr(LightView));
        for(int i = 0;i < ObjectList.size();i++){
            glUniformMatrix4fv(shadow_program.uniform("model"),1,GL_FALSE,glm::value_ptr(ObjectList[i].get_model_matrix()));
            for(int j = ObjectList[i].VBO_Pos;j < ObjectList[i].VBO_Pos + ObjectList[i].V.size();j+=3){
                glDrawArrays(GL_TRIANGLES, j, 3);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // Bind main shader 
        program.bind();
        glUniformMatrix4fv(program.uniform("light_perspective"),1,GL_FALSE,glm::value_ptr(LightPerspective));
        glUniformMatrix4fv(program.uniform("light_view"),1,GL_FALSE,glm::value_ptr(LightView));
        glUniform1i(program.uniform("shadow_red"),SHADOW_RED);
        glUniform1i(program.uniform("if_reflection"),false);
        glUniform1i(program.uniform("if_refraction"),false);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glUniform1i(glGetUniformLocation(program.program_shader,"skyboxmap"),0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glUniform1i(glGetUniformLocation(program.program_shader,"shadowmap"),1);
        glViewport(0, 0, width, height);
        
        // Axis Display
        glUniformMatrix4fv(program.uniform("model"),1,GL_FALSE,glm::value_ptr(UnitMatrix));
        glUniform1i(program.uniform("if_uni_color"),true);
        glUniform3f(program.uniform("uni_color"),1.0f,0.0f,0.0f);
        glDrawArrays(GL_LINES,0,2);
        glUniform3f(program.uniform("uni_color"),0.0f,1.0f,0.0f);
        glDrawArrays(GL_LINES,2,2);
        glUniform3f(program.uniform("uni_color"),0.0f,0.0f,1.0f);
        glDrawArrays(GL_LINES,4,2);
        glUniform1i(program.uniform("if_uni_color"),false);

        // Platform Display
        glUniformMatrix4fv(program.uniform("model"),1,GL_FALSE,glm::value_ptr(UnitMatrix));
        glDrawArrays(GL_TRIANGLES,6,3);
        glDrawArrays(GL_TRIANGLES,9,3);
        
        // Object Display
        for(int i = 0;i < ObjectList.size();i++){
            glUniformMatrix4fv(program.uniform("model"),1,GL_FALSE,glm::value_ptr(ObjectList[i].get_model_matrix()));
            NBO.update(N_v);
            if(ObjectList[i].Rmode == REFLECTION)
                glUniform1i(program.uniform("if_reflection"),true);
            if(ObjectList[i].Rmode == REFRACTION)
                glUniform1i(program.uniform("if_refraction"),true);
            if(OBJECT_SELECTED == i){
                glUniform1i(program.uniform("if_uni_color"),true);
                glUniform3f(program.uniform("uni_color"),1.0f,1.0f,0.0f);
            }
            for(int j = ObjectList[i].VBO_Pos;j < ObjectList[i].VBO_Pos + ObjectList[i].V.size();j+=3){
                glStencilFunc(GL_ALWAYS, i, -1);
                glDrawArrays(GL_TRIANGLES, j, 3);
            }
            glUniform1i(program.uniform("if_uni_color"),false);
            glUniform1i(program.uniform("if_reflection"),false);
            glUniform1i(program.uniform("if_refraction"),false);
        }
        
        // Skybox Display
        skybox_program.bind();
        glStencilFunc(GL_ALWAYS, -1, -1);
        glDepthMask(GL_FALSE);
        glUniformMatrix4fv(skybox_program.uniform("view"),1,GL_FALSE,glm::value_ptr(View));
        glUniformMatrix4fv(skybox_program.uniform("projection"),1,GL_FALSE,glm::value_ptr(Perspective));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glUniform1i(glGetUniformLocation(skybox_program.program_shader,"skybox"),0);
        glDrawArrays(GL_TRIANGLES, 12, 36);
        glDepthMask(GL_TRUE);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Deallocate opengl memory
    skybox_program.free();
    shadow_program.free();
    program.free();
    VAO.free();
    VBO.free();
    CBO.free();

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}
