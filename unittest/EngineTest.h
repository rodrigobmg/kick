//
//  EngineTest.h
//  KickCPP
//
//  Created by Morten Nobel-Jørgensen on 26/10/13.
//  Copyright (c) 2013 Morten Nobel-Joergensen. All rights reserved.
//

#pragma once


#include "EngineTest.h"
#include "tinytest/tinytest.h"
#include "kick/kick.h"
#include "kick/mesh/mesh_factory.h"
#include "kick/core/kickgl.h"
#include "kick/core/key_input.h"
#include "kick/core/mouse_input.h"

using namespace kick;
using namespace std;
using namespace glm;

Engine *engine;

void initEngine(int& argc, char** argv){
    // Manually create engine instance
    //GLUTContext cont;
    SDL2Context* cont = new SDL2Context{};
    cont->init(argc, argv);
    cont->showWindow();

    cout <<  glGetString(GL_VERSION) << endl;

    ::engine = new Engine (cont);
}

int TestShader() {
    Shader *shader;
    try {
        shader = Project::createAsset<Shader>("foo", "foo");
        shader->apply();
        TINYTEST_ASSERT(false);
    } catch (ShaderBuildException sbe) {
        TINYTEST_ASSERT(sbe.errorType == ShaderErrorType::VertexShader || sbe.errorType == ShaderErrorType::FragmentShader);
    }

    shader = Project::createAsset<Shader>();
    try {
        shader->apply();
        TINYTEST_ASSERT(false);
    } catch (ShaderBuildException sbe) {
        TINYTEST_ASSERT(sbe.errorType == ShaderErrorType::IncompleteShader);
    }

    // mismatch between out and in parameters
    string vertexShader =
            R"(#version 150
            uniform vec2 p;
            in vec4 position;
            void main (void)
            {
                gl_Position = vec4(p, 0.0, 0.0) + position;
            }
            )";


    string fragmentShader =
            R"(#version 150
            in vec4 colorV;
            out vec4 fragColor;
            void main(void)
            {
                fragColor = colorV;
            }
            )";


    try {
        shader = Project::createAsset<Shader>(vertexShader, fragmentShader);
        TINYTEST_ASSERT_MSG(false, "Warn linker error not found.");
    } catch (ShaderBuildException sbe) {
        TINYTEST_ASSERT(sbe.errorType == ShaderErrorType::Linker);
    }

    vertexShader =
            R"(#version 150
            uniform vec2 p;
            in vec4 position;
            in vec4 color;
            out vec4 colorV;
            void main (void)
            {
                colorV = color;
                gl_Position = vec4(p.xyz, 0.0) + position;
            }
            )";


    fragmentShader =
            R"(#version 150
            in vec4 colorV;
            out vec4 fragColor;
            void main(void)
            {
                fragColor = colorV;
            }
            )";


    try {
        shader = Project::createAsset<Shader>(vertexShader, fragmentShader);
        TINYTEST_ASSERT_MSG(false, "Vertex Shader Compiler error not found.");
    } catch (ShaderBuildException sbe) {
        TINYTEST_ASSERT(sbe.errorType == ShaderErrorType::VertexShader);
    }

    vertexShader =
            R"(#version 150
            uniform vec2 p;
            in vec4 position;
            in vec4 color;
            out vec4 colorV;
            void main (void)
            {
                colorV = color;
                gl_Position = vec4(p, 0.0, 0.0) + position;
            }
            )";


    fragmentShader =
            R"(#version 150
            in vec4 colorV;
            out vec4 fragColor;
            void main(void)
            {
                fragColor = colorV;
            }
            )";

    shader = Project::createAsset<Shader>(vertexShader, fragmentShader);
    shader->apply();

    auto shaderUniforms = shader->getShaderUniforms();
    TINYTEST_ASSERT(shaderUniforms.size()==1);
    TINYTEST_ASSERT(shaderUniforms[0].name.compare("p") == 0);
    TINYTEST_ASSERT(shaderUniforms[0].index == 0);
    TINYTEST_ASSERT(shaderUniforms[0].size == 1);
    TINYTEST_ASSERT(shaderUniforms[0].type == GL_FLOAT_VEC2);

    auto shaderAttributes = shader->getShaderAttributes();
    TINYTEST_ASSERT(shaderAttributes.size() == 2);
    for (auto shaderAttribute:shaderAttributes){
        TINYTEST_ASSERT(shaderAttribute.name.compare("position") == 0 || shaderAttribute.name.compare("color") == 0);
        TINYTEST_ASSERT(shaderAttribute.index < 2);
        TINYTEST_ASSERT(shaderAttribute.size == 1);
        TINYTEST_ASSERT(shaderAttribute.type == GL_FLOAT_VEC4);
    }

    return 1;
}

int TestMaterial() {
    std::string vertexShader =
            R"(#version 150
            uniform vec4 p;
            in vec4 position;
            in vec4 color;
            out vec4 colorV;
            void main (void)
            {
                colorV = color;
                gl_Position = p + position;
            }
            )";


    std::string fragmentShader =
            R"(#version 150
            in vec4 colorV;
            out vec4 fragColor;
            void main(void)
            {
                fragColor = colorV;
            }
            )";
    Shader *shader = Project::createAsset<Shader>(vertexShader, fragmentShader);
    try{
        shader->apply();
        Material* material = Project::createAsset<Material>();
        material->setShader(shader);
        material->setUniform("p", glm::vec4{0,1,0,1});
    } catch (ShaderBuildException sbe){
        cerr << sbe.errorMessage << endl;
        throw sbe;
    }

    return 1;
}

int TestMeshData(){
    MeshData* meshData = Project::createAsset<MeshData>();

    const vec3 p0{0,0,0};
    const vec3 p1{1,0,0};
    const vec3 p2{0.5f,0,sqrt(0.75f)};
    const vec3 p3{0.5f,sqrt(0.75f),sqrt(0.75f)/3};

    meshData->setPosition({
            p0,p1,p2,
            p0,p2,p3,
            p2,p1,p3,
            p0,p3,p1
    });

    meshData->setSubmesh(0, {
            0,1,2,
            3,4,5,
            6,7,8,
            9,10,11
    }, MeshType::Triangles);

    AABB aabb = meshData->getAabb();
    TINYTEST_ASSERT(aabb.min == p0);
    vec3 vec3max{1,sqrt(0.75f),sqrt(0.75f)};
    TINYTEST_ASSERT(aabb.max == vec3max);

    meshData->recalculate_normals();
    TINYTEST_ASSERT(meshData->getNormal().size() == meshData->getPosition().size());
    for (auto normal : meshData->getNormal()){
        TINYTEST_ASSERT(normal != vec3(0));
    }

    auto interleavedData = meshData->getInterleavedData();
    auto interleavedFormat = meshData->getInterleavedFormat();
    TINYTEST_ASSERT(interleavedFormat.size() == 2);

    //if (interleavedFormat[0].)


    return 1;
}



int TestMesh() {
    //Mesh mesh;
    return 1;
}

int TestMeshFactory(){
    MeshFactory::createPointData();
    MeshFactory::createTriangleData();
    MeshFactory::createDiscData();
    MeshFactory::createPlaneData();
    MeshFactory::createUVSphereData();
    MeshFactory::createCubeData();

    return 1;
}

int TestLoadTextFile(){
    string test = Project::loadTextResource("unittest/testasset/txt.txt");
    TINYTEST_ASSERT(test == "hello world");
    return 1;
}

int TestLoadBinaryFile(){
    vector<char> res = Project::loadBinaryResource("unittest/testasset/txt.txt");
    string helloWorld = "hello world";
    TINYTEST_ASSERT(10 == res[res.size()-1]);
    for (int i=0;i<res.size()-1;i++){
        TINYTEST_ASSERT(helloWorld[i] == res[i]);
    }
    return 1;
}

int TestLoadTexture(){
    auto img = Project::loadTexture("assets/images/logo.png");

    return 1;
}

int TestLoadTextureTypes(){
    GLenum error = glGetError();
    TINYTEST_ASSERT_MSG(error == GL_NO_ERROR, "Init");
    string imgPaths[6] = {
            "unittest/images/pnggrad8rgb.png",
            "unittest/images/pnggrad16rgb.png",
            "unittest/images/pngtest8rgba.png",
            "unittest/images/pngtest16rgba.png",
            "unittest/images/load_0_.png",
            "unittest/images/load_0.png"
    };
    int expectedTypes[6] = {GL_RGB,GL_RGB,GL_RGBA,GL_RGBA};
    for (int i=0;i<6;i++){
        Texture2D *img = Project::loadTexture(imgPaths[i]);
        error = glGetError();
        TINYTEST_ASSERT_MSG(error == GL_NO_ERROR, imgPaths[i].c_str());
    }

    return 1;

}

int TestDefaultShaders(){
    vector<string> shaders {
            "diffuse",
            "skybox",
            "specular",
            "transparent_diffuse",
            "transparent_point_sprite",
            "transparent_specular",
            "transparent_unlit",
            "unlit",
            "unlit_vertex_color",
            "__error",
            "__pick",
            "__pick_normal",
            "__pick_uv",
            "__shadowmap",
    };

    int errors = 0;
    for (auto & s : shaders){
        string vertexShaderURI = string{"assets/shaders/"}+s+"_vs.glsl";
        string fragmentShaderURI = string{"assets/shaders/"}+s+"_fs.glsl";
        string vertexShader = Project::loadTextResource(vertexShaderURI);
        string fragmentShader = Project::loadTextResource(fragmentShaderURI);

        Shader* shader = Project::createAsset<Shader>(vertexShader, fragmentShader);
        try{
            shader->apply();
        }
        catch (std::invalid_argument ia){
            cerr << endl<<"Shader:"<<s << endl << ia.what() << endl;
            errors++;
        }
        catch (ShaderBuildException sbe){
            cerr << endl<<"Shader:"<<s << endl << sbe.errorMessage << endl;
            cerr << "Error snippet:"<< endl << sbe.codeSnippet << endl;
            errors++;
        }
        GLenum error = glGetError();
        TINYTEST_ASSERT_MSG(error == GL_NO_ERROR, s.c_str());
    }
    TINYTEST_ASSERT(errors == 0);
    return 1;
}

int TestShaderPreCompiler(){
    string removeCommentedOutPragmas = R"(//#pragma include "light.glsl"
test
//#pragma include "light2.glsl"
#pragma include "assets/shaders/light.glsl")";
    string output = Shader::getPrecompiledSource(removeCommentedOutPragmas);

    TINYTEST_ASSERT(output.find("//#pragma") == -1);
    return 1;
}

int TestShaderLoading(){
    vector<string> shaders {
            "diffuse",
            "skybox",
            "specular",
            "transparent_diffuse",
            "transparent_point_sprite",
            "transparent_specular",
            "transparent_unlit",
            "unlit",
            "unlit_vertex_color",
            "__error",
            "__pick",
            "__pick_normal",
            "__pick_uv",
            "__shadowmap",
    };
    for (auto & s : shaders){
        string shaderURI = string{"assets/shaders/"}+s+".shader";
        auto shader = Project::loadShader(shaderURI);
        TINYTEST_ASSERT(shader->getShaderSource(kick::ShaderType::VertexShader).length()>0);
        TINYTEST_ASSERT(shader->getShaderSource(kick::ShaderType::FragmentShader).length()>0);
        bool isTransparent = s.substr(0,5) == "trans";
        TINYTEST_ASSERT(isTransparent == shader->getBlend());
    }
    return 1;
}

int TestKeyInputButton(){
    KeyInput keyInput;
    TINYTEST_EQUAL(false, keyInput.pressed(Key::x));
    TINYTEST_EQUAL(false, keyInput.down(Key::x));
    TINYTEST_EQUAL(false, keyInput.up(Key::x));

    keyInput.pressBegin(Key::x);
    TINYTEST_EQUAL(true, keyInput.pressed(Key::x));
    TINYTEST_EQUAL(true, keyInput.down(Key::x));
    TINYTEST_EQUAL(false, keyInput.up(Key::x));

    keyInput.reset();
    TINYTEST_EQUAL(true, keyInput.pressed(Key::x));
    TINYTEST_EQUAL(false, keyInput.down(Key::x));
    TINYTEST_EQUAL(false, keyInput.up(Key::x));

    keyInput.pressEnd(Key::x);
    TINYTEST_EQUAL(false, keyInput.pressed(Key::x));
    TINYTEST_EQUAL(false, keyInput.down(Key::x));
    TINYTEST_EQUAL(true, keyInput.up(Key::x));

    keyInput.reset();
    TINYTEST_EQUAL(false, keyInput.pressed(Key::x));
    TINYTEST_EQUAL(false, keyInput.down(Key::x));
    TINYTEST_EQUAL(false, keyInput.up(Key::x));

    return 1;
}

int TestKeyMapping(){
    KeyMapping keyMappingX{Key::x};

    KeyInput keyInput;
    TINYTEST_EQUAL(false, keyInput.pressed(keyMappingX));
    TINYTEST_EQUAL(false, keyInput.down(keyMappingX));

    keyInput.pressBegin(Key::x);
    TINYTEST_EQUAL(true, keyInput.pressed(keyMappingX));
    TINYTEST_EQUAL(true, keyInput.down(keyMappingX));

    keyInput.reset();
    TINYTEST_EQUAL(true, keyInput.pressed(keyMappingX));
    TINYTEST_EQUAL(false, keyInput.down(keyMappingX));

    keyInput.pressEnd(Key::x);
    TINYTEST_EQUAL(false, keyInput.pressed(keyMappingX));
    TINYTEST_EQUAL(false, keyInput.down(keyMappingX));

    keyInput.reset();
    TINYTEST_EQUAL(false, keyInput.pressed(keyMappingX));
    TINYTEST_EQUAL(false, keyInput.down(keyMappingX));

    return 1;
}

int TestKeyMappingMulti(){
    KeyMapping keyMappingXY{Key::x, Key::y};

    KeyInput keyInput;
    TINYTEST_EQUAL(false, keyInput.pressed(keyMappingXY));
    TINYTEST_EQUAL(false, keyInput.down(keyMappingXY));

    keyInput.pressBegin(Key::x);
    TINYTEST_EQUAL(false, keyInput.pressed(keyMappingXY));
    TINYTEST_EQUAL(false, keyInput.down(keyMappingXY));

    keyInput.reset();
    TINYTEST_EQUAL(false, keyInput.pressed(keyMappingXY));
    TINYTEST_EQUAL(false, keyInput.down(keyMappingXY));

    keyInput.reset();

    keyInput.pressBegin(Key::y);
    TINYTEST_EQUAL(true, keyInput.pressed(keyMappingXY));
    TINYTEST_EQUAL(true, keyInput.down(keyMappingXY));

    keyInput.reset();
    TINYTEST_EQUAL(true, keyInput.pressed(keyMappingXY));
    TINYTEST_EQUAL(false, keyInput.down(keyMappingXY));

    keyInput.pressEnd(Key::x);
    TINYTEST_EQUAL(false, keyInput.pressed(keyMappingXY));
    TINYTEST_EQUAL(false, keyInput.down(keyMappingXY));

    keyInput.reset();
    TINYTEST_EQUAL(false, keyInput.pressed(keyMappingXY));
    TINYTEST_EQUAL(false, keyInput.down(keyMappingXY));

    return 1;
}


int TestMouseInputButton(){
    MouseInput mouseInput;
    TINYTEST_EQUAL(false, mouseInput.pressed(0));
    TINYTEST_EQUAL(false, mouseInput.down(1));
    TINYTEST_EQUAL(false, mouseInput.up(2));
    TINYTEST_EQUAL(false, mouseInput.up(3));
    TINYTEST_EQUAL(false, mouseInput.up(4));

    mouseInput.buttonPressStarted(0);
    TINYTEST_EQUAL(true, mouseInput.pressed(0));
    TINYTEST_EQUAL(true, mouseInput.down(0));
    TINYTEST_EQUAL(false, mouseInput.up(0));

    mouseInput.reset();
    TINYTEST_EQUAL(true, mouseInput.pressed(0));
    TINYTEST_EQUAL(false, mouseInput.down(0));
    TINYTEST_EQUAL(false, mouseInput.up(0));

    mouseInput.buttonPressEnded(0);
    TINYTEST_EQUAL(false, mouseInput.pressed(0));
    TINYTEST_EQUAL(false, mouseInput.down(0));
    TINYTEST_EQUAL(true, mouseInput.up(0));

    mouseInput.reset();
    TINYTEST_EQUAL(false, mouseInput.pressed(0));
    TINYTEST_EQUAL(false, mouseInput.down(0));
    TINYTEST_EQUAL(false, mouseInput.up(0));

    return 1;
}
