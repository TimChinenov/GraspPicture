#ifndef LINE_H_
#define LINE_H_

class Line
{
public:
  Line();
  Line(Point pos1,Point pos2);
  double[] getPerpendicular();// returns perpendicular unit vector
  double getAngleWith(double[] l1);//finds angle between self and l1;
private:
  double uVec[2];
  double point[2];
  double mag;
};


#endif
