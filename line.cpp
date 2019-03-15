#include "line.h"
using namespace std;
using namespace cv;

Line::Line(){}

Line::Line(Point pos1, Point pos2)
{
  //find starting point of line
  startPoint.x = pos1.x;
  startPoint.y = pos1.y;

  //calculate magnitude of line
  mag = sqrt(pow(pos1.x-pos2.x,2)+pow(pos1.y-pos2.y,2));

  //find unit vector for direction
  uVec[0] = (pos2.x-pos1.x)/mag;
  uVec[1] = (pos2.y-pos2.y)/mag;

}

//following returns the perpendicular unit vector of the line
double* Line::getPerpendicular()
{
  //!!!Need to consider direction of new vector
  double* perp;
  double perpUVect[2];
  perp = perpUVect;
  perpUVect[0] = uVec[1];
  perpUVect[1] = uVec[0]*(-1);

  return perp;
}

//following finds the angle between two unit vectors
double Line::getAngleWith(double l1[2])
{
  return cos(l1[0]*uVec[0]+l1[1]*uVec[1]);
}
