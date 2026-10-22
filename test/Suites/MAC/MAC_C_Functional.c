#include "src/MAC/MultidimensionalArrayContainers.h"
#include "test/TSD/TSD.h"

TEST(MACFlatToMultidimensionalColumnMajor) {
  MAC   *Container = NULL;
  size_t Arr[3];

  Container = NewMAC(3, (size_t[]){1, 2, 3}, COLUMN_MAJOR);
  ASSERT(Container);

  MACFlatToMultidimensional(Container, 3, 0, Arr);
  ASSERT(Arr[0] == 0);
  ASSERT(Arr[1] == 0);
  ASSERT(Arr[2] == 0);
  MACFlatToMultidimensional(Container, 3, 1, Arr);
  ASSERT(Arr[0] == 0);
  ASSERT(Arr[1] == 1);
  ASSERT(Arr[2] == 0);
  MACFlatToMultidimensional(Container, 3, 2, Arr);
  ASSERT(Arr[0] == 0);
  ASSERT(Arr[1] == 0);
  ASSERT(Arr[2] == 1);
  MACFlatToMultidimensional(Container, 3, 3, Arr);
  ASSERT(Arr[0] == 0);
  ASSERT(Arr[1] == 1);
  ASSERT(Arr[2] == 1);
  MACFlatToMultidimensional(Container, 3, 4, Arr);
  ASSERT(Arr[0] == 0);
  ASSERT(Arr[1] == 0);
  ASSERT(Arr[2] == 2);
  MACFlatToMultidimensional(Container, 3, 5, Arr);
  ASSERT(Arr[0] == 0);
  ASSERT(Arr[1] == 1);
  ASSERT(Arr[2] == 2);

  FreeMAC(&Container);
  ASSERT(!Container);
}

TEST(MACFlatToMultidimensionalRowMajor) {
  MAC   *Container = NULL;
  size_t Arr[3];

  Container = NewMAC(3, (size_t[]){1, 2, 3}, ROW_MAJOR);
  ASSERT(Container);

  MACFlatToMultidimensional(Container, 3, 0, Arr);
  ASSERT(Arr[0] == 0);
  ASSERT(Arr[1] == 0);
  ASSERT(Arr[2] == 0);
  MACFlatToMultidimensional(Container, 3, 1, Arr);
  ASSERT(Arr[0] == 0);
  ASSERT(Arr[1] == 0);
  ASSERT(Arr[2] == 1);
  MACFlatToMultidimensional(Container, 3, 2, Arr);
  ASSERT(Arr[0] == 0);
  ASSERT(Arr[1] == 0);
  ASSERT(Arr[2] == 2);
  MACFlatToMultidimensional(Container, 3, 3, Arr);
  ASSERT(Arr[0] == 0);
  ASSERT(Arr[1] == 1);
  ASSERT(Arr[2] == 0);
  MACFlatToMultidimensional(Container, 3, 4, Arr);
  ASSERT(Arr[0] == 0);
  ASSERT(Arr[1] == 1);
  ASSERT(Arr[2] == 1);
  MACFlatToMultidimensional(Container, 3, 5, Arr);
  ASSERT(Arr[0] == 0);
  ASSERT(Arr[1] == 1);
  ASSERT(Arr[2] == 2);

  FreeMAC(&Container);
  ASSERT(!Container);
}

TEST_SUITE(MACFunctionalFlatToMultidimensionalSuite,
           MACFlatToMultidimensionalColumnMajor,
           MACFlatToMultidimensionalRowMajor);

TEST(MACMultidimensionalToFlatColumnMajor) {
  MAC   *Container = NULL;
  size_t Mem;

  Container = NewMAC(3, (size_t[]){1, 2, 3}, COLUMN_MAJOR);
  ASSERT(Container);

  MACMultidimensionalToFlat(Container, 3, (size_t[]){0, 0, 0}, &Mem);
  ASSERT(Mem == 0);
  MACMultidimensionalToFlat(Container, 3, (size_t[]){0, 1, 0}, &Mem);
  ASSERT(Mem == 1);
  MACMultidimensionalToFlat(Container, 3, (size_t[]){0, 0, 1}, &Mem);
  ASSERT(Mem == 2);
  MACMultidimensionalToFlat(Container, 3, (size_t[]){0, 1, 1}, &Mem);
  ASSERT(Mem == 3);
  MACMultidimensionalToFlat(Container, 3, (size_t[]){0, 0, 2}, &Mem);
  ASSERT(Mem == 4);
  MACMultidimensionalToFlat(Container, 3, (size_t[]){0, 1, 2}, &Mem);
  ASSERT(Mem == 5);

  FreeMAC(&Container);
  ASSERT(!Container);
}

TEST(MACMultidimensionalToFlatRowMajor) {
  MAC   *Container = NULL;
  size_t Mem;

  Container = NewMAC(3, (size_t[]){1, 2, 3}, ROW_MAJOR);
  ASSERT(Container);

  MACMultidimensionalToFlat(Container, 3, (size_t[]){0, 0, 0}, &Mem);
  ASSERT(Mem == 0);
  MACMultidimensionalToFlat(Container, 3, (size_t[]){0, 0, 1}, &Mem);
  ASSERT(Mem == 1);
  MACMultidimensionalToFlat(Container, 3, (size_t[]){0, 0, 2}, &Mem);
  ASSERT(Mem == 2);
  MACMultidimensionalToFlat(Container, 3, (size_t[]){0, 1, 0}, &Mem);
  ASSERT(Mem == 3);
  MACMultidimensionalToFlat(Container, 3, (size_t[]){0, 1, 1}, &Mem);
  ASSERT(Mem == 4);
  MACMultidimensionalToFlat(Container, 3, (size_t[]){0, 1, 2}, &Mem);
  ASSERT(Mem == 5);

  FreeMAC(&Container);
  ASSERT(!Container);
}

TEST_SUITE(MACFunctionalMultidimensionalToFlatSuite,
           MACMultidimensionalToFlatColumnMajor,
           MACMultidimensionalToFlatRowMajor);
