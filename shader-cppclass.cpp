// #include <glad/glad.h>
// #include <GLFW/glfw3.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdio.h>

#include <OpenGL/gl3.h> //(My code compiles without this line)
#define GLFW_INCLUDE_GLCOREARB
#include "GLFW/glfw3.h"

// #include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>

#include "AronLib.h"
#include "AronCLibNew.h"
#include "AronOpenGLLib.h"

using namespace std;
using namespace MatrixVector;


void shaderSetMatrix(GLuint shaderHandle, string uniformVarStr, float arr[16]);

class Triangle{
    public:
        float mat[16];
        int numFloat;
        float *pt;
        unsigned int VBO;
        unsigned int VAO;
    public: 
        Triangle(int numFloat, float* arr){
            this -> pt = (float*)malloc(numFloat*sizeof(float));
            this -> numFloat = numFloat;
            for(int i = 0; i < this -> numFloat; i++){
                pt[i] = arr[i];
            }
        }

        void setMatrix(GLuint shaderHandle, string uniformVarStr, float arr[16]){
            shaderSetMatrix(shaderHandle, uniformVarStr, arr); 
        }
        unsigned int setupVBOVAO(){
            glGenVertexArrays(1, &VAO);
            //                |
            //                + -> the number of vertex array object names to generate
            //
            glGenBuffers(1, &VBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*numFloat, pt, GL_STATIC_DRAW);

            //                       + -> specify number of components per generic vertex attribute
            //                       ↓ 
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
            //                                                 ↑ 
            //                                                 + next vertex shift 6 floats
            //
            glEnableVertexAttribArray(0);

            // color attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
            //                                                 ↑
            //                                                 + -> next color shift 6 floats
            //
            glEnableVertexAttribArray(1);
            // END_Draw_triangle
        }
        void draw(GLenum mode){
            // glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
            glBindVertexArray(VAO);
            glDrawArrays(mode, 0, 3);
        }
        ~Triangle(){
            free(pt);
        }
};

GLuint drawSetup(int num, float* triVex){
    unsigned int vertex_array_obj;
    unsigned int vertex_buffer_obj;

    float triVexkk[] = {    
        0.5f, -0.2f, 0.0f,   1.0f, 0.3f, 0.2f, 
        0.7f, -0.2f, 0.0f,  0.3f, 1.0f, 0.5f, 
        0.3f,  0.5f, 0.0f,   0.7f, 0.4f, 1.0f 
            //  R     G     B
    };

    float *vexPt = (float*)malloc(sizeof(float)*num);

    // float * vexPt = (float*)malloc(num*sizeof(float));

    for(int i = 0; i < num; i++){
        vexPt[i] = triVexkk[i];
    }

    printf("sizeof(vexPt)=%d", sizeof(vexPt));
    printf("sizeof(triVexkk)=%d", sizeof(triVexkk));

    const float* cpt = vexPt;

    glGenVertexArrays(1, &vertex_array_obj);
    //                |
    //                + -> the number of vertex array object names to generate
    //
    glGenBuffers(1, &vertex_buffer_obj);

    glBindVertexArray(vertex_array_obj);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_obj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vexPt), vexPt, GL_DYNAMIC_DRAW /* GL_STATIC_DRAW */);

    //                       + -> specify number of components per generic vertex attribute
    //                       ↓ 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    //                                                 ↑ 
    //                                                 + next vertex shift 6 floats
    //
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    //                                                 ↑
    //                                                 + -> next color shift 6 floats
    //
    glEnableVertexAttribArray(1);
    return vertex_array_obj;
}

void drawMe(GLuint vertex_array_obj){
    glBindVertexArray(vertex_array_obj);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
}



/*
Ref: https://learnopengl.com/Getting-started/Shaders
Data: 18-08-2020
KEY: opengl shader, vertex shader fragment shader, shader tutorial, shader uniform
Compile: opengl_compile.sh

UPDATE: Sat 18 Dec 23:33:42 2021 
1. Use raw string in shader string
2. Add translation matrix to translate the curve
3. Fixed bug: exit the program if there is shader error
4. Wrap shader code inside class

*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


std::string vertexShaderStr = R"(
#version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    uniform mat4 mymat;
    out vec3 ourColor;
    void main()
    {
        mat4 translate1;
        translate1 = mat4(1.0, 0.0, 0.0,  0.0,
                          0.0, 1.0, 0.0,  0.0,
                          0.0, 0.0, 1.0,  0.0,  
                          -0.5, -0.5, 0.0, 1.0);

        mat4 tmat;
        tmat = mat4(1.2, 0.0, 0.0, 0.0,
                    0.0, 1.2, 0.0, 0.0,
                    0.0, 0.0, 1.2, 0.0,  
                    0.0, 0.0, 0.0, 1.0);

        gl_Position = tmat*mymat * vec4(aPos, 1.0);
        ourColor = aPos; 

        // gl_Position = tmat * vec4(aPos, 1.0);
    }
)";

std::string fragStr = R"(
#version 330 core
    out vec4 FragColor;
    in  vec3 ourColor;
    // uniform vec4 ourColor;
    void main()
    {
       FragColor = vec4(ourColor, 1.0f);
    }
)";

const char *vertexShaderSource = vertexShaderStr.c_str();

const char *fragmentShaderSource = fragStr.c_str(); 

void fun(float a, int sz, float arr[300]){
    vector<float> vx;
    vector<float> vy;
    float del = (float)1/sz;
    for(int i = 0; i < sz; i++){
        float val = del*i;
        vx.push_back(val);
        vy.push_back(a*val*val);
        arr[i] = a*val*val;
    }

    /**
     *  0 1 2 3 4 5
     *  -3 -2 -1 0 1 2
     */
    vector<float> repv = repeatVec(100, (float)0.0);
    int k = 0;
    for(int i = 0; i < 100; i++){
        arr[k] = vx[i];
        arr[k+1] = vy[i];
        arr[k+2] = repv[i];
        k = k + 3;
    }
}

/**
 *   1/2
 *   1 x 1/2,     2 x 1/2
 *    (1/2)(1/2)    1 x 1
 *  
 *
 *  
 *   |               |             |
 *   offset+0     offset+1     offset+2
 */ 
const int xyzVertexes = 3*3;
void fillArray(float arr[xyzVertexes]){
    int nVert = (int)(xyzVertexes/3); 
    float delta = (float)1/nVert;
    for(int i = 0; i < nVert; i++){
        float val = delta*(i + 1);
        int offset = 3*i ;
        arr[offset+0] = val;
        arr[offset+1] = val*val;
        arr[offset+2] = 0.0f;
    }
}

int shaderSetVec4(GLuint shaderHandle, string uniformVarStr, float vec4[4]){
    glUseProgram(shaderHandle);
    int vec4Id = glGetUniformLocation(shaderHandle, uniformVarStr.c_str());
    printf("vec4Id=%d\n", vec4Id);
    glUniform4f(vec4Id, vec4[0], vec4[1], vec4[2], vec4[3]);
}

void shaderSetMatrix(GLuint shaderHandle, string uniformVarStr, float arr[16]){
    glUseProgram(shaderHandle);
    int matrixId = glGetUniformLocation(shaderHandle, uniformVarStr.c_str());
    if (matrixId != -1){
        glUniformMatrix4fv(matrixId, 1, GL_FALSE, arr);
    }else{
        printf("ERROR: glGetUniformLocation(..)");
        exit(1);
    }
    printf("matrixId=%d\n", matrixId);

}


int main(){
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Shader Triangle", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    /*
       if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
       {
       std::cout << "Failed to initialize GLAD" << std::endl;
       return -1;
       }
       */

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        exit(1);
    }
    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        exit(1);
    }
    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader); // vertexShader - vertex shader code
    glAttachShader(shaderProgram, fragmentShader); // fragmentShader - fragment shader code
    glLinkProgram(shaderProgram);                 // link shader
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        exit(1);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);



    // set up vertex data (and buffer(s)) and configure vertex attributes{{{
    // ------------------------------------------------------------------
    // float vertices[] = {
    // 0.5f, -0.5f, 0.0f,  // bottom right
    // -0.5f, -0.5f, 0.0f,  // bottom left
    // 0.0f,  0.5f, 0.0f,   // top

    // 0.8f, -0.2f, 0.0f,
    // 0.2f,  0.4f, 0.0f,
    // -0.3f,  0.6f, 0.0f
    // };

    /*
       float arr[xyzVertexes];
    // fun(1, 100, arr);
    fillArray(arr);
    printArray(arr, xyzVertexes);

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(arr), arr, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // http://localhost/html/indexUnderstandOpenGL.html
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    //                    |                            |                      |
    //                    |                            +-> 3 float(x,y,z)     +-> vertex offset
    //                    +-> id
    glEnableVertexAttribArray(0);
    //                        |
    //                        +-> id
    //

    glBindVertexArray(VAO);
    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    // glBindVertexArray(0);


    // bind the VAO (it was already bound, but just to demonstrate): seeing as we only have a single VAO we can 
    // just bind it beforehand before rendering the respective triangle; this is another approach.
    // glBindVertexArray(VAO);
    *//*}}}*/


    // BEG_Draw_triangle
    float triVert[] = {  //  R     G     B
        0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, 
        0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f 
    };

    unsigned int VBO2, VAO2;
    //            |      +-> vertex array object
    //            +-> vertex buffer object
    glGenVertexArrays(1, &VAO2);
    //                |
    //                + -> the number of vertex array object names to generate
    //
    glGenBuffers(1, &VBO2);

    glBindVertexArray(VAO2);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triVert), triVert, GL_STATIC_DRAW);

    //                       + -> specify number of components per generic vertex attribute
    //                       ↓ 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    //                                                 ↑ 
    //                                                 + next vertex shift 6 floats
    //
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    //                                                 ↑
    //                                                 + -> next color shift 6 floats
    //
    glEnableVertexAttribArray(1);
    // END_Draw_triangle


    float trixx[] = {    
        0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, 
        0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f 
            //  R     G     B
    };
    Triangle* triPt = new Triangle(18, trixx);


    // BEG_Draw_triangle
    float triVert3[] = {  //  R     G     B
        0.9f,  0.9f, 0.0f,   1.0f, 0.0f, 0.0f, 
        0.6f,  0.0f, 0.3f,   0.0f, 1.0f, 0.0f, 
        0.0f,  0.2f, 0.0f,   0.0f, 0.0f, 1.0f 
    };

    unsigned int VBO3, VAO3;
    //            |      +-> vertex array object
    //            +-> vertex buffer object
    glGenVertexArrays(1, &VAO3);
    //                |
    //                + -> the number of vertex array object names to generate
    //
    glGenBuffers(1, &VBO3);

    glBindVertexArray(VAO3);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triVert3), triVert3, GL_STATIC_DRAW);

    //                       + -> specify number of components per generic vertex attribute
    //                       ↓ 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    //                                                 ↑ 
    //                                                 + next vertex shift 6 floats
    //
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    //                                                 ↑
    //                                                 + -> next color shift 6 floats
    //
    glEnableVertexAttribArray(1);


    float mymat[4][4];
    for(int i = 0; i < 4; i++){
        for(int j =0; j < 4; j++){
            if (i == j){
                mymat[i][j] = 1.5f;
            }
            else{
                mymat[i][j] = 0.0f;
            }
        }
    }
    printArray2df(4, 4, &mymat[0][0]);


    float mat[] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    GLuint matrixId = glGetUniformLocation(shaderProgram, "mymat");
    if (matrixId == -1){
        printf("ERROR: matrixId=%d\n", matrixId);
        exit(1);
    }



    
    unsigned int VBO;
    unsigned int VAO;
    {
        float triVex[] = {    
            0.5f, -0.5f, 0.0f,   1.0f, 0.3f, 0.2f, 
            -0.5f, -0.5f, 0.0f,  0.3f, 1.0f, 0.5f, 
            0.0f,  0.5f, 0.0f,   0.7f, 0.4f, 1.0f 
                //  R     G     B
        };

        glGenVertexArrays(1, &VAO);
        //                |
        //                + -> the number of vertex array object names to generate
        //
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triVex), triVex, GL_STATIC_DRAW);

        //                       + -> specify number of components per generic vertex attribute
        //                       ↓ 
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
        //                                                 ↑ 
        //                                                 + next vertex shift 6 floats
        //
        glEnableVertexAttribArray(0);

        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
        //                                                 ↑
        //                                                 + -> next color shift 6 floats
        //
        glEnableVertexAttribArray(1);
    }

    unsigned int VBO4;
    unsigned int VAO4;
    {
        float triVex[] = {    
            0.1f, -0.2f, 0.0f,   1.0f, 0.3f, 0.2f, 
            0.5f, -0.2f, 0.0f,  0.3f, 1.0f, 0.5f, 
            0.3f,  0.5f, 0.0f,   0.7f, 0.4f, 1.0f 
                //  R     G     B
        };
    }

    float triVex2[] = {    
        0.5f, -0.2f, 0.0f,   1.0f, 0.3f, 0.2f, 
        0.7f, -0.2f, 0.0f,  0.3f, 1.0f, 0.5f, 
        0.3f,  0.5f, 0.0f,   0.7f, 0.4f, 1.0f 
                            //  R     G     B
    };


    int num = 18;
    float * vexPt = (float*)malloc(num*sizeof(float));

    {
        for(int i = 0; i < num; i++){
            vexPt[i] = triVex2[i];
        }
    }

    unsigned int VAO6 = drawSetup(sizeof(triVex2)/sizeof(float), triVex2);

    triPt -> setupVBOVAO();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // be sure to activate the shader before any calls to glUniform
        // glUseProgram(shaderProgram);

        // update shader uniform
        float timeValue = glfwGetTime();
        float greenValue = sin(timeValue) / 2.0f + 0.5f;
        float redValue   = cos(timeValue) / 2.0f + 0.5f;
        // int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
        // // int mat44 = glGetUniformLocation(shaderProgram, "tmat");
        // printf("vertexColorLocation=%d\n", vertexColorLocation);
        // // printf("mat44=%d\n", mat44);
        // glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

        // float fragColor[] = {redValue, greenValue, 0.0f, 1.0f};
        // shaderSetVec4(shaderProgram, "ourColor",  fragColor); 
        // mat[12] = sin(timeValue);
        // shaderSetMatrix(shaderProgram, "mymat", mat);

        // glUseProgram(shaderProgram);
        // glUniformMatrix4fv(matrixId, 1, GL_FALSE, mat);

        // render the triangle
        //

        // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
        // glEnableVertexAttribArray(1);
        // glBindVertexArray(VAO2);


        // int vertexColorLocation3 = glGetUniformLocation(shaderProgram, "ourColor");
        // glUniform4f(vertexColorLocation3, 0.9f, 0.3f, 0.4f, 1.0f);

        // BEG_triangle
        // glBindVertexArray(VAO2);
        // glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
        // END_triangle


        // glBindVertexArray(VAO3);
        // glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

        // glDrawArrays(GL_LINE_STRIP, 0, (int)(xyzVertexes/3));
        // END_line 

        // mat[12] = cos(timeValue);
        triPt -> setMatrix(shaderProgram, "mymat", mat);
        triPt -> draw(GL_TRIANGLE_STRIP);

        // glBindVertexArray(triPt -> VAO);
        // glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
        // drawMe(VAO);
        // drawMe(VAO4);
        // drawMe(VAO6);

        // drawMe(tri_vao);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    // glDeleteVertexArrays(0, &VAO);
    // glDeleteBuffers(0, &VBO);

    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO2);
    glDeleteProgram(shaderProgram);
    delete triPt;

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
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


