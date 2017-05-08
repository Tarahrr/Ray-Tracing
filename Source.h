#ifndef _SOURCE_H
#define _SOURCE_H value


#include "Vect.h"
#include "Color.h"
#include "Ray.h"

class Source
{
	

public:
	Source();
	virtual Vect getLightPosition() { return Vect (0,0,0);}
	virtual Color getLightColor() {return Color (1,1,1,0);}

	
};
Source :: Source (){
	
}

#endif

