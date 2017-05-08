#ifndef _LIGHT_H
#define _LIGHT_H value

#include "Source.h"
#include "Vect.h"
#include "Color.h"

class Light : public Source
{
	Vect position;
	Color color;

public:
	Light();
	Light(Vect,Color);

	virtual Vect getLightPosition (){return position;}
	virtual Color getLightColor (){return color;}
	void setLightPosition(Vect p) {position = p; }
	void setLightColor(Color c){color = c;}
	
};
Light :: Light (){
	position = Vect (0,0,0);
	color = Color(1,1,1,0);
}

Light :: Light (Vect p ,Color c){
	position = p;
	color = c;

}

#endif

