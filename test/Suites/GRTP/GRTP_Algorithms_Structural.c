#include "src/Core/Core.h"
#include "src/GRTP/GraphUtilities.h"
#include "test/CoreSupport/CoreSupport.h"
#include "test/TSD/TSD.h"

CONTEXT_FN(StructuralSetup) {
  (void)ContextData;
  Core_Malloc = FallibleMalloc;
  ResetFallibleMalloc();
  Core_MulOverflow = FallibleMulOverflow;
  ResetFallibleMulOverflow();
}
CONTEXT_FN(StructuralTeardown) {
  (void)ContextData;
  RESET_FN(Core_Malloc);
  RESET_FN(Core_MulOverflow);
}
CONTEXT(StructuralCtx,
        StructuralSetup,
        StructuralTeardown,
        (size_t)0);

TEST(FreeGraphWorkspace_BadInput) { FreeGraphWorkspace(NULL); }

TEST(FreeGraphWorkspace_Nominal) {
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT_X(Builder);
  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  Workspace = NewGraphWorkspace(Support);
  ASSERT(Workspace);
  FreeGraphWorkspace(&Workspace);
  ASSERT(!Workspace);

  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST(NewGraphWorkspace_BadInput) {
  GraphWorkspace *Workspace = NULL;

  Workspace = NewGraphWorkspace(NULL);
  ASSERT(!Workspace);
}

TEST_F(StructuralCtx,
       NewGraphWorkspace_MallocFail1) {
  (void)ContextData;
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT_X(Builder);
  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  SetFallibleMalloc(1);
  Workspace = NewGraphWorkspace(Support);
  ASSERT(!Workspace);

  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST_F(StructuralCtx,
       NewGraphWorkspace_MallocFail2) {
  (void)ContextData;
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT_X(Builder);
  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  SetFallibleMalloc(4);
  Workspace = NewGraphWorkspace(Support);
  ASSERT(!Workspace);

  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST_F(StructuralCtx,
       NewGraphWorkspace_MallocFail3) {
  (void)ContextData;
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT_X(Builder);
  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  SetFallibleMalloc(6);
  Workspace = NewGraphWorkspace(Support);
  ASSERT(!Workspace);

  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST_F(StructuralCtx,
       NewGraphWorkspace_MallocFail4) {
  (void)ContextData;
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT_X(Builder);
  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  SetFallibleMalloc(7);
  Workspace = NewGraphWorkspace(Support);
  ASSERT(!Workspace);

  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST_F(StructuralCtx,
       NewGraphWorkspace_HeapInstanceMallocFail) {
  (void)ContextData;
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT_X(Builder);
  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  SetFallibleMalloc(2);
  Workspace = NewGraphWorkspace(Support);
  ASSERT(!Workspace);

  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST_F(StructuralCtx,
       NewGraphWorkspace_HeapArrayMallocFail) {
  (void)ContextData;
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT_X(Builder);
  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  SetFallibleMalloc(3);
  Workspace = NewGraphWorkspace(Support);
  ASSERT(!Workspace);

  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST_F(StructuralCtx,
       NewGraphWorkspace_HeapPriorityMallocFail) {
  (void)ContextData;
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT_X(Builder);
  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  SetFallibleMalloc(5);
  Workspace = NewGraphWorkspace(Support);
  ASSERT(!Workspace);

  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST_F(StructuralCtx,
       NewGraphWorkspace_DistanceMulOverflowFail) {
  (void)ContextData;
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT_X(Builder);
  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  SetFallibleMulOverflow(3);
  Workspace = NewGraphWorkspace(Support);
  ASSERT(!Workspace);

  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST_F(StructuralCtx,
       NewGraphWorkspace_ParentMulOverflowFail) {
  (void)ContextData;
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;

  Builder = NewGraphSupportBuilder(1);
  ASSERT_X(Builder);
  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  SetFallibleMulOverflow(4);
  Workspace = NewGraphWorkspace(Support);
  ASSERT(!Workspace);

  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST(GraphWorkspaceGetDistance_BadInput) {
  ASSERT(!GraphWorkspaceGetDistance(NULL));
}

TEST(GraphWorkspaceGetParent_BadInput) {
  ASSERT(!GraphWorkspaceGetParent(NULL));
}

TEST(GraphDijkstra_BadInput) {
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;
  size_t               SourceCount[1];

  Builder = NewGraphSupportBuilder(1);
  ASSERT_X(Builder);
  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  Workspace = NewGraphWorkspace(Support);
  ASSERT(Workspace);

  ASSERT(!GraphDijkstra(NULL, 1, SourceCount, NULL, NULL));
  ASSERT(!GraphDijkstra(Workspace, 0, SourceCount, NULL, NULL));
  ASSERT(!GraphDijkstra(Workspace, 1, NULL, NULL, NULL));

  FreeGraphWorkspace(&Workspace);
  ASSERT(!Workspace);

  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST(GraphBidirectionalDijkstra_BadInput) {
  GraphWorkspace      *WorkspaceA = NULL;
  GraphWorkspace      *WorkspaceB = NULL;
  GraphSupport        *SupportA   = NULL;
  GraphSupport        *SupportB   = NULL;
  GraphSupportBuilder *BuilderA   = NULL;
  GraphSupportBuilder *BuilderB   = NULL;

  BuilderA = NewGraphSupportBuilder(1);
  ASSERT_X(BuilderA);
  SupportA = NewGraphSupport(BuilderA);
  ASSERT_X(SupportA);
  FreeGraphSupportBuilder(&BuilderA);
  ASSERT_X(!BuilderA);
  WorkspaceA = NewGraphWorkspace(SupportA);
  ASSERT(WorkspaceA);

  ASSERT(!GraphBidirectionalDijkstra(NULL, WorkspaceA, 0, 0, NULL, NULL, NULL,
                                     NULL, NULL));
  ASSERT(!GraphBidirectionalDijkstra(WorkspaceA, NULL, 0, 0, NULL, NULL, NULL,
                                     NULL, NULL));
  ASSERT(!GraphBidirectionalDijkstra(WorkspaceA, WorkspaceA, 0, 0, NULL, NULL,
                                     NULL, NULL, NULL));

  BuilderB = NewGraphSupportBuilder(2);
  ASSERT_X(BuilderB);

  SupportB = NewGraphSupport(BuilderB);
  ASSERT_X(SupportB);
  FreeGraphSupportBuilder(&BuilderB);
  ASSERT_X(!BuilderB);
  WorkspaceB = NewGraphWorkspace(SupportB);
  ASSERT(WorkspaceB);

  ASSERT(!GraphBidirectionalDijkstra(WorkspaceA, WorkspaceB, 0, 0, NULL, NULL,
                                     NULL, NULL, NULL));

  FreeGraphWorkspace(&WorkspaceB);
  ASSERT(!WorkspaceB);
  FreeGraphSupport(&SupportB);
  ASSERT_X(!SupportB);
  BuilderB = NewGraphSupportBuilder(1);
  ASSERT_X(BuilderB);
  ASSERT_X(GraphSupportBuilderAddEdge(BuilderB, 0, 0));
  SupportB = NewGraphSupport(BuilderB);
  ASSERT_X(SupportB);
  FreeGraphSupportBuilder(&BuilderB);
  ASSERT_X(!BuilderB);
  WorkspaceB = NewGraphWorkspace(SupportB);
  ASSERT(WorkspaceB);

  ASSERT(!GraphBidirectionalDijkstra(WorkspaceA, WorkspaceB, 0, 0, NULL, NULL,
                                     NULL, NULL, NULL));

  FreeGraphWorkspace(&WorkspaceA);
  ASSERT(!WorkspaceA);
  FreeGraphWorkspace(&WorkspaceB);
  ASSERT(!WorkspaceB);

  FreeGraphSupport(&SupportA);
  ASSERT_X(!SupportA);
  FreeGraphSupport(&SupportB);
  ASSERT_X(!SupportB);
}

TEST(GraphBellmanFord_BadInput) {
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;
  size_t               SourceCount[1];

  Builder = NewGraphSupportBuilder(1);
  ASSERT_X(Builder);
  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  Workspace = NewGraphWorkspace(Support);
  ASSERT(Workspace);

  ASSERT(!GraphBellmanFord(NULL, 1, SourceCount, NULL, NULL, NULL));
  ASSERT(!GraphBellmanFord(Workspace, 0, SourceCount, NULL, NULL, NULL));
  ASSERT(!GraphBellmanFord(Workspace, 1, NULL, NULL, NULL, NULL));

  FreeGraphWorkspace(&Workspace);
  ASSERT(!Workspace);

  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST_SUITE(GraphWorkspaceContractSuite,
           FreeGraphWorkspace_BadInput,
           FreeGraphWorkspace_Nominal,
           NewGraphWorkspace_BadInput,
           GraphWorkspaceGetDistance_BadInput,
           GraphWorkspaceGetParent_BadInput,
           GraphDijkstra_BadInput,
           GraphBidirectionalDijkstra_BadInput,
           GraphBellmanFord_BadInput);

TEST_SUITE(GraphWorkspaceAllocationSuite,
           NewGraphWorkspace_MallocFail1,
           NewGraphWorkspace_MallocFail2,
           NewGraphWorkspace_MallocFail3,
           NewGraphWorkspace_MallocFail4,
           NewGraphWorkspace_HeapInstanceMallocFail,
           NewGraphWorkspace_HeapArrayMallocFail,
           NewGraphWorkspace_HeapPriorityMallocFail,
           NewGraphWorkspace_DistanceMulOverflowFail,
           NewGraphWorkspace_ParentMulOverflowFail);
