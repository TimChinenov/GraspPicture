#ifndef SLIC_H
#define SLIC_H

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <float.h>

using namespace std;

//define data types for image vectors
#define vec2dd vector<vector<double> >
#define vec2di vector<vector<int> >
#define vec2db vector<vector<bool> >
//limit for cluster iterations
#define NR_ITERATIONS 10

class Slic {
  private:
    vec2di clusters;
    vec2dd distances;

    //LAB and xy values of centers
    vec2dd centers;
    //number of occurences of each center
    vector<int> center_counts;

    //step size per cluster, and the color distance
    int step,nc,ns;

    // compute the distance between a center and an individual pixel
    double compute_dist(int ci, CvPoint pixel, CvScalar color);
    //find the pixel with the lowest gradient in a 3x3 surrounding
    CvPoint find_local_minimum(IplImage *image, CvPoint center);

    //Remove and initialize the 2d vectors.
    void clear_data();
    void init_data(IplImage *image);

  public:
    // Class construcutros and deconstructors
    Slic();
    ~Slic();

    // generate an over-segmentation for an image
    void generate_superpixels(IplImage *image, int step, int nc);
    // enforce connectivity for an image
    void create_connectivity(IplImage *image);

    // Draw functions
    void display_center_grid(IplImage *image, CvScalar color);
    void display_contours(IplImage *image, CvScalar color);
    void color_with_cluster_means(IplImage *image);
}
