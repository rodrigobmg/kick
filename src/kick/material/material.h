//
//  material.h
//  KickCPP
//
//  Created by morten on 8/25/13.
//  Copyright (c) 2013 Morten Nobel-Joergensen. All rights reserved.
//

#pragma once

#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include <map>
#include "kick/core/project_asset.h"
#include "kick/core/event_listener.h"
#include "kick/core/kickgl.h"
#include "kick/texture/texture2d.h"

namespace kick {
    
    class Shader;
    
    struct MaterialData {
        MaterialData(int intValue):value{intValue}, glType{GL_INT} {}
        MaterialData(float floatValue):value{floatValue}, glType{GL_FLOAT}{}
        MaterialData(glm::vec4 vec4Value):value{vec4Value}, glType{GL_FLOAT_VEC4}{}
        MaterialData(glm::mat3 mat3Value):value{mat3Value}, glType{GL_FLOAT_MAT3}{}
        MaterialData(glm::mat4 mat4Value):value{mat4Value}, glType{GL_FLOAT_MAT4}{}
        MaterialData(Texture2D *textureValue):value{textureValue}, glType{GL_SAMPLER_2D}{}
        
        MaterialData(const MaterialData & val);
        
        int shaderLocation;
        int glType;
        union ValueType{
            // union constructors
            ValueType(int intValue):intValue{intValue}{}
            ValueType(float floatValue):floatValue{floatValue}{}
            ValueType(glm::vec4 vec4Value):vec4Value{vec4Value}{}
            ValueType(glm::mat3 mat3Value):mat3Value{mat3Value}{}
            ValueType(glm::mat4 mat4Value):mat4Value{mat4Value}{}
            ValueType(Texture2D* texture2D):texture2D{texture2D}{}
            int intValue;
            float floatValue;
            glm::vec4 vec4Value;
            glm::mat3 mat3Value;
            glm::mat4 mat4Value;
            Texture2D* texture2D;
        } value;
    };
    
    class Material : public ProjectAsset {
    public:
        explicit Material(Project *p, Shader* shader = nullptr);
        ~Material();
        void setShader(Shader *shader);
        Shader* getShader();
        void shaderProgramChanged(Shader *);
        
        template <class E>
        void setUniform(std::string name, E value);
        int bind();
    private:
        void updateShaderLocation(std::string name, MaterialData& value);
        void setUniformData(std::string name, MaterialData&& value);
        // current data (may misfit with current shader)
        std::map<std::string, MaterialData> currentMaterialData;
        Shader* shader = nullptr;
        EventListener<Shader*> shaderChangedListener;
    };
    
    std::string to_string(MaterialData & data);
    
    template <class E>
    void Material::setUniform(std::string name, E value){
        MaterialData data{value};
        setUniformData(name, std::move(data));
    }
}