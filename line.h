#ifndef LINE_H_
#define LINE_H_
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <math.h>

using namespace std;
using namespace cv;

class Line
{
  private:
  public:
    Point uVec;
    Point startPoint;
    double mag;

    Line();
    Line(Point pos1, Point pos2);
    Point getPerpendicular();// returns perpendicular unit vector
    double getAngleWith(double l1[2]);//finds angle between self and l1;
};


#endif
