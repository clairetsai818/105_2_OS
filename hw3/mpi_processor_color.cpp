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

    image = imread(argv[1], CV_LOAD_IMAGE_COLOR);
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
        MPI_Send(data_neg, partial_h * width * 3, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
    } else {
        Mat img_neg(height, width, CV_8UC3);
        for (int p = 0;p < nproc;p++) {
            if (p != 0) {
                MPI_Recv(data_neg, partial_h * width * 3, MPI_UNSIGNED_CHAR, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            for (int i = 0;i < partial_h;i++) {
                for (int j = 0;j < width;j++) {
                    for (int k = 0;k < 3;k++) {
                        img_neg.at<Vec3b>(i + partial_h * p, j)[k] = data_neg[i * width * 3 + j * 3 + k];
                    }
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
        MPI_Send(data_rot, partial_h * width * 3, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
    } else {
        Mat img_rot(height, width, CV_8UC3);
        for (int p = 0;p < nproc;p++) {
            if (p != 0) {
                MPI_Recv(data_rot, partial_h * width * 3, MPI_UNSIGNED_CHAR, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            for (int i = 0;i < partial_h;i++) {
                for (int j = 0;j < width;j++) {
                    for (int k = 0;k < 3;k++) {
                        img_rot.at<Vec3b>(j, width - (i + partial_h * p))[k] = data_rot[i * width * 3 + j * 3 + k];
                    }
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
        MPI_Send(data_blur, partial_h * width * 3, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
    } else {
        Mat img_blur(height, width, CV_8UC3);
        for (int p = 0;p < nproc;p++) {
            if (p != 0) {
                MPI_Recv(data_blur, partial_h * width * 3, MPI_UNSIGNED_CHAR, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            for (int i = 0;i < partial_h;i++) {
                for (int j = 0;j < width;j++) {
                    for (int k = 0;k < 3;k++) {
                        img_blur.at<Vec3b>(i + partial_h * p, j)[k] = data_blur[i * width * 3 + j * 3 + k];
                    }
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
    unsigned char * data = new unsigned char [w * h * 3];
    for (int i = h_init;i < h_init + h;i++) {
        for (int j = 0;j < w;j++) {
            for (int k = 0;k < 3;k++) {
                data[(i - h_init) * w * 3 + j * 3 + k] = 255 - img.at<Vec3b>(i, j)[k];
            }
        }
    }
    return data;
}
unsigned char* rotate (Mat img, int h_init, int w, int h) {
    unsigned char * data = new unsigned char [w * h * 3];
    for (int i = h_init;i < h_init + h;i++) {
        for (int j = 0;j < w;j++) {
            for (int k = 0;k < 3;k++) {
                data[j * 3 + (i - h_init) * w * 3 + k] = img.at<Vec3b>(i, j)[k];
            }
        }
    }
    return data;
}
unsigned char* blur (Mat img, int h_init, int w, int h) {
	unsigned char* data = new unsigned char [w * h *3];
	for (int i = h_init;i < h_init + h;i++) {
        for (int j = 0;j < w;j++) {
			for (int k = 0;k < 3;k++) {
				int tmp = 0;
				for (int m = -1;m < 2;m++) {
					for (int n = -1;n < 2;n++) {
						int weight = 4 / ((abs(n) + 1) * (abs(m) + 1));
						int rec_h = min(h_init + h - 1, (max(h_init, i + m)));
						int rec_w = min(w - 1, (max(0, j + n)));
						tmp += img.at<Vec3b>(rec_h, rec_w)[k] * weight; 
					}
				}
				data[(i - h_init) * w * 3 + j * 3 + k] = tmp / 16;
			}
        }
    }
    return data;
}
