#include <forward_list>
#include <iostream>
#include <GL/gl.h>
#include <cmath>
#include "RasterAlgs.hpp"
#include <vector>
using namespace std;

//funciones de ayuda
float redondear(float x) { return int(x)+.5f; }
glm::vec2 redondear(glm::vec2 p) { return {redondear(p.x),redondear(p.y)}; }//convierte el vec2 a enteros
bool puntoRepetido(const glm::vec2 &v , const vector<glm::vec2> &vV);//verifica que el punto v no halla sido pintado
bool comparaVec2(const glm::vec2 &u,const glm::vec2 &v);//simple criterio de comparacion (para puntoRepetido)
void intercambia (float &a, float &b) {float tmp=a; a=b; b=tmp;}
bool esDiscontinuo(auto r,auto aux);//verifica si se pierde la continuidad entre un pixel pintado(r) y el anterior(aux)
void puntoDiscontinuo(paintPixelFunction &paintPixel , auto r , auto aux, vector<glm::vec2> &puntosPintados);//arroja el punto donde se halla la discontinuidad
void checkAndPaint(paintPixelFunction &paintPixel, glm::vec2 &punto, vector<glm::vec2> &puntosPintados);

void drawSegment(paintPixelFunction paintPixel, glm::vec2 p0, glm::vec2 p1) {
	/// @todo: implementar algun algoritmo de rasterizacion de segmentos 
}



void drawCurve(paintPixelFunction paintPixel, curveEvalFunction evalCurve) {
	/// @todo: implementar algun algoritmo de rasterizacion de curvas
	float t0 = 0.0f;
	float t1 = 1.0f;
	if (t1<t0) intercambia(t0,t1); // t0 debe ser menor o igual que t1
	float t = t0;
	
	///vector de puntos pintados
	vector<glm::vec2> puntosPintados; //aca se acumularan los puntos ya pintados
	
	auto r = evalCurve(t);// {60.5,60.5} o casteado {60,60}
	auto aux = r;
	float diferencial;
	
	while(t<t1){
		aux = evalCurve(t); //en r se guardan el punto(r.p) y la derivada(r.d)
		
		if(abs(std::max( r.d.x , r.d.y )) > 0){
		diferencial = 1/abs(std::max( r.d.x , r.d.y ));//guardo el mayor entre las derivadas de x y de y
		}else{diferencial = 0.5;}
		t+=diferencial;
		
		r = evalCurve(t);
		float divisiondiferencial = 2.0f;
		//hacer un while para hacer mas denso el tiempo
		
		float saltoX = abs(redondear(r.p.x) - redondear(aux.p.x));
		float saltoY = abs(redondear(r.p.y) - redondear(aux.p.y));
		
		float saltoMayor = std::max(saltoX,saltoY);
		
		while ( std::max( abs(redondear(r.p.x) - redondear(aux.p.x)) ,  abs(redondear(r.p.y) - redondear(aux.p.y))) > 1 && std::max( abs(redondear(r.p.x) - redondear(aux.p.x)) ,  abs(redondear(r.p.y) - redondear(aux.p.y)))<5){ //mientras el mayor de los saltos (en y o en x) es mayor a 1 
			
			if( abs(redondear(r.p.x) - redondear(aux.p.x)) > 1){
				t-= diferencial/divisiondiferencial;
				r = evalCurve(t);
			}else if(abs(redondear(r.p.y) - redondear(aux.p.y)) > 1){
				t-=diferencial/divisiondiferencial;
				r = evalCurve(t);
			}
			divisiondiferencial++;
			
		}
		paintPixel(r.p);
//		if( abs(redondear(r.p.x) - redondear(aux.p.x)) > 1){
//			t-= diferencial/2;
//			r = evalCurve(t);
//		}else if(abs(redondear(r.p.y) - redondear(aux.p.y)) > 1){
//			t-=diferencial/2;
//			r = evalCurve(t);
//		}
		
		//checkAndPaint(paintPixel, r.p, puntosPintados);
		//puntoDiscontinuo(paintPixel, r, aux, puntosPintados);
		
	}
}



void checkAndPaint(paintPixelFunction &paintPixel, glm::vec2 &punto, vector<glm::vec2> &puntosPintados){ //chequea si el punto no fue pintado antes
	
	glm::vec2 punto_redondeado = redondear(punto);
	
	if(!puntoRepetido( punto_redondeado, puntosPintados )){
		paintPixel(punto_redondeado);
		puntosPintados.push_back(punto_redondeado);
	}
}

bool esDiscontinuo(auto r , auto aux){
	bool resultado;
	if( (abs((int)r.p.x - (int)aux.p.x) > 1) or (abs((int)r.p.y - (int)aux.p.y) > 1)){
		resultado = true;
	}else{
		resultado = false;
	}
	return resultado;
}





void puntoDiscontinuo(paintPixelFunction &paintPixel , auto r , auto aux, vector<glm::vec2> &puntosPintados){
	
	int diferenciaX = (int)r.p.x - (int)aux.p.x;
	int diferenciaY = (int)r.p.y - (int)aux.p.y;
	
	glm::vec2 pintalo = {(int)aux.p.x, (int)aux.p.y};
	
	if( abs(diferenciaX) > 1){
	
		if(diferenciaX < 0){
			for(int i=0; i>diferenciaX; i--) {
				pintalo = pintalo + glm::vec2{i, 0};
				checkAndPaint(paintPixel, pintalo, puntosPintados);
			}
		}
			
		if(diferenciaX > 0){
			for(int i=0; i<diferenciaX; i++) {
				pintalo = pintalo + glm::vec2{i, 0};
				checkAndPaint(paintPixel, pintalo, puntosPintados);
			}
		}
		
	}
	if( abs(diferenciaY) > 1){
			
		if(diferenciaY < 0){
			for(int i=0; i>diferenciaY; i--) {
				pintalo = pintalo + glm::vec2{0, i};
				checkAndPaint(paintPixel, pintalo, puntosPintados);
			}
		}
			
		if(diferenciaY > 0){
			for(int i=0; i<diferenciaY; i++) {
				pintalo = pintalo + glm::vec2{0, i};
				checkAndPaint(paintPixel, pintalo, puntosPintados);
			}
		}

	}
}
	
bool puntoRepetido(const glm::vec2 &v , const vector<glm::vec2> &vV) {
	bool resultado = false;	
	for(glm::vec2 o:vV){
		if(comparaVec2(o,v)) resultado = true;
	}
	return resultado;
}	
bool comparaVec2(const glm::vec2 &u,const glm::vec2 &v){
	bool resultado = false;
	if((u.x == v.x) and (u.y == v.y)){
		resultado = true;
	}
	return resultado;
}
