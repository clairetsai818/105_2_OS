#include <opencv2/opencv.hpp>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <mpi.h>
using namespace std;
using namespace cv;

unsigned char* negative (Mat img, int h_init, int w, int h);
unsigned char* rotate   (Mat img, int h_init, int w, int h);
unsigned char* blur   (Mat img, int h_init, int w, int h);

int main (int argc, char** argv) {
    int nproc;
    int my_rank;
    Mat image;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    if (argc != 2) {
        cout << "usage:" << endl;
        return -1;
    }

    image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    imwrite("origin.jpg", image);
    if (!image.data) {
        cout << "Could not open or find the image." << endl;
        return -1;
    }

    int height = image.rows;
    int width = image.cols;
    int partial_h = height / nproc;
    /*negative*/
    unsigned char* data_neg;
    data_neg = negative (image, my_rank * partial_h, width, partial_h);
    if (my_rank != 0) {
        MPI_Send(data_neg, partial_h * width, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
    } else {
        Mat img_neg(height, width, CV_8U);
        for (int p = 0;p < nproc;p++) {
            if (p != 0) {
                MPI_Recv(data_neg, partial_h * width, MPI_UNSIGNED_CHAR, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            for (int i = 0;i < partial_h;i++) {
                for (int j = 0;j < width;j++) {
                    img_neg.at<uchar>(i + partial_h * p, j) = data_neg[i * width + j];
                }
            }
        }
        imwrite("neg.jpg", img_neg);
    }
    delete [] data_neg;
    /*negative end*/
    /*rotate*/
    unsigned char* data_rot;
    data_rot = rotate(image, my_rank * partial_h, width, partial_h);
    if (my_rank != 0) {
        MPI_Send(data_rot, partial_h * width, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
    } else {
        Mat img_rot(height, width, CV_8U);
        for (int p = 0;p < nproc;p++) {
            if (p != 0) {
                MPI_Recv(data_rot, partial_h * width, MPI_UNSIGNED_CHAR, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            for (int i = 0;i < partial_h;i++) {
                for (int j = 0;j < width;j++) {
                    img_rot.at<uchar>(j, width - (i + partial_h * p)) = data_rot[i * width + j];
                }
            }
        }
        imwrite("rot.jpg", img_rot);
    }
	delete [] data_rot;
    /*rotate end*/
    /*blur*/
    unsigned char* data_blur;
    data_blur = blur (image, my_rank * partial_h, width, partial_h);
    if (my_rank != 0) {
        MPI_Send(data_blur, partial_h * width, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
    } else {
        Mat img_blur(height, width, CV_8U);
        for (int p = 0;p < nproc;p++) {
            if (p != 0) {
                MPI_Recv(data_blur, partial_h * width, MPI_UNSIGNED_CHAR, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            for (int i = 0;i < partial_h;i++) {
                for (int j = 0;j < width;j++) {
                    img_blur.at<uchar>(i + partial_h * p, j) = data_blur[i * width + j];
                }
            }
        }
        imwrite("blur.jpg", img_blur);
    }
    delete [] data_blur;
    /*blur end*/
    return 0;
}

unsigned char* negative (Mat img, int h_init, int w, int h) {
    unsigned char * data = new unsigned char [w * h];
    for (int i = h_init;i < h_init + h;i++) {
        for (int j = 0;j < w;j++) {
                data[(i - h_init) * w + j] = 255 - img.at<uchar>(i, j);
        }
    }
    return data;
}
unsigned char* rotate (Mat img, int h_init, int w, int h) {
    unsigned char * data = new unsigned char [w * h];
    for (int i = h_init;i < h_init + h;i++) {
        for (int j = 0;j < w;j++) {
            data[j + (i - h_init) * w] = img.at<uchar>(i, j);
        }
    }
    return data;
}
unsigned char* blur (Mat img, int h_init, int w, int h) {
	unsigned char* data = new unsigned char [w * h];
	for (int i = h_init;i < h_init + h;i++) {
        for (int j = 0;j < w;j++) {
				int tmp = 0;
				for (int m = -1;m < 2;m++) {
					for (int n = -1;n < 2;n++) {
						int weight = 4 / ((abs(n) + 1) * (abs(m) + 1));
						int rec_h = min(h_init + h - 1, (max(h_init, i + m)));
						int rec_w = min(w - 1, (max(0, j + n)));
						tmp += img.at<uchar>(rec_h, rec_w) * weight; 
					}
				}
				data[(i - h_init) * w + j] = tmp / 16;
			
        }
    }
    return data;
}
