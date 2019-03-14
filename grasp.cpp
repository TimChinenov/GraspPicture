#include "grasp.h"
using namespace std;
using namespace cv;

Grasp::Grasp(){}

Grasp::Grasp(const cv::Mat & binImg)
{
  rows =  binImg.rows;
  cols = binImg.cols;
  /* Need to perform a check that the image is correct form */


  //create variables to hold contour and hierarchy
  vector<Vec4i> hierarchy;
  vector<vector<Point> > contours;

  //find the contours of the input binary image
  findContours(binImg,contours,hierarchy,RETR_TREE,CHAIN_APPROX_SIMPLE,Point(0,0));

  //grab the largest contour
  double area = 0; //update area to find largest area
  vector<Point> *tracker; //set largest contour to this pointer
  vector<vector<Point>>::iterator cntItr = contours.begin();

  //iterate through all the contours <-- can we just pick the top of the hierarchy
  for(;cntItr != contours.end();cntItr++){
    double tmpArea = contourArea(*cntItr);
    if(tmpArea > area)
    {
      tracker = &(*cntItr);
      area = tmpArea;
    }
  }

  //polygonize the largest contour
  cerr << tracker->size() << endl;
  approxPolyDP(*tracker,polygon,5,true);
  cerr << polygon.size() << endl;

  //calculate perimeter of contour
  perimeter = arcLength(polygon,true);
}

void Grasp::displayPolygon()
{
  Mat result = Mat::zeros(rows,cols,CV_8UC1);
  cerr << polygon.size() << endl;
  vector<vector<Point>> tmpCont;
  tmpCont.push_back(polygon);
  drawContours(result,tmpCont,0,Scalar(255),1,LINE_4,noArray(),8,Point());

  namedWindow("polyon",WINDOW_NORMAL);
  imshow("polyon",result);
}

void generateGrasp()
{
  /* convert each set off coordinates into a line*/
  

}
