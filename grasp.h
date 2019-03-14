#ifndef GRASP_H_
#define GRASP_H_

#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ximgproc/slic.hpp>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/ximgproc.hpp>


#include <iostream>
#include <vector>
#include <map>
#include <string>

using namespace std;
using namespace cv;

class Grasp
{
  private:
    vector<Point> polygon;
    double perimeter;
    int rows;
    int cols;
  public:
    Grasp();
    Grasp(const cv::Mat & binImg);
    void displayPolygon();
};



#endif
