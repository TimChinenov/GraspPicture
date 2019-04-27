//g++ -std=c++11 *.cpp `pkg-config --libs --cflags opencv` -o main
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ximgproc/slic.hpp>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/ximgproc.hpp>
#include <opencv2/saliency.hpp>


#include <iostream>
#include <vector>
#include <map>
#include <string>

#include <stdio.h>
#include <math.h>
#include <float.h>
#include "grasp.h"
#include "line.h"

using namespace std;
using namespace cv;
using namespace saliency;

void getSuperPixels(const Mat &inputImg, Mat &outputImg, Mat &labels, int pixelSize);
void getSaliencyMap(Mat &inputImg, Mat &outputImg);
void binarizeSaliencyMap(const Mat &salMap, const Mat &pxlLabels, Mat &outputImg, double Thold);
void dilateBinary(const Mat &binaryImg, Mat &dilation, int pixelSize);
void cleanUpBinary(Mat &binaryImg);

int main( int argc, char* argv[] ) {
  if(argc != 4)
  {
    cout << "Usage: ./main [input image path] [number of superpixel] [compactness]" << endl;
    return 1;
  }

  Mat image; //variable holds main image
  Mat BlurImage; //Image holds alterations
  Mat gCutImg; //super pixel image
  Mat spxlImg; //Saliency image
  Mat spxlLabel;
  Mat salImg;
  Mat dilatedImg;
  Mat fgd;
  Mat bgd;
  Mat binMask;
  String file = argv[1];//grab image from command line

  /* parameters of the algorithm */
  int SPXLSIZE; //average super pixel size
  double Tvalue = 0.25; //threshold valie

  //intialize the primary image
  image = imread(file , IMREAD_COLOR);

  /* get information on the image, height and width */
  int imgH = image.rows;
  int imgW = image.cols;
  SPXLSIZE = imgH * imgW / argv[2];
  float compactness = argv[3]

  /* initialize the binarized image as all black */
  Mat spxlSal = Mat::zeros(imgH,imgW,CV_8UC1);

  /* Blur the image with 3x3 Gaussian */
  GaussianBlur(image,BlurImage,Size(5,5),0,0);

  /* Construct superpixel and generate mask*/
  getSuperPixels(image, spxlImg, spxlLabel, SPXLSIZE, compactness);

  /* Compute the saliency map for the image using OpenCV */
  getSaliencyMap(image, salImg);

  /* perfomr binarization on saliency map */
  binarizeSaliencyMap(salImg,spxlLabel,spxlSal,Tvalue);

  /* Perform dilation on image */
  dilateBinary(spxlSal,dilatedImg,SPXLSIZE);

  /* Find contours of the image */
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;
  findContours(spxlSal,contours,hierarchy,RETR_TREE,CHAIN_APPROX_SIMPLE,Point(0,0));

  /* Find contours with largest area */
  double area = 0;
  vector<Point> *tracker;
  vector<vector<Point>>::iterator cntItr = contours.begin();
  for(;cntItr != contours.end();cntItr++){
    double tmpArea = contourArea(*cntItr);
    if(tmpArea > area)
    {
      tracker = &(*cntItr);
      area = tmpArea;
    }
  }

  /* Find the bounding box of the largest contour */
  Rect bnds = boundingRect(*tracker);

  /* pad rectangle */
  int padSize=10;
  if(bnds.x-padSize >= 0)
  {
    bnds.x = bnds.x - 10;
    if(bnds.width + 2*padSize < imgW)
    {
      bnds.width = bnds.width + 2*padSize;
    }
  }
  if(bnds.y-padSize >= 0)
  {
    bnds.y = bnds.y - 10;
    if(bnds.height + 2*padSize < imgH)
    {
      bnds.height = bnds.height + 2*padSize;
    }
  }
  /* Use the above rectangle to grab cut the object */
  grabCut(BlurImage,gCutImg,bnds,bgd,fgd,3,GC_INIT_WITH_RECT);
  binMask.create(gCutImg.size(),CV_8UC1);
  binMask = gCutImg & 1;

  /* Get contours and keep only the largest one */
  cleanUpBinary(binMask);

  /* Make Grasp Object */
  Grasp grasp(binMask);
  grasp.displayPolygon();
  grasp.discretizePolygon(0.1);
  grasp.displayDiscretizedPolygon();
  grasp.generateGrasp();

  /* place super pixel mask on image*/
  Mat spxlMod = image.clone();
  binMask = binMask*255;
  cvtColor(binMask,binMask,COLOR_GRAY2BGR );
  int i,j;
  Vec3b* p;
  Vec3b* q;
  for( i = 0; i < imgH; ++i)
  {
      p = binMask.ptr<Vec3b>(i);
      for ( j = 0; j < imgW; ++j)
      {
          if(p[j][0] == 255 || p[j][1] == 255 || p[j][2] == 255){
            p[j][0] == 0;
            p[j][1] == 0;
            p[j][2] == 255;
          }
      }
  }

  for( i = 0; i < imgH; ++i)
  {
      p = binMask.ptr<Vec3b>(i);
      q = spxlMod.ptr<Vec3b>(i);
      for ( j = 0; j < imgW; ++j)
      {

        if(p[j][0] == 255 || p[j][1] == 255 ||  p[j][2] == 255){
          q[j][0] = 0;
          q[j][1] = 0;
          q[j][2] = 255;
        }
      }
  }

  namedWindow("two", WINDOW_NORMAL);
  imshow("two",binMask);
  namedWindow("three", WINDOW_NORMAL);
  imshow("three",salImg);
  namedWindow("five", WINDOW_NORMAL);
  imshow("five",spxlSal);
  namedWindow("four", WINDOW_NORMAL);
  imshow("four",spxlMod);

  waitKey(0);
}


/*****************************************/
/*      Functions for program below      */
/*****************************************/
/* Following finds the superpixels and its labels
   in image inputImg and and places them in outputImg and labels.
   the average superpixel size will be set to pixelSize*/
void getSuperPixels(const Mat & inputImg, Mat & outputImg, Mat & labels, int pixelSize, float compactness)
{
  /* Blur the image with 3x3 Gaussian */
  GaussianBlur(inputImg,outputImg,Size(5,5),0,0);

  /* Convert to LAB color space */
  cvtColor(outputImg,outputImg, 44);

  /* Construct superpixel and generate mask*/
  Ptr<ximgproc::SuperpixelSLIC> ptr =
                     ximgproc::createSuperpixelSLIC(outputImg,100,pixelSize,compactness);

  ptr->iterate(10);
  ptr->enforceLabelConnectivity(10); //reduces small fragments
  ptr->getLabelContourMask(outputImg,true); //returns contour mask
  ptr->getLabels(labels); //assigns labels
}


void getSaliencyMap(Mat &inputImg, Mat &outputImg)
{
  /* Convert to LAB color space */
  cvtColor(inputImg,outputImg, 44);
  Mat average = outputImg.clone();

  /* Blur the image with 3x3 Gaussian */
  GaussianBlur(outputImg,outputImg,Size(5,5),0,0);

  Ptr<Saliency> fine = StaticSaliencyFineGrained::create();
  if(!fine->computeSaliency(outputImg,outputImg)){
    cout << "error computing sliency" << endl;
  }
  normalize(outputImg,outputImg,0,255,NORM_MINMAX,0,noArray());
  /* Find average value */
  double li = 0;
  double a = 0;
  double b = 0;
  int numpxls = outputImg.rows*outputImg.cols;
  int i,j;
  Vec<unsigned char, 3>* p;
  for( i = 0; i < outputImg.rows; ++i)
  {
      p = outputImg.ptr<Vec<unsigned char, 3>>(i);
      for ( j = 0; j < outputImg.cols; ++j)
      {
          li += p[j][0];
          a += p[j][1];
          b += p[j][2];
      }
  }

  li = li/(numpxls);
  a = a/(numpxls);
  b = b/(numpxls);
  // cout << li <<"\n"<<a <<"\n" <<b <<endl;

  Vec3b *q;
  Mat tmpimag = Mat::zeros(outputImg.rows,outputImg.cols,CV_8UC1);
  for( i = 0; i < average.rows; ++i)
  {
      q = outputImg.ptr<Vec3b>(i);
      for ( j = 0; j < average.cols; ++j)
      {
          q[j][0] = li-q[j][0];
          q[j][1] = a-q[j][1];
          q[j][2] = b-q[j][2];
      }
  }
  outputImg = abs(outputImg);
}

void binarizeSaliencyMap(const Mat &salMap, const Mat &pxlLabels, Mat &outputImg, double Thold)
{
  map<int,vector<int>> values;
  map<int,vector<Point>> locations;
  int imgH = salMap.rows;
  int imgW = salMap.cols;

  double min, max;
  minMaxLoc(salMap, &min, &max);

  double minPxl, spxlNum;
  minMaxLoc(pxlLabels, &minPxl, &spxlNum);

  const int *imgPtr;
  const unsigned char *salPtr;
  for(int i = 0; i < imgH; i++){
    imgPtr = pxlLabels.ptr<const int>(i);
    for(int j = 0; j < imgW; j++){
      //int curLbl = pxlLabels.at<int>(i, j);
      int curLbl = imgPtr[j];
      //if the label is not in the map, add it.
      if(values.find(curLbl) == values.end())
      {
        vector<int> tempVals;
        vector<Point> tempPtns;
        values.insert(pair<int,vector<int>>(curLbl,tempVals));
        locations.insert(pair<int,vector<Point>>(curLbl,tempPtns));
      }
      Point ptn;
      ptn.x = j;
      ptn.y = i;
      salPtr = salMap.ptr<const unsigned char>(i);
      int val = salPtr[j];
      //int val = salMap.at<unsigned char>(i,j);
      values[curLbl].push_back(val);
      locations[curLbl].push_back(ptn);
    }
  }


  //check which superpixels pass the threshold
  map<int,vector<int>>::iterator itr;
  for(itr = values.begin(); itr != values.end(); itr++){
    int vecSize;
    double median;

    sort((itr->second).begin(),(itr->second).end());
    //find median value
    vecSize = (itr->second).size();
    //check whether odd or even length vector
    if(vecSize%2 == 0)
    {
      median = ((itr->second)[vecSize/2-1]+(itr->second)[vecSize/2])/2.0;
    }
    else
    {
      median = (itr->second)[vecSize/2];
    }
    //check if the super pixel is thresholded to foreground
    if(median>(max*Thold))
    {
      //make all the relevant super pixels white.
      vector<Point>::iterator Pts2Convs = locations[itr->first].begin();
      for(; Pts2Convs != locations[itr->first].end();Pts2Convs++)
      {
        outputImg.at<unsigned char>(*Pts2Convs) = 255;
      }
    }
  }
}

void dilateBinary(const Mat &binaryImg, Mat &dilation, int pixelSize)
{
  Mat element = getStructuringElement
      (MORPH_ELLIPSE,Size(2*pixelSize,2*pixelSize),Point(-1,-1));
  dilate(binaryImg,dilation,element);
}

void cleanUpBinary(Mat &binaryImg)
{
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;
  double maxArea = 0.0;
  int coolCont = -1;
  contours.clear();
  hierarchy.clear();
  findContours(binaryImg.clone(),contours,hierarchy,RETR_TREE,CHAIN_APPROX_SIMPLE,Point());

  for(int i = 0; i< contours.size(); i++)
  {
    double tmpArea =  contourArea(contours[i]);
    if(tmpArea > maxArea){
      maxArea = tmpArea;
      coolCont = i;
    }
  }
  Mat result;
  result = Mat::zeros(binaryImg.rows,binaryImg.cols,CV_8UC1);
  drawContours(result,contours,coolCont,Scalar(255),CV_FILLED,LINE_4,noArray(),8,Point());
  binaryImg &= result;
}
