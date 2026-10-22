#include "src/Core/Core.h"
#include "src/GRTP/GraphUtilities.h"
#include "test/CoreSupport/CoreSupport.h"
#include "test/TSD/TSD.h"

CONTEXT_FN(StructuralSetup) {
  (void)ContextData;
  Core_Malloc = FallibleMalloc;
  ResetFallibleMalloc();
  Core_Realloc = FallibleRealloc;
  ResetFallibleRealloc();
  Core_AddOverflow = FallibleAddOverflow;
  ResetFallibleAddOverflow();
  Core_MulOverflow = FallibleMulOverflow;
  ResetFallibleMulOverflow();
}
CONTEXT_FN(StructuralTeardown) {
  (void)ContextData;
  RESET_FN(Core_Malloc);
  RESET_FN(Core_Realloc);
  RESET_FN(Core_AddOverflow);
  RESET_FN(Core_MulOverflow);
}
CONTEXT(StructuralCtx,
        StructuralSetup,
        StructuralTeardown,
        (size_t)0);

TEST(FreeGraphSupportBuilder_BadInput) { FreeGraphSupportBuilder(NULL); }

TEST(FreeGraphSupportBuilder_Nominal) {
  GraphSupportBuilder *Builder = NULL;

  Builder = NewGraphSupportBuilder(2);
  ASSERT(Builder);
  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST(NewGraphSupportBuilder_BadInput) {
  GraphSupportBuilder *Builder = NULL;

  Builder = NewGraphSupportBuilder(0);
  ASSERT(!Builder);
}

TEST_F(StructuralCtx,
       NewGraphSupportBuilder_MallocFail1) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;

  SetFallibleMalloc(1);
  Builder = NewGraphSupportBuilder(1);
  ASSERT(!Builder);
}

TEST_F(StructuralCtx,
       NewGraphSupportBuilder_MallocFail2) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;

  SetFallibleMalloc(2);
  Builder = NewGraphSupportBuilder(1);
  ASSERT(!Builder);
}

TEST_F(StructuralCtx,
       NewGraphSupportBuilder_MallocFail3) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;

  SetFallibleMalloc(3);
  Builder = NewGraphSupportBuilder(1);
  ASSERT(!Builder);
}

TEST(GraphSupportBuilderAddEdge_BadInput) {
  GraphSupportBuilder *Builder = NULL;

  ASSERT(!GraphSupportBuilderAddEdge(Builder, 0, 0));

  Builder = NewGraphSupportBuilder(1);
  ASSERT(Builder);

  ASSERT(!GraphSupportBuilderAddEdge(Builder, 1, 0));
  ASSERT(!GraphSupportBuilderAddEdge(Builder, 0, 1));

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST_F(StructuralCtx,
       GraphSupportBuilderAddEdge_ReallocFail) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT(Builder);

  SetFallibleRealloc(1);
  ASSERT(!GraphSupportBuilderAddEdge(Builder, 0, 0));

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST(GraphSupportBuilderRemoveEdge_BadInput) {
  GraphSupportBuilder *Builder = NULL;

  ASSERT(!GraphSupportBuilderRemoveEdge(Builder, 0, 0));

  Builder = NewGraphSupportBuilder(1);
  ASSERT(Builder);

  ASSERT(!GraphSupportBuilderRemoveEdge(Builder, 1, 0));
  ASSERT(!GraphSupportBuilderRemoveEdge(Builder, 0, 1));

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST_F(StructuralCtx,
       GraphSupportBuilderRemoveEdge_ReallocFail) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;

  Builder = NewGraphSupportBuilder(2);
  ASSERT(Builder);
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));

  SetFallibleRealloc(1);
  ASSERT(!GraphSupportBuilderRemoveEdge(Builder, 0, 0));

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST(GraphSupportBuilderGetVertexCount_BadInput) {
  ASSERT(GraphSupportBuilderGetVertexCount(NULL) == 0);
}

TEST(GraphSupportBuilderGetEdgeCount_BadInput) {
  ASSERT(GraphSupportBuilderGetEdgeCount(NULL) == 0);
}

TEST(GraphSupportBuilderGetNeighborCount_BadInput) {
  ASSERT(GraphSupportBuilderGetNeighborCount(NULL, 0) == 0);
}

TEST(GraphSupportBuilderGetNeighbors_BadInput) {
  ASSERT(!GraphSupportBuilderGetNeighbors(NULL, 0));
}

TEST(GraphSupportBuilderHasEdge_BadInput) {
  GraphSupportBuilder *Builder = NULL;

  ASSERT(!GraphSupportBuilderHasEdge(Builder, 0, 0));

  Builder = NewGraphSupportBuilder(1);
  ASSERT(Builder);

  ASSERT(!GraphSupportBuilderHasEdge(Builder, 1, 0));
  ASSERT(!GraphSupportBuilderHasEdge(Builder, 0, 1));

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST(GraphSupportBuilderGetNeighborCount_OutOfRange) {
  GraphSupportBuilder *Builder = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT(Builder);

  ASSERT(GraphSupportBuilderGetNeighborCount(Builder, 1) == 0);
  ASSERT(GraphSupportBuilderGetNeighborCount(Builder, 100) == 0);

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST(GraphSupportBuilderGetNeighbors_OutOfRange) {
  GraphSupportBuilder *Builder = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT(Builder);

  ASSERT(!GraphSupportBuilderGetNeighbors(Builder, 1));
  ASSERT(!GraphSupportBuilderGetNeighbors(Builder, 100));

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST(GraphSupportBuilderHasEdge_OutOfRange) {
  GraphSupportBuilder *Builder = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT(Builder);

  ASSERT(!GraphSupportBuilderHasEdge(Builder, 1, 0));
  ASSERT(!GraphSupportBuilderHasEdge(Builder, 0, 1));
  ASSERT(!GraphSupportBuilderHasEdge(Builder, 100, 0));
  ASSERT(!GraphSupportBuilderHasEdge(Builder, 0, 100));

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST_F(StructuralCtx,
       NewGraphSupportBuilder_MulOverflowFail1) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;

  SetFallibleMulOverflow(1);
  Builder = NewGraphSupportBuilder(1);
  ASSERT(!Builder);
}

TEST_F(StructuralCtx,
       NewGraphSupportBuilder_MulOverflowFail2) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;

  SetFallibleMulOverflow(2);
  Builder = NewGraphSupportBuilder(1);
  ASSERT(!Builder);
}

TEST_F(StructuralCtx,
       GraphSupportBuilderAddEdge_AddOverflowFail) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT(Builder);

  SetFallibleAddOverflow(1);
  ASSERT(!GraphSupportBuilderAddEdge(Builder, 0, 0));
  ASSERT(GraphSupportBuilderGetEdgeCount(Builder) == 0);

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST_F(StructuralCtx,
       GraphSupportBuilderAddEdge_MulOverflowFail) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT(Builder);

  SetFallibleMulOverflow(1);
  ASSERT(!GraphSupportBuilderAddEdge(Builder, 0, 0));
  ASSERT(GraphSupportBuilderGetEdgeCount(Builder) == 0);

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST_F(StructuralCtx,
       GraphSupportBuilderAddEdge_ReallocFailPreservesState) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT(Builder);

  SetFallibleRealloc(1);
  ASSERT(!GraphSupportBuilderAddEdge(Builder, 0, 0));

  ASSERT(GraphSupportBuilderGetEdgeCount(Builder) == 0);
  ASSERT(GraphSupportBuilderGetNeighborCount(Builder, 0) == 0);
  ASSERT(!GraphSupportBuilderHasEdge(Builder, 0, 0));

  ASSERT(GraphSupportBuilderAddEdge(Builder, 0, 0));
  ASSERT(GraphSupportBuilderHasEdge(Builder, 0, 0));

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST_F(StructuralCtx,
       GraphSupportBuilderRemoveEdge_ReallocFailPreservesState) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;

  Builder = NewGraphSupportBuilder(2);
  ASSERT(Builder);
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));

  SetFallibleRealloc(1);
  ASSERT(!GraphSupportBuilderRemoveEdge(Builder, 0, 0));

  ASSERT(GraphSupportBuilderGetEdgeCount(Builder) == 2);
  ASSERT(GraphSupportBuilderGetNeighborCount(Builder, 0) == 2);
  ASSERT(GraphSupportBuilderHasEdge(Builder, 0, 0));
  ASSERT(GraphSupportBuilderHasEdge(Builder, 0, 1));

  ASSERT(GraphSupportBuilderRemoveEdge(Builder, 0, 0));
  ASSERT(!GraphSupportBuilderHasEdge(Builder, 0, 0));
  ASSERT(GraphSupportBuilderHasEdge(Builder, 0, 1));

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST_F(StructuralCtx,
       GraphSupportBuilderGetEdgeCount_AddOverflowFail) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;

  Builder = NewGraphSupportBuilder(2);
  ASSERT(Builder);
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 1));

  SetFallibleAddOverflow(1);
  ASSERT(GraphSupportBuilderGetEdgeCount(Builder) == 0);

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST_SUITE(GraphSupportBuilderContractSuite,
           FreeGraphSupportBuilder_BadInput,
           FreeGraphSupportBuilder_Nominal,
           NewGraphSupportBuilder_BadInput,
           GraphSupportBuilderAddEdge_BadInput,
           GraphSupportBuilderRemoveEdge_BadInput,
           GraphSupportBuilderGetVertexCount_BadInput,
           GraphSupportBuilderGetEdgeCount_BadInput,
           GraphSupportBuilderGetNeighborCount_BadInput,
           GraphSupportBuilderGetNeighbors_BadInput,
           GraphSupportBuilderHasEdge_BadInput,
           GraphSupportBuilderGetNeighborCount_OutOfRange,
           GraphSupportBuilderGetNeighbors_OutOfRange,
           GraphSupportBuilderHasEdge_OutOfRange);

TEST_SUITE(GraphSupportBuilderAllocationSuite,
           NewGraphSupportBuilder_MallocFail1,
           NewGraphSupportBuilder_MallocFail2,
           NewGraphSupportBuilder_MallocFail3,
           NewGraphSupportBuilder_MulOverflowFail1,
           NewGraphSupportBuilder_MulOverflowFail2,
           GraphSupportBuilderAddEdge_ReallocFail,
           GraphSupportBuilderRemoveEdge_ReallocFail,
           GraphSupportBuilderAddEdge_AddOverflowFail,
           GraphSupportBuilderAddEdge_MulOverflowFail,
           GraphSupportBuilderAddEdge_ReallocFailPreservesState,
           GraphSupportBuilderRemoveEdge_ReallocFailPreservesState,
           GraphSupportBuilderGetEdgeCount_AddOverflowFail);

TEST(FreeGraphSupport_BadInput) { FreeGraphSupport(NULL); }

TEST(FreeGraphSupport_Nominal) {
  GraphSupportBuilder *Builder = NULL;
  GraphSupport        *Support = NULL;

  Builder = NewGraphSupportBuilder(2);
  ASSERT_X(Builder);
  Support = NewGraphSupport(Builder);
  ASSERT(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);
  FreeGraphSupport(&Support);
  ASSERT(!Support);
}

TEST(NewGraphSupport_BadInput) {
  GraphSupport *Support = NULL;

  Support = NewGraphSupport(NULL);
  ASSERT(!Support);
}

TEST_F(StructuralCtx,
       NewGraphSupport_MallocFail1) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;
  GraphSupport        *Support = NULL;

  Builder = NewGraphSupportBuilder(2);
  ASSERT_X(Builder);
  SetFallibleMalloc(1);
  Support = NewGraphSupport(Builder);
  ASSERT(!Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);
}

TEST_F(StructuralCtx,
       NewGraphSupport_MallocFail2) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;
  GraphSupport        *Support = NULL;

  Builder = NewGraphSupportBuilder(2);
  ASSERT_X(Builder);
  SetFallibleMalloc(2);
  Support = NewGraphSupport(Builder);
  ASSERT(!Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);
}

TEST_F(StructuralCtx,
       NewGraphSupport_MallocFail3) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;
  GraphSupport        *Support = NULL;

  Builder = NewGraphSupportBuilder(2);
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 0));
  ASSERT_X(Builder);
  SetFallibleMalloc(3);
  Support = NewGraphSupport(Builder);
  ASSERT(!Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);
}

TEST(GraphSupportGetVertexCount_BadInput) {
  ASSERT(GraphSupportGetVertexCount(NULL) == 0);
}

TEST(GraphSupportGetEdgeCount_BadInput) {
  ASSERT(GraphSupportGetEdgeCount(NULL) == 0);
}

TEST(GraphSupportGetNeighborCount_BadInput) {
  ASSERT(GraphSupportGetNeighborCount(NULL, 0) == 0);
}

TEST(GraphSupportGetNeighbors_BadInput) {
  ASSERT(!GraphSupportGetNeighbors(NULL, 0));
}

TEST(GraphSupportGetEdgeIndex_BadInput) {
  size_t               EdgeIndex;
  GraphSupportBuilder *Builder = NULL;
  GraphSupport        *Support = NULL;

  Builder = NewGraphSupportBuilder(2);
  ASSERT_X(Builder);
  Support = NewGraphSupport(Builder);
  ASSERT(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);
  ASSERT(!GraphSupportGetEdgeIndex(NULL, 0, 0, &EdgeIndex));
  ASSERT(!GraphSupportGetEdgeIndex(Support, 0, 0, NULL));
  FreeGraphSupport(&Support);
  ASSERT(!Support);
}

TEST(GraphSupportGetEdgeVertices_BadInput) {
  size_t               Origin, Destination;
  GraphSupportBuilder *Builder = NULL;
  GraphSupport        *Support = NULL;

  Builder = NewGraphSupportBuilder(2);
  ASSERT_X(Builder);
  Support = NewGraphSupport(Builder);
  ASSERT(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);
  ASSERT(!GraphSupportGetEdgeVertices(NULL, 0, &Origin, &Destination));
  ASSERT(!GraphSupportGetEdgeVertices(Support, 0, NULL, &Destination));
  ASSERT(!GraphSupportGetEdgeVertices(Support, 0, &Origin, NULL));
  FreeGraphSupport(&Support);
  ASSERT(!Support);
}

TEST(GraphSupportHasEdge_BadInput) { ASSERT(!GraphSupportHasEdge(NULL, 0, 0)); }

TEST_F(StructuralCtx,
       NewGraphSupport_AddOverflowFail1) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;
  GraphSupport        *Support = NULL;

  Builder = NewGraphSupportBuilder(2);
  ASSERT_X(Builder);
  SetFallibleAddOverflow(1);
  Support = NewGraphSupport(Builder);
  ASSERT(!Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);
}

TEST_F(StructuralCtx,
       NewGraphSupport_MulOverflowFail1) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;
  GraphSupport        *Support = NULL;

  Builder = NewGraphSupportBuilder(2);
  ASSERT_X(Builder);
  SetFallibleMulOverflow(1);
  Support = NewGraphSupport(Builder);
  ASSERT(!Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);
}

TEST_F(StructuralCtx,
       NewGraphSupport_AddOverflowFail2) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;
  GraphSupport        *Support = NULL;

  Builder = NewGraphSupportBuilder(2);
  ASSERT_X(Builder);
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 0));
  SetFallibleAddOverflow(2);
  Support = NewGraphSupport(Builder);
  ASSERT(!Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);
}

TEST_F(StructuralCtx,
       NewGraphSupport_MulOverflowFail2) {
  (void)ContextData;
  GraphSupportBuilder *Builder = NULL;
  GraphSupport        *Support = NULL;

  Builder = NewGraphSupportBuilder(2);
  ASSERT_X(Builder);
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 0));
  SetFallibleMulOverflow(2);
  Support = NewGraphSupport(Builder);
  ASSERT(!Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);
}

TEST_SUITE(GraphSupportContractSuite,
           FreeGraphSupport_BadInput,
           FreeGraphSupport_Nominal,
           NewGraphSupport_BadInput,
           GraphSupportGetVertexCount_BadInput,
           GraphSupportGetEdgeCount_BadInput,
           GraphSupportGetNeighborCount_BadInput,
           GraphSupportGetNeighbors_BadInput,
           GraphSupportGetEdgeIndex_BadInput,
           GraphSupportGetEdgeVertices_BadInput,
           GraphSupportHasEdge_BadInput);

TEST_SUITE(GraphSupportAllocationSuite,
           NewGraphSupport_MallocFail1,
           NewGraphSupport_MallocFail2,
           NewGraphSupport_MallocFail3,
           NewGraphSupport_AddOverflowFail1,
           NewGraphSupport_MulOverflowFail1,
           NewGraphSupport_AddOverflowFail2,
           NewGraphSupport_MulOverflowFail2);
