#include "src/Core/Core.h"
#include "src/GRTP/GraphUtilities.h"
#include "src/GRTP/PriorityQueues.h"
#include <stdint.h>

struct GraphWorkspace {
  const GraphSupport   *Support;
  IndexedMinBinaryHeap *Heap;
  double               *Distance;
  size_t               *Parent;
};

void FreeGraphWorkspace(GraphWorkspace **WorkspaceReference) {
  if (!WorkspaceReference) {
    return;
  }
  if (*WorkspaceReference) {
    free((*WorkspaceReference)->Parent);
    free((*WorkspaceReference)->Distance);
    FreeIndexedMinBinaryHeap(&((*WorkspaceReference)->Heap));
    free((*WorkspaceReference));
    *WorkspaceReference = NULL;
  }
}

GraphWorkspace *NewGraphWorkspace(const GraphSupport *Support) {
  GraphWorkspace *Workspace = NULL;
  size_t          VertexCount, Size;
  bool            AllocatedWorkspace = false;
  bool            AllocatedHeap      = false;
  bool            AllocatedDistance  = false;

  if (!Support) {
    return NULL;
  }
  VertexCount = GraphSupportGetVertexCount(Support);
  Workspace   = (GraphWorkspace *)Core_Malloc(sizeof(GraphWorkspace));
  if (!Workspace) {
    return NULL;
  }
  AllocatedWorkspace = true;
  Workspace->Support = Support;
  Workspace->Heap    = NewIndexedMinBinaryHeap(VertexCount);
  if (!Workspace->Heap) {
    goto cleanup;
  }
  AllocatedHeap = true;
  if (Core_MulOverflow(VertexCount, sizeof(double), &Size)) {
    goto cleanup;
  }
  Workspace->Distance = (double *)Core_Malloc(Size);
  if (!Workspace->Distance) {
    goto cleanup;
  }
  AllocatedDistance = true;
  if (Core_MulOverflow(VertexCount, sizeof(size_t), &Size)) {
    goto cleanup;
  }
  Workspace->Parent = (size_t *)Core_Malloc(Size);
  if (!Workspace->Parent) {
    goto cleanup;
  }
  return Workspace;
cleanup:
  if (AllocatedDistance)
    free(Workspace->Distance);
  if (AllocatedHeap)
    FreeIndexedMinBinaryHeap(&(Workspace->Heap));
  if (AllocatedWorkspace)
    free(Workspace);
  return NULL;
}

const double *GraphWorkspaceGetDistance(const GraphWorkspace *Workspace) {
  if (!Workspace) {
    return NULL;
  }
  return (const double *)Workspace->Distance;
}

const size_t *GraphWorkspaceGetParent(const GraphWorkspace *Workspace) {
  if (!Workspace) {
    return NULL;
  }
  return (const size_t *)Workspace->Parent;
}

static bool DijkstraExpandFrontier(const GraphSupport       *Support,
                                   const GraphWeightFunction WeightFunction,
                                   void                     *UserContext,
                                   IndexedMinBinaryHeap     *Heap,
                                   double                   *Distance,
                                   size_t                   *Parent,
                                   size_t                   *ExpandedVertex) {
  size_t        Vertex, NeighborCount, Neighbor, i;
  double        Priority, Weight, CandidateDistance;
  const size_t *NeighborIndices = NULL;

  Priority = UnreachableDistance;
  IndexedMinBinaryHeapExtractMinimum(Heap, &Vertex, &Priority);
  if (isinf(Priority)) {
    return true;
  }
  NeighborCount   = GraphSupportGetNeighborCount(Support, Vertex);
  NeighborIndices = (const size_t *)GraphSupportGetNeighbors(Support, Vertex);
  for (i = 0; i < NeighborCount; i++) {
    Neighbor = NeighborIndices[i];
    if (WeightFunction) {
      Weight = fabs(WeightFunction(Support, Vertex, Neighbor, UserContext));
    } else {
      Weight = (double)1.0;
    }
    CandidateDistance = Weight + Priority;
    if (CandidateDistance < Distance[Neighbor]) {
      Distance[Neighbor] = CandidateDistance;
      Parent[Neighbor]   = Vertex;
      IndexedMinBinaryHeapDecreaseKey(Heap, Neighbor, CandidateDistance);
    }
  }
  if (ExpandedVertex) {
    *ExpandedVertex = Vertex;
  }
  return false;
}

static bool GraphDijkstra_Impl(const GraphSupport       *Support,
                               const GraphWeightFunction WeightFunction,
                               void                     *UserContext,
                               const size_t              SourceCount,
                               const size_t              Source[SourceCount],
                               IndexedMinBinaryHeap     *Heap,
                               double                   *Distance,
                               size_t                   *Parent) {
  size_t VertexCount, i;

  VertexCount = GraphSupportGetVertexCount(Support);
  IndexedMinBinaryHeapInitialize(Heap);
  for (i = 0; i < VertexCount; i++) {
    Distance[i] = UnreachableDistance;
    Parent[i]   = UnreachableVertex;
  }
  for (i = 0; i < SourceCount; i++) {
    if (Source[i] >= VertexCount) {
      return false;
    }
    Distance[Source[i]] = (double)0.0;
    IndexedMinBinaryHeapDecreaseKey(Heap, Source[i], (double)0.0);
  }
  while (!IndexedMinBinaryHeapIsEmpty(Heap)) {
    if (DijkstraExpandFrontier(Support, WeightFunction, UserContext, Heap,
                               Distance, Parent, NULL)) {
      break;
    }
  }
  return true;
}

static bool
GraphBidirectionalDijkstra_Impl(const GraphSupport       *ForwardSupport,
                                const GraphSupport       *BackwardSupport,
                                const GraphWeightFunction WeightFunction,
                                void                     *UserContext,
                                const size_t              Source,
                                const size_t              Target,
                                IndexedMinBinaryHeap     *ForwardHeap,
                                double                   *ForwardDistance,
                                size_t                   *ForwardParent,
                                IndexedMinBinaryHeap     *BackwardHeap,
                                double                   *BackwardDistance,
                                size_t                   *BackwardParent,
                                double                   *Distance,
                                size_t                   *Meeting) {
  size_t VertexCount, ExpandedVertex, BestMeeting, i;
  double ForwardPriority, BackwardPriority, TotalDistance, BestDistance;

  VertexCount = GraphSupportGetVertexCount(ForwardSupport);
  if ((Source >= VertexCount) || (Target >= VertexCount)) {
    return false;
  }
  BestDistance = UnreachableDistance;
  BestMeeting  = UnreachableVertex;
  IndexedMinBinaryHeapInitialize(ForwardHeap);
  IndexedMinBinaryHeapInitialize(BackwardHeap);
  for (i = 0; i < VertexCount; i++) {
    ForwardDistance[i]  = UnreachableDistance;
    ForwardParent[i]    = UnreachableVertex;
    BackwardDistance[i] = UnreachableDistance;
    BackwardParent[i]   = UnreachableVertex;
  }
  ForwardDistance[Source] = (double)0.0;
  IndexedMinBinaryHeapDecreaseKey(ForwardHeap, Source, (double)0.0);
  BackwardDistance[Target] = (double)0.0;
  IndexedMinBinaryHeapDecreaseKey(BackwardHeap, Target, (double)0.0);
  while ((!IndexedMinBinaryHeapIsEmpty(ForwardHeap)) &&
         (!IndexedMinBinaryHeapIsEmpty(BackwardHeap))) {
    IndexedMinBinaryHeapPeekMinimum(ForwardHeap, NULL, &ForwardPriority);
    IndexedMinBinaryHeapPeekMinimum(BackwardHeap, NULL, &BackwardPriority);
    if ((isinf(ForwardPriority)) || (isinf(BackwardPriority))) {
      break;
    }
    if ((!isinf(BestDistance)) &&
        (ForwardPriority + BackwardPriority >= BestDistance)) {
      break;
    }
    if (ForwardPriority <= BackwardPriority) {
      DijkstraExpandFrontier(ForwardSupport, WeightFunction, UserContext,
                             ForwardHeap, ForwardDistance, ForwardParent,
                             &ExpandedVertex);
    } else {
      DijkstraExpandFrontier(BackwardSupport, WeightFunction, UserContext,
                             BackwardHeap, BackwardDistance, BackwardParent,
                             &ExpandedVertex);
    }
    if ((!isinf(ForwardDistance[ExpandedVertex])) &&
        (!isinf(BackwardDistance[ExpandedVertex]))) {
      TotalDistance =
          ForwardDistance[ExpandedVertex] + BackwardDistance[ExpandedVertex];
      if (TotalDistance < BestDistance) {
        BestDistance = TotalDistance;
        BestMeeting  = ExpandedVertex;
      }
    }
  }
  *Distance = BestDistance;
  *Meeting  = BestMeeting;
  return true;
}

bool GraphDijkstra(GraphWorkspace           *Workspace,
                   const size_t              SourceCount,
                   const size_t              Source[SourceCount],
                   const GraphWeightFunction WeightFunction,
                   void                     *UserContext) {
  if (!Workspace) {
    return false;
  }
  if (SourceCount == 0) {
    return false;
  }
  if (!Source) {
    return false;
  }
  return GraphDijkstra_Impl(Workspace->Support, WeightFunction, UserContext,
                            SourceCount, Source, Workspace->Heap,
                            Workspace->Distance, Workspace->Parent);
}

bool GraphBidirectionalDijkstra(GraphWorkspace           *ForwardWorkspace,
                                GraphWorkspace           *BackwardWorkspace,
                                const size_t              Source,
                                const size_t              Target,
                                const GraphWeightFunction WeightFunction,
                                void                     *UserContext,
                                size_t                   *ShortestPath,
                                size_t *ShortestPathVertexCount,
                                double *ShortestDistance) {
  bool   Status = false;
  double Distance;
  size_t Meeting, Current, ForwardSteps, BackwardSteps, i;

  if (!ForwardWorkspace) {
    return Status;
  }
  if (!BackwardWorkspace) {
    return Status;
  }
  if (ForwardWorkspace == BackwardWorkspace) {
    return Status;
  }
  if (GraphSupportGetVertexCount(ForwardWorkspace->Support) !=
      GraphSupportGetVertexCount(BackwardWorkspace->Support)) {
    return Status;
  }
  if (GraphSupportGetEdgeCount(ForwardWorkspace->Support) !=
      GraphSupportGetEdgeCount(BackwardWorkspace->Support)) {
    return Status;
  }
  Status = GraphBidirectionalDijkstra_Impl(
      ForwardWorkspace->Support, BackwardWorkspace->Support, WeightFunction,
      UserContext, Source, Target, ForwardWorkspace->Heap,
      ForwardWorkspace->Distance, ForwardWorkspace->Parent,
      BackwardWorkspace->Heap, BackwardWorkspace->Distance,
      BackwardWorkspace->Parent, &Distance, &Meeting);
  if (Status) {
    if (Meeting != UnreachableVertex) {
      ForwardSteps = 0;
      Current      = Meeting;
      while (Current != Source) {
        Current = ForwardWorkspace->Parent[Current];
        ForwardSteps++;
      }
      BackwardSteps = 0;
      Current       = Meeting;
      while (Current != Target) {
        Current = BackwardWorkspace->Parent[Current];
        BackwardSteps++;
      }
    }
  }
  if ((Status) && (ShortestPath)) {
    if (Meeting != UnreachableVertex) {
      ShortestPath[ForwardSteps] = Meeting;
      for (i = 0; i < ForwardSteps; i++) {
        ShortestPath[ForwardSteps - (i + 1)] =
            ForwardWorkspace->Parent[ShortestPath[ForwardSteps - i]];
      }
      for (i = 0; i < BackwardSteps; i++) {
        ShortestPath[ForwardSteps + (i + 1)] =
            BackwardWorkspace->Parent[ShortestPath[ForwardSteps + i]];
      }
    }
  }
  if ((Status) && (ShortestPathVertexCount)) {
    if (Meeting != UnreachableVertex) {
      *ShortestPathVertexCount = ForwardSteps + BackwardSteps + 1;
    } else {
      *ShortestPathVertexCount = 0;
    }
  }
  if ((Status) && (ShortestDistance)) {
    *ShortestDistance = Distance;
  }
  return Status;
}

static bool GraphBellmanFord_Impl(const GraphSupport       *Support,
                                  const GraphWeightFunction WeightFunction,
                                  void                     *UserContext,
                                  const size_t              SourceCount,
                                  const size_t              Source[SourceCount],
                                  double                   *Distance,
                                  size_t                   *Parent,
                                  bool                     *NegativeCycles) {
  size_t VertexCount, EdgeCount, Edge, Origin, Destination, i;
  double Weight, CandidateDistance;
  bool   Changed;

  VertexCount = GraphSupportGetVertexCount(Support);
  EdgeCount   = GraphSupportGetEdgeCount(Support);
  for (i = 0; i < VertexCount; i++) {
    Distance[i] = UnreachableDistance;
    Parent[i]   = UnreachableVertex;
  }
  for (i = 0; i < SourceCount; i++) {
    if (Source[i] >= VertexCount) {
      return false;
    }
    Distance[Source[i]] = (double)0.0;
  }
  for (i = 0; i < VertexCount - 1; i++) {
    Changed = false;
    for (Edge = 0; Edge < EdgeCount; Edge++) {
      GraphSupportGetEdgeVertices(Support, Edge, &Origin, &Destination);
      if (isinf(Distance[Origin])) {
        continue;
      }
      if (WeightFunction) {
        Weight = WeightFunction(Support, Origin, Destination, UserContext);
      } else {
        Weight = (double)1.0;
      }
      CandidateDistance = Distance[Origin] + Weight;
      if (CandidateDistance < Distance[Destination]) {
        Distance[Destination] = CandidateDistance;
        Parent[Destination]   = Origin;
        Changed               = true;
      }
    }
    if (!Changed) {
      break;
    }
  }
  // Negative-cycle detection.
  if (NegativeCycles) {
    *NegativeCycles = false;
    for (Edge = 0; Edge < EdgeCount; Edge++) {
      GraphSupportGetEdgeVertices(Support, Edge, &Origin, &Destination);
      if (isinf(Distance[Origin])) {
        continue;
      }
      if (WeightFunction) {
        Weight = WeightFunction(Support, Origin, Destination, UserContext);
      } else {
        Weight = (double)1.0;
      }
      CandidateDistance = Distance[Origin] + Weight;
      if (CandidateDistance < Distance[Destination]) {
        *NegativeCycles = true;
        break;
      }
    }
  }
  return true;
}

bool GraphBellmanFord(GraphWorkspace           *Workspace,
                      const size_t              SourceCount,
                      const size_t              Source[SourceCount],
                      const GraphWeightFunction WeightFunction,
                      void                     *UserContext,
                      bool                     *NegativeCycles) {
  if (!Workspace) {
    return false;
  }
  if (SourceCount == 0) {
    return false;
  }
  if (!Source) {
    return false;
  }
  return GraphBellmanFord_Impl(Workspace->Support, WeightFunction, UserContext,
                               SourceCount, Source, Workspace->Distance,
                               Workspace->Parent, NegativeCycles);
}
