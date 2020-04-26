#ifndef SM64_MATH_H
#define SM64_MATH_H

struct Mat4f {
  float data[4][4];

  void print() {
    for(int i = 0; i < 4; i++) {
      printf("%6.3f %6.3f %6.3f %6.3f\n",
        data[i][0], data[i][1], data[i][2], data[i][3]);
    }
  }
};


inline void matmul(Mat4f* dest, Mat4f& a, Mat4f& b) {
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      (*dest).data[i][j] = 0;
      for(int k = 0; k < 4; k++) {
        (*dest).data[i][j] += a.data[i][k] * b.data[k][j];
      }
    }
  }
}

#endif //SM64_MATH_H
