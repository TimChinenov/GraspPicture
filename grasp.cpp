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
  approxPolyDP(*tracker,polygon,epsilon,true);

  //calculate perimeter of contour
  perimeter = arcLength(polygon,true);

  //find centroid of contour
  findCentroid();

}

void Grasp::generateGrasp()
{
  // first we want to determine the h-k space
  // determine regions of antipodal uncertainty.
  vector<vector<float>> antipodal_regions;
  findAntipodalRegions(antipodal_regions);
}

// Function displays the contour after being simplified to polygons
void Grasp::displayPolygon()
{
  Mat result = Mat::zeros(rows,cols,CV_8UC1);
  vector<vector<Point>> tmpCont;
  tmpCont.push_back(polygon);
  drawContours(result,tmpCont,0,Scalar(255),1,LINE_4,noArray(),8,Point());
  namedWindow("polygon",WINDOW_NORMAL);
  imshow("polygon",result);
}

void Grasp::displayDiscretizedPolygon()
{
  // If there are not enough discrete points, throw an error
  if (Poly_D.size() == 0)
  {
    throw "Polygon was not discretized or descretization failed";
  }
  // Make a blank image
  Mat discreteImg = Mat::zeros(rows,cols,CV_8UC1);

  // Iterate through polygon and plot its points
  for(int cnt = 0; cnt < Poly_D.size(); cnt++)
  {
      discreteImg.at<char>(Poly_D[cnt].y,Poly_D[cnt].x) = 255;
  }
  namedWindow("discrete polygon",WINDOW_NORMAL);
  imshow("discrete polygon",discreteImg);
}

// Function finds a discrete version of the polygon.
// The value of resolution needs to be a value between 0-1 exclusive,
// representing the fraction of pixels that are kept.
void Grasp::discretizePolygon(double resolution)
{
  if (resolution >= 1 || resolution <= 0)
  {
    throw "incorrect resolution value, must be 0 < resolution < 1";
  }

  // Go through each point in the contour
  int cnt = 0;
  for (; cnt != polygon.size(); cnt++)
  {
    int x2, y2, x1 , y1;
    if (cnt == 0)
    {
      x2 = polygon[cnt].x;
      y2 = polygon[cnt].y;
      x1 = polygon[polygon.size() - 1].x;
      y1 = polygon[polygon.size() - 1].y;
    }
    else
    {
      // Find distance between the current and previous points
      x2 = polygon[cnt].x;
      y2 = polygon[cnt].y;
      x1 = polygon[cnt-1].x;
      y1 = polygon[cnt-1].y;
    }
    double distance = sqrt(pow(x2 - x1,2) + pow(y2 - y1,2));

    // Determine number points to generate
    // Subtract one to account for starting point
    int numPoints = round(distance * resolution) - 1;

    // Create a line between the two points
    Line tmpLine = Line(Point(x1, y1),Point(x2, y2));

    // Add the first point to the descritized shape
    Point2d startPoint = Point(x1, y1);
    Poly_D.push_back(startPoint);

    // store "normal" point as corner
    // unit vector can't have magnitude of more than 1
    // so the below works
    Point2d corner = Point(2,2);
    normals_D.push_back(corner);

    // Generate numPoints number of points between the two ends
    for (int cp = 1; cp <= numPoints; cp++)
    {
      // Calculate next point
      Point2d nxtPoint = startPoint + (tmpLine.uVec * cp / resolution);

      nxtPoint.x = round(nxtPoint.x);
      nxtPoint.y = round(nxtPoint.y);

      // Add next point to discretized polygon list
      Poly_D.push_back(nxtPoint);

      // Add normal to vector
      normals_D.push_back(tmpLine.getPerpendicular());
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

void Grasp::findAntipodalRegions(vector<vector<float>> & regions)
{
  for(int itr = 0; itr < Poly_D.size(); itr++)
  {
    // points evaluated in current iteration
    Point * pt1;
    Point * pt2;
    // respective normals of points
    Point2d op_norm_1;
    Point2d op_norm_2;
    // this will store the smallest range found
    float minDist = 2*3.1415;

    // make sure to evaluate first and last index of array together
    if (itr == Poly_D.size()-1)
    {
      // set points
      pt1 = &Poly_D[itr];
      pt2 = &Poly_D[0];

      // set normals
      op_norm_1 = Point2d(normals_D[itr]);
      op_norm_2 = Point2d(normals_D[0]);
    }
    else
    {
      pt1 = &Poly_D[itr];
      pt2 = &Poly_D[itr+1];

      op_norm_1 = Point2d(normals_D[itr]);
      op_norm_2 = Point2d(normals_D[itr+1]);
    }

    // for now, we'll skip vertices
    if (op_norm_1.x == 2 || op_norm_2.x == 2)
    {
      continue;
    }
    //////////////////////////////

    // take the inverse of the normals
    op_norm_1 = -1 * op_norm_1;
    op_norm_2 = -1 * op_norm_2;

    // Future Edit: The following might be implementable in a
    // better way. Need to consider possibilities

    // iterate through each combination of vector pairs
    for (int vtr = 0; vtr < Poly_D.size(); vtr++)
    {
      for (int wtr = 0; wtr < Poly_D.size(); wtr++)
      {
        // make sure indexes are not the same
        if (vtr == wtr || vtr == itr || wtr == itr) continue;

        // get the normal vectors
        Point2d * norm_v = &normals_D[vtr];
        Point2d * norm_w = &normals_D[wtr];

        // convert unit vectors to radian angles


      }
    }

  }
}
