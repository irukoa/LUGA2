#include "src/GRTP/GraphUtilities.h"
#include "test/TSD/TSD.h"

typedef struct {
  size_t   VertexCount;
  size_t  *VertexEdgeCount; // VertexEdgeCount[i] = N_i. Number of connections
                            // from vertex #i = [0, VertexCount - 1].
  size_t **VertexConnectivity; // VertexConnectivity[i][j] = V_ij. The vertex
                               // that vertex #i connects to with connection
                               // label #j = [0, N_i - 1].
} GraphSupportBuilderClone;

TEST(GRTPSupportBuilder_New) {
  GraphSupportBuilder *Builder = NewGraphSupportBuilder(2);
  ASSERT(Builder);
  GraphSupportBuilderClone *Clone = (GraphSupportBuilderClone *)Builder;

  ASSERT(Clone->VertexCount == 2);
  ASSERT(Clone->VertexEdgeCount[0] == 0);
  ASSERT(Clone->VertexEdgeCount[1] == 0);
  ASSERT(Clone->VertexConnectivity[0] == NULL);
  ASSERT(Clone->VertexConnectivity[1] == NULL);

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST(GRTPSupportBuilder_AddEdge1) {
  GraphSupportBuilder *Builder = NewGraphSupportBuilder(2);
  ASSERT(Builder);
  GraphSupportBuilderClone *Clone = (GraphSupportBuilderClone *)Builder;

  ASSERT(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT(Clone->VertexCount == 2);
  ASSERT(Clone->VertexEdgeCount[0] == 1);
  ASSERT(Clone->VertexEdgeCount[1] == 0);
  ASSERT(Clone->VertexConnectivity[0] != NULL);
  ASSERT(Clone->VertexConnectivity[1] == NULL);
  ASSERT(Clone->VertexConnectivity[0][0] == 1);

  ASSERT(!GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT(Clone->VertexCount == 2);
  ASSERT(Clone->VertexEdgeCount[0] == 1);
  ASSERT(Clone->VertexEdgeCount[1] == 0);
  ASSERT(Clone->VertexConnectivity[0] != NULL);
  ASSERT(Clone->VertexConnectivity[1] == NULL);
  ASSERT(Clone->VertexConnectivity[0][0] == 1);

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST(GRTPSupportBuilder_AddEdge2) {
  GraphSupportBuilder *Builder = NewGraphSupportBuilder(2);
  ASSERT(Builder);
  GraphSupportBuilderClone *Clone = (GraphSupportBuilderClone *)Builder;

  ASSERT(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT(Clone->VertexCount == 2);
  ASSERT(Clone->VertexEdgeCount[0] == 1);
  ASSERT(Clone->VertexEdgeCount[1] == 0);
  ASSERT(Clone->VertexConnectivity[0] != NULL);
  ASSERT(Clone->VertexConnectivity[1] == NULL);
  ASSERT(Clone->VertexConnectivity[0][0] == 1);

  ASSERT(GraphSupportBuilderAddEdge(Builder, 0, 0));
  ASSERT(Clone->VertexCount == 2);
  ASSERT(Clone->VertexEdgeCount[0] == 2);
  ASSERT(Clone->VertexEdgeCount[1] == 0);
  ASSERT(Clone->VertexConnectivity[0] != NULL);
  ASSERT(Clone->VertexConnectivity[1] == NULL);
  ASSERT(Clone->VertexConnectivity[0][0] == 1);
  ASSERT(Clone->VertexConnectivity[0][1] == 0);

  ASSERT(!GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT(Clone->VertexCount == 2);
  ASSERT(Clone->VertexEdgeCount[0] == 2);
  ASSERT(Clone->VertexEdgeCount[1] == 0);
  ASSERT(Clone->VertexConnectivity[0] != NULL);
  ASSERT(Clone->VertexConnectivity[1] == NULL);
  ASSERT(Clone->VertexConnectivity[0][0] == 1);
  ASSERT(Clone->VertexConnectivity[0][1] == 0);

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST(GRTPSupportBuilder_RemoveEdge1) {
  GraphSupportBuilder *Builder = NewGraphSupportBuilder(2);
  ASSERT(Builder);
  GraphSupportBuilderClone *Clone = (GraphSupportBuilderClone *)Builder;

  ASSERT(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT(Clone->VertexCount == 2);
  ASSERT(Clone->VertexEdgeCount[0] == 1);
  ASSERT(Clone->VertexEdgeCount[1] == 0);
  ASSERT(Clone->VertexConnectivity[0] != NULL);
  ASSERT(Clone->VertexConnectivity[1] == NULL);
  ASSERT(Clone->VertexConnectivity[0][0] == 1);

  ASSERT(GraphSupportBuilderRemoveEdge(Builder, 0, 1));
  ASSERT(Clone->VertexCount == 2);
  ASSERT(Clone->VertexEdgeCount[0] == 0);
  ASSERT(Clone->VertexEdgeCount[1] == 0);
  ASSERT(Clone->VertexConnectivity[0] == NULL);
  ASSERT(Clone->VertexConnectivity[1] == NULL);

  ASSERT(!GraphSupportBuilderRemoveEdge(Builder, 0, 1));
  ASSERT(Clone->VertexCount == 2);
  ASSERT(Clone->VertexEdgeCount[0] == 0);
  ASSERT(Clone->VertexEdgeCount[1] == 0);
  ASSERT(Clone->VertexConnectivity[0] == NULL);
  ASSERT(Clone->VertexConnectivity[1] == NULL);

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST(GRTPSupportBuilder_RemoveEdge2) {
  GraphSupportBuilder *Builder = NewGraphSupportBuilder(2);
  ASSERT(Builder);
  GraphSupportBuilderClone *Clone = (GraphSupportBuilderClone *)Builder;

  ASSERT(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT(Clone->VertexCount == 2);
  ASSERT(Clone->VertexEdgeCount[0] == 1);
  ASSERT(Clone->VertexEdgeCount[1] == 0);
  ASSERT(Clone->VertexConnectivity[0] != NULL);
  ASSERT(Clone->VertexConnectivity[1] == NULL);
  ASSERT(Clone->VertexConnectivity[0][0] == 1);

  ASSERT(GraphSupportBuilderAddEdge(Builder, 0, 0));
  ASSERT(Clone->VertexCount == 2);
  ASSERT(Clone->VertexEdgeCount[0] == 2);
  ASSERT(Clone->VertexEdgeCount[1] == 0);
  ASSERT(Clone->VertexConnectivity[0] != NULL);
  ASSERT(Clone->VertexConnectivity[1] == NULL);
  ASSERT(Clone->VertexConnectivity[0][0] == 1);
  ASSERT(Clone->VertexConnectivity[0][1] == 0);

  ASSERT(GraphSupportBuilderRemoveEdge(Builder, 0, 0));
  ASSERT(Clone->VertexCount == 2);
  ASSERT(Clone->VertexEdgeCount[0] == 1);
  ASSERT(Clone->VertexEdgeCount[1] == 0);
  ASSERT(Clone->VertexConnectivity[0] != NULL);
  ASSERT(Clone->VertexConnectivity[1] == NULL);
  ASSERT(Clone->VertexConnectivity[0][0] == 1);

  ASSERT(!GraphSupportBuilderRemoveEdge(Builder, 0, 0));
  ASSERT(Clone->VertexCount == 2);
  ASSERT(Clone->VertexEdgeCount[0] == 1);
  ASSERT(Clone->VertexEdgeCount[1] == 0);
  ASSERT(Clone->VertexConnectivity[0] != NULL);
  ASSERT(Clone->VertexConnectivity[1] == NULL);
  ASSERT(Clone->VertexConnectivity[0][0] == 1);

  ASSERT(GraphSupportBuilderRemoveEdge(Builder, 0, 1));
  ASSERT(Clone->VertexCount == 2);
  ASSERT(Clone->VertexEdgeCount[0] == 0);
  ASSERT(Clone->VertexEdgeCount[1] == 0);
  ASSERT(Clone->VertexConnectivity[0] == NULL);
  ASSERT(Clone->VertexConnectivity[1] == NULL);

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST(GRTPSupportBuilder_GetVertexCount) {
  GraphSupportBuilder *Builder = NewGraphSupportBuilder(3);
  ASSERT(Builder);

  ASSERT(GraphSupportBuilderGetVertexCount(Builder) == 3);

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST(GRTPSupportBuilder_GetEdgeCount) {
  GraphSupportBuilder *Builder = NewGraphSupportBuilder(2);
  ASSERT(Builder);

  ASSERT(GraphSupportBuilderGetEdgeCount(Builder) == 0);

  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT(GraphSupportBuilderGetEdgeCount(Builder) == 1);

  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 0));
  ASSERT(GraphSupportBuilderGetEdgeCount(Builder) == 2);

  ASSERT_X(GraphSupportBuilderRemoveEdge(Builder, 0, 1));
  ASSERT(GraphSupportBuilderGetEdgeCount(Builder) == 1);

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST(GRTPSupportBuilder_GetNeighborCount) {
  GraphSupportBuilder *Builder = NewGraphSupportBuilder(2);
  ASSERT(Builder);

  ASSERT(GraphSupportBuilderGetNeighborCount(Builder, 0) == 0);
  ASSERT(GraphSupportBuilderGetNeighborCount(Builder, 1) == 0);

  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT(GraphSupportBuilderGetNeighborCount(Builder, 0) == 1);
  ASSERT(GraphSupportBuilderGetNeighborCount(Builder, 1) == 0);

  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 0));
  ASSERT(GraphSupportBuilderGetNeighborCount(Builder, 0) == 2);
  ASSERT(GraphSupportBuilderGetNeighborCount(Builder, 1) == 0);
  ASSERT(GraphSupportBuilderGetNeighborCount(Builder, 2) == 0);

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST(GRTPSupportBuilder_GetNeighbors) {
  GraphSupportBuilder *Builder = NewGraphSupportBuilder(2);
  ASSERT(Builder);
  const size_t *Neighbors = NULL;

  Neighbors = (const size_t *)GraphSupportBuilderGetNeighbors(Builder, 0);
  ASSERT(!Neighbors);

  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  Neighbors = (const size_t *)GraphSupportBuilderGetNeighbors(Builder, 0);
  ASSERT(Neighbors);
  ASSERT(Neighbors[0] == 1);

  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 0));
  Neighbors = (const size_t *)GraphSupportBuilderGetNeighbors(Builder, 0);
  ASSERT(Neighbors);
  ASSERT(Neighbors[0] == 1);
  ASSERT(Neighbors[1] == 0);

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST(GRTPSupportBuilder_HasEdge) {
  GraphSupportBuilder *Builder = NewGraphSupportBuilder(2);
  ASSERT(Builder);

  ASSERT(!GraphSupportBuilderHasEdge(Builder, 0, 1));
  ASSERT(!GraphSupportBuilderHasEdge(Builder, 1, 0));

  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT(GraphSupportBuilderHasEdge(Builder, 0, 1));
  ASSERT(!GraphSupportBuilderHasEdge(Builder, 1, 0));

  ASSERT_X(GraphSupportBuilderRemoveEdge(Builder, 0, 1));
  ASSERT(!GraphSupportBuilderHasEdge(Builder, 0, 1));

  FreeGraphSupportBuilder(&Builder);
  ASSERT(!Builder);
}

TEST_SUITE(GRTPSupportBuilderSuite,
           GRTPSupportBuilder_New,
           GRTPSupportBuilder_AddEdge1,
           GRTPSupportBuilder_AddEdge2,
           GRTPSupportBuilder_RemoveEdge1,
           GRTPSupportBuilder_RemoveEdge2,
           GRTPSupportBuilder_GetVertexCount,
           GRTPSupportBuilder_GetEdgeCount,
           GRTPSupportBuilder_GetNeighborCount,
           GRTPSupportBuilder_GetNeighbors,
           GRTPSupportBuilder_HasEdge);

typedef struct {
  size_t  VertexCount;
  size_t  EdgeCount;
  size_t *VertexOffsets;
  size_t *FlatVertexConnectivity;
} GraphSupportClone;

TEST(GRTPSupport_New) {
  GraphSupport        *Support = NULL;
  GraphSupportClone   *Clone   = NULL;
  GraphSupportBuilder *Builder = NewGraphSupportBuilder(3);
  ASSERT_X(Builder);
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 2));
  // Notice: next edges added unsorted.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 0));

  Support = NewGraphSupport(Builder);
  ASSERT(Support);

  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  Clone = (GraphSupportClone *)Support;
  ASSERT(Clone->VertexCount == 3);
  ASSERT(Clone->EdgeCount == 5);
  ASSERT(Clone->VertexOffsets[0] == 0);
  ASSERT(Clone->VertexOffsets[1] == 2);
  ASSERT(Clone->VertexOffsets[2] == 3);
  ASSERT(Clone->FlatVertexConnectivity[0] == 0);
  ASSERT(Clone->FlatVertexConnectivity[1] == 1);
  ASSERT(Clone->FlatVertexConnectivity[2] == 2);
  // Notice: next edges are sorted.
  ASSERT(Clone->FlatVertexConnectivity[3] == 0);
  ASSERT(Clone->FlatVertexConnectivity[4] == 1);

  FreeGraphSupport(&Support);
  ASSERT(!Support);
}

static void CreateExampleGraph(GraphSupport **SupportReference) {
  ASSERT_X(SupportReference);
  GraphSupportBuilder *Builder = NewGraphSupportBuilder(3);
  ASSERT_X(Builder);
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 2));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 1));
  (*SupportReference) = NewGraphSupport(Builder);
  ASSERT_X((*SupportReference));
  FreeGraphSupportBuilder(&Builder);
}

TEST(GRTPSupport_GetVertexCount) {
  GraphSupport *Support = NULL;

  CreateExampleGraph(&Support);
  ASSERT(GraphSupportGetVertexCount(Support) == 3);
  FreeGraphSupport(&Support);
  ASSERT(!Support);
}

TEST(GRTPSupport_GetEdgeCount) {
  GraphSupport *Support = NULL;

  CreateExampleGraph(&Support);
  ASSERT(GraphSupportGetEdgeCount(Support) == 5);
  FreeGraphSupport(&Support);
  ASSERT(!Support);
}

TEST(GRTPSupport_GetNeighborCount) {
  GraphSupport *Support = NULL;

  CreateExampleGraph(&Support);
  ASSERT(GraphSupportGetNeighborCount(Support, 0) == 2);
  ASSERT(GraphSupportGetNeighborCount(Support, 1) == 1);
  ASSERT(GraphSupportGetNeighborCount(Support, 2) == 2);
  ASSERT(GraphSupportGetNeighborCount(Support, 3) == 0);
  FreeGraphSupport(&Support);
  ASSERT(!Support);
}

TEST(GRTPSupport_GetNeighbors) {
  GraphSupport *Support   = NULL;
  const size_t *Neighbors = NULL;

  CreateExampleGraph(&Support);
  Neighbors = (const size_t *)GraphSupportGetNeighbors(Support, 0);
  ASSERT(Neighbors[0] == 0);
  ASSERT(Neighbors[1] == 1);
  Neighbors = (const size_t *)GraphSupportGetNeighbors(Support, 1);
  ASSERT(Neighbors[0] == 2);
  Neighbors = (const size_t *)GraphSupportGetNeighbors(Support, 2);
  ASSERT(Neighbors[0] == 0);
  ASSERT(Neighbors[1] == 1);
  Neighbors = (const size_t *)GraphSupportGetNeighbors(Support, 3);
  ASSERT(!Neighbors);
  FreeGraphSupport(&Support);
  ASSERT(!Support);
}

TEST(GRTPSupport_GetEdgeIndex) {
  GraphSupport *Support = NULL;
  size_t        EdgeIndex;

  CreateExampleGraph(&Support);
  ASSERT(GraphSupportGetEdgeIndex(Support, 0, 0, &EdgeIndex));
  ASSERT(EdgeIndex == 0);
  ASSERT(GraphSupportGetEdgeIndex(Support, 0, 1, &EdgeIndex));
  ASSERT(EdgeIndex == 1);
  ASSERT(GraphSupportGetEdgeIndex(Support, 1, 2, &EdgeIndex));
  ASSERT(EdgeIndex == 2);
  ASSERT(GraphSupportGetEdgeIndex(Support, 2, 0, &EdgeIndex));
  ASSERT(EdgeIndex == 3);
  ASSERT(GraphSupportGetEdgeIndex(Support, 2, 1, &EdgeIndex));
  ASSERT(EdgeIndex == 4);
  ASSERT(!GraphSupportGetEdgeIndex(Support, 3, 0, &EdgeIndex));
  ASSERT(EdgeIndex == 4);
  ASSERT(!GraphSupportGetEdgeIndex(Support, 0, 3, &EdgeIndex));
  ASSERT(EdgeIndex == 4);
  FreeGraphSupport(&Support);
  ASSERT(!Support);
}

TEST(GRTPSupport_GetEdgeVertices) {
  GraphSupport *Support = NULL;
  size_t        Origin, Destination;

  CreateExampleGraph(&Support);
  ASSERT(GraphSupportGetEdgeVertices(Support, 0, &Origin, &Destination));
  ASSERT(Origin == 0);
  ASSERT(Destination == 0);
  ASSERT(GraphSupportGetEdgeVertices(Support, 1, &Origin, &Destination));
  ASSERT(Origin == 0);
  ASSERT(Destination == 1);
  ASSERT(GraphSupportGetEdgeVertices(Support, 2, &Origin, &Destination));
  ASSERT(Origin == 1);
  ASSERT(Destination == 2);
  ASSERT(GraphSupportGetEdgeVertices(Support, 3, &Origin, &Destination));
  ASSERT(Origin == 2);
  ASSERT(Destination == 0);
  ASSERT(GraphSupportGetEdgeVertices(Support, 4, &Origin, &Destination));
  ASSERT(Origin == 2);
  ASSERT(Destination == 1);
  ASSERT(!GraphSupportGetEdgeVertices(Support, 5, &Origin, &Destination));
  ASSERT(Origin == 2);
  ASSERT(Destination == 1);
  FreeGraphSupport(&Support);
  ASSERT(!Support);
}

TEST(GRTPSupport_HasEdge) {
  GraphSupport *Support = NULL;

  CreateExampleGraph(&Support);
  ASSERT(GraphSupportHasEdge(Support, 0, 0));
  ASSERT(GraphSupportHasEdge(Support, 0, 1));
  ASSERT(GraphSupportHasEdge(Support, 1, 2));
  ASSERT(GraphSupportHasEdge(Support, 2, 0));
  ASSERT(GraphSupportHasEdge(Support, 2, 1));
  ASSERT(!GraphSupportHasEdge(Support, 3, 0));
  ASSERT(!GraphSupportHasEdge(Support, 0, 3));
  FreeGraphSupport(&Support);
  ASSERT(!Support);
}

TEST(GRTPSupport_GetEdgeIndex_LargerGraph) {
  GraphSupport *Support = NULL;
  size_t        EdgeIndex;

  GraphSupportBuilder *Builder = NewGraphSupportBuilder(4);
  ASSERT_X(Builder);
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 2));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 2));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 3));
  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);

  ASSERT(GraphSupportGetEdgeIndex(Support, 0, 0, &EdgeIndex));
  ASSERT(EdgeIndex == 0);
  ASSERT(GraphSupportGetEdgeIndex(Support, 0, 1, &EdgeIndex));
  ASSERT(EdgeIndex == 1);
  ASSERT(GraphSupportGetEdgeIndex(Support, 1, 2, &EdgeIndex));
  ASSERT(EdgeIndex == 2);
  ASSERT(GraphSupportGetEdgeIndex(Support, 2, 0, &EdgeIndex));
  ASSERT(EdgeIndex == 3);
  ASSERT(GraphSupportGetEdgeIndex(Support, 2, 1, &EdgeIndex));
  ASSERT(EdgeIndex == 4);
  ASSERT(GraphSupportGetEdgeIndex(Support, 2, 2, &EdgeIndex));
  ASSERT(EdgeIndex == 5);
  ASSERT(GraphSupportGetEdgeIndex(Support, 2, 3, &EdgeIndex));
  ASSERT(EdgeIndex == 6);
  ASSERT(!GraphSupportGetEdgeIndex(Support, 3, 0, &EdgeIndex));
  ASSERT(EdgeIndex == 6);
  ASSERT(!GraphSupportGetEdgeIndex(Support, 0, 3, &EdgeIndex));
  ASSERT(EdgeIndex == 6);
  FreeGraphSupport(&Support);
  ASSERT(!Support);
}

TEST(GRTPSupport_NeighborSorting) {
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;
  const size_t        *Neighbors = NULL;
  size_t               i;

  // Build a graph whose vertex 0 connects to every other vertex, added in
  // descending order. NewGraphSupport sorts each neighbor slice with
  // qsort/CompareSizeT, so the finalized slice must be strictly ascending.
  Builder = NewGraphSupportBuilder(7);
  ASSERT_X(Builder);
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 6));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 2));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 5));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 4));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 3));

  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  ASSERT(GraphSupportGetNeighborCount(Support, 0) == 7);
  Neighbors = GraphSupportGetNeighbors(Support, 0);
  ASSERT(Neighbors);
  ASSERT(Neighbors[0] == 0);
  ASSERT(Neighbors[6] == 6);
  for (i = 1; i < 7; i++) {
    ASSERT(Neighbors[i - 1] < Neighbors[i]);
  }

  FreeGraphSupport(&Support);
  ASSERT(!Support);
}

TEST(GRTPSupport_NoEdges) {
  GraphSupport        *Support = NULL;
  size_t               EdgeIndex, Origin, Destination;
  GraphSupportBuilder *Builder = NewGraphSupportBuilder(2);
  ASSERT_X(Builder);
  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);

  ASSERT(GraphSupportGetVertexCount(Support) == 2);
  ASSERT(GraphSupportGetEdgeCount(Support) == 0);
  ASSERT(GraphSupportGetNeighborCount(Support, 0) == 0);
  ASSERT(GraphSupportGetNeighborCount(Support, 1) == 0);
  ASSERT(!GraphSupportGetNeighbors(Support, 0));
  ASSERT(!GraphSupportGetNeighbors(Support, 1));
  ASSERT(!GraphSupportGetEdgeIndex(Support, 0, 0, &EdgeIndex));
  ASSERT(!GraphSupportGetEdgeVertices(Support, 0, &Origin, &Destination));
  ASSERT(!GraphSupportHasEdge(Support, 0, 0));

  FreeGraphSupport(&Support);
  ASSERT(!Support);
}

TEST_SUITE(GRTPSupportSuite,
           GRTPSupport_New,
           GRTPSupport_GetVertexCount,
           GRTPSupport_GetEdgeCount,
           GRTPSupport_GetNeighborCount,
           GRTPSupport_GetNeighbors,
           GRTPSupport_GetEdgeIndex,
           GRTPSupport_GetEdgeVertices,
           GRTPSupport_HasEdge,
           GRTPSupport_GetEdgeIndex_LargerGraph,
           GRTPSupport_NoEdges,
           GRTPSupport_NeighborSorting);
