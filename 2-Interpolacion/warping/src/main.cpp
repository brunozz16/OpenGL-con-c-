#include <algorithm>
#include <stdexcept>
#include <vector>
#include <limits>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "Model.hpp"
#include "Window.hpp"
#include "Callbacks.hpp"
#include "Debug.hpp"
#include "Shaders.hpp"
#include "Bezier.hpp"
#include "BezierRenderer.hpp"
#include "Delaunay.hpp"
#include "DelaunayRenderer.hpp"

#define VERSION 20220822
#include <iostream>
using namespace std;

// settings
std::vector<std::string> models_names = { "suzanne", "fish" };
void eliminaZ(glm::vec3 &v);
int current_model = 0;
bool wireframe = false, apply_warp = true, 
	 show_delaunay = false, show_points = true;

// triangulations
Delaunay new_delaunay() { float l=1.3f; return Delaunay({-l,-l,-l},{+l,+l,+l}); }
Delaunay delaunay0 = new_delaunay(), // triangulacion sobre la geometria original
	     delaunay1 = new_delaunay(); // triangulacion con puntos desplazados
Delaunay &current_delaunay() { return apply_warp?delaunay1:delaunay0; }
Delaunay &other_delaunay()   { return apply_warp?delaunay0:delaunay1; }
int selected_pt = -1;

// callbacks
void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods);

// funciones para aplicar o deshacer la distorsión
glm::vec3 warpPoint(const Delaunay &delaunay0, const Delaunay &delaunay1, glm::vec3 p);
void applyWarp(const Delaunay &delaunay0, const Delaunay &del_new, 
			   const Geometry &geometry, GeometryRenderer &renderer);
void restoreGeometry(const Delaunay &delaunay0, const Delaunay &del_new, 
			         const Geometry &geometry, GeometryRenderer &renderer);

// programa principal
int main() {
	
	// initialize window and setup callbacks
	Window window(win_width,win_height,"CG Demo",true);
	glfwSetFramebufferSizeCallback(window, common_callbacks::viewResizeCallback);
	glfwSetCursorPosCallback(window, mouseMoveCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetKeyCallback(window, keyboardCallback);
	
	
//	glm::vec3 A0={1.3f,1.3f,0};
//	glm::vec3 A1={-1.3f,1.3f,0};
//	glm::vec3 A2={1.3f,-1.3f,0};
//	glm::vec3 A3={0.4333f,0.4333f,0};
//	
//	std::array<float,3> vectorsillo=calcularPesos(A0,A1,A2,A3);
//	cout<<vectorsillo[0]<<" "<<vectorsillo[1]<<" "<<vectorsillo[2];
//	string var1(std::to_string(vectorsillo[0]));
//	auto c_var1=var1.c_str();
//	string var2(std::to_string(vectorsillo[1]));
//	auto c_var2=var2.c_str();
//	string var3(std::to_string(vectorsillo[2]));
//	auto c_var3=var3.c_str();
	
	
	// setup OpenGL state
	glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS); 
	glEnable(GL_CULL_FACE);
	use_perspective = false; view_angle = 0.f;
	glClearColor(0.2f,0.2f,0.5f,1.f);
	
	// model and triangulation
	Shader shader_phong("shaders/phong"),
		   shader_wire("shaders/wireframe");
	int loaded_model = -1;
	std::vector<Model> models;
	DelaunayRenderer delaunay_renderer;
	
	// main loop
	do {
		
		// cargar el modelo si es necesario
		if (loaded_model!=current_model) {
			models = Model::load(models_names[current_model],Model::fKeepGeometry|Model::fDynamic);
			loaded_model = current_model;
		}
		
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		// dibujar el modelo
		glPolygonMode(GL_FRONT_AND_BACK,wireframe?GL_LINE:GL_FILL);
		for(Model &part : models) {
			Shader &shader = wireframe ? shader_wire : shader_phong;
			shader.use();
			setMatrixes(shader);
			shader.setLight(glm::vec4{-2.f,-2.f,-4.f,0.f}, glm::vec3{1.f,1.f,1.f}, 0.15f);
			// aplicar deformacion
			auto func = apply_warp?applyWarp:restoreGeometry;
			func(delaunay0,delaunay1,part.geometry,part.buffers);
			shader.setBuffers(part.buffers);
			shader.setMaterial(part.material);
			part.buffers.draw();
		}
		
		// dibujar la triangulacion
		if (show_delaunay||show_points) {
			glDisable(GL_DEPTH_TEST);
			setMatrixes(delaunay_renderer.getShader());
			delaunay_renderer.draw(current_delaunay().getPuntos(),
								   show_delaunay ? delaunay0.getTriangulos() : std::vector<Triangulo>{},
								   selected_pt);
			glEnable(GL_DEPTH_TEST);
		}
		
		// settings sub-window
		window.ImGuiDialog("CG Example",[&](){
			ImGui::Combo(".obj (O)", &current_model,models_names);		
			ImGui::Checkbox("Apply Warp (A)",&apply_warp);
			ImGui::Checkbox("Delaunay (D)",&show_delaunay);
			ImGui::Checkbox("Wireframe (W)",&wireframe);
			ImGui::Checkbox("Control Points(P)",&show_points);
//			ImGui::Text(c_var1);
//			ImGui::Text(c_var2);
//			ImGui::Text(c_var3);
			
			if (ImGui::Button("Reset Positions (R)"))
				delaunay1 = delaunay0;
			if (ImGui::Button("Reset All (C)")) 
				delaunay1 = delaunay0 = new_delaunay();
		});
		
		// finish frame
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	} while( glfwGetKey(window,GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window) );
}

// distorsiona un vértice de la geometría
glm::vec3 warpPoint(const Delaunay &delaunay0, const Delaunay &delaunay1, glm::vec3 p) {
	/// @todo: completar
	
	//Defino el vector de triangulos de delaunay0 (originales)
	vector<Triangulo> vectorTrianguloOriginal=delaunay0.getTriangulos();
	//Defino el vector de puntos de delaunay0
	vector<glm::vec3> PO = delaunay0.getPuntos();
	
	//Defino el indice del triangulo donde se encuentra el punto p
	int indice = delaunay0.enQueTriangulo(p);
	
	//Recupero el triangulo donde se encuentra ese punto
	Triangulo TO = vectorTrianguloOriginal[indice];
	
	//Calculo los pesos para el punto p dentro del triangulo
	Pesos pesos = calcularPesos(PO[TO[0]],PO[TO[1]],PO[TO[2]],p);	
	
	//Defino el vector de puntos de delaunay1 (Puntos Modificados)
	vector<glm::vec3> PN = delaunay1.getPuntos();			
	
	p.x = ( pesos[0]* PN[TO[0]].x ) + ( pesos[1]* PN[TO[1]].x ) + ( pesos[2]* PN[TO[2]].x );  
	p.y = ( pesos[0]* PN[TO[0]].y ) + ( pesos[1]* PN[TO[1]].y ) + ( pesos[2]* PN[TO[2]].y );
	//p=(pesos[0]*PN[TO[0]])+( pesos[1]* PN[TO[1]] )+( pesos[2]* PN[TO[2]] );
	//p={p.x,p.y,-1};
	return p;
}

void eliminaZ(glm::vec3 &v){
	glm::vec3 u={v.x,v.y,0};
	v=u;
}

// distorsiona toda la geometría
void applyWarp(const Delaunay &delaunay0, const Delaunay &del_new,
			   const Geometry &geometry, GeometryRenderer &renderer) 
{
	// obtener vertices deformados
	Geometry new_geom;
	new_geom.positions.reserve(geometry.positions.size());
	for(glm::vec3 p : geometry.positions)
		new_geom.positions.push_back( warpPoint(delaunay0,delaunay1,p) );
	
	// recalcular normales y enviar los nuevos datos a la gpu
	new_geom.triangles = geometry.triangles;
	new_geom.generateNormals();
	renderer.updatePositions(new_geom.positions,false);
	renderer.updateNormals(new_geom.normals,false);
}

// restablece los vertices originales
void restoreGeometry(const Delaunay &delaunay0, const Delaunay &del_new,
					 const Geometry &geometry, GeometryRenderer &renderer) 
{
	// enviar los datos originales a la gpu
	renderer.updatePositions(geometry.positions,false);
	renderer.updateNormals(geometry.normals,false);
}

// teclado: atajos para las settings
void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods) {
	if (action!=GLFW_PRESS) return;
	switch (key) {
		case 'A': apply_warp = !apply_warp; break;
		case 'D': show_delaunay = !show_delaunay; break;
		case 'P': show_points = !show_points; break;
		case 'W': wireframe = !wireframe; break;
		case 'R': delaunay1 = delaunay0; break;
		case 'C': delaunay1 = delaunay0 = new_delaunay(); break;
		case 'O': case 'M': current_model = (current_model+1)%models_names.size(); break;
	}
}

// dadas las coordenadas de un click, calcula el punto del plano z=0 al que
// corresponde según la configuración de la cámara
glm::vec3 viewportToPlane(double xpos, double ypos) {
	auto ms = common_callbacks::getMatrixes(); // { model, view, projection }
	auto inv_matrix = glm::inverse(ms[2]*ms[1]*ms[0]); // ndc->world
	auto pa = inv_matrix * glm::vec4{ float(xpos)/win_width*2.f-1.f,
                                      (1.f-float(ypos)/win_height)*2.f-1.f,
									  0.f, 1.f }; // point on near
	auto pb = inv_matrix * glm::vec4{ float(xpos)/win_width*2.f-1.f,
		                              (1.f-float(ypos)/win_height)*2.f-1.f,
		                              1.f, 1.f }; // point on far
	float alpha = pa[2]/(pa[2]-pb[2]);
	auto p = pa*(1-alpha) + pb*alpha; // point on plane
	return {p[0]/p[3],p[1]/p[3],0.f};
}

// indice del vertice de la triangulación actual cercano a p (si no hay ninguno, -1)
int closestPoint(glm::vec3 p, int ignorar_este = -1) {
	const auto &vp = current_delaunay().getPuntos();
	float min_d2 = std::numeric_limits<float>::max();
	int imin = -1;
	for(size_t i=0;i<vp.size();++i) {
		if (i==ignorar_este) continue;
		auto v_aux = vp[i]-p;
		float aux_d2 = glm::dot(v_aux,v_aux);
		if (aux_d2<min_d2) {
			imin = i;
			min_d2 = aux_d2;
		}
	}
	if (min_d2>0.001) imin = -1;
	return imin;
}

// drag: mover el vertice de la triangulación seleccionado
void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
	if (selected_pt!=-1) {
		if (selected_pt<4) return; // no mover los del bbox
		glm::vec3 p = viewportToPlane(xpos,ypos);
		if (closestPoint(p,selected_pt)!=-1) return; // no acercar demasiado a otro
		current_delaunay().moverPunto(selected_pt,p);
	}
}

// click izquierdo: agregar o mover vertice de la triangulación
// click derecho: eliminar vertice de la triangulacion
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (ImGui::GetIO().WantCaptureMouse) return;
	if (action == GLFW_PRESS) {
		// pasar de coords de la ventana a coords del modelo
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		auto p = viewportToPlane(xpos,ypos);
		// buscar vertice de la triangulacion cercano al click
		selected_pt = closestPoint(p);
		if (button==GLFW_MOUSE_BUTTON_RIGHT) { // click derecho: eliminar punto
			if (selected_pt<4) return; // no eliminar vertices del bounding box
			delaunay1.eliminarPunto(selected_pt);
			delaunay0.eliminarPunto(selected_pt);
			selected_pt = -1;
		} else { // click izquierdo: agregar o mover punto
			if (selected_pt!=-1) return; // seleccionado para mover, no hacer nada mas en este evento
			if (not current_delaunay().getBoundingBox().contiene(p)) return; // no agregar fuera del bb
			auto q = warpPoint(current_delaunay(),other_delaunay(),p); // pto equivalente en la otra triangulacion
			selected_pt = current_delaunay().agregarPunto(p);
			other_delaunay().agregarPunto(q);
		}
	} else {
		selected_pt = -1; // soltar el pto al soltar el boton
	}
}

