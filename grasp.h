#ifndef GRASP_H_
#define GRASP_H_

#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/ximgproc.hpp>


#include <iostream>
#include <vector>
#include <map>
#include <string>

#include "line.h"

using namespace std;
using namespace cv;

class Grasp
{
  private:
    //polygonized curve
    vector<Point> polygon;
    //descretized Polygon
    vector<Point> Poly_D;
    // holds the normal unit vectors of index associated
    // discrete points
    vector<Point2d> normals_D;
    //perimater of polygonized curve
    double perimeter;
    //centroid of the polygonized curve
    Point centroid;
    //center of mass of polygonized curve
    double CoM;
    //dimensions of original image
    int rows;
    int cols;


    //private functions
    void findCentroid();
  public:
    Grasp();
    Grasp(const cv::Mat & binImg);
    void displayPolygon();
    void displayDiscretizedPolygon();
    void generateGrasp();
    void descretizePolygon();
    void discretizePolygon(double resolution);

};



#endif
