#ifndef _TRIANGLE_H
#define _TRIANGLE_H value


#include "math.h"
#include "Object.h"
#include "Vect.h"
#include "Color.h"

class Triangle : public Object
{
	Vect A,B,C;
	Vect normal;
	double distance;
	Color color;

public:
	Triangle();
	Triangle(Vect,Vect,Vect,Color);

	Vect getTriangleNormal (){
	Vect CA (C.getVectX() - A.getVectX(), C.getVectY() - A.getVectY(), C.getVectZ() - A.getVectZ());
	Vect BA (B.getVectX() - A.getVectX(), B.getVectY() - A.getVectY(), B.getVectZ() - A.getVectZ());
	normal = CA.crossProduct(BA).normalize();
	
		return normal;
	}
	double getTriangleDistance (){
		normal = getTriangleNormal();
		distance = normal.dotProduct(A);
		return distance;
	}
	virtual Color getColor (){return color;}

	virtual Vect getNormalAt (Vect point){ 
		normal = getTriangleNormal();
		return normal;
	}
	virtual double findIntersection(Ray ray){
		Vect ray_direction = ray.getRayDirection();
		Vect ray_origin = ray.getRayOrigin();

		normal = getTriangleNormal();
		distance =getTriangleDistance();

		double a = ray_direction.dotProduct(normal);

		
	
		if(a == 0){
			// ray is parallel to the Triangle 
			return -1;
		}
		else{
			double b = normal.dotProduct(ray .getRayOrigin().vectAdd(normal.vectMult(distance).negative()));
		
		double distanceToPlane = -1*b/a;

		double Q_x =ray_direction.vectMult(distanceToPlane).getVectX()+ray_origin.getVectX();
		double Q_y =ray_direction.vectMult(distanceToPlane).getVectY()+ray_origin.getVectY();
		double Q_z =ray_direction.vectMult(distanceToPlane).getVectZ()+ray_origin.getVectZ();

		Vect Q(Q_x,Q_y,Q_z);
		//3 condition for inside triangle
						//Q is point of intersection
					// [CAxQA]*n >= 0
					// [BCxQC]*n >= 0
					// [ABxQB]*n >= 0

			Vect CA (C.getVectX() - A.getVectX(), C.getVectY() - A.getVectY(), C.getVectZ() - A.getVectZ());
			Vect BC (B.getVectX() - C.getVectX(), B.getVectY() - C.getVectY(), B.getVectZ() - C.getVectZ());
			Vect AB (A.getVectX() - B.getVectX(), A.getVectY() - B.getVectY(), A.getVectZ() - B.getVectZ());

			Vect QB (Q.getVectX() - B.getVectX(), Q.getVectY() - B.getVectY(), Q.getVectZ() - B.getVectZ());
			Vect QC (Q.getVectX() - C.getVectX(), Q.getVectY() - C.getVectY(), Q.getVectZ() - C.getVectZ());
			Vect QA (Q.getVectX() - A.getVectX(), Q.getVectY() - A.getVectY(), Q.getVectZ() - A.getVectZ());
			
			double testInside1 = (CA.crossProduct(QA)).dotProduct(normal);
			double testInside2 = (BC.crossProduct(QC)).dotProduct(normal);
			double testInside3 = (AB.crossProduct(QB)).dotProduct(normal);
			if( (testInside3 >= 0) && (testInside2 >= 0) && (testInside1 >= 0) ){
					//inside triangle
					return -1*b/a;
			}
			else{
				return -1;
			}
		}
			
	}
};
Triangle :: Triangle (){
	A = Vect (1,0,0);
	B = Vect (0,1,0);
	C = Vect (0,0,1);
	color = Color(0.5,0.5,0.5,0);
}

Triangle :: Triangle (Vect pointA,Vect pointB, Vect pointC, Color colorValue){
	A = pointA;
	B = pointB;
	C = pointC;
	color = colorValue;

}

#endif

