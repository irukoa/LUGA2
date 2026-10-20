#include "src/Core/Core.h"
#include "src/GRTP/GraphUtilities.h"

const double C_INFINITY = INFINITY;
const size_t C_SIZE_MAX = SIZE_MAX;

static int CompareSizeT(const void *Lhs,
                        const void *Rhs) {
  const size_t x = *(const size_t *)Lhs;
  const size_t y = *(const size_t *)Rhs;

  return (x > y) - (x < y);
}

struct GraphSupportBuilder {
  size_t   VertexCount;
  size_t  *VertexEdgeCount; // VertexEdgeCount[i] = N_i. Number of connections
                            // from vertex #i = [0, VertexCount - 1].
  size_t **VertexConnectivity; // VertexConnectivity[i][j] = V_ij. The vertex
                               // that vertex #i connects to with connection
                               // label #j = [0, N_i - 1].
};

void FreeGraphSupportBuilder(GraphSupportBuilder **BuilderReference) {
  size_t i;
  if (!BuilderReference) {
    return;
  }
  if (*BuilderReference) {
    for (i = 0; i < (*BuilderReference)->VertexCount; i++) {
      free((*BuilderReference)->VertexConnectivity[i]);
    }
    free((*BuilderReference)->VertexConnectivity);
    free((*BuilderReference)->VertexEdgeCount);
    free((*BuilderReference));
    *BuilderReference = NULL;
  }
}

GraphSupportBuilder *NewGraphSupportBuilder(const size_t VertexCount) {
  GraphSupportBuilder *Builder = NULL;
  size_t               i, Size;
  bool                 AllocatedBuilder         = false;
  bool                 AllocatedVertexEdgeCount = false;

  if (VertexCount == 0) {
    return NULL;
  }
  Builder = (GraphSupportBuilder *)Core_Malloc(sizeof(GraphSupportBuilder));
  if (!Builder) {
    return NULL;
  }
  AllocatedBuilder     = true;
  Builder->VertexCount = VertexCount;
  if (Core_MulOverflow(VertexCount, sizeof(size_t), &Size)) {
    goto cleanup;
  }
  Builder->VertexEdgeCount = (size_t *)Core_Malloc(Size);
  if (!Builder->VertexEdgeCount) {
    goto cleanup;
  }
  AllocatedVertexEdgeCount = true;
  for (i = 0; i < VertexCount; i++) {
    Builder->VertexEdgeCount[i] = 0;
  }
  if (Core_MulOverflow(VertexCount, sizeof(size_t *), &Size)) {
    goto cleanup;
  }
  Builder->VertexConnectivity = (size_t **)Core_Malloc(Size);
  if (!Builder->VertexConnectivity) {
    goto cleanup;
  }
  for (i = 0; i < VertexCount; i++) {
    Builder->VertexConnectivity[i] = NULL;
  }
  return Builder;
cleanup:
  if (AllocatedVertexEdgeCount)
    free(Builder->VertexEdgeCount);
  if (AllocatedBuilder)
    free(Builder);
  return NULL;
}

bool GraphSupportBuilderAddEdge(GraphSupportBuilder *Builder,
                                const size_t         Origin,
                                const size_t         Destination) {
  size_t i, NewElementCount, NewSize;
  void  *TmpConnectivity = NULL;

  if (!Builder) {
    return false;
  }
  if ((Origin >= Builder->VertexCount) ||
      (Destination >= Builder->VertexCount)) {
    return false;
  }
  for (i = 0; i < Builder->VertexEdgeCount[Origin]; i++) {
    if (Builder->VertexConnectivity[Origin][i] == Destination) {
      return false;
    }
  }
  if (Core_AddOverflow(Builder->VertexEdgeCount[Origin], (size_t)1,
                       &NewElementCount)) {
    return false;
  }
  if (Core_MulOverflow(sizeof(size_t), NewElementCount, &NewSize)) {
    return false;
  }
  TmpConnectivity =
      Core_Realloc((void *)Builder->VertexConnectivity[Origin], NewSize);
  if (!TmpConnectivity) {
    return false;
  }
  Builder->VertexConnectivity[Origin] = (size_t *)TmpConnectivity;
  Builder->VertexConnectivity[Origin][Builder->VertexEdgeCount[Origin]] =
      Destination;
  Builder->VertexEdgeCount[Origin] = NewElementCount;
  return true;
}

bool GraphSupportBuilderRemoveEdge(GraphSupportBuilder *Builder,
                                   const size_t         Origin,
                                   const size_t         Destination) {
  size_t i, TmpIndex;
  void  *TmpConnectivity = NULL;
  bool   Match           = false;

  if (!Builder) {
    return false;
  }
  if ((Origin >= Builder->VertexCount) ||
      (Destination >= Builder->VertexCount)) {
    return false;
  }
  if (Builder->VertexEdgeCount[Origin] == 0) {
    return false;
  }
  for (i = 0; i < Builder->VertexEdgeCount[Origin]; i++) {
    if (Builder->VertexConnectivity[Origin][i] == Destination) {
      Match = true;
      break;
    }
  }
  if (!Match) {
    return false;
  }
  TmpIndex =
      Builder->VertexConnectivity[Origin][Builder->VertexEdgeCount[Origin] - 1];
  Builder->VertexConnectivity[Origin][Builder->VertexEdgeCount[Origin] - 1] =
      Builder->VertexConnectivity[Origin][i];
  Builder->VertexConnectivity[Origin][i] = TmpIndex;

  if (Builder->VertexEdgeCount[Origin] - 1 == 0) {
    free(Builder->VertexConnectivity[Origin]);
    Builder->VertexConnectivity[Origin] = NULL;
  } else {
    TmpConnectivity =
        Core_Realloc((void *)Builder->VertexConnectivity[Origin],
                     sizeof(size_t) * (Builder->VertexEdgeCount[Origin] - 1));
    if (!TmpConnectivity) {
      return false;
    }
    Builder->VertexConnectivity[Origin] = (size_t *)TmpConnectivity;
  }
  Builder->VertexEdgeCount[Origin] = Builder->VertexEdgeCount[Origin] - 1;
  return true;
}

size_t GraphSupportBuilderGetVertexCount(const GraphSupportBuilder *Builder) {
  if (!Builder) {
    return 0;
  }
  return Builder->VertexCount;
}

size_t GraphSupportBuilderGetEdgeCount(const GraphSupportBuilder *Builder) {
  size_t i, Count;

  if (!Builder) {
    return 0;
  }
  Count = 0;
  for (i = 0; i < Builder->VertexCount; i++) {
    if (Core_AddOverflow(Count, Builder->VertexEdgeCount[i], &Count)) {
      return 0;
    }
  }
  return Count;
}

size_t GraphSupportBuilderGetNeighborCount(const GraphSupportBuilder *Builder,
                                           const size_t               Vertex) {
  if (!Builder) {
    return 0;
  }
  if (Vertex >= Builder->VertexCount) {
    return 0;
  }
  return Builder->VertexEdgeCount[Vertex];
}

const size_t *
GraphSupportBuilderGetNeighbors(const GraphSupportBuilder *Builder,
                                const size_t               Vertex) {
  if (!Builder) {
    return NULL;
  }
  if (Vertex >= Builder->VertexCount) {
    return NULL;
  }
  return Builder->VertexConnectivity[Vertex];
}

bool GraphSupportBuilderHasEdge(const GraphSupportBuilder *Builder,
                                const size_t               Origin,
                                const size_t               Destination) {
  bool   Match = false;
  size_t i;

  if (!Builder) {
    return Match;
  }
  if ((Origin >= Builder->VertexCount) ||
      (Destination >= Builder->VertexCount)) {
    return Match;
  }
  for (i = 0; i < Builder->VertexEdgeCount[Origin]; i++) {
    if (Builder->VertexConnectivity[Origin][i] == Destination) {
      Match = true;
      break;
    }
  }
  return Match;
}

struct GraphSupport {
  size_t  VertexCount;
  size_t  EdgeCount;
  size_t *VertexOffsets;
  size_t *FlatVertexConnectivity;
};

void FreeGraphSupport(GraphSupport **SupportReference) {
  if (!SupportReference) {
    return;
  }
  if (*SupportReference) {
    free((*SupportReference)->VertexOffsets);
    free((*SupportReference)->FlatVertexConnectivity);
    free((*SupportReference));
    *SupportReference = NULL;
  }
}

GraphSupport *NewGraphSupport(const GraphSupportBuilder *Builder) {
  GraphSupport *Support = NULL;
  size_t        i, j, Counter, Size, EdgeCount;
  bool          AllocatedSupport       = false;
  bool          AllocatedVertexOffsets = false;

  if (!Builder) {
    return NULL;
  }
  Support = (GraphSupport *)Core_Malloc(sizeof(GraphSupport));
  if (!Support) {
    return NULL;
  }
  AllocatedSupport     = true;
  Support->VertexCount = Builder->VertexCount;
  if (Core_AddOverflow(Support->VertexCount, 1, &Size)) {
    goto cleanup;
  }
  if (Core_MulOverflow(Size, sizeof(size_t), &Size)) {
    goto cleanup;
  }
  Support->VertexOffsets = (size_t *)Core_Malloc(Size);
  if (!Support->VertexOffsets) {
    goto cleanup;
  }
  AllocatedVertexOffsets = true;
  EdgeCount              = 0;
  for (i = 0; i < Builder->VertexCount; i++) {
    Support->VertexOffsets[i] = EdgeCount;
    if (Core_AddOverflow(EdgeCount, Builder->VertexEdgeCount[i], &EdgeCount)) {
      goto cleanup;
    }
  }
  Support->EdgeCount                           = EdgeCount;
  Support->VertexOffsets[Builder->VertexCount] = Support->EdgeCount;
  if (Support->EdgeCount > 0) {
    if (Core_MulOverflow(Support->EdgeCount, sizeof(size_t), &Size)) {
      goto cleanup;
    }
    Support->FlatVertexConnectivity = (size_t *)Core_Malloc(Size);
    if (!Support->FlatVertexConnectivity) {
      goto cleanup;
    }
  } else {
    Support->FlatVertexConnectivity = NULL;
  }
  Counter = 0;
  for (i = 0; i < Builder->VertexCount; i++) {
    for (j = 0; j < Builder->VertexEdgeCount[i]; j++) {
      Support->FlatVertexConnectivity[Counter] =
          Builder->VertexConnectivity[i][j];
      Counter++;
    }
    qsort(&(Support->FlatVertexConnectivity[Support->VertexOffsets[i]]),
          Builder->VertexEdgeCount[i], sizeof(size_t), CompareSizeT);
  }
  return Support;
cleanup:
  if (AllocatedVertexOffsets)
    free(Support->VertexOffsets);
  if (AllocatedSupport)
    free(Support);
  return NULL;
}

size_t GraphSupportGetVertexCount(const GraphSupport *Support) {
  if (!Support) {
    return 0;
  }
  return Support->VertexCount;
}

size_t GraphSupportGetEdgeCount(const GraphSupport *Support) {
  if (!Support) {
    return 0;
  }
  return Support->EdgeCount;
}

size_t GraphSupportGetNeighborCount(const GraphSupport *Support,
                                    const size_t        Vertex) {
  if (!Support) {
    return 0;
  }
  if (Vertex >= Support->VertexCount) {
    return 0;
  }
  return Support->VertexOffsets[Vertex + 1] - Support->VertexOffsets[Vertex];
}

const size_t *GraphSupportGetNeighbors(const GraphSupport *Support,
                                       const size_t        Vertex) {
  if (!Support) {
    return NULL;
  }
  if (Vertex >= Support->VertexCount) {
    return NULL;
  }
  return &(Support->FlatVertexConnectivity[Support->VertexOffsets[Vertex]]);
}

bool GraphSupportGetEdgeIndex(const GraphSupport *Support,
                              const size_t        Origin,
                              const size_t        Destination,
                              size_t             *EdgeIndex) {
  size_t NeighborCount;
  size_t Left, Right, Middle;
  size_t Offset;

  if (!Support) {
    return false;
  }
  if (!EdgeIndex) {
    return false;
  }
  if ((Origin >= Support->VertexCount) ||
      (Destination >= Support->VertexCount)) {
    return false;
  }
  NeighborCount = GraphSupportGetNeighborCount(Support, Origin);
  Left          = 0;
  Right         = NeighborCount;
  while (Left < Right) {
    Middle = Left + (Right - Left) / 2;
    Offset = Support->VertexOffsets[Origin] + Middle;
    if (Support->FlatVertexConnectivity[Offset] < Destination) {
      Left = Middle + 1;
    } else if (Support->FlatVertexConnectivity[Offset] > Destination) {
      Right = Middle;
    } else {
      *EdgeIndex = Offset;
      return true;
    }
  }
  return false;
}

bool GraphSupportGetEdgeVertices(const GraphSupport *Support,
                                 const size_t        EdgeIndex,
                                 size_t             *Origin,
                                 size_t             *Destination) {
  size_t Left, Right, Middle;

  if (!Support) {
    return false;
  }
  if ((!Origin) || (!Destination)) {
    return false;
  }
  if (EdgeIndex >= Support->EdgeCount) {
    return false;
  }
  Left  = 0;
  Right = Support->VertexCount;
  while (Left + 1 < Right) {
    Middle = Left + (Right - Left) / 2;
    if (Support->VertexOffsets[Middle] <= EdgeIndex) {
      Left = Middle;
    } else {
      Right = Middle;
    }
  }
  *Origin      = Left;
  *Destination = Support->FlatVertexConnectivity[EdgeIndex];
  return true;
}

bool GraphSupportHasEdge(const GraphSupport *Support,
                         const size_t        Origin,
                         const size_t        Destination) {
  size_t EdgeIndex;

  return GraphSupportGetEdgeIndex(Support, Origin, Destination, &EdgeIndex);
}
