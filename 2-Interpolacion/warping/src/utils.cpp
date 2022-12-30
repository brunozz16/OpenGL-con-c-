#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "utils.hpp"
#include "Debug.hpp"

BoundingBox::BoundingBox(glm::vec3 &p1, glm::vec3 &p2) 
	: pmin({std::min(p1.x,p2.x),std::min(p1.y,p2.y),std::min(p1.z,p2.z)}),
      pmax({std::max(p1.x,p2.x),std::max(p1.y,p2.y),std::max(p1.z,p2.z)}) 
{
	
}
	
bool BoundingBox::contiene(glm::vec3 &p) const {
	return p.x>=pmin.x && p.x<=pmax.x &&
		p.y>=pmin.y && p.y<=pmax.y &&
		p.z>=pmin.z && p.z<=pmax.z;
}

Pesos calcularPesos(glm::vec3 x0, glm::vec3 x1, glm::vec3 x2, glm::vec3 &x) {
	/// @todo: implementar
	
	///Defino los vectores que utilizare para hacer el producto cruz
	glm::vec3 A=x0-x2;
	glm::vec3 B=x1-x2;
	glm::vec3 C=x-x2;	
	glm::vec3 D=x1-x0;
	glm::vec3 E=x-x0;
	
	glm::vec3 AT=cross(A,B);///esto devuelve 2*AT pero no lo divido por 2 porque mas adelante no tendra relevancia 
	glm::vec3 A0=cross(C,B);///esto devuelve 2*A0 pero no lo divido por 2 porque mas adelante no tendra relevancia
	glm::vec3 A1=cross(A,C);///esto devuelve 2*A1 pero no lo divido por 2 porque mas adelante no tendra relevancia
	glm::vec3 A2=cross(D,E);///esto devuelve 2*A2 pero no lo divido por 2 porque mas adelante no tendra relevancia
	
	///Denominador para el calculo de las areas de cada vertice
	float ATxAT=dot(AT,AT);
	///Calculo de las areas para cada vertice
	float alpha0=dot(A0,AT)/ATxAT;
	float alpha1=dot(A1,AT)/ATxAT;
	float alpha2=dot(A2,AT)/ATxAT;
	
	return {alpha0,alpha1,alpha2};
}
