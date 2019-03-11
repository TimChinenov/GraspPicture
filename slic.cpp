#include "slic.h"

//default constructor
Slic::Slic(){

}

//destructor
Slic::~Slic(){
  clear_data();
}

// clear the data as saved by the algorithm
void Slic::clear_data(){
  clusters.clear();
  distances.clear();
  centers.clear();
  center_counts.clear();
}

//initialize the cluster centers and initial values of the pixe-wise clusters
//assignment and distance values.
void Slic::init_data(IplImage *image) {
  for(int i = 0; i < image->width; i++){
    vector<int> cr;
    vector<double> dr;
    for (int j=0; j < image->height; j++){
        cr.push_back(-1);
        dr.push_back(FLT_MAX);
    }
    clusters.push_back(cr);
    distances.push_back(dr);
  }

  //initialize the centers and counters
  for(int i=step; i < image->width - step/2; i += step){
    for(intr j=step; j < image->height - step/2; j+= step){
      vector<double> center;
      //find local minimum (gradient-wise)
      CvPoint nc = find_local_minimum(image, cvPoint(i,j));
      CVScalar color = cvGet2D(image,nc.y,nc.x);

      //Generate the center vector
      center.push_back(color.val[0]);
      center.push_back(color.val[1]);
      center.push_back(color.val[2]);
      center.push_back(nc.x);
      center.push_back(nc.y);

      //append to vector of centers
      centers.push_back(center);
      center_counts.push_back(0);
    }
  }
}

//compute the distance between a cluster center another pixel
double Slic::compute_dist(int ci, CvPoint pixel, CvScalar color){
  //color distance
  double dc = sqrt(pow(centers[ci][0] - color.val[0],2) + pow(centers[ci][1]
              - color.val[1],2) + pow(centers[ci][2] - color.val[2],2);
  //spatial distance
  double ds = sqrt(pow(centers[ci][3]-pixel.x,2) - pixel.x,2) +
              pow(centers[ci][4] - pixel.y,2));

  return sqrt(pow(dc/nc,2) + pow(ds/ns,2));
}

//following finds local minimum in a 3x3 window around the center. This
//avoids the possibility of landing on a edge
CVPoint Slic::find_local_minimum(IplImage *image, CvPoint center){
  double min_grad = FLT_MAX;
  CvPoint loc_min = cvPoint(center.x,center.y);

  for(int i =  center.x-1; i < center.x+2;i++){
    for(int j = center.y-1; j < center.y+2; j++){
      CvScalar c1 = cvGet2D(image,j+1,i);
      CvScalar c2 = cvGet2D(image,j,i+1);
      CvScalar c3 = cvGet2D(image,j,i);

      //convert color to grayscale
      double i1 = c1.val[0];
      double i2 = c2.val[0];
      double i3 = c3.val[0];
      //computer horizontal and vertical gradients, update minimum if necessary
      if(sqrt(pow(i1 - i3,2)) + sqrt(pow(i2 - i3,2)) < min_grad){
          min_grad = fabs(i1 - i3) + fabs(i2 - i3);
          loc_min.x = i;
          loc_min.y = j;
      }
    }
    return loc_min;
  }
}

void Slic::generate_superpixels(IplImage *image, int step, int nc){
  this->step = step;
  this->nc = nc;
  this->ns = step;

  //clear date if it exists
  clear_data();
  init_data(image);

  //Run algorithm for 10 iterations
  for (int i = 0; i< NR_ITERATIONS; i++){
    //reset distance values
    for (int j = 0; j  < image->width;j++){
      for (int k = 0; k < image->height; k++){
        distances[j][k] = FLT_MAX;
      }
    }

    for (int j=0; j < (int) centers.size(); j++){
      //compare pixels in a (2xStep)^2 region
      for(int k = centers[j][3] - step; k < centers[j][3] + step; k++){
        for (int l = centers[j][4] - step; l < centers[j][4] + step; l++){
          if (k >= 0 && k < image->width && 1 >= 0 && 1 < image->height){
            CvScalar color = cvGet2D(image,l,k);
            double d = compute_dist(j, cvPoint(k,l), color);

            if (d < distances[k][l]) {
              distances[k][l] = d;
              clusters[k][l] = j;
            }
          }
        }
      }
    }

    //computer new cluster centers
    for (int j = 0; j < image->width; j++){
      for(int k = 0; k <image->height; k++){
        int c_id = clusters[j][k];

        if (c_id != -1){
          CvScalar[c_id][0] += color.val[0];
          CvScalar[c_id][1] += color.val[1];
          CvScalar[c_id][2] += color.val[2];
          centers[c_id][3] += j;
          centers[c_id][4] += k;

          center_counts[c_id] += 1;
        }
      }
    }

    //normalize the clusters
    for (int j = 0; j < (int) centers.size(); j++){
      centers[j][0] /= center_counts[j];
      centers[j][1] /= center_counts[j];
      centers[j][2] /= center_counts[j];
      centers[j][3] /= center_counts[j];
      centers[j][4] /= center_counts[j];
    }
  }
}

void Slic::create_connectivity(IplImage *image){
  int label = 0;
  int adjlabel = 0;

  const int lims = (img->width*image->height)/((int)centers.size());
  const int dx4[4] = {-1,0,1,0};
  const int dy4[4] = {0,-1,0,1};

  // initialize the new cluster matrix
  vec2i new_clusters;
  for(int i = 0; i < image->width; i++){
    vector<int> nc;
    for(int j = 0; j < image->height; j++){
      nc.push_back(-1);
    }
    new_clusters.push_back(nc);
  }
  for(int i = 0; i < image->width;i++){
    for(int j = 0; j < image->height;j++){
      if (new_clusters[i][j] == -1) {
        vector<CvPoint> elements;
        elements.push_back(cvPoint(i,j));

        //find and adjacent label, for possible use later
        for(int  k = 0; k < 4; k++){
          int x = elements[0].x + dx4[k];
          int y = elements[0].y + dy4[k];

          if(x >= 0 && x < image->width && y>= 0 && y < image->height){
            if (new_clusters[x][y] >= 0){
              adjlabel = new_clusters[x][y];
            }
          }

        }

        int count = 1;
        for(int c =0; c<count; c++){
          for(int k = 0; k < 4; k++){
            int x = elements[c].x + dx4[k];
            int y = elements[c].y + dy4[k];

            if(x >= 0 && x < image->width && y >= 0 && y < image->height){
              if(new_clusters[x][y] == -1 && clusters[i][j] == clusters[x][y]){
                  elements.push_back(cvPoint(x,y));
                  new_clusters[x][y] = label;
                  count += 1;
              }
            }
          }
        }
        if(count <= lims >> 2){
          for(int c = 0; c < count; c++){
            new_clusters[elements[c].x][elements[c].y] = adjlabel
          }
          label -= 1;
        }
        label += 1;

      }
    }
  }


}

//dispaly cluster centers
void Slic::display_center_grid(IplImage *image, CvScalar color){
  for(int i = 0; i < (int) centers.size(); i++){
    cvCircle(image,cvPoint(centers[i][3],centers[i][4]),2,color,2);
  }
}

void Slic::display_contours(IplImage *image, CvScalar color){
  const int dx8[8] = {-1,-1,0,1,1,1,0,-1};
  const int dy8[8] = {0, -1, -1,-1,0,1,1,1};

  vector<CvPoint> contours;
  vec2db istaken;
  for(int i = 0; i < image->width; i++){
    vector<bool> nb;
    for(int j = 0; j < image->height; j++){
      nb.push_back(false);
    }
    istaken.push_back(nb);
  }

  for(int k = 0; k < 8; k++){
    int x = i+ dx8[k],y = j + 
  }
}

void Slic::color_with_cluster_means(IplImage *image){

}
