#include "src/GRTP/GraphUtilities.h"
#include "test/TSD/TSD.h"
#include <float.h>
#include <math.h>

TEST(GRTPDijkstraIsolatedVertices) {
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;
  const double        *Distance  = NULL;
  const size_t        *Parent    = NULL;

  /*

      A      B



      C      D

  */

  Builder = NewGraphSupportBuilder(4);
  ASSERT_X(Builder);

  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  Workspace = NewGraphWorkspace(Support);
  ASSERT_X(Workspace);

  ASSERT(!GraphDijkstra(Workspace, 1, (const size_t[]){8}, NULL, NULL));
  ASSERT(GraphDijkstra(Workspace, 1, (const size_t[]){0}, NULL, NULL));
  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(Workspace));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(Workspace));

  ASSERT(fabs(Distance[0]) < 10 * DBL_EPSILON);
  ASSERT(isinf(Distance[1]));
  ASSERT(isinf(Distance[2]));
  ASSERT(isinf(Distance[3]));

  ASSERT(Parent[0] == UnreachableVertex);
  ASSERT(Parent[1] == UnreachableVertex);
  ASSERT(Parent[2] == UnreachableVertex);
  ASSERT(Parent[3] == UnreachableVertex);

  FreeGraphWorkspace(&Workspace);
  ASSERT_X(!Workspace);
  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST(GRTPDijkstraUnitWeightsCycle) {
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;
  const double        *Distance  = NULL;
  const size_t        *Parent    = NULL;

  /*
         1
      A ---- B
      |      |
    1 |      | 1
      |      |
      C ---- D
         1
  */

  Builder = NewGraphSupportBuilder(4);
  ASSERT_X(Builder);
  // A.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 2));
  // B.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 3));
  // C.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 3));
  // D.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 2));

  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  Workspace = NewGraphWorkspace(Support);
  ASSERT_X(Workspace);

  ASSERT(GraphDijkstra(Workspace, 1, (const size_t[]){0}, NULL, NULL));
  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(Workspace));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(Workspace));

  ASSERT(fabs(Distance[0]) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[1] - (double)1.0) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[2] - (double)1.0) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[3] - (double)2.0) < 10 * DBL_EPSILON);

  ASSERT(Parent[0] == UnreachableVertex);
  ASSERT(Parent[1] == 0);
  ASSERT(Parent[2] == 0);
  ASSERT((Parent[3] == 1) || (Parent[3] == 2));

  FreeGraphWorkspace(&Workspace);
  ASSERT_X(!Workspace);
  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

#define MAX_EDGES 1000

typedef struct {
  size_t EdgeCount;
  double Weights[MAX_EDGES];
} CustomContext;

static double WeightFunction(const GraphSupport *Support,
                             const size_t        Origin,
                             const size_t        Destination,
                             void               *UserContext) {
  CustomContext *Context = (CustomContext *)UserContext;
  size_t         EdgeIndex;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, Origin, Destination, &EdgeIndex));
  if (EdgeIndex < Context->EdgeCount) {
    return Context->Weights[EdgeIndex];
  }
  return (double)1.0;
}

TEST(GRTPDijkstraWeightedCycle) {
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;
  CustomContext        Context;
  size_t               EdgeIndex;
  const double        *Distance = NULL;
  const size_t        *Parent   = NULL;

  /*
         2
      A ---- B
      |      |
    1 |      | 3
      |      |
      C ---- D
         1
  */

  Builder = NewGraphSupportBuilder(4);
  ASSERT_X(Builder);
  // A.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 2));
  // B.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 3));
  // C.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 3));
  // D.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 2));

  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  Context.EdgeCount = GraphSupportGetEdgeCount(Support);
  ASSERT_X(Context.EdgeCount < MAX_EDGES);

  // A.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 0, 1, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)2.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 0, 2, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;
  // C.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 1, 0, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)2.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 1, 3, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)3.0;
  // C.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 2, 0, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 2, 3, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;
  // D.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 3, 1, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)3.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 3, 2, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;

  Workspace = NewGraphWorkspace(Support);
  ASSERT_X(Workspace);

  ASSERT(GraphDijkstra(Workspace, 1, (const size_t[]){0}, WeightFunction,
                       (void *)&Context));
  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(Workspace));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(Workspace));

  ASSERT(fabs(Distance[0]) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[1] - (double)2.0) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[2] - (double)1.0) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[3] - (double)2.0) < 10 * DBL_EPSILON);

  ASSERT(Parent[0] == UnreachableVertex);
  ASSERT(Parent[1] == 0);
  ASSERT(Parent[2] == 0);
  ASSERT(Parent[3] == 2);

  FreeGraphWorkspace(&Workspace);
  ASSERT_X(!Workspace);
  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST(GRTPDijkstraWeightedSingleSource) {
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;
  CustomContext        Context;
  size_t               EdgeIndex;
  const double        *Distance = NULL;
  const size_t        *Parent   = NULL;

  /*
 - A --7-- B --2-- D --3-- F
 | |       |       |
 | 9       6       4
 | |       |       |
 | C --1-- E ------+
 \----12--/
  */

  Builder = NewGraphSupportBuilder(6);
  ASSERT_X(Builder);
  // A.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 2));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 4));
  // B.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 3));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 4));
  // C.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 4));
  // D.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 4));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 5));
  // E.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 4, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 4, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 4, 2));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 4, 3));
  // F.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 5, 3));

  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  Context.EdgeCount = GraphSupportGetEdgeCount(Support);
  ASSERT_X(Context.EdgeCount < MAX_EDGES);

  // A.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 0, 1, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)7.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 0, 2, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)9.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 0, 4, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)12.0;
  // B.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 1, 0, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)7.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 1, 3, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)2.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 1, 4, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)6.0;
  // C.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 2, 0, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)9.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 2, 4, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;
  // D.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 3, 1, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)2.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 3, 4, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)4.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 3, 5, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)3.0;
  // E.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 4, 0, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)12.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 4, 1, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)6.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 4, 2, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 4, 3, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)4.0;
  // F.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 5, 3, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)3.0;

  Workspace = NewGraphWorkspace(Support);
  ASSERT_X(Workspace);

  ASSERT(GraphDijkstra(Workspace, 1, (const size_t[]){0}, WeightFunction,
                       (void *)&Context));
  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(Workspace));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(Workspace));

  ASSERT(fabs(Distance[0]) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[1] - (double)7.0) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[2] - (double)9.0) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[3] - (double)9.0) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[4] - (double)10.0) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[5] - (double)12.0) < 10 * DBL_EPSILON);

  ASSERT(Parent[0] == UnreachableVertex);
  ASSERT(Parent[1] == 0);
  ASSERT(Parent[2] == 0);
  ASSERT(Parent[3] == 1);
  ASSERT(Parent[4] == 2);
  ASSERT(Parent[5] == 3);

  FreeGraphWorkspace(&Workspace);
  ASSERT_X(!Workspace);
  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST(GRTPDijkstraWeightedMultiSource) {
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;
  CustomContext        Context;
  size_t               EdgeIndex;
  const double        *Distance = NULL;
  const size_t        *Parent   = NULL;

  /*
 - A --7-- B --2-- D --3-- F
 | |       |       |
 | 9       6       4
 | |       |       |
 | C --1-- E ------+
 \----12--/
  */

  Builder = NewGraphSupportBuilder(6);
  ASSERT_X(Builder);
  // A.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 2));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 4));
  // B.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 3));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 4));
  // C.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 4));
  // D.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 4));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 5));
  // E.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 4, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 4, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 4, 2));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 4, 3));
  // F.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 5, 3));

  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  Context.EdgeCount = GraphSupportGetEdgeCount(Support);
  ASSERT_X(Context.EdgeCount < MAX_EDGES);

  // A.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 0, 1, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)7.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 0, 2, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)9.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 0, 4, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)12.0;
  // B.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 1, 0, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)7.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 1, 3, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)2.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 1, 4, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)6.0;
  // C.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 2, 0, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)9.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 2, 4, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;
  // D.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 3, 1, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)2.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 3, 4, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)4.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 3, 5, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)3.0;
  // E.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 4, 0, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)12.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 4, 1, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)6.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 4, 2, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 4, 3, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)4.0;
  // F.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 5, 3, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)3.0;

  Workspace = NewGraphWorkspace(Support);
  ASSERT_X(Workspace);

  ASSERT(GraphDijkstra(Workspace, 2, (const size_t[]){0, 3}, WeightFunction,
                       (void *)&Context));
  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(Workspace));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(Workspace));

  ASSERT(fabs(Distance[0]) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[1] - (double)2.0) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[2] - (double)5.0) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[3]) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[4] - (double)4.0) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[5] - (double)3.0) < 10 * DBL_EPSILON);

  ASSERT(Parent[0] == UnreachableVertex);
  ASSERT(Parent[1] == 3);
  ASSERT(Parent[2] == 4);
  ASSERT(Parent[3] == UnreachableVertex);
  ASSERT(Parent[4] == 3);
  ASSERT(Parent[5] == 3);

  FreeGraphWorkspace(&Workspace);
  ASSERT_X(!Workspace);
  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST_SUITE(GRTPDijkstraSuite,
           GRTPDijkstraIsolatedVertices,
           GRTPDijkstraUnitWeightsCycle,
           GRTPDijkstraWeightedCycle,
           GRTPDijkstraWeightedSingleSource,
           GRTPDijkstraWeightedMultiSource);

TEST(GRTPBidirectionalIsolatedVertices) {
  GraphWorkspace      *WorkspaceA = NULL;
  GraphWorkspace      *WorkspaceB = NULL;
  GraphSupport        *Support    = NULL;
  GraphSupportBuilder *Builder    = NULL;
  const double        *Distance   = NULL;
  const size_t        *Parent     = NULL;
  size_t               VerticesInPath;
  double               ShortestDistance;

  /*

      A      B



      C      D

  */

  Builder = NewGraphSupportBuilder(4);
  ASSERT_X(Builder);

  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  WorkspaceA = NewGraphWorkspace(Support);
  ASSERT_X(WorkspaceA);
  WorkspaceB = NewGraphWorkspace(Support);
  ASSERT_X(WorkspaceB);

  ASSERT(!GraphBidirectionalDijkstra(WorkspaceA, WorkspaceB, 5, 0, NULL, NULL,
                                     NULL, NULL, NULL));
  ASSERT(!GraphBidirectionalDijkstra(WorkspaceA, WorkspaceB, 0, 5, NULL, NULL,
                                     NULL, NULL, NULL));
  ASSERT(GraphBidirectionalDijkstra(WorkspaceA, WorkspaceB, 0, 1, NULL, NULL,
                                    NULL, NULL, NULL));

  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(WorkspaceA));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(WorkspaceA));
  ASSERT(fabs(Distance[0]) < 10 * DBL_EPSILON);
  ASSERT(isinf(Distance[1]));
  ASSERT(isinf(Distance[2]));
  ASSERT(isinf(Distance[3]));
  ASSERT(Parent[0] == UnreachableVertex);
  ASSERT(Parent[1] == UnreachableVertex);
  ASSERT(Parent[2] == UnreachableVertex);
  ASSERT(Parent[3] == UnreachableVertex);

  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(WorkspaceB));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(WorkspaceB));
  ASSERT(isinf(Distance[0]));
  ASSERT(fabs(Distance[1]) < 10 * DBL_EPSILON);
  ASSERT(isinf(Distance[2]));
  ASSERT(isinf(Distance[3]));
  ASSERT(Parent[0] == UnreachableVertex);
  ASSERT(Parent[1] == UnreachableVertex);
  ASSERT(Parent[2] == UnreachableVertex);
  ASSERT(Parent[3] == UnreachableVertex);

  ASSERT(GraphBidirectionalDijkstra(WorkspaceA, WorkspaceB, 0, 1, NULL, NULL,
                                    NULL, &VerticesInPath, &ShortestDistance));
  ASSERT(VerticesInPath == 0);
  ASSERT(isinf(ShortestDistance));

  FreeGraphWorkspace(&WorkspaceA);
  ASSERT_X(!WorkspaceA);
  FreeGraphWorkspace(&WorkspaceB);
  ASSERT_X(!WorkspaceB);
  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST(GRTPBidirectionalUnitWeightsCycle) {
  GraphWorkspace      *WorkspaceA   = NULL;
  GraphWorkspace      *WorkspaceB   = NULL;
  GraphSupport        *Support      = NULL;
  GraphSupportBuilder *Builder      = NULL;
  const double        *Distance     = NULL;
  const size_t        *Parent       = NULL;
  size_t              *ShortestPath = NULL;
  size_t               ShortestPathVertexCount;

  /*
         1
      A ---- B
      |      |
    1 |      | 1
      |      |
      C ---- D
         1
  */

  Builder      = NewGraphSupportBuilder(4);
  ShortestPath = (size_t *)malloc(4 * sizeof(size_t));
  ASSERT_X(ShortestPath);
  ASSERT_X(Builder);
  // A.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 2));
  // B.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 3));
  // C.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 3));
  // D.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 2));

  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  WorkspaceA = NewGraphWorkspace(Support);
  ASSERT_X(WorkspaceA);
  WorkspaceB = NewGraphWorkspace(Support);
  ASSERT_X(WorkspaceB);

  ASSERT(GraphBidirectionalDijkstra(WorkspaceA, WorkspaceB, 0, 3, NULL, NULL,
                                    ShortestPath, &ShortestPathVertexCount,
                                    NULL));

  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(WorkspaceA));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(WorkspaceA));
  ASSERT(fabs(Distance[0]) < 10 * DBL_EPSILON);
  ASSERT(Parent[0] == UnreachableVertex);

  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(WorkspaceB));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(WorkspaceB));
  ASSERT(fabs(Distance[3]) < 10 * DBL_EPSILON);
  ASSERT(Parent[3] == UnreachableVertex);

  ASSERT(ShortestPathVertexCount == 3);
  ASSERT(ShortestPath[0] == 0);
  ASSERT((ShortestPath[1] == 1) || (ShortestPath[1] == 2));
  ASSERT(ShortestPath[2] == 3);

  free(ShortestPath);
  FreeGraphWorkspace(&WorkspaceA);
  ASSERT_X(!WorkspaceA);
  FreeGraphWorkspace(&WorkspaceB);
  ASSERT_X(!WorkspaceB);
  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST(GRTPBidirectionalDisjointComponents) {
  GraphWorkspace      *WorkspaceA = NULL;
  GraphWorkspace      *WorkspaceB = NULL;
  GraphSupport        *Support    = NULL;
  GraphSupportBuilder *Builder    = NULL;
  const double        *Distance   = NULL;
  const size_t        *Parent     = NULL;
  size_t               ShortestPathVertexCount;

  /*
         1
      A ---- B

      C ---- D
         1
  */

  Builder = NewGraphSupportBuilder(4);
  ASSERT_X(Builder);
  // A.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  // B.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 0));
  // C.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 3));
  // D.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 2));

  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  WorkspaceA = NewGraphWorkspace(Support);
  ASSERT_X(WorkspaceA);
  WorkspaceB = NewGraphWorkspace(Support);
  ASSERT_X(WorkspaceB);

  ASSERT(GraphBidirectionalDijkstra(WorkspaceA, WorkspaceB, 0, 3, NULL, NULL,
                                    NULL, &ShortestPathVertexCount, NULL));

  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(WorkspaceA));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(WorkspaceA));
  ASSERT(fabs(Distance[0]) < 10 * DBL_EPSILON);
  ASSERT(Parent[0] == UnreachableVertex);
  ASSERT(fabs(Distance[1] - (double)1.0) < 10 * DBL_EPSILON);
  ASSERT(Parent[1] == 0);

  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(WorkspaceB));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(WorkspaceB));
  ASSERT(fabs(Distance[3]) < 10 * DBL_EPSILON);
  ASSERT(Parent[3] == UnreachableVertex);
  ASSERT(fabs(Distance[2] - (double)1.0) < 10 * DBL_EPSILON);
  ASSERT(Parent[2] == 3);

  ASSERT(ShortestPathVertexCount == 0);

  FreeGraphWorkspace(&WorkspaceA);
  ASSERT_X(!WorkspaceA);
  FreeGraphWorkspace(&WorkspaceB);
  ASSERT_X(!WorkspaceB);
  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST(GRTPBidirectionalRejectsLongerMeeting) {
  GraphWorkspace      *Forward  = NULL;
  GraphWorkspace      *Backward = NULL;
  GraphSupport        *Support  = NULL;
  GraphSupportBuilder *Builder  = NULL;
  CustomContext        Context;
  size_t               EdgeIndex;
  size_t               ShortestPath[4];
  size_t               ShortestPathVertexCount = 0;
  double               ShortestDistance        = -1.0;
  const double        *Distance                = NULL;
  const size_t        *Parent                  = NULL;

  /*
            A
          / | \
        3/  |3 \1
        /   |   \
       B -- C    D
          1

      B is the source and A the target: the direct edge costs 3, the detour
      through C costs 4. D hangs off A with weight 1.
  */

  Builder = NewGraphSupportBuilder(4);
  ASSERT_X(Builder);
  // A.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 2));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 3));
  // B.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 2));
  // C.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 1));
  // D.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 0));

  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  Context.EdgeCount = GraphSupportGetEdgeCount(Support);
  ASSERT_X(Context.EdgeCount < MAX_EDGES);

  // A.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 0, 1, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)3.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 0, 2, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)3.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 0, 3, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;
  // B.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 1, 0, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)3.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 1, 2, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;
  // C.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 2, 0, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)3.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 2, 1, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;
  // D.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 3, 0, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;

  Forward = NewGraphWorkspace(Support);
  ASSERT_X(Forward);
  Backward = NewGraphWorkspace(Support);
  ASSERT_X(Backward);

  /*
      The searches first meet at A, after 3 units, and then at C, after
      ForwardDistance[C] + BackwardDistance[C] = 4 units. The later rendezvous
      is longer than the one already recorded, so it must be discarded instead
      of widening the answer, and the remaining frontiers must be pruned: their
      smallest priorities already add up to the best distance. D is therefore
      never reached by the forward search.
  */
  ASSERT(GraphBidirectionalDijkstra(
      Forward, Backward, 1, 0, WeightFunction, (void *)&Context, ShortestPath,
      &ShortestPathVertexCount, &ShortestDistance));

  ASSERT(ShortestPathVertexCount == 2);
  ASSERT(ShortestPath[0] == 1);
  ASSERT(ShortestPath[1] == 0);
  ASSERT(fabs(ShortestDistance - (double)3.0) < 10 * DBL_EPSILON);

  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(Forward));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(Forward));
  ASSERT(fabs(Distance[0] - (double)3.0) < 10 * DBL_EPSILON);
  ASSERT(Parent[0] == 1);
  ASSERT(fabs(Distance[1]) < 10 * DBL_EPSILON);
  ASSERT(Parent[1] == UnreachableVertex);
  ASSERT(fabs(Distance[2] - (double)1.0) < 10 * DBL_EPSILON);
  ASSERT(Parent[2] == 1);
  ASSERT(isinf(Distance[3]));
  ASSERT(Parent[3] == UnreachableVertex);

  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(Backward));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(Backward));
  ASSERT(fabs(Distance[0]) < 10 * DBL_EPSILON);
  ASSERT(Parent[0] == UnreachableVertex);
  ASSERT(fabs(Distance[1] - (double)3.0) < 10 * DBL_EPSILON);
  ASSERT(Parent[1] == 0);
  ASSERT(fabs(Distance[2] - (double)3.0) < 10 * DBL_EPSILON);
  ASSERT(Parent[2] == 0);
  ASSERT(fabs(Distance[3] - (double)1.0) < 10 * DBL_EPSILON);
  ASSERT(Parent[3] == 0);

  FreeGraphWorkspace(&Forward);
  ASSERT_X(!Forward);
  FreeGraphWorkspace(&Backward);
  ASSERT_X(!Backward);
  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST(GRTPBidirectionalKeepsFirstMeetingOnTie) {
  GraphWorkspace      *Forward  = NULL;
  GraphWorkspace      *Backward = NULL;
  GraphSupport        *Support  = NULL;
  GraphSupportBuilder *Builder  = NULL;
  CustomContext        Context;
  size_t               EdgeIndex;
  size_t               ShortestPath[4];
  size_t               ShortestPathVertexCount = 0;
  double               ShortestDistance        = -1.0;
  const double        *Distance                = NULL;
  const size_t        *Parent                  = NULL;

  /*
            A
          / | \
        3/  |2 \1
        /   |   \
       B -- C    D
          1

      Two routes from B to A cost 3: the direct edge and B - C - A.
  */

  Builder = NewGraphSupportBuilder(4);
  ASSERT_X(Builder);
  // A.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 2));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 3));
  // B.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 2));
  // C.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 1));
  // D.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 0));

  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  Context.EdgeCount = GraphSupportGetEdgeCount(Support);
  ASSERT_X(Context.EdgeCount < MAX_EDGES);

  // A.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 0, 1, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)3.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 0, 2, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)2.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 0, 3, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;
  // B.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 1, 0, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)3.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 1, 2, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;
  // C.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 2, 0, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)2.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 2, 1, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;
  // D.
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 3, 0, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;

  Forward = NewGraphWorkspace(Support);
  ASSERT_X(Forward);
  Backward = NewGraphWorkspace(Support);
  ASSERT_X(Backward);

  /*
      The first rendezvous is A, at 3 units, and the second one is C, at
      ForwardDistance[C] + BackwardDistance[C] = 3 units as well. Only a
      strictly shorter rendezvous may replace the recorded one, so the answer
      keeps the direct edge instead of switching to the equal-length detour.
  */
  ASSERT(GraphBidirectionalDijkstra(
      Forward, Backward, 1, 0, WeightFunction, (void *)&Context, ShortestPath,
      &ShortestPathVertexCount, &ShortestDistance));

  ASSERT(ShortestPathVertexCount == 2);
  ASSERT(ShortestPath[0] == 1);
  ASSERT(ShortestPath[1] == 0);
  ASSERT(fabs(ShortestDistance - (double)3.0) < 10 * DBL_EPSILON);

  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(Forward));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(Forward));
  ASSERT(fabs(Distance[0] - (double)3.0) < 10 * DBL_EPSILON);
  ASSERT(Parent[0] == 1);
  ASSERT(fabs(Distance[1]) < 10 * DBL_EPSILON);
  ASSERT(Parent[1] == UnreachableVertex);
  ASSERT(fabs(Distance[2] - (double)1.0) < 10 * DBL_EPSILON);
  ASSERT(Parent[2] == 1);
  ASSERT(isinf(Distance[3]));
  ASSERT(Parent[3] == UnreachableVertex);

  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(Backward));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(Backward));
  ASSERT(fabs(Distance[0]) < 10 * DBL_EPSILON);
  ASSERT(Parent[0] == UnreachableVertex);
  ASSERT(fabs(Distance[1] - (double)3.0) < 10 * DBL_EPSILON);
  ASSERT(Parent[1] == 0);
  ASSERT(fabs(Distance[2] - (double)2.0) < 10 * DBL_EPSILON);
  ASSERT(Parent[2] == 0);
  ASSERT(fabs(Distance[3] - (double)1.0) < 10 * DBL_EPSILON);
  ASSERT(Parent[3] == 0);

  FreeGraphWorkspace(&Forward);
  ASSERT_X(!Forward);
  FreeGraphWorkspace(&Backward);
  ASSERT_X(!Backward);
  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST_SUITE(GRTPBidirectionalDijkstraSuite,
           GRTPBidirectionalIsolatedVertices,
           GRTPBidirectionalUnitWeightsCycle,
           GRTPBidirectionalDisjointComponents,
           GRTPBidirectionalRejectsLongerMeeting,
           GRTPBidirectionalKeepsFirstMeetingOnTie);

TEST(GRTPBellmanFordIsolatedVertices) {
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;
  const double        *Distance  = NULL;
  const size_t        *Parent    = NULL;

  /*

      A      B



      C      D

  */

  Builder = NewGraphSupportBuilder(4);
  ASSERT_X(Builder);

  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  Workspace = NewGraphWorkspace(Support);
  ASSERT_X(Workspace);

  ASSERT(
      !GraphBellmanFord(Workspace, 1, (const size_t[]){8}, NULL, NULL, NULL));
  ASSERT(GraphBellmanFord(Workspace, 1, (const size_t[]){0}, NULL, NULL, NULL));
  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(Workspace));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(Workspace));

  ASSERT(fabs(Distance[0]) < 10 * DBL_EPSILON);
  ASSERT(isinf(Distance[1]));
  ASSERT(isinf(Distance[2]));
  ASSERT(isinf(Distance[3]));

  ASSERT(Parent[0] == UnreachableVertex);
  ASSERT(Parent[1] == UnreachableVertex);
  ASSERT(Parent[2] == UnreachableVertex);
  ASSERT(Parent[3] == UnreachableVertex);

  FreeGraphWorkspace(&Workspace);
  ASSERT_X(!Workspace);
  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST(GRTPBellmanFordUnitWeightsCycle) {
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;
  const double        *Distance  = NULL;
  const size_t        *Parent    = NULL;
  bool                 NegativeCycles;

  /*
         1
      A ---- B
      |      |
    1 |      | 1
      |      |
      C ---- D
         1
  */

  Builder = NewGraphSupportBuilder(4);
  ASSERT_X(Builder);
  // A.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 2));
  // B.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 3));
  // C.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 0));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 3));
  // D.
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 3, 2));

  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  Workspace = NewGraphWorkspace(Support);
  ASSERT_X(Workspace);

  ASSERT(GraphBellmanFord(Workspace, 1, (const size_t[]){0}, NULL, NULL, NULL));
  ASSERT(Distance = (const double *)GraphWorkspaceGetDistance(Workspace));
  ASSERT(Parent = (const size_t *)GraphWorkspaceGetParent(Workspace));

  ASSERT(fabs(Distance[0]) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[1] - (double)1.0) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[2] - (double)1.0) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[3] - (double)2.0) < 10 * DBL_EPSILON);

  ASSERT(Parent[0] == UnreachableVertex);
  ASSERT(Parent[1] == 0);
  ASSERT(Parent[2] == 0);
  ASSERT((Parent[3] == 1) || (Parent[3] == 2));

  ASSERT(GraphBellmanFord(Workspace, 1, (const size_t[]){0}, NULL, NULL,
                          &NegativeCycles));
  ASSERT(!NegativeCycles);

  FreeGraphWorkspace(&Workspace);
  ASSERT_X(!Workspace);
  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST(GRTPBellmanFordDirectedDisjointComponents) {
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;
  CustomContext        Context;
  size_t               EdgeIndex;
  const double        *Distance = NULL;
  const size_t        *Parent   = NULL;
  bool                 NegativeCycles;

  /*
   *  A --5--> B
   *
   *  C --2--> D
   */

  Builder = NewGraphSupportBuilder(4);
  ASSERT_X(Builder);

  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 3));

  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  Context.EdgeCount = GraphSupportGetEdgeCount(Support);
  ASSERT_X(Context.EdgeCount < MAX_EDGES);

  ASSERT_X(GraphSupportGetEdgeIndex(Support, 0, 1, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)5.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 2, 3, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)2.0;

  Workspace = NewGraphWorkspace(Support);
  ASSERT_X(Workspace);

  ASSERT(GraphBellmanFord(Workspace, 1, (const size_t[]){0}, WeightFunction,
                          (void *)&Context, &NegativeCycles));
  ASSERT(!NegativeCycles);
  Distance = GraphWorkspaceGetDistance(Workspace);
  Parent   = GraphWorkspaceGetParent(Workspace);
  ASSERT(Distance);
  ASSERT(Parent);

  ASSERT(fabs(Distance[0]) < 10 * DBL_EPSILON);
  ASSERT(fabs(Distance[1] - (double)5.0) < 10 * DBL_EPSILON);
  ASSERT(isinf(Distance[2]));
  ASSERT(isinf(Distance[3]));

  ASSERT(Parent[0] == UnreachableVertex);
  ASSERT(Parent[1] == 0);
  ASSERT(Parent[2] == UnreachableVertex);
  ASSERT(Parent[3] == UnreachableVertex);

  FreeGraphWorkspace(&Workspace);
  ASSERT_X(!Workspace);
  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST(GRTPBellmanFordNegativeCycle) {
  GraphWorkspace      *Workspace = NULL;
  GraphSupport        *Support   = NULL;
  GraphSupportBuilder *Builder   = NULL;
  CustomContext        Context;
  size_t               EdgeIndex;
  bool                 NegativeCycles;
  const double        *Distance = NULL;
  const size_t        *Parent   = NULL;

  /*
   *  A --1--> B --(-2)--> C
   *             ^          |
   *             |          |
   *             +---(-2)---+
   */

  Builder = NewGraphSupportBuilder(3);
  ASSERT_X(Builder);

  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 0, 1));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 1, 2));
  ASSERT_X(GraphSupportBuilderAddEdge(Builder, 2, 1));

  Support = NewGraphSupport(Builder);
  ASSERT_X(Support);
  FreeGraphSupportBuilder(&Builder);
  ASSERT_X(!Builder);

  Context.EdgeCount = GraphSupportGetEdgeCount(Support);
  ASSERT_X(Context.EdgeCount < MAX_EDGES);

  ASSERT_X(GraphSupportGetEdgeIndex(Support, 0, 1, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)1.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 1, 2, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)-2.0;
  ASSERT_X(GraphSupportGetEdgeIndex(Support, 2, 1, &EdgeIndex));
  Context.Weights[EdgeIndex] = (double)-2.0;

  Workspace = NewGraphWorkspace(Support);
  ASSERT_X(Workspace);

  ASSERT(GraphBellmanFord(Workspace, 1, (const size_t[]){0}, WeightFunction,
                          (void *)&Context, &NegativeCycles));
  ASSERT(NegativeCycles);

  Distance = GraphWorkspaceGetDistance(Workspace);
  Parent   = GraphWorkspaceGetParent(Workspace);
  ASSERT(Distance);
  ASSERT(Parent);

  FreeGraphWorkspace(&Workspace);
  ASSERT_X(!Workspace);
  FreeGraphSupport(&Support);
  ASSERT_X(!Support);
}

TEST_SUITE(GRTPBellmanFordSuite,
           GRTPBellmanFordIsolatedVertices,
           GRTPBellmanFordUnitWeightsCycle,
           GRTPBellmanFordDirectedDisjointComponents,
           GRTPBellmanFordNegativeCycle);
