#include <iostream>     // cout, cerr
#include <cstdlib>      // EXIT_FAILURE
#include <GL/glew.h>    // GLEW library
#include <GLFW/glfw3.h> // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>

#include "camera.h" // Camera class
#include "objloader.h"// Loader for .obj files
#include "shader.h"// Shader functions
#include "images.h" // Images functions

using namespace std; // Standard namespace

// Unnamed namespace
namespace
{
    const char *const WINDOW_TITLE = "Japan street"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;        // Handle for the vertex array name
        GLuint vbo;        // Handle for the vertex buffer name
        GLuint nVertices;  // Number of indices of the mesh
        GLuint gTextureId; // Texture
        bool isLamp;
        string name;
    };

    // Main GLFW window
    GLFWwindow *gWindow = nullptr;
    // Vector with our meshs data
    vector<GLMesh> gMeshs;

    // Texture parameters
    glm::vec2 gUVScale(1.0f, 1.0f);
    GLint gTexWrapMode = GL_REPEAT;

    // Shader programs
    GLuint gCubeProgramId;
    GLuint gLampProgramId;

    // camera
    Camera gCamera(glm::vec3(0.0f, 2.0f, 15.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // Subject position and scale
    glm::vec3 gScenePos(0.0f, 0.0f, 0.0f);
    glm::vec3 gSceneScale(1.0f);

    // Colors
    glm::vec3 gObjectColor(1.f, 0.8f, 1.0f);
    glm::vec3 gLightColor(1.0f, 1.0f, 0.0f);
    glm::vec3 gLightColor2(0.0f, 1.0f, 0.0f);

    // Light position and scale
    glm::vec3 gLightPosition(2.1f, 2.4f, -1.5f);
    glm::vec3 gLightPosition2(3.4f, 2.4f, 5.2f);
    glm::vec3 gLightScale(1.0f);
    glm::vec3 gLightScale2(1.0f);

    // List of obj files
    vector<std::string> objsFiles = {
        "Lamp2.obj",
        "BoLamp2.obj",
        "Building5.obj",
        "Signs1.obj",
        "Cone1.obj",
        "Building1.obj",
        "Lamp1.obj",
        "Cone2.obj",
        "Building3.obj",
        "Signs2.obj",
        "Block.obj",
        "Cone3.obj",
        "Building4.obj",
        "Boards.obj",
        "Building2.obj",
        "Road.obj"};
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char *[], GLFWwindow **window);
void UResizeWindow(GLFWwindow *window, int width, int height);
void UProcessInput(GLFWwindow *window);
void UMousePositionCallback(GLFWwindow *window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
void UCreateMesh(GLMesh &mesh, string name);
void UDestroyMesh(GLMesh &mesh);
bool UCreateTexture(const char *filename, GLuint &textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char *vtxShaderSource, const char *fragShaderSource, GLuint &programId);
void UDestroyShaderProgram(GLuint programId);

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow *window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow *window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}
int main(int argc, char *argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the meshs
    for (string obj : objsFiles)
    {
        GLMesh gMesh;
        UCreateMesh(gMesh, obj); // Calls the function to create the Vertex Buffer name
        gMeshs.push_back(gMesh);
    }

    // Create the shader programs
    gCubeProgramId = LoadShaders(
        "shaders/cubeVertexShader.fragmentshader",
        "shaders/cubeFragmentShader.fragmentshader");

    gLampProgramId = LoadShaders(
        "shaders/lampVertexShader.fragmentshader",
        "shaders/lampFragmentShader.fragmentshader");

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    for (GLMesh gMesh : gMeshs)
    {
        UDestroyMesh(gMesh);
        // Release texture
        UDestroyTexture(gMesh.gTextureId);
    }

    // Release shader programs
    UDestroyShaderProgram(gCubeProgramId);
    UDestroyShaderProgram(gLampProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}

// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char *argv[], GLFWwindow **window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}

// Functioned called to render a frame
void URender()
{

    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (GLMesh gMesh : gMeshs)
    {
        // Activate the mesh VAO (used by cube and lamp)
        glBindVertexArray(gMesh.vao);

        // Set the shader to be used
        glUseProgram(gCubeProgramId);

        // Model matrix: transformations are applied right-to-left order
        glm::mat4 model = glm::translate(gScenePos) * glm::scale(gSceneScale);

        // camera/view transformation
        glm::mat4 view = gCamera.GetViewMatrix();

        // Creates a perspective projection
        glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

        // Retrieves and passes transform matrices to the Shader program
        GLint modelLoc = glGetUniformLocation(gCubeProgramId, "model");
        GLint viewLoc = glGetUniformLocation(gCubeProgramId, "view");
        GLint projLoc = glGetUniformLocation(gCubeProgramId, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
        GLint objectColorLoc = glGetUniformLocation(gCubeProgramId, "objectColor");
        GLint lightColorLoc1 = glGetUniformLocation(gCubeProgramId, "lightColor1");
        GLint lightPositionLoc1 = glGetUniformLocation(gCubeProgramId, "lightPos1");
        GLint lightColorLoc2 = glGetUniformLocation(gCubeProgramId, "lightColor2");
        GLint lightPositionLoc2 = glGetUniformLocation(gCubeProgramId, "lightPos2");
        GLint viewPositionLoc = glGetUniformLocation(gCubeProgramId, "viewPosition");

        if (!gMesh.isLamp)
        {

            // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms

            // For object
            glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);

            // For light 1
            glUniform3f(lightColorLoc1, gLightColor.r, gLightColor.g, gLightColor.b);
            glUniform3f(lightPositionLoc1, gLightPosition.x, gLightPosition.y, gLightPosition.z);

            // For light 2
            glUniform3f(lightColorLoc2, gLightColor2.r, gLightColor2.g, gLightColor2.b);
            glUniform3f(lightPositionLoc2, gLightPosition2.x, gLightPosition2.y, gLightPosition2.z);

            // For camera
            const glm::vec3 cameraPosition = gCamera.Position;
            glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

            // Applying uvScale
            GLint UVScaleLoc = glGetUniformLocation(gCubeProgramId, "uvScale");
            glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

            // bind textures on corresponding texture units
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gMesh.gTextureId);

            // Draws the triangles
            glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
        }
        else
        {
            // LAMP: draw lamp
            //----------------
            glUseProgram(gLampProgramId);
            GLint lampColorLoc = glGetUniformLocation(gLampProgramId, "objectColor");

            // Select lamp by name
            if (gMesh.name == "Lamp2.obj")
            {
                glUniform4f(lampColorLoc, gLightColor.r, gLightColor.g, gLightColor.b, 0.5f);
                model = glm::translate(gLightPosition) * glm::scale(gLightScale);
            }
            else
            {
                glUniform4f(lampColorLoc, gLightColor2.r, gLightColor2.g, gLightColor2.b, 0.5f);
                model = glm::translate(gLightPosition2) * glm::scale(gLightScale2);
            }

            // Reference matrix uniforms from the Lamp Shader program
            modelLoc = glGetUniformLocation(gLampProgramId, "model");
            viewLoc = glGetUniformLocation(gLampProgramId, "view");
            projLoc = glGetUniformLocation(gLampProgramId, "projection");

            // Pass matrix data to the Lamp Shader program's matrix uniforms
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

            glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

            // Deactivate the Vertex Array name and shader program
            glBindVertexArray(0);
            glUseProgram(0);
        }
    }

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow); // Flips the the back buffer with the front buffer every frame.
}

// Implements the UCreateMesh function
void UCreateMesh(GLMesh &mesh, string name)
{
    // Position and Color data
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<GLfloat> verts;

    loadOBJ("res/objs/" + name, vertices, uvs, normals, verts);

    mesh.nVertices = vertices.size();

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);                                                  // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), &verts[0], GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(GLfloat) * (floatsPerVertex + floatsPerNormal + floatsPerUV); // The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void *)(sizeof(GLfloat) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void *)(sizeof(GLfloat) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    // Load texture, select by name
    const char *texFilename;
    if (name[0] == 'S') // For signs
    {
        texFilename = "res/imgs/sign.jpg";
    }
    else if (name == "Boards.obj" || name == "BoLamp2.obj") // For boards
    {
        texFilename = "res/imgs/wood.jpg";
    }
    else if (name[0] == 'B') // For buildings
    {
        texFilename = "res/imgs/bricks.jpg";
    }
    else if (name[0] == 'R') // For roads
    {
        texFilename = "res/imgs/Road.jpg";
    }
    else if (name[0] == 'C') // For cones
    {
        texFilename = "res/imgs/cones.jpg";
    }
    else // Else light texture
    {
        texFilename = "res/imgs/light.jpg";
    }
    if (!UCreateTexture(texFilename, mesh.gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
    }

    if (name[0] == 'L') // if object is lamp
    {
        mesh.isLamp = true;
    }
    else
    {
        mesh.isLamp = false;
    }
    mesh.name = name; // Saving object name
}

void UDestroyMesh(GLMesh &mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}

void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}
