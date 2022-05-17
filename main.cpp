// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/transform2.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/text2D.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>

// Frame counter
bool isPaused = false;
glm::mat4 Projection;
glm::mat4 View;
GLuint MatrixID;
GLuint vertexbuffer;
GLuint colorbuffer;

bool play = false;

glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 0.0f));

float x_position = 0.0f;
float y_position = 0.0f;
float z_position = 0.0f;
int frameCount = 0;
int score = 0;

std::vector <glm::vec3> position_array;
std::vector <float> size_array;

GLuint programID;
GLuint Texture;
GLuint TextureID;
GLuint vertexbuffer_ast;
GLuint uvbuffer_ast;
std::vector<glm::vec3> vertices_ast;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    
  // pause
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        isPaused = !isPaused;
    if (key == GLFW_KEY_W && !isPaused && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        position_array[0].z += 1.0f;
        ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(position_array[0].x, position_array[0].y, position_array[0].z));
    } else if (key == GLFW_KEY_S && !isPaused && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        position_array[0].z -= 1.0f;
        ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(position_array[0].x, position_array[0].y, position_array[0].z));
    } else if (key == GLFW_KEY_A && !isPaused && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        position_array[0].x -= 1.0f;
        ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(position_array[0].x, position_array[0].y, position_array[0].z));
    } else if (key == GLFW_KEY_D && !isPaused && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        position_array[0].x += 1.0f;
        ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(position_array[0].x, position_array[0].y, position_array[0].z));
    } else if (key == GLFW_KEY_UP && !isPaused && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        position_array[0].y += 1.0f;
        ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(position_array[0].x, position_array[0].y, position_array[0].z));
    } else if (key == GLFW_KEY_DOWN && !isPaused && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        position_array[0].y -= 1.0f;
        ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(position_array[0].x, position_array[0].y, position_array[0].z));
    }
    if (key == GLFW_KEY_ENTER) {
        play = true;
    }
    if (key == GLFW_KEY_R) {
        score = 0;
        size_array[0] = .5f;
        position_array[0] = glm::vec3(0, 0, 0);
    }
}

bool isCollision(float x, float y, float z) {
    float displace = 0.5f;
    
    if ((position_array[0].x - x) < displace && (position_array[0].x - x) > displace * -1) {
        return true;
    }
    if ((position_array[0].y - y) < displace && (position_array[0].y - y) > displace * -1) {
        return true;
    }
    if ((position_array[0].z - z) < displace && (position_array[0].z - z) > displace * -1) {
        return true;
    }
    
    return false;
}

void draw(glm::mat4 M, int i) {
    // Use our shader
    glUseProgram(programID);
    
    

    // Compute the MVP matrix from keyboard and mouse input
    computeMatricesFromInputs();
    glm::mat4 ProjectionMatrix = getProjectionMatrix();
    glm::mat4 ViewMatrix = glm::lookAt(glm::vec3(30,30,30),   // Camera location in World Space
                                   glm::vec3(0,0,0),                 // and looks at the origin
                                   glm::vec3(0,1,0)                  // Head is up (set to 0,-1,0 to look upside-down)
                                   );
    //glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * M;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // Bind our texture in Texture Unit
    glActiveTexture(GL_TEXTURE0);
    
    if (i == 0) {
        glBindTexture(GL_TEXTURE_2D, GL_RED);
    } else {
        glBindTexture(GL_TEXTURE_2D, Texture);
    }
    // Set our "myTextureSampler" sampler to use Texture Unit 0
    glUniform1i(TextureID, 0);
    
    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_ast);
    glVertexAttribPointer(
        0,                  // attribute
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_ast);
    glVertexAttribPointer(
        1,                                // attribute
        2,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );
    
    // Draw the asteroid!
    glDrawArrays(GL_TRIANGLES, 0, vertices_ast.size() );

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

int main( void )
{
    
        position_array.push_back(glm::vec3(0,0,0));
        size_array.push_back(0.5f);
        
        position_array.push_back(glm::vec3(5, 5, 5));
        position_array.push_back(glm::vec3(3, 14, 14));
        position_array.push_back(glm::vec3(10, 15, 7));
        position_array.push_back(glm::vec3(14, 9, 3));
        size_array.push_back(1.0f);
        size_array.push_back(1.0f);
        size_array.push_back(1.0f);
        size_array.push_back(1.0f);
        
        // Initialise GLFW
        if( !glfwInit() )
        {
            fprintf( stderr, "Failed to initialize GLFW\n" );
            getchar();
            return -1;
        }

        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Open a window and create its OpenGL context
        window = glfwCreateWindow( 1024, 768, "Homework 5", NULL, NULL);
        if( window == NULL ){
            fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
            getchar();
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);

        // Initialize GLEW
        glewExperimental = true; // Needed for core profile
        if (glewInit() != GLEW_OK) {
            fprintf(stderr, "Failed to initialize GLEW\n");
            getchar();
            glfwTerminate();
            return -1;
        }

        // Ensure we can capture the escape key being pressed below
        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
        // Hide the mouse and enable unlimited mouvement
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        
        // Set the mouse at the center of the screen
        glfwPollEvents();

        // Dark blue background
        glClearColor(0.0f, 0.0f, 0.1f, 0.0f);

        // Enable depth test
        glEnable(GL_DEPTH_TEST);
        // Accept fragment if it closer to the camera than the former one
        glDepthFunc(GL_LESS);

        // Cull triangles which normal is not towards the camera
        glEnable(GL_CULL_FACE);
        
        static const GLfloat g_vertex_buffer_data[] = {
          -1.0f, -1.0f, 0.0f,
          1.0f, -1.0f, 0.0f,
          0.0f,  1.0f, 0.0f,
        };
        static const GLushort g_element_buffer_data[] = { 0, 1, 2 };
        
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

        GLuint VertexArrayID;
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);

        // Create and compile our GLSL program from the shaders
        programID = LoadShaders("TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader" );

        // Get a handle for our "MVP" uniform
        GLuint MatrixID = glGetUniformLocation(programID, "MVP");

        // Load the texture
        // TEXTURE FOR ASTEROID
        Texture = loadDDS("TexturesCom_RockSmooth0188_1_seamless_S.dds");
        //std::vector <std::string> texvarname;
        //texvarname.push_back("monster-base");
        
        // Get a handle for our "myTextureSampler" uniform
        TextureID  = glGetUniformLocation(programID, "myTextureSampler");

        // Read our .obj file
        std::vector<glm::vec2> uvs_ast;
        std::vector<glm::vec3> normals_ast; // Won't be used at the moment.
        bool ast = loadOBJ("asteroid.obj", vertices_ast, uvs_ast, normals_ast);
        
        // register all callbacks
        glfwSetKeyCallback(window, key_callback);

        // Load it into a VBO
        glGenBuffers(1, &vertexbuffer_ast);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_ast);
        glBufferData(GL_ARRAY_BUFFER, vertices_ast.size() * sizeof(glm::vec3), &vertices_ast[0], GL_STATIC_DRAW);

        glGenBuffers(1, &uvbuffer_ast);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_ast);
        glBufferData(GL_ARRAY_BUFFER, uvs_ast.size() * sizeof(glm::vec2), &uvs_ast[0], GL_STATIC_DRAW);
        
        initText2D( "Holstein.DDS" );
        char text[256];

        do{
            
            // Clear the screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            if (play) {
            
                for (int i = 1; i < position_array.size(); i++) {
                    if ((frameCount % 5 == 0) && (!isPaused)) {
                        float delta1 = position_array[i].x + (.1f - rand() % 3 / 10);
                        float delta2 = position_array[i].y + (.1f - rand() % 3 / 10);
                        float delta3 = position_array[i].z + (.1f - rand() % 3 / 10);
                        
                        if (delta1 > 15) {
                            delta1 = 15;
                        }
                        if (delta1 < -15) {
                            delta1 = -15;
                        }
                        if (delta2 > 15) {
                            delta2 = 15;
                        }
                        if (delta2 < -15) {
                            delta2 = -15;
                        }
                        if (delta3 > 15) {
                            delta3 = 15;
                        }
                        if (delta3 < -15) {
                            delta3 = -15;
                        }
                        
                        position_array[i].x = delta1;
                        position_array[i].y = delta2;
                        position_array[i].z = delta3;
                    }
                }
                
                for (int i = 0; i < position_array.size(); i++) {
                    glm::mat4 ScaleMatrix;
                    glm::mat4 TranslationMatrix;
                    
                    ScaleMatrix = glm::scale(glm::vec3(size_array[i], size_array[i], size_array[i]));
                    TranslationMatrix = glm::translate(glm::mat4(1.0), glm::vec3(position_array[i].x, position_array[i].y, position_array[i].z));
                    
                    if (i != 0 && isCollision(position_array[i].x, position_array[i].y, position_array[i].z)) {
                        position_array[i].x = 15 - rand() % 30;
                        position_array[i].y = 15 - rand() % 30;
                        position_array[i].z = 15 - rand() % 30;
                        
                        size_array[0] = size_array[0] + 0.02f;
                        score++;
                        
                        TranslationMatrix = glm::translate(glm::mat4(1.0), glm::vec3(position_array[i].x, position_array[i].y, position_array[i].z));
                    }
                    
                    if (i == 0) {
                        TranslationMatrix = glm::translate(glm::mat4(1.0), glm::vec3(position_array[i].x, position_array[i].y, position_array[i].z));
                    }
                    draw(TranslationMatrix * ScaleMatrix, i);
                }

                sprintf(text, "Score: %d", score);
                printText2D(text, 10, 30, 60);
                sprintf(text, "Press 'p' to pause/unpause.");
                printText2D(text, 10, 500, 15);
                sprintf(text, "Press 'r' to restart the game.");
                printText2D(text, 10, 485, 15);
            } else {
                sprintf(text, "Asteroid");
                printText2D(text, 150, 500, 60);
                sprintf(text, "Collectors");
                printText2D(text, 100, 400, 60);
                sprintf(text, "Press 'enter' to start!");
                printText2D(text, 50, 200, 30);
            }
        
            // Swap buffers
            glfwSwapBuffers(window);
            glfwPollEvents();
            frameCount++;

        } // Check if the ESC key was pressed or the window was closed
        while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
               glfwWindowShouldClose(window) == 0 );

        // Cleanup VBO and shader
        glDeleteBuffers(1, &vertexbuffer_ast);
        glDeleteBuffers(1, &uvbuffer_ast);
        glDeleteProgram(programID);
        glDeleteTextures(1, &Texture);
        glDeleteVertexArrays(1, &VertexArrayID);
        
        cleanupText2D();

        // Close OpenGL window and terminate GLFW
        glfwTerminate();

        return 0;
    }
