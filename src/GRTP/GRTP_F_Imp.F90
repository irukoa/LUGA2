#include "src/GRTP/GRTP_C_Int.M90"
#include "src/Core/CoreFortran.M90"

submodule (GraphUtilities:GRTP_C_Interfaces) GRTP_F_Implementation

  use, intrinsic :: ISO_C_BINDING, only: C_F_POINTER, &
                                         C_ASSOCIATED, &
                                         C_FUNLOC, &
                                         C_LOC, &
                                         C_NULL_FUNPTR, &
                                         C_SIZEOF

  use CoreFortran, only: FortranMulOverflow, FortranMalloc, FortranFree

  implicit none

  type :: FortranContextWrapper
    procedure(GraphWeightFunction), pointer, nopass :: WeightFunction => null()
    class(*),                       pointer         :: Context      => null()
  end type FortranContextWrapper

  contains

  !< Weighting callback exposed to C.
  !< Declared in the ancestor module with the BIND attribute and implemented
  !< here as a separate module procedure, thus reachable through a plain code
  !< address. All the call state travels through the opaque user context
  !< pointer. Shared by every graph algorithm.
  !< RECURSIVE so that each invocation owns its locals even when a weight
  !< function starts a nested algorithm run.
  module procedure FortranWeightCallback

    type(GraphSupport)                      :: LocalSupport
    type(FortranContextWrapper),    pointer :: LocalWrapper
    procedure(GraphWeightFunction), pointer :: LocalWeightFunction

    !Non-owning view of the C-side support (avoids finalization).
    LocalSupport%OpaqueHandle = FSideCSupport
    LocalSupport%Init         = .true.
    LocalSupport%Owner        = .false.

    call C_F_POINTER(FSideCUserContext, LocalWrapper)
    LocalWeightFunction => LocalWrapper%WeightFunction
    if (associated(LocalWrapper%Context)) then
      FSideCWeight = LocalWeightFunction(Support = LocalSupport, &
                                         Origin = FSideCOrigin, &
                                         Destination = FSideCDestination, &
                                         UserContext = LocalWrapper%Context)
    else
      FSideCWeight = LocalWeightFunction(Support = LocalSupport, &
                                         Origin = FSideCOrigin, &
                                         Destination = FSideCDestination)
    endif
  end procedure FortranWeightCallback

  module procedure FortranCopyGraphSupportBuilder
    Copy%OpaqueHandle = Original%OpaqueHandle
    Copy%Init         = Original%Init
    Copy%Owner        = .false.
  end procedure FortranCopyGraphSupportBuilder

  module procedure FortranFreeGraphSupportBuilder
    if (Builder%Owner) then
      call FreeGraphSupportBuilder(Builder%OpaqueHandle)
    else
      Builder%OpaqueHandle = C_NULL_PTR
    endif
    Builder%Init = .false.
  end procedure FortranFreeGraphSupportBuilder

  module procedure FortranFreeGraphSupportBuilder_Final
    if (Builder%Owner) then
      call FreeGraphSupportBuilder(Builder%OpaqueHandle)
    else
      Builder%OpaqueHandle = C_NULL_PTR
    endif
    Builder%Init = .false.
  end procedure FortranFreeGraphSupportBuilder_Final

  module procedure FortranNewGraphSupportBuilder
    if (C_ASSOCIATED(Builder%OpaqueHandle)) call FortranFreeGraphSupportBuilder(Builder)

    Builder%OpaqueHandle = NewGraphSupportBuilder(VertexCount = VertexCount)

    if (C_ASSOCIATED(Builder%OpaqueHandle)) Builder%Init  = .true.
    if (C_ASSOCIATED(Builder%OpaqueHandle)) Builder%Owner = .true.
  end procedure FortranNewGraphSupportBuilder

  module procedure FortranGraphSupportBuilderInitialized
    Initialized = Builder%Init
  end procedure FortranGraphSupportBuilderInitialized

  module procedure FortranGraphSupportBuilderAddEdge
    Added = logical(GraphSupportBuilderAddEdge(OpaqueHandle = Builder%OpaqueHandle, &
                                               Origin      = Origin, &
                                               Destination = Destination))
  end procedure FortranGraphSupportBuilderAddEdge

  module procedure FortranGraphSupportBuilderRemoveEdge
    Removed = logical(GraphSupportBuilderRemoveEdge(OpaqueHandle = Builder%OpaqueHandle, &
                                                    Origin      = Origin, &
                                                    Destination = Destination))
  end procedure FortranGraphSupportBuilderRemoveEdge

  module procedure FortranGraphSupportBuilderVertexCount
    VertexCount = GraphSupportBuilderGetVertexCount(Builder%OpaqueHandle)
  end procedure FortranGraphSupportBuilderVertexCount

  module procedure FortranGraphSupportBuilderEdgeCount
    EdgeCount = GraphSupportBuilderGetEdgeCount(Builder%OpaqueHandle)
  end procedure FortranGraphSupportBuilderEdgeCount

  module procedure FortranGraphSupportBuilderNeighborCount
    NeighborCount = GraphSupportBuilderGetNeighborCount(OpaqueHandle = Builder%OpaqueHandle, &
                                                        Vertex      = Vertex)
  end procedure FortranGraphSupportBuilderNeighborCount

  module procedure FortranGraphSupportBuilderNeighbors
    type(C_PTR)                :: NeighborsCPtr
    integer(C_SIZE_T), pointer :: NeighborsFPtr(:)

    if (size(Neighbors) == 0) return

    NeighborsCPtr = GraphSupportBuilderGetNeighbors(OpaqueHandle = Builder%OpaqueHandle, &
                                                    Vertex      = Vertex)

    if (C_ASSOCIATED(NeighborsCPtr)) then
      call C_F_POINTER(NeighborsCPtr, &
                       NeighborsFPtr, &
                       [size(Neighbors)])
      Neighbors = NeighborsFPtr
    endif
  end procedure FortranGraphSupportBuilderNeighbors

  module procedure FortranGraphSupportBuilderHasEdge
    HasEdge = logical(GraphSupportBuilderHasEdge(OpaqueHandle = Builder%OpaqueHandle, &
                                                 Origin      = Origin, &
                                                 Destination = Destination))
  end procedure FortranGraphSupportBuilderHasEdge

  module procedure FortranCopyGraphSupport
    Copy%OpaqueHandle = Original%OpaqueHandle
    Copy%Init         = Original%Init
    Copy%Owner        = .false.
  end procedure FortranCopyGraphSupport

  module procedure FortranFreeGraphSupport
    if (Support%Owner) then
      call FreeGraphSupport(Support%OpaqueHandle)
    else
      Support%OpaqueHandle = C_NULL_PTR
    endif
    Support%Init = .false.
  end procedure FortranFreeGraphSupport

  module procedure FortranFreeGraphSupport_Final
    if (Support%Owner) then
      call FreeGraphSupport(Support%OpaqueHandle)
    else
      Support%OpaqueHandle = C_NULL_PTR
    endif
    Support%Init = .false.
  end procedure FortranFreeGraphSupport_Final

  module procedure FortranNewGraphSupport
    if (C_ASSOCIATED(Support%OpaqueHandle)) call FortranFreeGraphSupport(Support)

    Support%OpaqueHandle = NewGraphSupport(BuilderHandle = Builder%OpaqueHandle)

    if (C_ASSOCIATED(Support%OpaqueHandle)) Support%Init  = .true.
    if (C_ASSOCIATED(Support%OpaqueHandle)) Support%Owner = .true.
  end procedure FortranNewGraphSupport

  module procedure FortranGraphSupportInitialized
    Initialized = Support%Init
  end procedure FortranGraphSupportInitialized

  module procedure FortranGraphSupportVertexCount
    VertexCount = GraphSupportGetVertexCount(Support%OpaqueHandle)
  end procedure FortranGraphSupportVertexCount

  module procedure FortranGraphSupportEdgeCount
    EdgeCount = GraphSupportGetEdgeCount(Support%OpaqueHandle)
  end procedure FortranGraphSupportEdgeCount

  module procedure FortranGraphSupportNeighborCount
    NeighborCount = GraphSupportGetNeighborCount(OpaqueHandle = Support%OpaqueHandle, &
                                                 Vertex      = Vertex)
  end procedure FortranGraphSupportNeighborCount

  module procedure FortranGraphSupportNeighbors
    type(C_PTR)                :: NeighborsCPtr
    integer(C_SIZE_T), pointer :: NeighborsFPtr(:)

    if (size(Neighbors) == 0) return

    NeighborsCPtr = GraphSupportGetNeighbors(OpaqueHandle = Support%OpaqueHandle, &
                                             Vertex      = Vertex)

    if (C_ASSOCIATED(NeighborsCPtr)) then
      call C_F_POINTER(NeighborsCPtr, &
                       NeighborsFPtr, &
                       [size(Neighbors)])
      Neighbors = NeighborsFPtr
    endif
  end procedure FortranGraphSupportNeighbors

  module procedure FortranGraphSupportEdgeIndex
    EdgeIndex = UnreachableVertex

    HasEdge = logical(GraphSupportGetEdgeIndex(OpaqueHandle = Support%OpaqueHandle, &
                                               Origin      = Origin, &
                                               Destination = Destination, &
                                               EdgeIndex   = EdgeIndex))
  end procedure FortranGraphSupportEdgeIndex

  module procedure FortranGraphSupportEdgeVertices
    Origin      = UnreachableVertex
    Destination = UnreachableVertex

    HasEdge = logical(GraphSupportGetEdgeVertices(OpaqueHandle = Support%OpaqueHandle, &
                                                  EdgeIndex   = EdgeIndex, &
                                                  Origin      = Origin, &
                                                  Destination = Destination))
  end procedure FortranGraphSupportEdgeVertices

  module procedure FortranGraphSupportHasEdge
    HasEdge = logical(GraphSupportHasEdge(OpaqueHandle = Support%OpaqueHandle, &
                                          Origin      = Origin, &
                                          Destination = Destination))
  end procedure FortranGraphSupportHasEdge

  module procedure FortranCopyGraphWorkspace
    Copy%OpaqueHandle = Original%OpaqueHandle
    Copy%VertexCount  = Original%VertexCount
    Copy%Init         = Original%Init
    Copy%Owner        = .false.
  end procedure FortranCopyGraphWorkspace

  module procedure FortranFreeGraphWorkspace
    if (Workspace%Owner) then
      call FreeGraphWorkspace(Workspace%OpaqueHandle)
    else
      Workspace%OpaqueHandle = C_NULL_PTR
    endif
    Workspace%VertexCount = 0_C_SIZE_T
    Workspace%Init        = .false.
  end procedure FortranFreeGraphWorkspace

  module procedure FortranFreeGraphWorkspace_Final
    if (Workspace%Owner) then
      call FreeGraphWorkspace(Workspace%OpaqueHandle)
    else
      Workspace%OpaqueHandle = C_NULL_PTR
    endif
    Workspace%VertexCount = 0_C_SIZE_T
    Workspace%Init        = .false.
  end procedure FortranFreeGraphWorkspace_Final

  module procedure FortranNewGraphWorkspace
    if (C_ASSOCIATED(Workspace%OpaqueHandle)) call FortranFreeGraphWorkspace(Workspace)

    Workspace%OpaqueHandle = NewGraphWorkspace(SupportHandle = Support%OpaqueHandle)

    if (C_ASSOCIATED(Workspace%OpaqueHandle)) Workspace%Init        = .true.
    if (C_ASSOCIATED(Workspace%OpaqueHandle)) Workspace%Owner       = .true.
    if (C_ASSOCIATED(Workspace%OpaqueHandle)) Workspace%VertexCount = Support%VertexCount()
  end procedure FortranNewGraphWorkspace

  module procedure FortranGraphWorkspaceInitialized
    Initialized = Workspace%Init
  end procedure FortranGraphWorkspaceInitialized

  module procedure FortranGraphWorkspaceDistance
    type(C_PTR)             :: DistanceCPtr
    real(C_DOUBLE), pointer :: DistanceFPtr(:)

    if (size(Distance) == 0) return

    DistanceCPtr = GraphWorkspaceGetDistance(Workspace%OpaqueHandle)

    if (C_ASSOCIATED(DistanceCPtr)) then
      call C_F_POINTER(DistanceCPtr, &
                       DistanceFPtr, &
                       [size(Distance)])
      Distance = DistanceFPtr
    endif
  end procedure FortranGraphWorkspaceDistance

  module procedure FortranGraphWorkspaceParent
    type(C_PTR)                :: ParentCPtr
    integer(C_SIZE_T), pointer :: ParentFPtr(:)

    if (size(Parent) == 0) return

    ParentCPtr = GraphWorkspaceGetParent(Workspace%OpaqueHandle)

    if (C_ASSOCIATED(ParentCPtr)) then
      call C_F_POINTER(ParentCPtr, &
                       ParentFPtr, &
                       [size(Parent)])
      Parent = ParentFPtr
    endif
  end procedure FortranGraphWorkspaceParent

  module procedure GraphDijkstra
    type(FortranContextWrapper), target :: FSideUserContextWrapper
    type(C_FUNPTR)                      :: WeightFunctionPtr
    type(C_PTR)                         :: UserContextPtr

    Success = .false.

    if (.not.Workspace%Initialized()) return

    WeightFunctionPtr = C_NULL_FUNPTR
    UserContextPtr    = C_NULL_PTR
    if (present(WeightFunction)) then
      if (associated(WeightFunction)) then
        FSideUserContextWrapper%WeightFunction => WeightFunction
        if (present(UserContext)) then
          if (associated(UserContext)) then
            FSideUserContextWrapper%Context => UserContext
          endif
        endif
        WeightFunctionPtr = C_FUNLOC(FortranWeightCallback)
        UserContextPtr    = C_LOC(FSideUserContextWrapper)
      endif
    endif

    Success = GraphDijkstra_C(WorkspaceHandle = Workspace%OpaqueHandle, &
                              SourceCount = SourceCount, &
                              Source = Source, &
                              WeightFunction = WeightFunctionPtr, &
                              UserContext = UserContextPtr)
  end procedure GraphDijkstra

  module procedure GraphBidirectionalDijkstra
    type(FortranContextWrapper), target  :: FSideUserContextWrapper
    integer(C_SIZE_T),           pointer :: LocalShortestPath(:) => null()
    integer(C_SIZE_T)                    :: LocalShortestPathVertexCount
    real(C_DOUBLE)                       :: LocalShortestDistance
    type(C_PTR)                          :: ShortestPathHandle
    type(C_FUNPTR)                       :: WeightFunctionPtr
    type(C_PTR)                          :: UserContextPtr
    integer(C_SIZE_T)                    :: BufferSize

    Success = .false.

    if (.not.ForwardWorkspace%Initialized()) return
    if (.not.BackwardWorkspace%Initialized()) return

    if (ForwardWorkspace%VertexCount /= &
        BackwardWorkspace%VertexCount) return

    ShortestPathHandle = C_NULL_PTR
    if (present(ShortestPath)) then
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(1_C_SIZE_T), C_SIZE_T), &
                                     B   = max(1_C_SIZE_T, ForwardWorkspace%VertexCount), &
                                     Res = BufferSize))) return

      ShortestPathHandle = FortranMalloc(BufferSize)
      if (.not.C_ASSOCIATED(ShortestPathHandle)) return
      call C_F_POINTER(ShortestPathHandle, &
                       LocalShortestPath, &
                       [max(1_C_SIZE_T, ForwardWorkspace%VertexCount)])
    endif

    WeightFunctionPtr = C_NULL_FUNPTR
    UserContextPtr    = C_NULL_PTR
    if (present(WeightFunction)) then
      if (associated(WeightFunction)) then
        FSideUserContextWrapper%WeightFunction => WeightFunction
        if (present(UserContext)) then
          if (associated(UserContext)) then
            FSideUserContextWrapper%Context => UserContext
          endif
        endif
        WeightFunctionPtr = C_FUNLOC(FortranWeightCallback)
        UserContextPtr    = C_LOC(FSideUserContextWrapper)
      endif
    endif

    Success = GraphBidirectionalDijkstra_C(ForwardHandle = ForwardWorkspace%OpaqueHandle, &
                                           BackwardHandle = BackwardWorkspace%OpaqueHandle, &
                                           Source = Source, &
                                           Target = Target, &
                                           WeightFunction = WeightFunctionPtr, &
                                           UserContext = UserContextPtr, &
                                           ShortestPath = ShortestPathHandle, &
                                           ShortestPathVertexCount = LocalShortestPathVertexCount, &
                                           ShortestDistance = LocalShortestDistance)

    if (Success) then
      if (present(ShortestPath)) ShortestPath(1_C_SIZE_T:LocalShortestPathVertexCount) = &
        LocalShortestPath(1_C_SIZE_T:LocalShortestPathVertexCount)
      if (present(ShortestPathVertexCount)) ShortestPathVertexCount = LocalShortestPathVertexCount
      if (present(ShortestDistance)) ShortestDistance = LocalShortestDistance
    endif
    if (C_ASSOCIATED(ShortestPathHandle)) call FortranFree(ShortestPathHandle)
  end procedure GraphBidirectionalDijkstra

  module procedure GraphBellmanFord
    type(FortranContextWrapper), target :: FSideUserContextWrapper
    logical(C_BOOL),             target :: LocalNegativeCycles = .false._C_BOOL
    type(C_PTR)                         :: NegativeCyclesHandle
    type(C_FUNPTR)                      :: WeightFunctionPtr
    type(C_PTR)                         :: UserContextPtr

    Success = .false.

    if (.not.Workspace%Initialized()) return

    if (present(NegativeCycles)) then
      NegativeCyclesHandle = C_LOC(LocalNegativeCycles)
    else
      NegativeCyclesHandle = C_NULL_PTR
    endif

    WeightFunctionPtr = C_NULL_FUNPTR
    UserContextPtr    = C_NULL_PTR
    if (present(WeightFunction)) then
      if (associated(WeightFunction)) then
        FSideUserContextWrapper%WeightFunction => WeightFunction
        if (present(UserContext)) then
          if (associated(UserContext)) then
            FSideUserContextWrapper%Context => UserContext
          endif
        endif
        WeightFunctionPtr = C_FUNLOC(FortranWeightCallback)
        UserContextPtr    = C_LOC(FSideUserContextWrapper)
      endif
    endif

    Success = GraphBellmanFord_C(WorkspaceHandle = Workspace%OpaqueHandle, &
                                 SourceCount = SourceCount, &
                                 Source = Source, &
                                 WeightFunction = WeightFunctionPtr, &
                                 UserContext = UserContextPtr, &
                                 NegativeCycles = NegativeCyclesHandle)

    if (Success) then
      if (present(NegativeCycles)) NegativeCycles = logical(LocalNegativeCycles)
    endif
  end procedure GraphBellmanFord

end submodule GRTP_F_Implementation
