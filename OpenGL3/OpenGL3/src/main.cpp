//Necessary Includes:
#include <glew.h>

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

#include <glfw3.h>

#include <glm.hpp>
#include <vec2.hpp>
#include <vec3.hpp>
#include <vec4.hpp>
#include <mat3x3.hpp>
#include <mat4x4.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <SOIL2.h>

#include <fstream>
#include <math.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <string>
#include <map>

#define WIDTH 640
#define HEIGHT 480


// !!!!!!! capsule.obj MAKE SURE obj PATH IS CORRECT. !!!!!!!!!! 
const char* PATH = "cube.obj";

bool fullscreen,LeftMousebutton=false;

// Struct for sending data and render.
struct Vertex
{
    glm::vec3 position; 
    glm::vec3 color;
    glm::vec2 texcoord;
    glm::vec3 normal;
};

// For switch to fullscreen.
struct key { bool curr, prev; };
std::map<int, key>KeyMap;

// For scaling obj.
bool zoomin, zoomout;

//Camera Movemement with mouse
float yaw = -90.f, pitch = 0.f, fov = 45.f, LastX = WIDTH / 2, LastY = HEIGHT / 2, deltaTime = 0.f, LastFrame = 0.f;
bool FirstMouse = true, isEntered=false;

glm::vec3 CamFront(0.f, 0.f, -1.f);
glm::vec3 CamPosition(0.f, 0.f, 1.f);
glm::vec3 WorldUp(0.f, 1.f, 0.f);
glm::vec3 WorldRight = glm::normalize(glm::cross(WorldUp, CamPosition));

// Camera rotation with mouse.
static void MouseCallback(GLFWwindow* Window,double xpos, double ypos)
{
    if (FirstMouse)
    {
        LastX = xpos; 
        LastY = ypos;
        FirstMouse = false;
    }

    float Xoffset = xpos - LastX;
    float Yoffset = ypos - LastY;
    LastX = xpos;
    LastY = ypos;
    
    // Sensitivy
    float sensitivy = 0.1f;
    Xoffset *= sensitivy;
    Yoffset *= sensitivy;

    yaw += Xoffset;
    pitch += Yoffset;

    // Make sure that when pitch is out of bounds, screen doesnt get flips.
    if (pitch > 80.f)
        pitch = 89.f;
    if (pitch < -89.f)
        pitch = -89;

    // While holding LMB, Camera can rotate.
     if (!LeftMousebutton) return;
     glm::vec3 front;
     front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
     front.y = sin(glm::radians(pitch));
     front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
     CamFront = glm::normalize(front);   
}

// Cursor enters the OpenGL Window.
void CursorEnterCallback(GLFWwindow* Window, int entered)
{
    if (entered) isEntered=true;
    if (!entered) isEntered = false;
}

//It is true as long as LMB is holding, when release, it is false
void MouseButtonCallback(GLFWwindow* Window, int button, int mode, int action)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        LeftMousebutton = true;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        LeftMousebutton = false;
}

// Scroll funciton doesnt work. Dunno why :(
void Scroll(GLFWwindow* Window, double x,double y)
{  
    // Scroll up means get close to the obj. So; decrease the fov. Scrolling down is minus. -- means +. That increases fov.
    fov -= (float)y; 
    // Make sure fov is between 1-45.
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

// For resizing OpenGL Window.
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Read and Load Vertex and Fragment Shaders.
bool LoadShaders(GLuint& program)
{
    bool loadsuccess=true;
    char InfoLog[512];
    GLint success;

    std::string temp = "", src = "";
    std::ifstream in_file;

    // VERTEX SHADERS:
    in_file.open("vertex_core.glsl");
    if (in_file.is_open())
    {
        while (std::getline(in_file, temp))
            src += temp+ "\n";
    }
    else
    {
        //There is something wrong. Return false.
        std::cout << "ERROR::VERTEX FILE COULDNT OPEN::LoadShadersFuc\n";
        loadsuccess = false;  
    }

    in_file.close();// 

    GLint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* VertexSrc = src.c_str();
    glShaderSource(VertexShader, 1, &VertexSrc, NULL);  // Compile vertex shader.
    glCompileShader(VertexShader);
    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        //There is something wrong. Return false.
        glGetShaderInfoLog(VertexShader,512,NULL,InfoLog),
        std::cout << "ERROR::VERTEX SHADER COMPILE STATUS in func. LoadShaders.\n " << InfoLog << "\n";
        loadsuccess = false; 
    }
 
    temp = ""; src = "";
    //FRAGMENT SHADER:
    in_file.open("fragment_core.glsl");
    if (in_file.is_open())
    {
        while (std::getline(in_file, temp))
            src += temp + "\n";
    }
    else
    {
        //There is something wrong. Return false.
        std::cout << "ERROR= LoadShadersFuc::FRAGMENT FILE COULDNT OPEN ";
        loadsuccess = false;
    }

    in_file.close();

    GLint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* FragmentSrc = src.c_str();
    glShaderSource(FragmentShader, 1, &FragmentSrc, NULL); // Compile fragment shader.
    glCompileShader(FragmentShader);
    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        //There is something wrong. Return false.
        glGetShaderInfoLog(FragmentShader, 512, NULL, InfoLog);
        std::cout << "ERROR::Fragment SHADER COMPILE STATUS in func. LoadShaders. " << InfoLog << " \n";
        loadsuccess = false;
    }
    
    
    program = glCreateProgram();
    glAttachShader(program,VertexShader);       // Attach shaders and link the program.
    glAttachShader(program,FragmentShader);
    glLinkProgram(program);
    glGetProgramiv(program,GL_LINK_STATUS,&success);
    if (!success)
    {
        glGetShaderInfoLog(program, 512, NULL, InfoLog);
        std::cout << "ERROR::glLINKPROGRAM COULDNT SUCCESSED in func LoadShaders.";
        loadsuccess = false;
    }

    //END
    glUseProgram(0);            
    glDeleteShader(VertexShader);       // Clear memory
    glDeleteShader(FragmentShader);
    return loadsuccess;     // If nothings wrong, return true.
}

// User Input.
void UpdateInput(GLFWwindow* Window, GLFWmonitor* Monitor)
{
    // If there is no window, do nothing.
    if (!Window) return; 

    // Close the window when ESC press.
    if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(Window, GLFW_TRUE);
       

    if (glfwGetKey(Window, GLFW_KEY_F) == GLFW_PRESS && KeyMap[GLFW_KEY_F].prev)
    {
        // for fullscreen,Resolution set the 1920x1080.
        if (!fullscreen) glfwSetWindowMonitor(Window, Monitor, 0, 0, 1920, 1080, 0); 

        // Exit from fullscren.
        else if (fullscreen) glfwSetWindowMonitor(Window, NULL, 0, 0, WIDTH, HEIGHT, 0);
      
        fullscreen =! fullscreen;
    }

    KeyMap[GLFW_KEY_F].prev = glfwGetKey(Window, GLFW_KEY_F);

}

// User input.
void KeyPressed(GLFWwindow* Window, glm::vec3& position, glm::vec3& rotation, glm::vec3& scale) 
{
    float cameraSpeed = 2.5 * deltaTime;

    // While Holding left mouse button, camera can rotate.
    if (glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        LeftMousebutton = true; 
    
    if (glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
         LeftMousebutton = false;
    
    //Move Obj or png
    if (glfwGetKey(Window, GLFW_KEY_UP) == GLFW_PRESS)
        position.y += 0.001f;

    if (glfwGetKey(Window, GLFW_KEY_DOWN) == GLFW_PRESS)
        position.y -= 0.001f;

    if (glfwGetKey(Window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        position.x += 0.001f;

    if (glfwGetKey(Window, GLFW_KEY_LEFT) == GLFW_PRESS)
        position.x -= 0.001f;

    if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
        position.z += 0.001f;

    if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
        position.z -= 0.001f;

    // Rotate Obj or png
    if (glfwGetKey(Window, GLFW_KEY_E) == GLFW_PRESS)
        rotation.y += 0.1;

    if (glfwGetKey(Window, GLFW_KEY_Q) == GLFW_PRESS)
        rotation.y -= 0.1;

    if (glfwGetKey(Window, GLFW_KEY_Z) == GLFW_PRESS)
        rotation.x += 0.1;

    if (glfwGetKey(Window, GLFW_KEY_X) == GLFW_PRESS)
        rotation.x -= 0.1;

    if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
         rotation.z += 0.1;
    
    if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
        rotation.z -= 0.1;
    // Obj or png scale
    if (glfwGetKey(Window, GLFW_KEY_C) == GLFW_PRESS)
    {
        zoomin = true;
        zoomout = false;
    }

    if (glfwGetKey(Window, GLFW_KEY_V) == GLFW_PRESS)
    {
        zoomin = false;
        zoomout = true;
    }

    // Camera Movement
    if (glfwGetKey(Window, GLFW_KEY_I) == GLFW_PRESS)
        CamPosition += cameraSpeed * CamFront;

    if (glfwGetKey(Window, GLFW_KEY_K) == GLFW_PRESS)
        CamPosition -= cameraSpeed * CamFront;
        
    if (glfwGetKey(Window, GLFW_KEY_L) == GLFW_PRESS)
      CamPosition += glm::normalize(glm::cross(CamFront, WorldUp) * cameraSpeed); // Right  vector
        
    if (glfwGetKey(Window, GLFW_KEY_J) == GLFW_PRESS)
        CamPosition -= glm::normalize(glm::cross(CamFront, WorldUp) * cameraSpeed); // -Right  vector
    
    // Stop obj or png movement
    if (glfwGetKey(Window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        rotation = glm::vec3(0.f, 0.f, 0.f);
        position = glm::vec3(0.f, 0.f, 0.f);
        scale=glm::vec3(1.f);
    }

    //Reset Camera
    if (glfwGetKey(Window, GLFW_KEY_R) == GLFW_PRESS)
    {
        CamPosition = glm::vec3(0.f, 0.f, 2.f);
        CamFront = glm::vec3(0.f, 0.f, -1.f);
    }
}

// Some GL Options
void GLOptions()
{
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);   // My OpenGL Version is 4.6
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE); //I dunno if it is fullscreeen.
    glewExperimental = GL_TRUE;
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST); // Enable Z 
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); // Counter Clockwise
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC0_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnableClientState(GL_VERTEX_ARRAY);
}

// Invoke only InitOpenGL() in main.
void InitOpenGL()
{
    // Init glfw.
    if (glfwInit() == GLFW_FALSE)
    {
        std::cout << "ERROR::GLFW_INIT_FAILED" << "\n";
        glfwTerminate();
    }
    // Create Window.
    int FrameBufferWidth = 640; int FrameBufferHeight = 480;
    GLFWwindow* Window = glfwCreateWindow(WIDTH, HEIGHT, "TASK 1", NULL, NULL);
    glfwGetFramebufferSize(Window, &FrameBufferWidth, &FrameBufferHeight);
    glViewport(0, 0, FrameBufferWidth, FrameBufferHeight);
    glfwMakeContextCurrent(Window);
    glfwSetFramebufferSizeCallback(Window, framebuffer_size_callback);
    GLOptions();
    
    //Camera and scroll.
    glfwSetCursorPosCallback(Window, MouseCallback);
    glfwSetCursorEnterCallback(Window, CursorEnterCallback);
    glfwSetMouseButtonCallback(Window, MouseButtonCallback);
    glfwSetScrollCallback(Window, Scroll);
    glfwSetInputMode(Window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);

    //Init glew
    if (glewInit() != GLEW_OK)
    {
        std::cout << "ERROR= InitGLFW::GLEW INIT FAIL\n";
        glfwTerminate();
    }

    //Init Shader.
    GLuint CoreProgram;
    if (!LoadShaders(CoreProgram))
        glfwTerminate();

    //Here we go...
    fastObjMesh* mesh = fast_obj_read(PATH);
    if (!mesh) printf("Error loading %s: file not found\n", PATH); // If path isnt correct, blow a message.

    //Print some .obj values.
    std::cout << "\n\n << .obj FILE Values >> \n";
    std::cout << " Vertex  Count: " << mesh->position_count << "\n";
    std::cout << " Position Count: " << array_size(mesh->positions) << "\n";
    std::cout << " Texcoord Count: " << mesh->texcoord_count << "\n";
    std::cout << " Normal Count: " << mesh->normal_count << "\n";
    std::cout << " Face Vertices: " << array_size(mesh->face_vertices) << "\n";
    std::cout << " Array_size indices: " << array_size(mesh->indices) << "\n";
    std::cout << " Face Count: " << mesh->face_count << "\n";
    std::cout << " Group Count: " << mesh->group_count << "\n";

    std::vector<unsigned int> indexes;
    indexes.resize(mesh->face_count * 3); // 10200 * 3 = 30600
    unsigned int index1 = 0;

    // Create Vertex Vector and allocate for it from memory.
    std::vector<Vertex> ObjVertices;
    ObjVertices.resize(mesh->position_count);
 
    // Fill the ObjVertives with mesh data.
    for (std::size_t i = 0; i < ObjVertices.size(); ++i)
    {
        //We got position datas from mesh to our Vertex Struct.
        ObjVertices[i].position.x = mesh->positions[3 * i];
        ObjVertices[i].position.y = mesh->positions[(3 * i)+1];
        ObjVertices[i].position.z = mesh->positions[(3 * i) + 2];

        //We got texcoord datas from mesh to our Vertex Struct.
        ObjVertices[i].texcoord.x = mesh->texcoords[2 * i];
        ObjVertices[i].texcoord.y = mesh->texcoords[(2 * i)+1];
       
        // //We got normal datas from mesh to our Vertex Struct.
        ObjVertices[i].normal.x = mesh->normals[3 * i];
        ObjVertices[i].normal.x = mesh->normals[(3 * i)+1];
        ObjVertices[i].normal.x = mesh->normals[(3 * i)+ 2];

        // Set red color for all vertices.
        ObjVertices[i].color = glm::vec3(0.f, 1.f, 0.f); 
    }

    // 
    for (std::size_t j = 0; j < indexes.size(); j++)
        indexes[j] = mesh->indices[j].p;
    


    GLuint VAO; // Vertex  array Object. ID for the whole model.

    // Render the Obj file;  
    glCreateVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO; // Vertex Buffer Object.
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, ObjVertices.size() * sizeof(Vertex), &ObjVertices[0], GL_STATIC_DRAW);
         
    GLuint EBO; // Element Buffer Object. 
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size()*sizeof(unsigned int), &indexes[0], GL_STATIC_DRAW);

    // Set Vertex attrib points and enable.
    // Find the attribute location for position. 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // Find the attribute location for color.
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    // Find the attribute location for texcoord.
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texcoord));
    glEnableVertexAttribArray(2);

    // Find the attribute location for normal.
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
    

    // Init Matrices for model:
    glm::vec3 position(0.f);
    glm::vec3 rotation(0.f);
    glm::vec3 scale(1.f);
        
    // Model Matrix:
    glm::mat4 ModelMatrix(1.f);
    ModelMatrix = glm::translate(ModelMatrix, position);
    ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.x), glm::vec3(1.f,0.f,0.f));
    ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.y), glm::vec3(0.f, 1.f, 0.f));
    ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.z), glm::vec3(0.f, 0.f, 1.f));
    ModelMatrix = glm::scale(ModelMatrix, scale);
   
    // ViewMatrix:
    glm::mat4 ViewMatrix(1.f);
    ViewMatrix = glm::lookAt(CamPosition, CamPosition + CamFront, WorldUp);

    // ProjectionMatrix:
    float NearPlane = 0.1f, FarPlane = 100.f;
    glm::mat4 ProjectionMatrix(1.f);
    ProjectionMatrix = glm::perspective(
        glm::radians(fov),
        static_cast<float>(FrameBufferWidth )/ FrameBufferHeight,
        NearPlane,
        FarPlane);

    //LIGHT
    glm::vec3 lightPos0(0.f,0.f,-2.f);

    //INIT Uniforms:
    glUseProgram(CoreProgram);
    glUniformMatrix4fv(glGetUniformLocation(CoreProgram, "ModelMatrix"), 1, GL_TRUE, glm::value_ptr(ModelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(CoreProgram, "ViewMatrix"), 1, GL_TRUE, glm::value_ptr(ViewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(CoreProgram, "ProjectionMatrix"), 1, GL_TRUE, glm::value_ptr(ProjectionMatrix));
    glUniformMatrix3fv(glGetUniformLocation(CoreProgram, "lightPos0"), 1, GL_TRUE, glm::value_ptr(lightPos0));
    glUniformMatrix3fv(glGetUniformLocation(CoreProgram, "CamPosition"), 1, GL_TRUE, glm::value_ptr(CamPosition));
    glUseProgram(0);

    // For fullscreen.
    GLFWmonitor* Monitor = glfwGetPrimaryMonitor();
    fullscreen = false;

    //For Mouse input.
    double xpos=0, ypos=0;

    while (!glfwWindowShouldClose(Window)) // Main Loop
    {
        //Frametime logic
        float CurrentFrameTime = glfwGetTime();
        deltaTime = CurrentFrameTime - LastFrame;
        LastFrame = CurrentFrameTime;

        /* Poll for and process events */
        glfwPollEvents();
        KeyPressed(Window, position, rotation, scale);
        UpdateInput(Window,Monitor);

        glfwGetCursorPos(Window, &xpos, &ypos);
        glfwSetScrollCallback(Window, Scroll);
    
        // Draw --- First; clear the screen.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear Screen
        glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
        glUseProgram(CoreProgram);

        // Model Matrix
        ModelMatrix = glm::translate(ModelMatrix, position);
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.x), glm::vec3(1.f, 0.f, 0.f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.y), glm::vec3(0.f, 1.f, 0.f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.z), glm::vec3(0.f, 0.f, 1.f));
        ModelMatrix = glm::scale(ModelMatrix, scale);
        glUniformMatrix4fv(glGetUniformLocation(CoreProgram, "ModelMatrix"), 1, GL_FALSE, glm::value_ptr(ModelMatrix));

        // View Matrix
        ViewMatrix = glm::lookAt(CamPosition, CamPosition + CamFront, WorldUp);
        glUniformMatrix4fv(glGetUniformLocation(CoreProgram, "ViewMatrix"), 1, GL_TRUE, glm::value_ptr(ViewMatrix));

        // Projection Matrix
        ProjectionMatrix = glm::perspective(
            glm::radians(fov),
            static_cast<float>(FrameBufferWidth) / FrameBufferHeight,
            NearPlane = 0.1f,
            FarPlane = 100.f);
        glUniformMatrix4fv(glGetUniformLocation(CoreProgram, "ProjectionMatrix"), 1, GL_TRUE, glm::value_ptr(ProjectionMatrix));

        // For scale
        if (zoomin)
        {
            scale += 0.0001;
            zoomin = false;
        }
        if (zoomout)
        {
            scale -= 0.0001;
            zoomout = false;
        }

        glfwGetFramebufferSize(Window, &FrameBufferWidth, &FrameBufferHeight);

        // Render the .obj file. Choose one of cube or capsule.
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexes.size(), GL_UNSIGNED_INT,0); 
            
        /* Swap front and back buffers */
        glfwSwapBuffers(Window);
        glFlush();
    }
    
    // This is the sart of how it all ends.
    fast_obj_destroy(mesh);
    
    // Exiitng while loop means windows is closed.
    glfwDestroyWindow(Window);
    glfwTerminate();
    glDeleteProgram(CoreProgram);
    // All those momemnts will be lost in time... Like tears in rain. 
}

int main() 
{ 
    // Buttons.
    std::cout << " Arrows: Move object up-down-right-left\n";
    std::cout << " W-S: Move object back-forht\n";
    std::cout << " Z-X: Roate object along X Axis \n";
    std::cout << " Q-E: Roate object along Y Axis \n";
    std::cout << " A-D: Roate object along Z Axis \n";
    std::cout << " C-V: Scale object \n";
    std::cout << " SPACE: Stop object movement \n";
    std::cout << " I-K-J-L: Camera Movement \n";
    std::cout << " (While Holding LMB) Mouse: Camera Rotate \n";
    std::cout << " R: Reset Camera \n";
    std::cout << " F: Full Screen Switch \n";
    std::cout << " ESC: Close \n";

    InitOpenGL();
 
    return 0;
}