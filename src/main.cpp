#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadTexture(char const * path);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 backpackPosition = glm::vec3(0.0f);
    float backpackScale = 1.0f;
    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES,4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    // build and compile shaders
    // -------------------------
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader ourShader1("resources/shaders/shader1.vs", "resources/shaders/shader1.fs");
    Shader ourShader2("resources/shaders/shader2.vs", "resources/shaders/shader2.fs");
    Shader ourShader3("resources/shaders/shader3.vs", "resources/shaders/shader3.fs");
    Shader ourShader4("resources/shaders/shader4.vs", "resources/shaders/shader4.fs");
    Shader ourShader5("resources/shaders/shader5.vs", "resources/shaders/shader5.fs");
    Shader lightShader("resources/shaders/lightcube.vs", "resources/shaders/lightcube.fs");

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    float vertices0[] = {
            // Positions            // Normals           // Texture Coords
            // Front face
            -0.2f, -0.2f, -0.2f,    0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
            0.2f, -0.2f, -0.2f,     0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
            0.2f,  0.2f, -0.2f,     0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
            0.2f,  0.2f, -0.2f,     0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
            -0.2f,  0.2f, -0.2f,    0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
            -0.2f, -0.2f, -0.2f,    0.0f, 0.0f, -1.0f,  0.0f, 0.0f,

            // Back face
            0.2f, -0.2f, 0.2f,      0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
            -0.2f, -0.2f, 0.2f,     0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
            -0.2f,  0.2f, 0.2f,     0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
            -0.2f,  0.2f, 0.2f,     0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
            0.2f,  0.2f, 0.2f,      0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
            0.2f, -0.2f, 0.2f,      0.0f, 0.0f, 1.0f,   0.0f, 0.0f,

            // Left face
            -0.2f,  0.2f, -0.2f,    -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
            -0.2f,  0.2f,  0.2f,    -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
            -0.2f, -0.2f,  0.2f,    -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
            -0.2f, -0.2f,  0.2f,    -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
            -0.2f, -0.2f, -0.2f,    -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
            -0.2f,  0.2f, -0.2f,    -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,

            // Right face
            0.2f,  0.2f,  0.2f,     1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
            0.2f, -0.2f,  0.2f,     1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
            0.2f, -0.2f, -0.2f,     1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
            0.2f, -0.2f, -0.2f,     1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
            0.2f,  0.2f, -0.2f,     1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
            0.2f,  0.2f,  0.2f,     1.0f, 0.0f, 0.0f,   1.0f, 0.0f,

            // Bottom face
            -0.2f, -0.2f, -0.2f,    0.0f, -1.0f, 0.0f,  0.0f, 1.0f,
            0.2f, -0.2f, -0.2f,     0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
            0.2f, -0.2f,  0.2f,     0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
            0.2f, -0.2f,  0.2f,     0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
            -0.2f, -0.2f,  0.2f,    0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
            -0.2f, -0.2f, -0.2f,    0.0f, -1.0f, 0.0f,  0.0f, 1.0f,

            // Top face
            -0.2f,  0.2f, -0.2f,    0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
            0.2f,  0.2f, -0.2f,     0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
            0.2f,  0.2f,  0.2f,     0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
            0.2f,  0.2f,  0.2f,     0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
            -0.2f,  0.2f,  0.2f,    0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
            -0.2f,  0.2f, -0.2f,    0.0f, 1.0f, 0.0f,   0.0f, 1.0f
    };


    float vertices1[] = {
            // Right face
            // Positions         // Normals          // Texture Coords
            0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,  1.0f, 1.0f,  // top right
            0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,  1.0f, 0.0f,  // bottom right
            0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,  0.0f, 0.0f,  // bottom front
            0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,  1.0f, 1.0f,  // top right
            0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,  0.0f, 0.0f,  // bottom front
            0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,  0.0f, 1.0f,  // top front

            // Top face
            // Positions         // Normals          // Texture Coords
            0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,  1.0f, 0.0f,  // top right
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,  // top left
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,  // bottom left
            0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,  1.0f, 0.0f,  // top right
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,  // bottom left
            0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,  1.0f, 1.0f,  // bottom right

            // Back face
            // Positions         // Normals          // Texture Coords
            0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,  1.0f, 0.0f,  // top right
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,  // top left
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // bottom left
            0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,  1.0f, 0.0f,  // top right
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // bottom left
            0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,  1.0f, 1.0f,  // bottom right

            // Bottom face
            // Positions         // Normals          // Texture Coords
            0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // top right
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f, 0.0f, 1.0f,  // top left
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom left
            0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // top right
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom left
            0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f, 1.0f, 0.0f   // bottom right
    };
    unsigned VBO, VAO;
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);


    glGenBuffers(1,&VBO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices1),vertices1,GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);

    unsigned VBO1, VAO1;
    glGenVertexArrays(1,&VAO1);
    glBindVertexArray(VAO1);


    glGenBuffers(1,&VBO1);
    glBindBuffer(GL_ARRAY_BUFFER,VBO1);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices0),vertices0,GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);

    unsigned int diffuseMap1 = loadTexture(FileSystem::getPath("resources/textures/plocice.png").c_str());
    ourShader1.use();
    ourShader1.setInt("material.diffuse",0);

    unsigned int diffuseMap2 = loadTexture(FileSystem::getPath("resources/textures/woodfloor2.png").c_str());
    ourShader2.use();
    ourShader2.setInt("material.diffuse",0);

    unsigned int diffuseMap3 = loadTexture(FileSystem::getPath("resources/textures/plafon1.jpg").c_str());
    ourShader3.use();
    ourShader3.setInt("material.diffuse",0);

    unsigned int diffuseMap4 = loadTexture(FileSystem::getPath("resources/textures/zemlja.png").c_str());
    ourShader4.use();
    ourShader4.setInt("material.diffuse",0);

    unsigned int diffuseMap5 = loadTexture(FileSystem::getPath("resources/textures/plant1.png").c_str());
    ourShader5.use();
    ourShader5.setInt("material.diffuse",0);

    //----------------------------------------------
    // load models

    // -----------

    Model ourModel("resources/objects/backpack/backpack.obj");
    ourModel.SetShaderTextureNamePrefix("material.");

    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(1.0, 1.0, 1.0);
    pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;


    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;



        // input
        // -----
        processInput(window);


        //pointLight.position = glm::vec3(4.0 * cos(currentFrame), 4.0f, 4.0 * sin(currentFrame));
        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        ourShader1.use();
        ourShader1.setVec3("pointLight.position", pointLight.position);
        ourShader1.setVec3("viewPos", programState->camera.Position);


        ourShader1.setVec3("pointLight.ambient", 0.2f, 0.2f, 0.2f);
        ourShader1.setVec3("pointLight.diffuse", 0.5f, 0.5f, 0.5f);
        ourShader1.setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);

        ourShader1.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        ourShader1.setFloat("material.shininess", 100.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap1);

        glBindVertexArray(VAO);
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model,glm::radians(30.0f),glm::vec3(0.0f,1.0f,0.0f));
        model = glm::scale(model,glm::vec3(30.0,30.0,30.0));
        ourShader1.setMat4("model",model);
        ourShader1.setMat4("projection",projection);
        glm::mat4 view1 = programState->camera.GetViewMatrix();
        ourShader1.setMat4("view",view1);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //----------------------------------------------------------------------------------------------------------------

        ourShader3.use();
        ourShader3.setVec3("pointLight.position", pointLight.position);
        ourShader3.setVec3("viewPos", programState->camera.Position);


        ourShader3.setVec3("pointLight.ambient", 0.2f, 0.2f, 0.2f);
        ourShader3.setVec3("pointLight.diffuse", 0.5f, 0.5f, 0.5f);
        ourShader3.setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);

        ourShader3.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        ourShader3.setFloat("material.shininess", 80.0f);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,diffuseMap3);
        ourShader3.setMat4("model",model);
        ourShader3.setMat4("projection",projection);
        ourShader3.setMat4("view",view1);
        glDrawArrays(GL_TRIANGLES,6,6);

        //----------------------------------------------------------------------------------------------------------------

        ourShader1.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,diffuseMap1);
        glDrawArrays(GL_TRIANGLES,12,6);


        //----------------------------------------------------------------------------------------------------------------

        ourShader2.use();
        ourShader2.setVec3("pointLight.position", pointLight.position);
        ourShader2.setVec3("viewPos", programState->camera.Position);


        ourShader2.setVec3("pointLight.ambient", 0.2f, 0.2f, 0.2f);
        ourShader2.setVec3("pointLight.diffuse", 0.5f, 0.5f, 0.5f);
        ourShader2.setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);

        ourShader2.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        ourShader2.setFloat("material.shininess", 50.0f);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap2);
        ourShader2.setMat4("model",model);
        ourShader2.setMat4("projection",projection);
        ourShader2.setMat4("view",view1);
        glDrawArrays(GL_TRIANGLES,18,6);

        //----------------------------------------------------------------------------------------------------------------

        ourShader4.use();
        ourShader4.setVec3("pointLight.position", pointLight.position);
        ourShader4.setVec3("viewPos", programState->camera.Position);


        ourShader4.setVec3("pointLight.ambient", 0.2f, 0.2f, 0.2f);
        ourShader4.setVec3("pointLight.diffuse", 0.5f, 0.5f, 0.5f);
        ourShader4.setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);

        ourShader4.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        ourShader4.setFloat("material.shininess", 45.0f);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        glBindVertexArray(VAO1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,diffuseMap4);
        glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1,glm::vec3(0.0f,-12.35f,0.0f));
        model1 = glm::rotate(model1,glm::radians(30.0f),glm::vec3(0.0f,1.0f,0.0f));
        model1 = glm::scale(model1,glm::vec3(13.0f,13.0f,13.0f));
        ourShader4.setMat4("model",model1);
        ourShader4.setMat4("view",view1);
        ourShader4.setMat4("projection",projection);
        glDrawArrays(GL_TRIANGLES,0,36);

        glDisable(GL_CULL_FACE);

        ourShader5.use();
        ourShader5.setVec3("pointLight.position", pointLight.position);
        ourShader5.setVec3("viewPos", programState->camera.Position);


        ourShader5.setVec3("pointLight.ambient", 0.2f, 0.2f, 0.2f);
        ourShader5.setVec3("pointLight.diffuse", 0.5f, 0.5f, 0.5f);
        ourShader5.setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);

        ourShader5.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        ourShader5.setFloat("material.shininess", 30.0f);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,diffuseMap5);

        model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1,glm::vec3(1.0f,-6.0f,2.5f));
        model1 = glm::rotate(model1,glm::radians(30.0f),glm::vec3(0.0f,1.0f,0.0f));
        model1 = glm::scale(model1,glm::vec3(13.0f,18.0f,13.0f));
        ourShader5.setMat4("model",model1);
        ourShader5.setMat4("view",view1);
        ourShader5.setMat4("projection",projection);
        glDrawArrays(GL_TRIANGLES,0,6);


        /*
        ourShader.use();
        ourShader.setVec3("pointLight.position", pointLight.position);
        ourShader.setVec3("pointLight.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight.specular", pointLight.specular);
        ourShader.setFloat("pointLight.constant", pointLight.constant);
        ourShader.setFloat("pointLight.linear", pointLight.linear);
        ourShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);
        // view/projection transformations

        glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render the loaded model

        model = glm::mat4(1.0f);
        model = glm::translate(model,
                               programState->backpackPosition); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(programState->backpackScale));    // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);
        */
        if (programState->ImGuiEnabled)
            DrawImGui(programState);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);
        ImGui::DragFloat3("Backpack position", (float*)&programState->backpackPosition);
        ImGui::DragFloat("Backpack scale", &programState->backpackScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


