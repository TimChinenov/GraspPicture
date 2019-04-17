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
    Point2d uVec;
    Point2d startPoint;
    double mag;
    // true if point is a corner, false otherwise
    bool corner;

    Line();
    Line(Point2d pos1, Point2d pos2);
    Point2d getPerpendicular();// returns perpendicular unit vector
    double getAngleWith(double l1[2]);//finds angle between self and l1;
};


#endif
