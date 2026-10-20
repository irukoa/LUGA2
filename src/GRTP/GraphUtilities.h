#ifndef GraphUtilities_CH
#define GraphUtilities_CH

/**
 * @file GraphUtilities.h
 *
 * @brief Graph topology representation and shortest-path algorithms.
 *
 * @details
 * This module provides:
 *
 * - construction of directed graph topologies,
 * - immutable optimized graph representations,
 * - shortest-path algorithms operating on immutable graphs, and
 * - reusable algorithm workspaces to avoid repeated memory allocations.
 *
 * Graphs are constructed through GraphSupportBuilder and finalized into
 * immutable GraphSupport objects. Once constructed, GraphSupport instances
 * provide efficient access to graph connectivity and are intended for use
 * by graph algorithms such as Dijkstra's algorithm.
 */

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Sentinel value representing the absence of a predecessor.
 *
 * Used by shortest-path algorithms to indicate that no parent vertex
 * exists for a particular vertex.
 */
#define UnreachableVertex SIZE_MAX

/**
 * @brief Sentinel value representing the distance to an unreachable vertex.
 *
 * Used by shortest-path algorithms to indicate that no parent vertex
 * exists for a particular vertex.
 *
 * @note
 * The value can be passed to `isinf`.
 */
#define UnreachableDistance INFINITY

/* GRAPH TOPOLOGY */

/**
 * @brief Opaque graph topology builder.
 *
 * @details
 * Represents a mutable graph construction object used to incrementally
 * define the topology of a directed graph.
 *
 * A GraphSupportBuilder stores the connectivity information required to
 * construct an immutable GraphSupport instance. Edges may be freely added
 * and removed until the desired topology is obtained.
 *
 * @note
 * - The internal structure definition is intentionally hidden from users.
 * - Instances are managed exclusively through the public API.
 * - The object owns all internally allocated graph construction data.
 *
 * @see GraphSupport
 */
typedef struct GraphSupportBuilder GraphSupportBuilder;

/**
 * @brief Frees a graph topology builder.
 *
 * @param[in,out] BuilderReference
 *     Reference to the GraphSupportBuilder pointer to be freed.
 *
 * @details
 * Releases all memory owned by the specified GraphSupportBuilder instance.
 * After the call, the containing pointer is set to NULL.
 *
 * @note
 * - Safe to call with NULL.
 * - Safe to call with a pointer containing NULL.
 * - After successful destruction, the pointed object becomes invalid.
 * - The caller must not access the object after this call.
 */
void FreeGraphSupportBuilder(GraphSupportBuilder **BuilderReference);

/**
 * @brief Allocates and initializes a graph topology builder.
 *
 * @details
 * Creates a new GraphSupportBuilder capable of representing a directed
 * graph containing the specified number of vertices.
 *
 * Initially, the graph contains no edges. Connectivity may subsequently be
 * modified using GraphSupportBuilderAddEdge() and
 * GraphSupportBuilderRemoveEdge().
 *
 * @param[in] VertexCount
 *     Number of vertices in the graph.
 *
 * @return
 * Pointer to a newly allocated GraphSupportBuilder on success, or NULL on
 * failure.
 *
 * @note
 * - The returned object is owned by the caller.
 * - Vertex identifiers range from 0 to VertexCount - 1.
 * - VertexCount should be greater than zero.
 *
 * @see GraphSupportBuilderAddEdge
 * @see GraphSupportBuilderRemoveEdge
 * @see NewGraphSupport
 */
GraphSupportBuilder *NewGraphSupportBuilder(const size_t VertexCount);

/**
 * @brief Adds a directed edge to a graph topology builder.
 *
 * @details
 * Inserts the directed edge Origin -> Destination into the graph under
 * construction.
 *
 * The operation fails if either vertex identifier is invalid or if the
 * specified edge already exists.
 *
 * @param[in,out] Builder
 *     Pointer to the GraphSupportBuilder instance.
 *
 * @param[in] Origin
 *     Origin vertex of the directed edge.
 *
 * @param[in] Destination
 *     Destination vertex of the directed edge.
 *
 * @retval true
 *     The edge was successfully added.
 *
 * @retval false
 *     The edge could not be added.
 *
 * @note
 * - Parallel edges are not permitted.
 * - Self-loops are permitted.
 *
 * @see GraphSupportBuilderRemoveEdge
 */
bool GraphSupportBuilderAddEdge(GraphSupportBuilder *Builder,
                                const size_t         Origin,
                                const size_t         Destination);

/**
 * @brief Removes a directed edge from a graph topology builder.
 *
 * @details
 * Removes the directed edge Origin -> Destination from the graph under
 * construction.
 *
 * The operation fails if either vertex identifier is invalid or if the
 * specified edge does not exist.
 *
 * @param[in,out] Builder
 *     Pointer to the GraphSupportBuilder instance.
 *
 * @param[in] Origin
 *     Origin vertex of the directed edge.
 *
 * @param[in] Destination
 *     Destination vertex of the directed edge.
 *
 * @retval true
 *     The edge was successfully removed.
 *
 * @retval false
 *     The edge could not be removed.
 *
 * @see GraphSupportBuilderAddEdge
 */
bool GraphSupportBuilderRemoveEdge(GraphSupportBuilder *Builder,
                                   const size_t         Origin,
                                   const size_t         Destination);

/**
 * @brief Returns the number of vertices in a graph topology builder.
 *
 * @param[in] Builder
 *     Pointer to the GraphSupportBuilder instance.
 *
 * @return
 * Number of vertices contained in the builder.
 */
size_t GraphSupportBuilderGetVertexCount(const GraphSupportBuilder *Builder);

/**
 * @brief Returns the total number of directed edges in a graph topology
 * builder.
 *
 * @param[in] Builder
 *     Pointer to the GraphSupportBuilder instance.
 *
 * @return
 * Total number of directed edges.
 */
size_t GraphSupportBuilderGetEdgeCount(const GraphSupportBuilder *Builder);

/**
 * @brief Returns the number of outgoing neighbors of a vertex in a graph
 * topology builder.
 *
 * @param[in] Builder
 *     Pointer to the GraphSupportBuilder instance.
 *
 * @param[in] Vertex
 *     Vertex whose neighborhood is queried.
 *
 * @return
 * Number of outgoing neighbors of Vertex.
 */
size_t GraphSupportBuilderGetNeighborCount(const GraphSupportBuilder *Builder,
                                           const size_t               Vertex);

/**
 * @brief Returns the outgoing neighbors of a vertex in a graph topology
 * builder.
 *
 * @param[in] Builder
 *     Pointer to the GraphSupportBuilder instance.
 *
 * @param[in] Vertex
 *     Vertex whose neighborhood is queried.
 *
 * @return
 * Pointer to an internal array containing the outgoing neighbors of Vertex.
 *
 * @note
 * - The returned pointer is owned by GraphSupportBuilder.
 * - The caller must not modify or free the returned array.
 * - The array contains exactly
 *   GraphSupportBuilderGetNeighborCount(Builder, Vertex) elements.
 *
 * @see GraphSupportBuilderGetNeighborCount
 */
const size_t *
GraphSupportBuilderGetNeighbors(const GraphSupportBuilder *Builder,
                                const size_t               Vertex);

/**
 * @brief Determines whether a directed edge exists in a graph topology
 * builder.
 *
 * @param[in] Builder
 *     Pointer to the GraphSupportBuilder instance.
 *
 * @param[in] Origin
 *     Origin vertex of the directed edge.
 *
 * @param[in] Destination
 *     Destination vertex of the directed edge.
 *
 * @return
 * true if the edge exists; false otherwise.
 */
bool GraphSupportBuilderHasEdge(const GraphSupportBuilder *Builder,
                                const size_t               Origin,
                                const size_t               Destination);

/**
 * @brief Opaque immutable graph topology representation.
 *
 * @details
 * Stores the connectivity of a directed graph in an internal compact
 * representation optimized for graph algorithm execution.
 *
 * A GraphSupport instance is constructed from a GraphSupportBuilder and
 * provides efficient read-only access to:
 * - the graph topology,
 * - the outgoing neighbors of each vertex,
 * - edge identifiers, and
 * - edge endpoint queries.
 *
 * Once created, the graph topology cannot be modified.
 *
 * @note
 * - The internal representation is intentionally hidden.
 * - Instances are managed exclusively through the public API.
 * - The object owns all internally allocated memory.
 */
typedef struct GraphSupport GraphSupport;

/**
 * @brief Frees an immutable graph topology.
 *
 * @param[in,out] SupportReference
 *     Reference to the GraphSupport pointer to be freed.
 *
 * @details
 * Releases all memory owned by the specified GraphSupport instance.
 * After the call, the containing pointer is set to NULL.
 *
 * @note
 * - Safe to call with NULL.
 * - Safe to call with a pointer containing NULL.
 * - The caller must not access the object after destruction.
 */
void FreeGraphSupport(GraphSupport **SupportReference);

/**
 * @brief Builds an immutable graph topology from a graph builder.
 *
 * @param[in] Builder
 *     Graph topology builder.
 *
 * @return
 * Pointer to a newly allocated GraphSupport instance on success,
 * or NULL on failure.
 *
 * @details
 * Finalizes the graph stored in Builder into an immutable representation
 * optimized for graph traversal and shortest-path algorithms.
 *
 * Neighbor lists are internally sorted in ascending vertex order to enable
 * efficient edge lookup.
 *
 * @note
 * - The returned object is owned by the caller.
 * - Builder remains valid after this call.
 * - The graph topology stored in the returned object cannot be modified.
 */
GraphSupport *NewGraphSupport(const GraphSupportBuilder *Builder);

/**
 * @brief Returns the number of vertices in a graph.
 *
 * @param[in] Support
 *     Graph topology.
 *
 * @return
 * Number of vertices contained in the graph.
 */
size_t GraphSupportGetVertexCount(const GraphSupport *Support);

/**
 * @brief Returns the number of directed edges in a graph.
 *
 * @param[in] Support
 *     Graph topology.
 *
 * @return
 * Total number of directed edges.
 */
size_t GraphSupportGetEdgeCount(const GraphSupport *Support);

/**
 * @brief Returns the number of outgoing neighbors of a vertex.
 *
 * @param[in] Support
 *     Graph topology.
 *
 * @param[in] Vertex
 *     Vertex whose neighborhood is queried.
 *
 * @return
 * Number of outgoing neighbors of Vertex.
 */
size_t GraphSupportGetNeighborCount(const GraphSupport *Support,
                                    const size_t        Vertex);

/**
 * @brief Returns the outgoing neighbors of a vertex.
 *
 * @param[in] Support
 *     Graph topology.
 *
 * @param[in] Vertex
 *     Vertex whose neighborhood is queried.
 *
 * @return
 * Pointer to an internal read-only array containing the outgoing
 * neighbors of Vertex.
 *
 * @note
 * - The returned pointer is owned by GraphSupport.
 * - The caller must not modify or free the returned array.
 * - The array contains exactly
 *   GraphSupportGetNeighborCount(Support, Vertex) elements.
 *
 * @see GraphSupportGetNeighborCount
 */
const size_t *GraphSupportGetNeighbors(const GraphSupport *Support,
                                       const size_t        Vertex);

/**
 * @brief Returns the internal identifier of a directed edge.
 *
 * @param[in] Support
 *     Graph topology.
 *
 * @param[in] Origin
 *     Origin vertex.
 *
 * @param[in] Destination
 *     Destination vertex.
 *
 * @param[out] EdgeIndex
 *     Receives the internal edge identifier.
 *
 * @return
 * true if the edge exists; false otherwise.
 *
 * @details
 * Each directed edge stored in GraphSupport is assigned a unique internal
 * identifier in the range [0, GraphSupportGetEdgeCount(Support) - 1].
 *
 * This identifier remains stable for the lifetime of the GraphSupport
 * instance and can be used by algorithms that associate information with
 * individual edges.
 *
 * @see GraphSupportGetEdgeVertices
 */
bool GraphSupportGetEdgeIndex(const GraphSupport *Support,
                              const size_t        Origin,
                              const size_t        Destination,
                              size_t             *EdgeIndex);

/**
 * @brief Returns the endpoints of an edge.
 *
 * @param[in] Support
 *     Graph topology.
 *
 * @param[in] EdgeIndex
 *     Internal edge identifier.
 *
 * @param[out] Origin
 *     Receives the origin vertex.
 *
 * @param[out] Destination
 *     Receives the destination vertex.
 *
 * @return
 * true on success; false if EdgeIndex is invalid.
 *
 * @details
 * This function is the inverse operation of
 * GraphSupportGetEdgeIndex().
 *
 * @see GraphSupportGetEdgeIndex
 */
bool GraphSupportGetEdgeVertices(const GraphSupport *Support,
                                 const size_t        EdgeIndex,
                                 size_t             *Origin,
                                 size_t             *Destination);

/**
 * @brief Determines whether a directed edge exists.
 *
 * @param[in] Support
 *     Graph topology.
 *
 * @param[in] Origin
 *     Origin vertex.
 *
 * @param[in] Destination
 *     Destination vertex.
 *
 * @return
 * true if the edge exists; false otherwise.
 *
 * @note
 * This function is equivalent to querying GraphSupportGetEdgeIndex()
 * while ignoring the returned edge identifier.
 *
 * @see GraphSupportGetEdgeIndex
 */
bool GraphSupportHasEdge(const GraphSupport *Support,
                         const size_t        Origin,
                         const size_t        Destination);

/* GRAPH ALGORITHMS */

/**
 * @brief Edge weighting callback used by graph algorithms.
 *
 * @details
 * Computes the weight associated with the directed edge
 * (Origin, Destination).
 *
 * The callback is invoked by graph algorithms whenever the cost of
 * traversing an edge is required.
 *
 * @param[in] Support
 *     Graph topology.
 *
 * @param[in] Origin
 *     Origin vertex.
 *
 * @param[in] Destination
 *     Destination vertex.
 *
 * @param[in] UserContext
 *     Optional caller-defined context passed unchanged to every callback
 *     invocation.
 *
 * @return
 * Weight associated with the specified directed edge.
 *
 * @note
 * - Passing NULL as the weighting function causes graph algorithms to
 *   assume unit edge weights.
 * - Dijkstra's algorithm requires all returned weights to be
 *   non-negative.
 */
typedef double (*GraphWeightFunction)(const GraphSupport *Support,
                                      const size_t        Origin,
                                      const size_t        Destination,
                                      void               *UserContext);

/**
 * @brief Opaque workspace for shortest-path algorithms.
 *
 * @details
 * Stores all temporary and output data required by shortest-path algorithms.
 *
 * A workspace is associated with a particular graph topology and may be
 * reused across multiple shortest-path computations, avoiding repeated
 * memory allocations.
 *
 * The workspace internally owns all memory required by the algorithm,
 * including distance and predecessor information.
 *
 * @note
 * - The internal representation is intentionally hidden.
 * - Instances are managed exclusively through the public API.
 */
typedef struct GraphWorkspace GraphWorkspace;

/**
 * @brief Frees a graph workspace.
 *
 * @param[in,out] WorkspaceReference
 *     Reference to the workspace pointer to be freed.
 *
 * @details
 * Releases all memory owned by the specified workspace.
 * After the call, the containing pointer is set to NULL.
 *
 * @note
 * - Safe to call with NULL.
 * - Safe to call with a pointer containing NULL.
 */
void FreeGraphWorkspace(GraphWorkspace **WorkspaceReference);

/**
 * @brief Allocates a graph workspace for shortest-path algorithms.
 *
 * @param[in] Support
 *     Graph topology on which the workspace will operate.
 *
 * @return
 * Pointer to a newly allocated workspace on success, or NULL on failure.
 *
 * @details
 * Allocates all internal memory required to execute shortest-path algorithms
 * on the specified graph.
 *
 * The returned workspace may be reused for multiple shortest-path
 * computations on the same graph.
 *
 * @note
 * - The returned object is owned by the caller.
 * - A workspace is associated with the specified GraphSupport instance.
 */
GraphWorkspace *NewGraphWorkspace(const GraphSupport *Support);

/**
 * @brief Returns the distance array computed by the most recent execution of
 * shortest path algorithms which take a GraphWorkspace instance as an
 * input.
 *
 * @param[in] Workspace
 *     Graph workspace.
 *
 * @return
 * Pointer to the internal distance array.
 *
 * @details
 * The returned array contains one entry per graph vertex.
 *
 * Vertices that are unreachable from the selected source have distance
 * UnreachableDistance.
 *
 * @note
 * - The returned pointer is owned by the workspace.
 * - The caller must not modify or free the returned array.
 */
const double *GraphWorkspaceGetDistance(const GraphWorkspace *Workspace);

/**
 * @brief Returns the predecessor array computed by the most recent execution of
 * shortest path algorithms which take a GraphWorkspace instance as an
 * input.
 *
 * @param[in] Workspace
 *     Graph workspace.
 *
 * @return
 * Pointer to the internal predecessor array.
 *
 * @details
 * For every reachable vertex, the predecessor array stores the previous
 * vertex along one shortest path from the selected source.
 *
 * The source vertex and unreachable vertices contain
 * UnreachableVertex.
 *
 * @note
 * - The returned pointer is owned by the workspace.
 * - The caller must not modify or free the returned array.
 */
const size_t *GraphWorkspaceGetParent(const GraphWorkspace *Workspace);

/**
 * @brief Computes multiple-source shortest paths using Dijkstra's algorithm.
 *
 * @param[in,out] Workspace
 *     Graph workspace.
 *
 * @param[in] SourceCount
 *     Number of vertices in the Source array.
 *
 * @param[in] Source
 *     Array containing source vertices.
 *
 * @param[in] WeightFunction
 *     Edge weighting callback.
 *
 * @param[in] UserContext
 *     Optional caller-defined context passed to WeightFunction.
 *
 * @return
 * true on success; false if the computation could not be performed.
 *
 * @details
 * Computes the shortest-path tree rooted at the vertices specified in the
 * Source array.
 *
 * Upon successful completion, the workspace contains the shortest-path
 * distance and predecessor arrays, which remain valid until the next
 * execution of the algorithm using the same workspace.
 *
 * If WeightFunction is NULL, every edge is assumed to have unit weight.
 *
 * @note
 * Dijkstra's algorithm assumes that all edge weights are non-negative.
 *
 * @see GraphWorkspaceGetDistance
 * @see GraphWorkspaceGetParent
 */
bool GraphDijkstra(GraphWorkspace           *Workspace,
                   const size_t              SourceCount,
                   const size_t              Source[SourceCount],
                   const GraphWeightFunction WeightFunction,
                   void                     *UserContext);

/**
 * @brief Computes the shortest path between two vertices using
 * bidirectional Dijkstra's algorithm.
 *
 * @param[in,out] ForwardWorkspace
 *     Graph workspace associated with a given graph.
 *
 * @param[in,out] BackwardWorkspace
 *     Graph workspace associated with the transpose graph.
 *
 * @param[in] Source
 *     Source vertex.
 *
 * @param[in] Target
 *     Target vertex.
 *
 * @param[in] WeightFunction
 *     Edge weighting callback.
 *
 * @param[in] UserContext
 *     Optional caller-defined context passed to WeightFunction.
 *
 * @param[out] ShortestPath
 *     Optional array receiving the sequence of vertices defining the
 *     shortest path from Source to Target.
 *
 * @param[out] ShortestPathVertexCount
 *     Optional reference receiving the number of vertices written to
 *     ShortestPath.
 *
 * @param[out] ShortestDistance
 *     Optional reference receiving the shortest-path distance.
 *
 * @return
 * true on success; false if the computation could not be performed.
 *
 * @details
 * Computes the shortest path between Source and Target by simultaneously
 * expanding shortest-path frontiers from both vertices until the optimal
 * meeting point is identified.
 *
 * Upon successful completion, both workspaces contain the corresponding
 * forward and backward shortest-path trees produced during the search.
 *
 * If ShortestPath is not NULL and a path exists, the shortest path is
 * reconstructed and written to the caller-provided array. The array must
 * be large enough to store the complete path. A capacity equal to the
 * number of vertices in the graph is always sufficient.
 *
 * If no path exists between Source and Target:
 * - ShortestDistance, if provided, receives UnreachableDistance.
 * - ShortestPathVertexCount, if provided, receives zero.
 * - No vertices are written to ShortestPath.
 *
 * If WeightFunction is NULL, every edge is assumed to have unit weight.
 *
 * @note
 * - Dijkstra's algorithm assumes that all edge weights are non-negative.
 * - BackwardWorkspace must be associated with the transpose of the graph
 *   associated with ForwardWorkspace.
 *
 * @see GraphWorkspaceGetDistance
 * @see GraphWorkspaceGetParent
 */
bool GraphBidirectionalDijkstra(GraphWorkspace           *ForwardWorkspace,
                                GraphWorkspace           *BackwardWorkspace,
                                const size_t              Source,
                                const size_t              Target,
                                const GraphWeightFunction WeightFunction,
                                void                     *UserContext,
                                size_t                   *ShortestPath,
                                size_t *ShortestPathVertexCount,
                                double *ShortestDistance);

/**
 * @brief Computes multiple-source shortest paths using the Bellman-Ford
 * algorithm.
 *
 * @param[in,out] Workspace
 *     Graph workspace.
 *
 * @param[in] SourceCount
 *     Number of vertices in the Source array.
 *
 * @param[in] Source
 *     Array containing source vertices.
 *
 * @param[in] WeightFunction
 *     Edge weighting callback.
 *
 * @param[in] UserContext
 *     Optional caller-defined context passed to WeightFunction.
 *
 * @param[out] NegativeCycles
 *     Optional pointer receiving whether a reachable negative-weight cycle
 *     exists.
 *
 * @return
 * true on success; false if the computation could not be performed.
 *
 * @details
 * Computes the shortest-path tree rooted at the vertices specified in the
 * Source array.
 *
 * Upon successful completion, the workspace contains the shortest-path
 * distance and predecessor arrays, which remain valid until the next
 * execution of the algorithm using the same workspace.
 *
 * If WeightFunction is NULL, every edge is assumed to have unit weight.
 *
 * If NegativeCycles is not NULL, the function performs an additional
 * verification pass after computing the shortest paths to determine whether
 * any reachable negative-weight cycle exists. The result is written to the
 * location pointed to by NegativeCycles.
 *
 * @note
 * Bellman-Ford supports graphs containing negative-weight edges.
 *
 * If a reachable negative-weight cycle exists, shortest paths are not
 * mathematically well-defined. In this case, NegativeCycles is set to true
 * (when requested), while the function itself still returns true to indicate
 * successful execution.
 *
 * @see GraphWorkspaceGetDistance
 * @see GraphWorkspaceGetParent
 */
bool GraphBellmanFord(GraphWorkspace           *Workspace,
                      const size_t              SourceCount,
                      const size_t              Source[SourceCount],
                      const GraphWeightFunction WeightFunction,
                      void                     *UserContext,
                      bool                     *NegativeCycles);

#endif
