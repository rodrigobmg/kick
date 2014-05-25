//
//  camera.cpp
//  KickCPP
//
//  Created by morten on 8/12/13.
//  Copyright (c) 2013 Morten Nobel-Joergensen. All rights reserved.
//

#include "kick/scene/camera.h"
#include <iostream>
#include "kick/core/kickgl.h"
#include <glm/ext.hpp>
#include <cmath>
#include <algorithm>
#include "kick/scene/game_object.h"
#include "kick/scene/transform.h"
#include "kick/scene/scene.h"
#include "engine.h"

using namespace std;
using namespace glm;

namespace kick {
    
    Camera::Camera(GameObject *gameObject)
    :Component(gameObject, false, false), clearFlag(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT),
     projectionMatrix{1}{

    }
    
    void Camera::activated(){
        Scene* scene = gameObject->getScene();
        componentListener = scene->componentEvents.createListener([&](pair<Component*, ComponentUpdateStatus> value){
            if (value.second == ComponentUpdateStatus::Added){
                if (includeComponent(value.first)){
                    renderableComponents.push_back(value.first);
                }
            } else if (value.second == ComponentUpdateStatus::Removed){
                auto iter = find(renderableComponents.begin(),renderableComponents.end(), value.first);
                if (iter != renderableComponents.end()){
                    renderableComponents.erase(iter);
                }
            }
        });
        rebuildComponentList();
    }
    
    void Camera::rebuildComponentList(){
        renderableComponents.clear();
        for (auto & gameObject : *gameObject->getScene()) {
            for (auto & component :  *gameObject){
                if (includeComponent(component)){
                    renderableComponents.push_back(component);
                }
            }
        }
    }
    
    bool Camera::includeComponent(Component* component){
        if (!component->isRenderable()){
            return false;
        }
        if (dynamic_cast<Camera*>(component)){
            return false;
        }
        // todo - filter
        return true;
    }
    
    void Camera::deactivated(){
        // remove listener
        componentListener = {};
    }
    
    glm::vec4 Camera::getClearColor(){
        return clearColor;
    }
    
    void Camera::setClearColor(glm::vec4 clearColor){
        this->clearColor = clearColor;
    }
    
    void Camera::setupViewport(vec2 &offset, vec2 &dim){
        offset = round(offset);
        dim = round(dim);
        int offsetX = (int)offset.x;
        int offsetY = (int)offset.y;
        int width = (int)dim.x;
        int height = (int)dim.y;
        glViewport(offsetX, offsetY, width, height);
        glScissor(offsetX, offsetY, width, height);
    }
    
    void Camera::setupCamera(EngineUniforms *engineUniforms) {
        vec2 viewportDimension = (vec2)/*renderTarget? renderTarget.dimension : */ engineUniforms->viewportDimension.getValue();
        vec2 offset = viewportDimension * normalizedViewportOffset;
        vec2 dim = viewportDimension * normalizedViewportDim;
        setupViewport(offset, dim);
        if (clearFlag) {
            if (isClearColor()) {
                glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
            }
            glClear(clearFlag);
        }

        engineUniforms->viewMatrix = gameObject->getTransform()->getGlobalTRSInverse();
        engineUniforms->viewProjectionMatrix = projectionMatrix * engineUniforms->viewMatrix;
        engineUniforms->projectionMatrix = projectionMatrix;
    }
    
    void Camera::render(EngineUniforms *engineUniforms){
        setupCamera(engineUniforms);
        engineUniforms->sceneLights->recomputeLight(engineUniforms->viewMatrix);
        for (auto c : renderableComponents){
            c->render(engineUniforms);
        }
    }
    
    void Camera::setClearColor(bool clear){
        if (clear){
            clearFlag |= GL_COLOR_BUFFER_BIT;
        } else {
            clearFlag &= ~GL_COLOR_BUFFER_BIT;
        }
    }
    
    bool Camera::isClearColor(){
        return clearFlag & GL_COLOR_BUFFER_BIT;
    }
    
    void Camera::setClearDepth(bool clear){
        if (clear){
            clearFlag |= GL_DEPTH_BUFFER_BIT;
        } else {
            clearFlag &= ~GL_DEPTH_BUFFER_BIT;
        }
    }
    
    bool Camera::isClearDepth(){
        return clearFlag & GL_DEPTH_BUFFER_BIT;
    }
    
    void Camera::setClearStencil(bool clear){
        if (clear){
            clearFlag |= GL_STENCIL_BUFFER_BIT;
        } else {
            clearFlag &= ~GL_STENCIL_BUFFER_BIT;
        }
    }
    
    bool Camera::isClearStencil(){
        return clearFlag & GL_STENCIL_BUFFER_BIT;
    }

    glm::mat4 Camera::getProjectionMatrix() {
        return projectionMatrix;
    }

    void Camera::setProjectionMatrix(glm::mat4 projectionMatrix) {
        this->projectionMatrix = projectionMatrix;
    }

    void Camera::resetProjectionMatrix(){
    }
}