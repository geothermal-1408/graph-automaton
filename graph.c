#include <assert.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define o 4

typedef struct
{
  float *items;
  size_t rows,cols;
}Mat;

typedef struct
{
  float *items;
  size_t count;
}Vec;

Mat mat_alloc(size_t rows, size_t cols)
{
  Mat m = {
    .rows = rows,
    .cols = cols,
  };

  m.items = malloc(sizeof(*m.items)*rows*cols);
  assert(m.items);
  return m;
}

#define MAT_AT(m,row,col) (m).items[(row)*(m).cols + (col)]

void mat_dump(Mat m)
{
  for(size_t row = 0; row < m.rows; ++row){
    for(size_t col = 0; col < m.cols; ++col) {
      float no = MAT_AT(m, row, col);
      if (no < 0) {
	printf(" --- ");
      } else {
	printf(" %.1f ",no);
      }
    }

    printf("\n");
  }
}

void vec_dump(Vec v)
{
  for(size_t i = 0; i < v.count; ++i){
    printf(" %f\n",v.items[i]);
  }
  //printf("\n");
}

Vec vec_alloc(size_t count)
{
  Vec v = {
    .count = count,
  };

  v.items = malloc(sizeof(*v.items)*count);
  assert(v.items);
  return v;
  
}

size_t mapped_big_to_small(size_t row_or_col, size_t index, size_t n)
{
  if (row_or_col < index) return row_or_col;
  if (row_or_col > index + n) return row_or_col - n;
  return index;
  
}

size_t mapped_small_to_big(size_t row_or_col, size_t index, size_t n)
{
  if (row_or_col < index) return row_or_col;
  if (row_or_col > index) return row_or_col + n;
  return index; 
}

void mat_vec_mul(Mat m, Vec v, Vec output)
{
  assert(m.cols == v.count);
  assert(v.count == output.count);
  for(size_t row = 0; row < m.rows; ++row) {
    for(size_t col = 0; col < m.cols; ++col) {
      output.items[row] += MAT_AT(m, row, col) * v.items[col];
    }
  }
}
 
/*
  0.0  1.0  0.0  0.0  1.0  1.0 
  1.0  0.0  1.0  1.0  0.0  0.0 
  0.0  1.0  0.0  1.0  1.0  0.0 
  0.0  1.0  1.0  0.0  0.0  1.0 
  1.0  0.0  1.0  0.0  0.0  1.0 
  1.0  0.0  0.0  1.0  1.0  0.0 
*/

void mat_cross(Mat bm, size_t row, size_t col)
{
  for(size_t i = 0; i < bm.cols; ++i) {
    if (MAT_AT(bm, row, i) < 0) MAT_AT(bm, row, i) = 0;
    if (MAT_AT(bm, i, col) < 0) MAT_AT(bm, i, col) = 0;
  }
}

void mat_expand(Mat *m, size_t idx, size_t n)
{
  assert(idx < m->rows);
  size_t n_row = m->rows+n;
  size_t n_col = m->cols+n;
  Mat bm = mat_alloc(n_row, n_col);
  
  for(size_t row = 0; row < n_row ; ++row){
    for(size_t col = 0; col < n_col; ++col){
      if ((idx <= row && row <= idx + n) || (idx <= col && col <= idx + n)) {
	MAT_AT(bm, row, col) = 0.f;
	continue;
	
      }
      size_t mrow = mapped_big_to_small(row, idx, n);
      size_t mcol = mapped_big_to_small(col, idx, n);
      MAT_AT(bm, row, col) = MAT_AT(*m, mrow, mcol);
     
    }
  }
  
  for(size_t row = idx; row <= idx+n; ++row){
    for(size_t col = idx; col <= idx+n; ++col){
      if (row == col) MAT_AT(bm, row, col) = 0.0f;
      else MAT_AT(bm, row, col) = 1.0f;
    }
  }

  size_t bcol = idx;
  for(size_t row = 0; row < m->rows; ++row){
    if(row != idx) {
      if(MAT_AT(*m, row, idx) == 1) {
	size_t brow = mapped_small_to_big(row, idx, n);
	MAT_AT(bm, brow, bcol) = 1;
	bcol += 1;
      }
    }
  }

  size_t brow = idx;
  for(size_t col = 0; col < m->cols; ++col){
    if(col != idx) {
      if(MAT_AT(*m, idx, col) == 1) {
	size_t bcol = mapped_small_to_big(col, idx, n);
	MAT_AT(bm, brow, bcol) = 1;
	brow += 1;
      }
    }
  }
  
  free(m->items);
  *m = bm;
}

void vec_scale(Vec v, float scaler)
{
  for(size_t i = 0; i < v.count; ++i) {
    v.items[i] *= scaler;
  }
}

void vec_copy(Vec dest, Vec src)
{
  assert(dest.count == src.count);
  for(size_t i = 0; i < src.count; ++i) {
    dest.items[i] = src.items[i];
  }
}

void vec_expand(Vec *v, size_t index, size_t d)
{
  assert(index < v->count);
  size_t count = v->count + d;
  float *items = malloc(sizeof(*items)*count);

  // memcpy(items, v->items, index*sizeof(*items));
  for(size_t i = 0; i < count; ++i) {
    size_t old_i = mapped_big_to_small(i, index, d);
    items[i] = v->items[old_i];
  }

  //  memcpy(items+index+d, v->items + 1, (v->count - index - 1)* sizeof(*items));
  free(v->items);
  v->items = items;
  v->count = count;
}

float R(float c)
{
  return c < 2;
}

float Rp(float c)
{
  return c >= 2;
}

void rule(Vec *s_ptr, Mat *a_ptr, int rule_no)
{
  Vec s = *s_ptr;
  Mat a = *a_ptr;
  size_t d = 3;
  Vec c = vec_alloc(s.count);
  
  vec_copy(c, s);
  vec_scale(c, d+1);
  mat_vec_mul(a, s, c);

  for(size_t i = 0; i < c.count; ++i) {
    /* s.items[i] = R(c.items[i]); */
    /* c.items[i] = Rp(c.items[i]); */
    int c_val = (int)(c.items[i]);
    /**
     * state_bit and repro_bit is taking too much time and i am not sure wheater they got struck or not
     * or maybe this is eexpected
     * So need some way to way to accelarate this operation
     **/
    int state_bit = (rule_no>>(2 * c_val + 1)) &1;
    int repro_bit = (rule_no>>(2 * c_val )) &1;

    s.items[i] = (float)state_bit;
    c.items[i] = (float)repro_bit;
  }

  Vec D = c;  
  
  for(size_t i = 0; i < D.count; ++i) {
    if(D.items[i] > 0) {
      D.items[i] = 0;
      vec_expand(&s, i, d-1);
      mat_expand(&a, i, d-1);
      vec_expand(&D, i, d-1);
    }
  }
  free(D.items);
  *s_ptr = s;
  *a_ptr = a;
}

void dump_graph(FILE *file, Vec s, Mat a)
{
  
  fprintf(file, "graph GRA{\n");
  fprintf(file, "    bgcolor=transparent\n");
  fprintf(file, "    edge[color=white]\n");
  fprintf(file, "    node[label=\"\", color=white, style=filled, shape=circle]\n");
  for(size_t i = 0; i < s.count; ++i) {
    if (s.items[i] > 0) 
      fprintf(file, "    %zu[fillcolor=purple]\n",i);
    else 
      fprintf(file, "    %zu[fillcolor=orange]\n",i);
  }
  for(size_t i = 0; i < a.rows; ++i) {
    for(size_t j = i+1; j < a.cols; ++j) {
      if (MAT_AT(a, i, j) > 0) {
	fprintf(file, "    %zu -- %zu\n",i,j);
      }
    }
  }
  
  fprintf(file, "\n}");
}

void print_progress(size_t current, size_t total)
{
  int bar_width = 40;
  float progress = (float)current/(float)total;
  int filled = (int)(progress * bar_width);

  printf("\r["); //cursor back to start
  for(int i = 0; i < bar_width; ++i) {
    if(i < filled) printf("█");
    else printf(" ");
  }
  printf("] %zu/%zu (%.0f%%)", current, total, progress * 100);
  fflush(stdout);
}

int main(void) 
{
  size_t rule_no =  549;
  size_t t = 2;
  
  Mat a = mat_alloc(o, o); // adjancecy matrix
  for(size_t r = 0; r < a.rows; ++r) {
    for(size_t c = 0; c < a.cols; ++c) {
      if (r == c) {
        MAT_AT(a, r, c) = 0.0f;
      } else {
        MAT_AT(a, r, c) = 1.0f;
      }
    }
  }
  
  Vec s = vec_alloc(o); // state vector
  for(size_t i = 0; i < s.count; ++i) {
    s.items[i] = 0.0f;
  }
  s.items[0] = 1.0f;
  
  char *base_file = "png_file";
  for(size_t c = 0; c <= t; ++c) {
   
    char png_file[50];
    snprintf(png_file, sizeof(png_file), "%s%zu.dot", base_file, c);
    FILE *f = fopen(png_file, "w");
    assert(f != NULL);
    dump_graph(f, s, a);
    fclose(f);  
    print_progress(c + 1, t + 1);
    if (c < t) rule(&s, &a, rule_no);
  }
  printf("\nDone! Generated %zu dot files.\n", t + 1);
  return 0;
 
#if 0
  InitWindow(800, 600, "force graph");
  SetTargetFPS(60);
  while(!WindowShouldClose()){
    BeginDrawing();
    ClearBackground(GetColor(0x181818FF));
    EndDrawing();
  }
  CloseWindow();
  #endif
  return 0;
}
