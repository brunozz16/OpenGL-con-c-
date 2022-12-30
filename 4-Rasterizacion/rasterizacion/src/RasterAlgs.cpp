#include <forward_list>
#include <iostream>
#include <GL/gl.h>
#include <cmath>
#include "RasterAlgs.hpp"
#include <vector>
using namespace std;

//funciones de ayuda
static float redon(float x) { return int(x)+.5f; }
static glm::vec2 redon(glm::vec2 p) { return glm::vec2{ redon(p.x) , redon(p.y) }; }
bool comparaVec2(const glm::vec2 u,const glm::vec2 v);
void checkAndPaint(paintPixelFunction &paintPixel, glm::vec2 &punto, glm::vec2 &punto_anterior);

void drawSegment(paintPixelFunction paintPixel, glm::vec2 p0, glm::vec2 p1) {
	/// @todo: implementar algun algoritmo de rasterizacion de segmentos
	   
	    float dx = p1.x - p0.x; //distancia entre dos puntos en x
	    float dy = p1.y - p0.y; //distancia entre dos puntos en y
		
	    if (fabs(dx) >= fabs(dy)){ //linea con tendencia horizontal
	        if (dx<0){ //si la tendencia es hacia la izquierda hago swap
				glm::vec2 paux = p0;
	            p0 = p1;
	            p1 = paux;
	        }
	        float parametrox = redon( p0.x ); //variable para moverse en x (posicion inicial)
	        while(parametrox < redon( p1.x )){ //pintar pixeles hasta llegar al punto p1
	            paintPixel(glm::vec2{ parametrox , redon(p0.y) });
	            parametrox = parametrox + 1; //avanzar de a un pixel
	            p0.y = p0.y + (dy/dx); //avanza en y dependiendo de la pendiente
	        }
	    }
		
		
	    if (fabs(dx)<fabs(dy)){ //linea con tendencia vertical
	        if (dy<0){ //si la tendencia es hacia la izquierda hago swap
				glm::vec2 paux2 = p0;
	            p0 = p1;
	            p1 = paux2;
	        }
	        float parametroy = redon(p0.y); //variable para moverse en y (posicion inicial)
	        while(parametroy < redon(p1.y)){ //pintar pixeles hasta llegar al punto p1
	            paintPixel(glm::vec2{ redon(p0.x),parametroy }); 
	            parametroy = parametroy + 1; //avanzar de a un pixel
	            p0.x = p0.x + (dx/dy); //avanza en x dependiendo de la pendiente
	        }
	    }

}

void drawCurve(paintPixelFunction paintPixel, curveEvalFunction evalCurve) {
	/// @todo: implementar algun algoritmo de rasterizacion de curvas

		
	 
}

void checkAndPaint(paintPixelFunction &paintPixel, glm::vec2 &punto, glm::vec2 &punto_anterior){
	if( !comparaVec2( redon(punto), redon(punto_anterior) ) )
	{  
		paintPixel( redon(punto) );//pinto el puntoB 
	}
}
bool comparaVec2(const glm::vec2 u,const glm::vec2 v){
	bool resultado = false;
	
	if((u.x == v.x) and (u.y == v.y)){
		resultado = true;
	}
	return resultado;
}








