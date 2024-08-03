#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#define STB_IMAGE_IMPLEMENTATION 
#include <learnopengl/stb_image.h>

#include <iostream>
#include <vector>
#include <random>



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
bool cubesStopped = false;
bool controlesVisible = true;
bool gameOverVisible = false;



bool CheckCollision(glm::vec3 objectPos, glm::vec3 objectScale, glm::vec3 obstaclePos, glm::vec3 obstacleScale) {
    bool collisionX = objectPos.x + objectScale.x >= obstaclePos.x &&
        obstaclePos.x + obstacleScale.x >= objectPos.x;
    bool collisionY = objectPos.y + objectScale.y >= obstaclePos.y &&
        obstaclePos.y + obstacleScale.y >= objectPos.y;
    bool collisionZ = objectPos.z + objectScale.z >= obstaclePos.z &&
        obstaclePos.z + obstacleScale.z >= objectPos.z;
    return collisionX && collisionY && collisionZ;
}


unsigned int loadTexture(const char* path);
// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// camera
Camera camera(glm::vec3(0.0f, 3.7f, -11.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// movement control
bool movingForward = false;
bool cameraStopped = false;

// Obstacle structure
struct Obstacle {
    glm::vec3 position;
    glm::vec3 scale;
};

const float LEFT_LIMIT = -27.0f;
const float RIGHT_LIMIT = 13.0f;
// List of obstacles
std::vector<Obstacle> obstacles;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// Function to initialize obstacles
void InitializeObstacles() {
    obstacles.clear(); // Limpiar la lista de obstáculos antes de reinicializarla
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(LEFT_LIMIT, RIGHT_LIMIT);

    for (int i = 0; i < 10; ++i) {
        Obstacle obstacle;
        obstacle.position = glm::vec3(distribution(generator), 1.5f, -float(i * 20));
        obstacle.scale = glm::vec3(3.0f, 3.0f, 3.0f);
        obstacles.push_back(obstacle);
    }
}


// Function to update obstacles
void UpdateObstacles(float deltaTime, glm::vec3 carPosition) {
    if (!movingForward || cubesStopped) {
        return;
    }

    for (auto& obstacle : obstacles) {
        obstacle.position.z += deltaTime * 5.0f; // Mover obstáculos hacia adelante

        if (obstacle.position.z > carPosition.z + 50.0f) {
            obstacle.position.z = carPosition.z - 50.0f;
            obstacle.position.x = LEFT_LIMIT + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (RIGHT_LIMIT - LEFT_LIMIT)));
        }
    }
}


int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Get the monitor resolution
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int SCR_WIDTH = mode->width;
    int SCR_HEIGHT = mode->height;

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Obstacle Avoidance", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    Shader ourShader("shaders/shader_exercise16_mloading.vs", "shaders/shader_exercise16_mloading.fs");

    // load models
    Model objetoModel("C:/Users/USER/source/repos/OpenGL/model/carro/carro.obj");
    Model pistaModel("C:/Users/USER/source/repos/OpenGL/model/pista/pista.obj");
    Model llegadaModel("C:/Users/USER/source/repos/OpenGL/model/llegada/llegada.obj");
    Model textoModel("C:/Users/USER/source/repos/OpenGL/model/textojuego/textojuego.obj");
    Model controlesModel("C:/Users/USER/source/repos/OpenGL/model/controles/controles.obj");
    Model moonModel("C:/Users/USER/source/repos/OpenGL/model/moon/moon.obj");
    Model gameoverModel("C:/Users/USER/source/repos/OpenGL/model/gameover/gameover.obj");


    // Initialize obstacles
    InitializeObstacles();

    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    // Setup cube VAO
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    // Bind and set the vertex buffer data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Bind the vertex array object
    glBindVertexArray(cubeVAO);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Load and bind the texture
    unsigned int diffuseMap = loadTexture("textures/container2.png");

    ourShader.use();
    ourShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    ourShader.setVec3("lightPos", lightPos);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        if (movingForward && !cameraStopped)
        {
            camera.MovementSpeed += 0.5f * deltaTime;
            camera.ProcessKeyboard(FORWARD, deltaTime);

            if (camera.Position.z <= -7075.0f && camera.Position.x <= 10000.0f)
            {
                cameraStopped = true;
                camera.MovementSpeed = 0.0f;
            }
        }

        ourShader.setVec3("viewPos", camera.Position);
        // Update obstacles
        UpdateObstacles(deltaTime, camera.Position);

        // Collision detection
        glm::vec3 carPosition = camera.Position + glm::vec3(0.0f, -3.0f, -12.0f);
        glm::vec3 carScale = glm::vec3(1.0f, 1.0f, 1.0f); // Ajustar según el tamaño del modelo del carro
        for (const auto& obstacle : obstacles) {
            if (CheckCollision(carPosition, carScale, obstacle.position, obstacle.scale)) {
                cameraStopped = true;
                camera.MovementSpeed = 0.0f;
                cubesStopped = true; // Detener los cubos
                gameOverVisible = true; // Mostrar el modelo "gameover.obj"
                break;
            }
        }


        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // Render the car model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, carPosition);
        ourShader.setMat4("model", model);
        objetoModel.Draw(ourShader);

        // Render the pista model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, -3475.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        ourShader.setMat4("model", model);
        pistaModel.Draw(ourShader);

        // Render the moon model
        model = glm::mat4(1.0f);
        // Ajusta la posición de la luna para que se mueva hacia atrás en relación con la cámara
        glm::vec3 moonPosition = glm::vec3(camera.Position.x, 10.0f, camera.Position.z - 200.0f);
        model = glm::translate(model, moonPosition);
        model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f)); // Ajusta el tamaño según sea necesario
        ourShader.setMat4("model", model);
        moonModel.Draw(ourShader);


        // Render the llegada model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, -7075.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        ourShader.setMat4("model", model);
        llegadaModel.Draw(ourShader);

        // Render the controles model
        if (controlesVisible) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-2.0f, -1.0f, -55.0f));
            model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
            ourShader.setMat4("model", model);
            controlesModel.Draw(ourShader);
        }

        //Render gameover panel
        if (gameOverVisible) {
            model = glm::mat4(1.0f);
            glm::vec3 gameOverPosition = glm::vec3((camera.Position.x), -5.0f, camera.Position.z - 35.0f); // Calcula la posición Z
            model = glm::translate(model, gameOverPosition);
            model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
            ourShader.setMat4("model", model);
            gameoverModel.Draw(ourShader);
        }



        // En tu bucle de renderizado
        ourShader.use();
        ourShader.setInt("texture_diffuse1", 0);

        // Vincula la textura
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);

        // Dibujar los cubos
        glBindVertexArray(cubeVAO);
        for (const auto& obstacle : obstacles) {
            glm::mat4 modelObstacle = glm::mat4(1.0f);
            modelObstacle = glm::translate(modelObstacle, obstacle.position);
            modelObstacle = glm::scale(modelObstacle, obstacle.scale);
            ourShader.setMat4("model", modelObstacle);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        movingForward = true;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        controlesVisible = false; // Desaparecer el modelo controles

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        glm::vec3 position = camera.Position;
        if (position.x > LEFT_LIMIT)
            camera.ProcessKeyboard(LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        glm::vec3 position = camera.Position;
        if (position.x < RIGHT_LIMIT)
            camera.ProcessKeyboard(RIGHT, deltaTime);
    }

    // Reiniciar el juego con la tecla R
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        cameraStopped = false;
        cubesStopped = false;
        movingForward = false;
        camera.MovementSpeed = 2.5f;
        camera.Position = glm::vec3(0.0f, 3.7f, -11.0f);
        gameOverVisible = false; // Ocultar el modelo "gameover.obj"

        InitializeObstacles(); // Reinicializar obstáculos
    }

}





void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
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
