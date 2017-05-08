#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <limits>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#include "Vect.h"
#include "Ray.h"
#include "Camera.h"
#include "Color.h"
#include "Light.h"
#include "Sphere.h"
#include "Source.h"
#include "Object.h"
#include "Plane.h"
#include "Triangle.h"



using namespace std;


struct RGBType{
	double r,g,b;
};

void savebmp(const char *filename, int w, int h, int dpi, RGBType *data){
	FILE *f;
	int k = w*h;
	int s = 4*k;
	int filesize = 54 + s;
	
	double factor = 39.375;
	int m = static_cast<int>(factor);
	
	int ppm = dpi*m;
	
	unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0,0,0, 54,0,0,0};
	unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0,24,0};
	
	bmpfileheader[ 2] = (unsigned char)(filesize);
	bmpfileheader[ 3] = (unsigned char)(filesize>>8);
	bmpfileheader[ 4] = (unsigned char)(filesize>>16);
	bmpfileheader[ 5] = (unsigned char)(filesize>>24);
	
	bmpinfoheader[ 4] = (unsigned char)(w);
	bmpinfoheader[ 5] = (unsigned char)(w>>8);
	bmpinfoheader[ 6] = (unsigned char)(w>>16);
	bmpinfoheader[ 7] = (unsigned char)(w>>24);
	
	bmpinfoheader[ 8] = (unsigned char)(h);
	bmpinfoheader[ 9] = (unsigned char)(h>>8);
	bmpinfoheader[10] = (unsigned char)(h>>16);
	bmpinfoheader[11] = (unsigned char)(h>>24);
	
	bmpinfoheader[21] = (unsigned char)(s);
	bmpinfoheader[22] = (unsigned char)(s>>8);
	bmpinfoheader[23] = (unsigned char)(s>>16);
	bmpinfoheader[24] = (unsigned char)(s>>24);
	
	bmpinfoheader[25] = (unsigned char)(ppm);
	bmpinfoheader[26] = (unsigned char)(ppm>>8);
	bmpinfoheader[27] = (unsigned char)(ppm>>16);
	bmpinfoheader[28] = (unsigned char)(ppm>>24);
	
	bmpinfoheader[29] = (unsigned char)(ppm);
	bmpinfoheader[30] = (unsigned char)(ppm>>8);
	bmpinfoheader[31] = (unsigned char)(ppm>>16);
	bmpinfoheader[32] = (unsigned char)(ppm>>24);
	
	f = fopen(filename,"wb");
	
	fwrite(bmpfileheader,1,14,f);
	fwrite(bmpinfoheader,1,40,f);
	
	for (int i = 0; i < k; i++) {
		RGBType rgb = data[i];
		
		double red = (data[i].r)*255;
		double green = (data[i].g)*255;
		double blue = (data[i].b)*255;
		
		unsigned char color[3] = {(int)floor(blue),(int)floor(green),(int)floor(red)};
		
		fwrite(color,1,3,f);
	}
	
    fclose(f);
}

int winningObjectIndex(vector<double> object_intersections){
	//return the index of winning intersection
	int index_of_minimum_value;
	//perevent unnessary  calculations
	if(object_intersections.size() == 0 ){
		// if there are no intersections 
		return -1; 
	} else if(object_intersections.size() == 1 ){
		if(object_intersections.at(0) > 0){
			// if that intersection is greater than than zero then its our index of minimum value
			return 0;
		}else{
			//otherwise the only intersection value is negative
			return -1; 
		}
	}
	else{
		//otherwise there is more than one intersections
		//first find the maximum value
		double max = 0 ;
		for(int i = 0 ; i < object_intersections.size() ; i++){
			if(max < object_intersections.at(i)) {
				max = object_intersections.at(i);
			}
		}
		//then starting from maximum value find the minimum positive value 
		if(max > 0){
			//we only want positive intersections
			for (int index = 0 ; index < object_intersections.size() ; index++){
				if(object_intersections.at(index) > 0  && object_intersections.at(index) <= max){
					max = object_intersections.at(index);
					index_of_minimum_value = index;
				}
			}
			return index_of_minimum_value;
		}else{
			// all the intersections were negative
			return -1;
		}
	}
}
Color getColorAt(Vect intersection_position, Vect intersecting_ray_direction , vector<Object*> scene_objects , int index_of_winning_object , vector<Source*> light_sources , double accuracy , Color ambientlight){

	Color winning_object_color = scene_objects.at(index_of_winning_object) -> getColor();
	Vect winning_object_normal = scene_objects.at(index_of_winning_object) -> getNormalAt(intersection_position);

if (winning_object_color.getColorSpecial() == 2) {
		// checkered/tile floor pattern
		
		int square = (int)floor(intersection_position.getVectX()) + (int)floor(intersection_position.getVectZ());
		
		if ((square % 2) == 0) {
			// black tile
			winning_object_color.setColorRed(0);
			winning_object_color.setColorGreen(0);
			winning_object_color.setColorBlue(0);
		}
		else {
			// white tile
			winning_object_color.setColorRed(1);
			winning_object_color.setColorGreen(1);
			winning_object_color.setColorRed(1);
		}
	}

	Color final_color = winning_object_color.colorMultiply(ambientlight);

	if(winning_object_color.getColorSpecial() > 0 && winning_object_color.getColorSpecial() <= 1){
		//reflection from objects with specular value intensity

		double dot1 = winning_object_normal.dotProduct(intersecting_ray_direction.negative());
		Vect scalar1 = winning_object_normal.vectMult(dot1);
		Vect add1 =scalar1.vectAdd(intersecting_ray_direction);
		Vect scalar2 = add1.vectMult(2);
		Vect add2 = intersecting_ray_direction.negative().vectAdd(scalar2);
		Vect reflection_direction = add2.normalize();

		Ray reflection_ray (intersection_position,reflection_direction);

		//determine what the ray intersects with first
		vector<double> reflection_intersections;

		for(int reflection_index=0; reflection_index < scene_objects.size() ; reflection_index++){
			reflection_intersections.push_back(scene_objects.at(reflection_index)->findIntersection(reflection_ray));	
		}
		int index_of_winning_object_with_reflection = winningObjectIndex(reflection_intersections);
		if(index_of_winning_object_with_reflection != -1){
			//reflection ray missed everything else
			if(reflection_intersections.at(index_of_winning_object_with_reflection) > accuracy){
				//determine the position and direction at the point of intersection with the reflection ray
				//the ray only effects the color if it reflected off sth

				Vect reflection_intersection_position = intersection_position.vectAdd(reflection_direction.vectMult(reflection_intersections.at(index_of_winning_object_with_reflection)));
				Vect reflection_intersection_ray_direction = reflection_direction;
				//Recursive Point
				Color reflection_intersection_color = getColorAt(reflection_intersection_position,reflection_intersection_ray_direction, scene_objects, index_of_winning_object_with_reflection, light_sources, accuracy, ambientlight);

				final_color = final_color.colorAdd(reflection_intersection_color.colorScalar(winning_object_color.getColorSpecial()));
			}
		}

	}

	for (int light_index = 0; light_index < light_sources.size(); light_index++) {
		Vect light_direction = light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()).normalize();
		
		float cosine_angle = winning_object_normal.dotProduct(light_direction);
		
		if (cosine_angle > 0) {
			// test for shadows
			bool shadowed = false;//false;
			
			Vect distance_to_light = light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()).normalize();
			float distance_to_light_magnitude = distance_to_light.magnitude();
			
			Ray shadow_ray (intersection_position, light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()).normalize());
			
			vector<double> secondary_intersections;
			
			for (int object_index = 0; object_index < scene_objects.size() && shadowed == false; object_index++) {
				secondary_intersections.push_back(scene_objects.at(object_index)->findIntersection(shadow_ray));
			}
			
			for (int c = 0; c < secondary_intersections.size(); c++) {
				if (secondary_intersections.at(c) > accuracy) {
					if (secondary_intersections.at(c) <= distance_to_light_magnitude) {
						shadowed = true;
					}
                    
                    break;
				}
			}
			
			if (shadowed == false) {
				final_color = final_color.colorAdd(winning_object_color.colorMultiply(light_sources.at(light_index)->getLightColor()).colorScalar(cosine_angle));
				
				if (winning_object_color.getColorSpecial() > 0 && winning_object_color.getColorSpecial() <= 1) {
					// special [0-1]

					double dot1 = winning_object_normal.dotProduct(intersecting_ray_direction.negative());
					Vect scalar1 = winning_object_normal.vectMult(dot1);
					Vect add1 = scalar1.vectAdd(intersecting_ray_direction);
					Vect scalar2 = add1.vectMult(2);
					Vect add2 = intersecting_ray_direction.negative().vectAdd(scalar2);
					Vect reflection_direction = add2.normalize();
					
					double specular = reflection_direction.dotProduct(light_direction);
					if (specular > 0) {
						specular = pow(specular, 10);
						final_color = final_color.colorAdd(light_sources.at(light_index)->getLightColor().colorScalar(specular*winning_object_color.getColorSpecial()));
					}
				}
			}	
		}
	}
	return final_color.clip();
}


int thisone;

void Read(string name, int* n1 ,int* n2, int* n3 ){
	char * pch;
    ifstream inputFile(name);
    string line,p;
    char str[1024];
   
    while (getline(inputFile, line)){
        strncpy(str, line.c_str(), sizeof(str));
        str[sizeof(str) - 1] = 0;
        pch = strtok (str," [],");
        p = pch;  
        if(p == "light") *n1 = *n1+1;
      	if(p == "sphere") *n2= *n2+1;
      	if(p == "triangle") *n3 = *n3+1;
    }
}
int main(int argc, char const *argv[])
{
	cout<<"rendering ..."<<endl;
	clock_t t1,t2;
	t1 = clock();	
 
	int dpi =72;
	
	double width = 0, height = 0;

  
    Color ambientlight (0,0,0,1);
	double accuracy = 0.00000001;	

	Vect O (0,0,0);
    Vect X (1,0,0);
    Vect Y (0,1,0);
    Vect Z (0,0,1);


    Color white_light (1.0, 1.0, 1.0, 0.0);
    Color pretty_green (0.5, 1.0, 0.5, 0.3);
    Color maroon (0.5, 0.25, 0.25, 0);
    Color gray (0.5, 0.5, 0.5, 0.0);
    Color black (0.0, 0.0, 0.0, 0.0);
    Color tile_floor (1, 1, 1, 2);
    Color Plane_color(0.4, 0.4, 0.7,0);
    Vect new_sphere_location (1.75, -0.25, 0);


    int light_counter1 = 0 ,sphere_counter1 = 0 , triangle_counter1=0;
    Read( "test.scn", &light_counter1,&sphere_counter1,&triangle_counter1);

    int light_counter = 0 ,sphere_counter = 0 , triangle_counter=0;
    Vect light_position (0,0,0);
    Color light_color (0,0,0,0);
    Light *scene_light = new Light[light_counter1]; 
 
    vector<Source*>	light_sources;

    Vect sphere_center;
    double sphere_radius;
    Color sphere_color;
    Sphere *scene_sphere = new Sphere[sphere_counter1];
 
    Color plane_color (0,0,0,0);
    double distance = 0;
    Vect plane_normal (0,0,0);

    Color triangle_color (0,0,0,0);
    Triangle *scene_triangle = new Triangle[triangle_counter1];
   

    Vect A(0,0,0);
    Vect B(0,0,0);
    Vect C(0,0,0);
    
 	// store a object of the scene
    vector<Object*>	scence_objects;
    
   
    char * pch;
    ifstream inputFile(argv[1]);
    string line,p;
    char str[1024];
    Vect campos (0,0,0);
    Vect look_at (0,0,0);

    Color background (0,0,0,0);

    while (getline(inputFile, line)){

        strncpy(str, line.c_str(), sizeof(str));
        str[sizeof(str) - 1] = 0;
        pch = strtok (str," [],");
        p = pch;  
	    if( p == "xres"){               // Width
	        
	        pch = strtok (NULL, " [],");
	        width = stof(pch);
	        
	    }else if(p == "yres"){          //Height
	        
	        pch = strtok (NULL, " [],");
	        height = stof(pch);
	        
	    }else if(p == "camera" ){       //Camera
	        
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	        
	        if(p == "eye"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	            campos.setVectX(stof(p));
	             
	            pch = strtok (NULL, " [],");
	            p = pch;
	            campos.setVectY(stof(p));
	            
	            pch = strtok (NULL, " [],");
	            p = pch;
	            campos.setVectZ(stof(p));
	   
	        }
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	        
	        if(p == "lookat"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	            look_at.setVectX(stof(p));
	                              
	            pch = strtok (NULL, " [],");
	            p = pch;
	            look_at.setVectY(stof(p));
	                                 
	            pch = strtok (NULL, " [],");
	            p = pch;
	            look_at.setVectZ(stof(p));        
	        }

	    }
	    else if(p == "background"){          //BackGround
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	        
	           
	            pch = strtok (NULL, " [],");
	            p = pch;
	            background.setColorRed(stof(p));
	                              
	            pch = strtok (NULL, " [],");
	            p = pch;
	            background.setColorGreen(stof(p));
	                                 
	            pch = strtok (NULL, " [],");
	            p = pch;
	            background.setColorBlue(stof(p));
	    }else if(p == "ambient"){          //ambientlight
	        
	           
	            pch = strtok (NULL, " [],");
	            p = pch;
	            ambientlight.setColorRed(stof(p));
	                              
	            pch = strtok (NULL, " [],");
	            p = pch;
	            ambientlight.setColorGreen(stof(p));
	                                 
	            pch = strtok (NULL, " [],");
	            p = pch;
	            ambientlight.setColorBlue(stof(p));

	    }else if(p == "light" ){       //light
	        
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	        
	        if(p == "position"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	            light_position.setVectX(stof(p));
	             
	            pch = strtok (NULL, " [],");
	            p = pch;
	            light_position.setVectY(stof(p));
	            
	            pch = strtok (NULL, " [],");
	            p = pch;
	            light_position.setVectZ(stof(p));
	   
	        }
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	        
	        if(p == "color"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	            light_color.setColorRed(stof(p));
	                              
	            pch = strtok (NULL, " [],");
	            p = pch;
	            light_color.setColorGreen(stof(p));
	                                 
	            pch = strtok (NULL, " [],");
	            p = pch;
	            light_color.setColorBlue(stof(p));        
	        }
	        Light light (light_position, light_color);
	        scene_light[light_counter]= light;
			light_sources.push_back(dynamic_cast<Source*>(&scene_light[light_counter]));
			light_counter++;
	    }
	   	else if(p == "sphere" ){       //sphere
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;

	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	        
	        if(p == "color"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	            sphere_color.setColorRed(stof(p));
	             
	            pch = strtok (NULL, " [],");
	            p = pch;
	            sphere_color.setColorGreen(stof(p));
	            
	            pch = strtok (NULL, " [],");
	            p = pch;
	            sphere_color.setColorBlue(stof(p));

	            
	   
	        }
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	     	
	        if(p == "Ks"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	           sphere_color.setColorSpecial(stof(p));
	   
	        }	
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	     	
	        if(p == "Ka"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	            sphere_color.setColorRed(sphere_color.getColorRed() * (stof(p)));
	            sphere_color.setColorGreen(sphere_color.getColorGreen() * (stof(p)));
	            sphere_color.setColorBlue(sphere_color.getColorBlue() * (stof(p)));
	        }	

	        getline(inputFile, line);
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	        
	        if(p == "center"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	            sphere_center.setVectX(stof(p));
	                              
	            pch = strtok (NULL, " [],");
	            p = pch;
	            sphere_center.setVectY(stof(p));
	                                 
	            pch = strtok (NULL, " [],");
	            p = pch;
	            sphere_center.setVectZ(stof(p));        
	        }
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	        
	        if(p == "radius"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	         sphere_radius =  stof(p);
	     
	        }
	        Sphere sphere (sphere_center,sphere_radius,sphere_color);
	        scene_sphere[sphere_counter] = sphere;
		    scence_objects.push_back(dynamic_cast<Object*> (&scene_sphere[sphere_counter]));
		    sphere_counter++;
	    }
	    else if(p == "triangle" ){       //triangle
	         
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;

	        getline(inputFile, line);	  
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	       
	        if(p == "color"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	          
	            triangle_color.setColorRed(stof(p));
	              
	            pch = strtok (NULL, " [],");
	            p = pch;
	            triangle_color.setColorGreen(stof(p));
	            
	            pch = strtok (NULL, " [],");
	            p = pch;
	            triangle_color.setColorBlue(stof(p));

	            
	   
	        }
	       
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	     	
	        if(p == "Ks"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	           triangle_color.setColorSpecial(stof(p));
	   
	        }	
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	     	
	        if(p == "Ka"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	            triangle_color.setColorRed(triangle_color.getColorRed() * (stof(p)));
	            triangle_color.setColorGreen(triangle_color.getColorGreen() * (stof(p)));
	            triangle_color.setColorBlue(triangle_color.getColorBlue() * (stof(p)));
	            
	        }	
	        	
	        getline(inputFile, line);
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	        
	        if(p == "point1"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	            A.setVectX(stof(p));
	                              
	            pch = strtok (NULL, " [],");
	            p = pch;
	            A.setVectY(stof(p));
	                                 
	            pch = strtok (NULL, " [],");
	            p = pch;
	            A.setVectZ(stof(p));        
	        }
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	        
	        if(p == "point2"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	            B.setVectX(stof(p));
	                              
	            pch = strtok (NULL, " [],");
	            p = pch;
	            B.setVectY(stof(p));
	                                 
	            pch = strtok (NULL, " [],");
	            p = pch;
	            B.setVectZ(stof(p));        
	        } 
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	         
	        if(p == "point3"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	            C.setVectX(stof(p));
	                              
	            pch = strtok (NULL, " [],");
	            p = pch;
	            C.setVectY(stof(p));
	                                 
	            pch = strtok (NULL, " [],");
	            p = pch;
	            C.setVectZ(stof(p));        
	        }

	        Triangle triangle (A,B,C,triangle_color);
	        scene_triangle[triangle_counter] = triangle;
 			scence_objects.push_back(dynamic_cast<Object*> (&scene_triangle[triangle_counter]));
	        triangle_counter++;
	    }
	    else if(p == "plane" ){       //plane
	        
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	        getline(inputFile, line);
	      
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	        
	        if(p == "color"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	            plane_color.setColorRed(stof(p));
	             
	            pch = strtok (NULL, " [],");
	            p = pch;
	            plane_color.setColorGreen(stof(p));
	            
	            pch = strtok (NULL, " [],");
	            p = pch;
	            plane_color.setColorBlue(stof(p));

	        
	   
	        }
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	     	
	        if(p == "Ks"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	         plane_color.setColorSpecial(stof(p));
	   
	        }
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;

	        if(p == "Ka"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	            plane_color.setColorRed(plane_color.getColorRed() * (stof(p)));
	            plane_color.setColorGreen(plane_color.getColorGreen() * (stof(p)));
	            plane_color.setColorBlue(plane_color.getColorBlue() * (stof(p)));
	        }
	        getline(inputFile, line);

	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	      
	        if(p == "normal"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	            plane_normal.setVectX(stof(p));
	                              
	            pch = strtok (NULL, " [],");
	            p = pch;
	            plane_normal.setVectY(stof(p));
	                                 
	            pch = strtok (NULL, " [],");
	            p = pch;
	            plane_normal.setVectZ(stof(p));        
	        }
	        getline(inputFile, line);
	        strncpy(str, line.c_str(), sizeof(str));
	        str[sizeof(str) - 1] = 0;
	        pch = strtok (str," [],");
	        p = pch;
	        
	        if(p == "point"){

	            pch = strtok (NULL, " [],");
	            p = pch;
	            pch = strtok (NULL, " [],");
	            p = pch;
	            pch = strtok (NULL, " [],");
	            p = pch;
	         distance =  stof(p);
	     
	        }
	       	Plane scene_plane (plane_normal,  distance , plane_color);
	        scence_objects.push_back(dynamic_cast<Object*> (&scene_plane));

	    }
}
 	
	double  aspectratio = (double)width/(double)height;
    int n = width*height;
    RGBType *pixels = new RGBType[n];
	
    Vect diff_btw (campos.getVectX() - look_at.getVectX(), campos.getVectY() - look_at.getVectY() ,campos.getVectZ() - look_at.getVectZ());


    Vect camdir = diff_btw.negative().normalize();
    Vect camright= Y.crossProduct(camdir).normalize();
    Vect camdown = camright.crossProduct(camdir).normalize();

    Camera scene_cam (campos,camdir,camright,camdown);

   double xamnt,yamnt;
	for (int x = 0; x < width; x++){
		for (int y = 0; y < height; y++){
			thisone =y*width+x; 	// index value of pixels
			if (width > height){
				// the image is wider than it is tall 
				xamnt = ((x+0.5)/width)*aspectratio - (((width-height)/(double)height)/2);
				yamnt = ((height-y)+0.5)/height;
			}
			else if (height > width){
				// the image is taller than it is wide
				xamnt = (x+0.5)/width;
				yamnt = (((height-y)+0.5)/height)/aspectratio - (((height-width)/(double)width)/2);
			}else{
				// the image is square
				xamnt = (x+0.5)/width;
				yamnt = ((height-y)+0.5)/height;
			}

			Vect cam_ray_origin = scene_cam.getCameraPosition();
		 	Vect cam_ray_direction = camdir.vectAdd(camright.vectMult(xamnt - 0.5).vectAdd(camdown.vectMult(yamnt - 0.5))).normalize();
			Ray cam_ray (cam_ray_origin,cam_ray_direction);

			vector<double> intersections;

			for (int index = 0 ; index < scence_objects.size() ; index ++){
				
				intersections.push_back(scence_objects.at(index)->findIntersection(cam_ray));
			}
			// which object is closer to the camera 
			int index_of_winning_object = winningObjectIndex(intersections);

			if(index_of_winning_object == -1){
				//TODO
				//set the background color
				pixels[thisone].r =background.getColorRed();
				pixels[thisone].g =background.getColorGreen();
				pixels[thisone].b =background.getColorBlue();
			}
			else{
				// index coresponds to an object in our scene
				if(intersections.at(index_of_winning_object) > accuracy){
					//determine the position and directionvectors at the point of intersection
					Vect intersection_position = cam_ray_origin.vectAdd(cam_ray_direction.vectMult(intersections.at(index_of_winning_object)));
					Vect intersection_ray_direction = cam_ray_direction;
					
					Color intersection_color = getColorAt(intersection_position, intersection_ray_direction , scence_objects , index_of_winning_object , light_sources , accuracy , ambientlight);
					// simple not for shadow -> scence_objects.at(index_of_winning_object)->getColor();
					
					pixels[thisone].r =intersection_color.getColorRed();
					pixels[thisone].g =intersection_color.getColorGreen();
					pixels[thisone].b =intersection_color.getColorBlue();

				}
			}

		}
	}
	savebmp("scene.bmp",width,height,dpi,pixels); 
	delete pixels;
	t2 = clock();
	float diff = ((float)t2 - (float)t1)/1000;
	cout << "Render time:\t "<< diff << " seconds \n";
	delete [] scene_light;
	delete [] scene_triangle;
	delete [] scene_sphere;

	return 0;
}