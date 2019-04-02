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
  approxPolyDP(*tracker,polygon,5,true);

  //calculate perimeter of contour
  perimeter = arcLength(polygon,true);
  //find centroid of contour
  findCentroid();


}

void Grasp::generateGrasp()
{
}

// Function displays the contour after being simplified to polygons
void Grasp::displayPolygon()
{
  Mat result = Mat::zeros(rows,cols,CV_8UC1);
  vector<vector<Point>> tmpCont;
  tmpCont.push_back(polygon);
  drawContours(result,tmpCont,0,Scalar(255),1,LINE_4,noArray(),8,Point());
  // fillPoly(result,tmpCont,Scalar(255));
  namedWindow("polygon",WINDOW_NORMAL);
  imshow("polygon",result);
}

void Grasp::displayDescrtizedPolygon()
{
  // If there are not enough discrete points, throw an error
  if (Poly_D.size() == 0)
  {
    throw "Polygon was not descretized or descretization failed";
  }
  // Make a blank image
  Mat result = Mat::zeros(rows,cols,CV_8UC1);

  // Iterate through polygon and plot its points
  for(int cnt = 0; cnt < Poly_D.size(); cnt++)
  {
      result.at<int>(Poly_D[cnt]) = 255;
  }
  namedWindow("discrete polygon",WINDOW_NORMAL);
  imshow("discrete polygon",result);
}

// Function finds a discrete version of the polygon.
// The value of resolution needs to be a value between 0-1 exclusive,
// representing the fraction of pixels that are kept.
void Grasp::descretizePolygon(double resolution)
{
  if (resolution >= 1 || resolution <= 0)
  {
    throw "incorrect resolution value, must be 0 < resolution < 1";
  }

  // Go through each point in the contour
  int cnt = 1;
  for (; cnt != polygon.size(); cnt++)
  {
    // Find distance between the current and previous points
    int x2 = polygon[cnt].x;
    int y2 = polygon[cnt].y;
    int x1 = polygon[cnt-1].x;
    int y1 = polygon[cnt-1].y;
    double distance = sqrt(pow(x2 - x1,2) + pow(y2 - y1,2));

    // Determine number points to generate
    // Subtract one to account for starting point
    int numPoints = int(distance * resolution) - 1;

    // Create a line between the two points
    Line tmpLine = Line(polygon[cnt-1],polygon[cnt]);

    // Add the first point to the descritized shape
    Point startPoint = Point(polygon[cnt-1]);
    Poly_D.push_back(startPoint);

    // Generate numPoints number of points between the two ends
    for (int cp = 1; cp <= numPoints; cp++)
    {
      // Calculate next point
      Point nxtPoint = startPoint + tmpLine.uVec * cp;
      cout << startPoint + tmpLine.uVec * cp <<endl;
      nxtPoint.x = (nxtPoint.x);
      nxtPoint.y = (nxtPoint.y);
      
      // Add next point to descretized polygon list
      Poly_D.push_back(nxtPoint);
    }
  }
}

void Grasp::findCentroid()
{
  //get moments of polygon
  Moments mmnts = moments(polygon);

  //calculate centroid of polygon
  int cx = int(mmnts.m10/mmnts.m00);
  int cy = int(mmnts.m01/mmnts.m00);

  //assign cx and cy to array
  centroid.x = cx;
  centroid.y = cy;
}
