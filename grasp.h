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
    vector<Point> polygon;
    double perimeter;
    Point centroid;
    //center of mass
    double CoM;
    int rows;
    int cols;

    //private functions
    void findCentroid();
  public:
    Grasp();
    Grasp(const cv::Mat & binImg);
    void displayPolygon();
    void generateGrasp();
    void descretizePolygon();
};



#endif
