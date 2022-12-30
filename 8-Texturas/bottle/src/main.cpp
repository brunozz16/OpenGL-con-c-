#include <algorithm>
#include <stdexcept>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "ObjMesh.hpp"
#include "Shaders.hpp"
#include "Texture.hpp"
#include "Window.hpp"
#include "Callbacks.hpp"
#include "Model.hpp"

#define VERSION 20221019
#include <iostream>
#include <GL/gl.h>

const float PI=3.14f;

std::vector<glm::vec2> generateTextureCoordinatesForBottle(const std::vector<glm::vec3> &v) {
	
	std::vector<glm::vec2> retorno(v.size());
	float angulo, s, t;
	
	// METODO: MAPEO EN DOS PARTES (CILINDRICO) 
	
	for(size_t i=0;i<v.size();i++) { 
		// obtengo angulo de coordenadas cilindricas:
		angulo = atan2(v[i].x,v[i].z);
		// s = angulo, t = v[i].y (altura), pero con regla de 3 para que vaya de 0 a 1:
		s = (angulo) / (PI); 
		t = (v[i].y + 0.15f) / 0.4f;
		retorno[i] = glm::vec2(s,t);
	}
	
	return retorno;
}

std::vector<glm::vec2> generateTextureCoordinatesForLid(const std::vector<glm::vec3> &v) {
	std::vector<glm::vec2> retorno(v.size());
	float s,t;
	
	// METODO: MAPEO PLANO
	/* El plano ax+by+cz+d=0 se define mediante coordenadas {a,b,c,d}. Los primeros
	tres valores especifican un vector normal al plano y el cuarto es funcion lineal
	de la distancia del plano al origen. A cada coordenada de textura se le asigna
	un plano, definido por la distancia del punto al plano. */
	
	// textura debe posicionarse en el plano xz, el plano y es normal a ella.
	
	for(size_t i=0; i<v.size(); i++){
		// tapa va a estar en el plano xz:
		auto planoS = glm::vec4{ 0.f,0.f,1.f,v[i].z+ 0.5f };
		auto planoT = glm::vec4{ 1.f,0.f,0.f,v[i].x+ 0.5f }; 
		auto punto_actual = glm::vec4(v[i].x*5.f,v[i].y*5.f,v[i].z*5.f,1.f);  //escalo la imagen a la mitad
		
		// producto punto: me da un numero proporcional a la distancia entre el punto y el plano
		s = glm::dot(planoS, punto_actual) ; // traslado 0.5 en s
		t = glm::dot(planoT, punto_actual) ; // traslado 0.5 en t
		
		retorno[i] = glm::vec2(s,t);
	}
	
	return retorno;
}

int main() {
	
	// initialize window and setup callbacks
	Window window(win_width,win_height,"CG Texturas");
	setCommonCallbacks(window);
	
	// setup OpenGL state and load shaders
	glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
	glEnable(GL_BLEND); glad_glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.6f,0.6f,0.8f,1.f);
	Shader shader("shaders/texture");
	
	// load model and assign texture
	auto models = Model::load("suzanne",Model::fKeepGeometry);
	Model &bottle = models[0], &lid = models[1];
	bottle.buffers.updateTexCoords(generateTextureCoordinatesForBottle(bottle.geometry.positions),true);
	bottle.texture = Texture("models/label.png",true,false);
	lid.buffers.updateTexCoords(generateTextureCoordinatesForLid(lid.geometry.positions),true);
	lid.texture = Texture("models/lid.png",false,false);
	
	do {
		
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		shader.use();
		setMatrixes(shader);
		shader.setLight(glm::vec4{2.f,-2.f,-4.f,0.f}, glm::vec3{1.f,1.f,1.f}, 0.15f);
		for(Model &mod : models) {
			mod.texture.bind();
			shader.setMaterial(mod.material);
			shader.setBuffers(mod.buffers);
			glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
			mod.buffers.draw();
		}
		
		// finish frame
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	} while( glfwGetKey(window,GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window) );
}

