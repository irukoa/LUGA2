#include "src/TBCI/TBCI_C_Int.M90"
#include "src/Core/CoreFortran.M90"
#include "src/MAC/MultidimensionalArrayContainers.M90"

submodule (TightBindingCrystalInterpolation:TBCI_C_Interfaces) TBCI_F_Implementation

  use CoreFortran, only: FortranAddOverflow, &
                         FortranMulOverflow, &
                         FortranMalloc, &
                         FortranFree
  use MultidimensionalArrayContainers, only: MAC, COLUMN_MAJOR
  use, intrinsic :: ISO_C_BINDING, only: C_ASSOCIATED, &
                                         C_F_POINTER, &
                                         C_SIZEOF

  implicit none

  complex(C_DOUBLE_COMPLEX), parameter :: TBCICmplx0 = cmplx(0.0_C_DOUBLE, 0.0_C_DOUBLE, C_DOUBLE_COMPLEX), &
                                          TBCICmplx1 = cmplx(1.0_C_DOUBLE, 0.0_C_DOUBLE, C_DOUBLE_COMPLEX), &
                                          TBCICmplxI = cmplx(0.0_C_DOUBLE, 1.0_C_DOUBLE, C_DOUBLE_COMPLEX)

  real(C_DOUBLE), parameter :: TBCIPi = acos(-1.0_C_DOUBLE)

  contains

  module procedure FortranCopyTBCrystal
    Copy%OpaqueHandle   = Original%OpaqueHandle
    Copy%Init           = Original%Init
    Copy%Owner          = .false.
    Copy%Dimensionality = Original%Dimensionality
    Copy%SiteCount      = Original%SiteCount
    Copy%BandCount      = Original%BandCount
  end procedure FortranCopyTBCrystal

  module procedure FortranFreeTBCrystal
    if (Crystal%Owner) then
      call FreeTBCrystal(Crystal%OpaqueHandle)
    else
      Crystal%OpaqueHandle = C_NULL_PTR
    endif
    Crystal%Init           = .false.
    Crystal%Dimensionality = 0_C_SIZE_T
    Crystal%SiteCount      = 0_C_SIZE_T
    Crystal%BandCount      = 0_C_SIZE_T
  end procedure FortranFreeTBCrystal

  module procedure FortranFreeTBCrystal_Final
    if (Crystal%Owner) then
      call FreeTBCrystal(Crystal%OpaqueHandle)
    else
      Crystal%OpaqueHandle = C_NULL_PTR
    endif
    Crystal%Init = .false.
  end procedure FortranFreeTBCrystal_Final

  module procedure FortranNewTBCrystal
    if (C_ASSOCIATED(Crystal%OpaqueHandle)) call FortranFreeTBCrystal(Crystal)

    Crystal%OpaqueHandle = NewTBCrystal(Dimensionality = Dimensionality, &
                                        SiteCount      = SiteCount, &
                                        BandCount      = BandCount)

    if (C_ASSOCIATED(Crystal%OpaqueHandle)) then
      Crystal%Init           = .true.
      Crystal%Owner          = .true.
      Crystal%Dimensionality = Dimensionality
      Crystal%SiteCount      = SiteCount
      Crystal%BandCount      = BandCount
    endif
  end procedure FortranNewTBCrystal

  module procedure FortranTBCrystalInitialized
    Initialized = Crystal%Init
  end procedure FortranTBCrystalInitialized

  module procedure FortranTBCrystalGetDimensionality
    Dimensionality = TBCrystalGetDimensionality(OpaqueHandle = Crystal%OpaqueHandle)
  end procedure FortranTBCrystalGetDimensionality

  module procedure FortranTBCrystalGetSiteCount
    SiteCount = TBCrystalGetSiteCount(OpaqueHandle = Crystal%OpaqueHandle)
  end procedure FortranTBCrystalGetSiteCount

  module procedure FortranTBCrystalGetBandCount
    BandCount = TBCrystalGetBandCount(OpaqueHandle = Crystal%OpaqueHandle)
  end procedure FortranTBCrystalGetBandCount

  module procedure FortranTBCrystalSetDirectLatticeVal
    Success = logical(TBCrystalSetDirectLatticeVal(OpaqueHandle = Crystal%OpaqueHandle, &
                                                   VectorId    = VectorId, &
                                                   ComponentId = ComponentId, &
                                                   Val         = Val))
  end procedure FortranTBCrystalSetDirectLatticeVal

  module procedure FortranTBCrystalGetDirectLatticeVal
    Success = logical(TBCrystalGetDirectLatticeVal(OpaqueHandle = Crystal%OpaqueHandle, &
                                                   VectorId    = VectorId, &
                                                   ComponentId = ComponentId, &
                                                   Val         = Val))
  end procedure FortranTBCrystalGetDirectLatticeVal

  module procedure FortranTBCrystalSetSiteVal
    Success = logical(TBCrystalSetSiteVal(OpaqueHandle = Crystal%OpaqueHandle, &
                                          SiteId      = SiteId, &
                                          ComponentId = ComponentId, &
                                          Val         = Val))
  end procedure FortranTBCrystalSetSiteVal

  module procedure FortranTBCrystalGetSiteVal
    Success = logical(TBCrystalGetSiteVal(OpaqueHandle = Crystal%OpaqueHandle, &
                                          SiteId      = SiteId, &
                                          ComponentId = ComponentId, &
                                          Val         = Val))
  end procedure FortranTBCrystalGetSiteVal

  module procedure FortranTBCrystalSetTBHamiltonian
    Success = logical(TBCrystalSetTBHamiltonian(OpaqueHandle = Crystal%OpaqueHandle, &
                                                BandId1     = BandId1, &
                                                BandId2     = BandId2, &
                                                SiteId      = SiteId, &
                                                Val         = Val))
  end procedure FortranTBCrystalSetTBHamiltonian

  module procedure FortranTBCrystalGetTBHamiltonian
    Success = logical(TBCrystalGetTBHamiltonian(OpaqueHandle = Crystal%OpaqueHandle, &
                                                BandId1     = BandId1, &
                                                BandId2     = BandId2, &
                                                SiteId      = SiteId, &
                                                Val         = Val))
  end procedure FortranTBCrystalGetTBHamiltonian

  module procedure FortranTBCrystalSetTBPositionOperator
    Success = logical(TBCrystalSetTBPositionOperator(OpaqueHandle = Crystal%OpaqueHandle, &
                                                     BandId1     = BandId1, &
                                                     BandId2     = BandId2, &
                                                     SiteId      = SiteId, &
                                                     ComponentId = ComponentId, &
                                                     Val         = Val))
  end procedure FortranTBCrystalSetTBPositionOperator

  module procedure FortranTBCrystalGetTBPositionOperator
    Success = logical(TBCrystalGetTBPositionOperator(OpaqueHandle = Crystal%OpaqueHandle, &
                                                     BandId1     = BandId1, &
                                                     BandId2     = BandId2, &
                                                     SiteId      = SiteId, &
                                                     ComponentId = ComponentId, &
                                                     Val         = Val))
  end procedure FortranTBCrystalGetTBPositionOperator

  module procedure FortranTBCrystalHamiltonian
    integer(C_SIZE_T) :: LocalDerivative
    integer(C_SIZE_T) :: ArrayShapeSize
    integer(C_SIZE_T) :: BufferSize
    integer(C_SIZE_T) :: ElementCount
    integer(C_SIZE_T) :: ElementSize

    type(C_PTR) :: DirectLatticePtr
    type(C_PTR) :: SiteCoordsPtr
    type(C_PTR) :: HamiltonianPtr
    type(C_PTR) :: ArrayHandlePtr
    type(C_PTR) :: PhasePtr
    type(C_PTR) :: PosPtr

    real(C_DOUBLE),            pointer :: DirectLattice(:, :)
    integer(C_LONG),           pointer :: SiteCoords(:, :)
    complex(C_DOUBLE_COMPLEX), pointer :: Hamiltonian(:, :, :)
    integer(C_SIZE_T),         pointer :: ArrayHandle(:)
    complex(C_DOUBLE_COMPLEX), pointer :: Phase(:)
    real(C_DOUBLE),            pointer :: Pos(:, :)

    type(MAC) :: ArraySpec

    integer(C_SIZE_T)         :: i, j, iSites, iBand1, iBand2
    complex(C_DOUBLE_COMPLEX) :: Sum, Ipow
    real(C_DOUBLE)            :: KdotR, DerProd

    if (present(Derivative)) then
      LocalDerivative = Derivative
    else
      LocalDerivative = 0_C_SIZE_T
    endif

    if (.not.C_ASSOCIATED(Crystal%OpaqueHandle)) then
      Success = .false.
      return
    endif

    !Verify KPoint's size.
    if (Crystal%Dimensionality /= size(KPoint, KIND = C_SIZE_T)) then
      Success = .false.
      return
    endif

    !Verify possible overflow.
    ArrayShapeSize = 0_C_SIZE_T
    if (FortranAddOverflow(LocalDerivative, 2_C_SIZE_T, ArrayShapeSize)) then
      Success = .false.
      return
    endif
    if (ArrayShapeSize /= size(ArrayShape, KIND = C_SIZE_T)) then
      Success = .false.
      return
    endif

    !Fill shape.
    ArrayShape(1_C_SIZE_T) = Crystal%BandCount
    ArrayShape(2_C_SIZE_T) = Crystal%BandCount
    if (LocalDerivative > 0_C_SIZE_T) then
      do i = 1, LocalDerivative
        ArrayShape(2_C_SIZE_T + i) = Crystal%Dimensionality
      enddo
    endif

    !The next call checks for possible overflows in
    !BandCount^2 * Dimensionality^Derivative.
    call ArraySpec%Specify(Shape = ArrayShape, Layout = COLUMN_MAJOR)
    if (.not.ArraySpec%Initialized()) then
      Success = .false.
      return
    endif
    if (ArraySpec%ElementCount() /= size(Array, KIND = C_SIZE_T)) then
      Success = .false.
      return
    endif

    DirectLatticePtr = C_NULL_PTR
    SiteCoordsPtr    = C_NULL_PTR
    HamiltonianPtr   = C_NULL_PTR

    DirectLatticePtr = TBCI_InternalGetDirectLatticePtr(Crystal%OpaqueHandle)
    SiteCoordsPtr    = TBCI_InternalGetSiteCoordsPtr(Crystal%OpaqueHandle)
    HamiltonianPtr   = TBCI_InternalGetTBHamiltonianPtr(Crystal%OpaqueHandle)

    if (C_ASSOCIATED(DirectLatticePtr)) then
      call C_F_POINTER(DirectLatticePtr, &
                       DirectLattice, &
                       [Crystal%Dimensionality, Crystal%Dimensionality])
    else
      Success = .false.
      return
    endif

    if (C_ASSOCIATED(SiteCoordsPtr)) then
      call C_F_POINTER(SiteCoordsPtr, &
                       SiteCoords, &
                       [Crystal%Dimensionality, Crystal%SiteCount])
    else
      Success = .false.
      return
    endif

    if (C_ASSOCIATED(HamiltonianPtr)) then
      call C_F_POINTER(HamiltonianPtr, &
                       Hamiltonian, &
                       [Crystal%SiteCount, Crystal%BandCount, Crystal%BandCount])
    else
      Success = .false.
      return
    endif

    !Powers of i are exact: mod 4 selects among 1, i, -1, -i.
    select case (mod(LocalDerivative, 4_C_SIZE_T))
    case (0_C_SIZE_T)
      Ipow = TBCICmplx1
    case (1_C_SIZE_T)
      Ipow = TBCICmplxI
    case (2_C_SIZE_T)
      Ipow = -TBCICmplx1
    case default
      Ipow = -TBCICmplxI
    end select

    ArrayHandlePtr = C_NULL_PTR
    PhasePtr       = C_NULL_PTR
    PosPtr         = C_NULL_PTR
    Success        = .false.

    Computation: block
      ElementSize = int(C_SIZEOF(1_C_SIZE_T), C_SIZE_T)
      if (logical(FortranMulOverflow(A   = ElementSize, &
                                     B   = ArrayShapeSize, &
                                     Res = BufferSize))) exit Computation

      ArrayHandlePtr = FortranMalloc(BufferSize)
      if (.not.C_ASSOCIATED(ArrayHandlePtr)) exit Computation
      call C_F_POINTER(ArrayHandlePtr, &
                       ArrayHandle, &
                       [ArrayShapeSize])

      ElementSize = int(C_SIZEOF(TBCICmplx1), C_SIZE_T)
      if (logical(FortranMulOverflow(A   = ElementSize, &
                                     B   = Crystal%SiteCount, &
                                     Res = BufferSize))) exit Computation

      PhasePtr = FortranMalloc(BufferSize)
      if (.not.C_ASSOCIATED(PhasePtr)) exit Computation
      call C_F_POINTER(PhasePtr, &
                       Phase, &
                       [Crystal%SiteCount])

      !The Cartesian offsets are only formed for the derivatives.
      if (LocalDerivative > 0_C_SIZE_T) then
        ElementSize = int(C_SIZEOF(1.0_C_DOUBLE), C_SIZE_T)
        if (logical(FortranMulOverflow(A   = Crystal%Dimensionality, &
                                       B   = Crystal%SiteCount, &
                                       Res = ElementCount))) exit Computation
        if (logical(FortranMulOverflow(A   = ElementCount, &
                                       B   = ElementSize, &
                                       Res = BufferSize))) exit Computation

        PosPtr = FortranMalloc(BufferSize)
        if (.not.C_ASSOCIATED(PosPtr)) exit Computation
        call C_F_POINTER(PosPtr, &
                         Pos, &
                         [Crystal%Dimensionality, Crystal%SiteCount])
      endif

      do iSites = 1_C_SIZE_T, Crystal%SiteCount
        KdotR         = 2 * TBCIPi * dot_product(real(SiteCoords(:, iSites), C_DOUBLE), KPoint)
        Phase(iSites) = exp(TBCICmplxI * KdotR)
        if (LocalDerivative > 0_C_SIZE_T) then
          Pos(:, iSites) = 0.0_C_DOUBLE
          do j = 1_C_SIZE_T, Crystal%Dimensionality
            Pos(:, iSites) = Pos(:, iSites) + DirectLattice(:, j) * &
              SiteCoords(j, iSites)
          enddo
        endif
      enddo

      do i = 0_C_SIZE_T, ArraySpec%ElementCount() - 1_C_SIZE_T

        ArrayHandle = ArraySpec%Multidimensional(i)
        Sum         = TBCICmplx0
        iBand1      = ArrayHandle(1_C_SIZE_T) + 1_C_SIZE_T
        iBand2      = ArrayHandle(2_C_SIZE_T) + 1_C_SIZE_T

        do iSites = 1_C_SIZE_T, Crystal%SiteCount

          DerProd = 1_C_DOUBLE
          do j = 1_C_SIZE_T, LocalDerivative
            DerProd = DerProd * Pos(ArrayHandle(j + 2_C_SIZE_T) + 1_C_SIZE_T, iSites)
          enddo

          Sum = Sum + Ipow * DerProd * Phase(iSites) * &
            Hamiltonian(iSites, &
                        iBand1, &
                        iBand2)

        enddo
        Array(i + 1_C_SIZE_T) = Sum
      enddo

      Success = .true.
    end block Computation

    nullify(ArrayHandle)
    nullify(Phase)
    nullify(Pos)

    if (C_ASSOCIATED(ArrayHandlePtr)) call FortranFree(ArrayHandlePtr)
    if (C_ASSOCIATED(PhasePtr))       call FortranFree(PhasePtr)
    if (C_ASSOCIATED(PosPtr))         call FortranFree(PosPtr)

  end procedure FortranTBCrystalHamiltonian

  module procedure TBCrystalHamiltonian_C
    type(TBCrystal)                    :: FortranCrystal
    real(C_DOUBLE),            pointer :: KPointFPtr(:)
    complex(C_DOUBLE_COMPLEX), pointer :: ArrayFPtr(:)
    integer(C_SIZE_T),         pointer :: ArrayShapeFPtr(:)

    if (C_ASSOCIATED(Crystal)) then
      FortranCrystal%OpaqueHandle   = Crystal
      FortranCrystal%Init           = .true.
      FortranCrystal%Owner          = .false.
      FortranCrystal%Dimensionality = FortranCrystal%GetDimensionality()
      FortranCrystal%SiteCount      = FortranCrystal%GetSiteCount()
      FortranCrystal%BandCount      = FortranCrystal%GetBandCount()
    else
      Success = .false.
      return
    endif

    if (C_ASSOCIATED(KPoint)) then
      call C_F_POINTER(KPoint, &
                       KPointFPtr, &
                       [Dimensionality])
    else
      Success = .false.
      return
    endif

    if (C_ASSOCIATED(Array)) then
      call C_F_POINTER(Array, &
                       ArrayFPtr, &
                       [ArraySize])
    else
      Success = .false.
      return
    endif

    if (C_ASSOCIATED(ArrayShape)) then
      call C_F_POINTER(ArrayShape, &
                       ArrayShapeFPtr, &
                       [ArrayShapeSize])
    else
      Success = .false.
      return
    endif

    Success = logical(FortranCrystal%Hamiltonian(KPoint = KPointFPtr, &
                                                 Derivative = Derivative, &
                                                 Array = ArrayFPtr, &
                                                 ArrayShape = ArrayShapeFPtr), C_BOOL)

  end procedure TBCrystalHamiltonian_C

  module procedure FortranTBPositionOperator
    integer(C_SIZE_T) :: LocalDerivative
    integer(C_SIZE_T) :: ArrayShapeSize
    integer(C_SIZE_T) :: BufferSize
    integer(C_SIZE_T) :: ElementCount
    integer(C_SIZE_T) :: ElementSize

    type(C_PTR) :: DirectLatticePtr
    type(C_PTR) :: SiteCoordsPtr
    type(C_PTR) :: PositionPtr
    type(C_PTR) :: ArrayHandlePtr
    type(C_PTR) :: PhasePtr
    type(C_PTR) :: PosPtr

    real(C_DOUBLE),            pointer :: DirectLattice(:, :)
    integer(C_LONG),           pointer :: SiteCoords(:, :)
    complex(C_DOUBLE_COMPLEX), pointer :: Position(:, :, :, :)
    integer(C_SIZE_T),         pointer :: ArrayHandle(:)
    complex(C_DOUBLE_COMPLEX), pointer :: Phase(:)
    real(C_DOUBLE),            pointer :: Pos(:, :)

    type(MAC) :: ArraySpec

    integer(C_SIZE_T)         :: i, j, iSites, iBand1, iBand2, iCart
    complex(C_DOUBLE_COMPLEX) :: Sum, Ipow
    real(C_DOUBLE)            :: KdotR, DerProd

    if (present(Derivative)) then
      LocalDerivative = Derivative
    else
      LocalDerivative = 0_C_SIZE_T
    endif

    if (.not.C_ASSOCIATED(Crystal%OpaqueHandle)) then
      Success = .false.
      return
    endif

    !Verify KPoint's size.
    if (Crystal%Dimensionality /= size(KPoint, KIND = C_SIZE_T)) then
      Success = .false.
      return
    endif

    !Verify possible overflow.
    ArrayShapeSize = 0_C_SIZE_T
    if (FortranAddOverflow(LocalDerivative, 3_C_SIZE_T, ArrayShapeSize)) then
      Success = .false.
      return
    endif
    if (ArrayShapeSize /= size(ArrayShape, KIND = C_SIZE_T)) then
      Success = .false.
      return
    endif

    !Fill shape.
    ArrayShape(1_C_SIZE_T) = Crystal%BandCount
    ArrayShape(2_C_SIZE_T) = Crystal%BandCount
    ArrayShape(3_C_SIZE_T) = Crystal%Dimensionality
    if (LocalDerivative > 0_C_SIZE_T) then
      do i = 1, LocalDerivative
        ArrayShape(3_C_SIZE_T + i) = Crystal%Dimensionality
      enddo
    endif

    !The next call checks for possible overflows in
    !BandCount^2 * Dimensionality^(Derivative + 1).
    call ArraySpec%Specify(Shape = ArrayShape, Layout = COLUMN_MAJOR)
    if (.not.ArraySpec%Initialized()) then
      Success = .false.
      return
    endif
    if (ArraySpec%ElementCount() /= size(Array, KIND = C_SIZE_T)) then
      Success = .false.
      return
    endif

    DirectLatticePtr = C_NULL_PTR
    SiteCoordsPtr    = C_NULL_PTR
    PositionPtr      = C_NULL_PTR

    DirectLatticePtr = TBCI_InternalGetDirectLatticePtr(Crystal%OpaqueHandle)
    SiteCoordsPtr    = TBCI_InternalGetSiteCoordsPtr(Crystal%OpaqueHandle)
    PositionPtr      = TBCI_InternalGetPositionOpPtr(Crystal%OpaqueHandle)

    if (C_ASSOCIATED(DirectLatticePtr)) then
      call C_F_POINTER(DirectLatticePtr, &
                       DirectLattice, &
                       [Crystal%Dimensionality, Crystal%Dimensionality])
    else
      Success = .false.
      return
    endif

    if (C_ASSOCIATED(SiteCoordsPtr)) then
      call C_F_POINTER(SiteCoordsPtr, &
                       SiteCoords, &
                       [Crystal%Dimensionality, Crystal%SiteCount])
    else
      Success = .false.
      return
    endif

    if (C_ASSOCIATED(PositionPtr)) then
      call C_F_POINTER(PositionPtr, &
                       Position, &
                       [Crystal%SiteCount, Crystal%BandCount, Crystal%BandCount, Crystal%Dimensionality])
    else
      Success = .false.
      return
    endif

    !Powers of i are exact: mod 4 selects among 1, i, -1, -i.
    select case (mod(LocalDerivative, 4_C_SIZE_T))
    case (0_C_SIZE_T)
      Ipow = TBCICmplx1
    case (1_C_SIZE_T)
      Ipow = TBCICmplxI
    case (2_C_SIZE_T)
      Ipow = -TBCICmplx1
    case default
      Ipow = -TBCICmplxI
    end select

    ArrayHandlePtr = C_NULL_PTR
    PhasePtr       = C_NULL_PTR
    PosPtr         = C_NULL_PTR
    Success        = .false.

    Computation: block
      ElementSize = int(C_SIZEOF(1_C_SIZE_T), C_SIZE_T)
      if (logical(FortranMulOverflow(A   = ElementSize, &
                                     B   = ArrayShapeSize, &
                                     Res = BufferSize))) exit Computation

      ArrayHandlePtr = FortranMalloc(BufferSize)
      if (.not.C_ASSOCIATED(ArrayHandlePtr)) exit Computation
      call C_F_POINTER(ArrayHandlePtr, &
                       ArrayHandle, &
                       [ArrayShapeSize])

      ElementSize = int(C_SIZEOF(TBCICmplx1), C_SIZE_T)
      if (logical(FortranMulOverflow(A   = ElementSize, &
                                     B   = Crystal%SiteCount, &
                                     Res = BufferSize))) exit Computation

      PhasePtr = FortranMalloc(BufferSize)
      if (.not.C_ASSOCIATED(PhasePtr)) exit Computation
      call C_F_POINTER(PhasePtr, &
                       Phase, &
                       [Crystal%SiteCount])

      !The Cartesian offsets are only formed for the derivatives.
      if (LocalDerivative > 0_C_SIZE_T) then
        ElementSize = int(C_SIZEOF(1.0_C_DOUBLE), C_SIZE_T)
        if (logical(FortranMulOverflow(A   = Crystal%Dimensionality, &
                                       B   = Crystal%SiteCount, &
                                       Res = ElementCount))) exit Computation
        if (logical(FortranMulOverflow(A   = ElementCount, &
                                       B   = ElementSize, &
                                       Res = BufferSize))) exit Computation

        PosPtr = FortranMalloc(BufferSize)
        if (.not.C_ASSOCIATED(PosPtr)) exit Computation
        call C_F_POINTER(PosPtr, &
                         Pos, &
                         [Crystal%Dimensionality, Crystal%SiteCount])
      endif

      do iSites = 1_C_SIZE_T, Crystal%SiteCount
        KdotR         = 2 * TBCIPi * dot_product(real(SiteCoords(:, iSites), C_DOUBLE), KPoint)
        Phase(iSites) = exp(TBCICmplxI * KdotR)
        if (LocalDerivative > 0_C_SIZE_T) then
          Pos(:, iSites) = 0.0_C_DOUBLE
          do j = 1_C_SIZE_T, Crystal%Dimensionality
            Pos(:, iSites) = Pos(:, iSites) + DirectLattice(:, j) * &
              SiteCoords(j, iSites)
          enddo
        endif
      enddo

      do i = 0_C_SIZE_T, ArraySpec%ElementCount() - 1_C_SIZE_T

        ArrayHandle = ArraySpec%Multidimensional(i)
        Sum         = TBCICmplx0
        iBand1      = ArrayHandle(1_C_SIZE_T) + 1_C_SIZE_T
        iBand2      = ArrayHandle(2_C_SIZE_T) + 1_C_SIZE_T
        iCart       = ArrayHandle(3_C_SIZE_T) + 1_C_SIZE_T

        do iSites = 1_C_SIZE_T, Crystal%SiteCount

          DerProd = 1_C_DOUBLE
          do j = 1_C_SIZE_T, LocalDerivative
            DerProd = DerProd * Pos(ArrayHandle(j + 3_C_SIZE_T) + 1_C_SIZE_T, iSites)
          enddo

          Sum = Sum + Ipow * DerProd * Phase(iSites) * &
            Position(iSites, &
                     iBand1, &
                     iBand2, &
                     iCart)

        enddo
        Array(i + 1_C_SIZE_T) = Sum
      enddo

      Success = .true.
    end block Computation

    nullify(ArrayHandle)
    nullify(Phase)
    nullify(Pos)

    if (C_ASSOCIATED(ArrayHandlePtr)) call FortranFree(ArrayHandlePtr)
    if (C_ASSOCIATED(PhasePtr))       call FortranFree(PhasePtr)
    if (C_ASSOCIATED(PosPtr))         call FortranFree(PosPtr)

  end procedure FortranTBPositionOperator

  module procedure TBPositionOperator_C
    type(TBCrystal)                    :: FortranCrystal
    real(C_DOUBLE),            pointer :: KPointFPtr(:)
    complex(C_DOUBLE_COMPLEX), pointer :: ArrayFPtr(:)
    integer(C_SIZE_T),         pointer :: ArrayShapeFPtr(:)

    if (C_ASSOCIATED(Crystal)) then
      FortranCrystal%OpaqueHandle   = Crystal
      FortranCrystal%Init           = .true.
      FortranCrystal%Owner          = .false.
      FortranCrystal%Dimensionality = FortranCrystal%GetDimensionality()
      FortranCrystal%SiteCount      = FortranCrystal%GetSiteCount()
      FortranCrystal%BandCount      = FortranCrystal%GetBandCount()
    else
      Success = .false.
      return
    endif

    if (C_ASSOCIATED(KPoint)) then
      call C_F_POINTER(KPoint, &
                       KPointFPtr, &
                       [Dimensionality])
    else
      Success = .false.
      return
    endif

    if (C_ASSOCIATED(Array)) then
      call C_F_POINTER(Array, &
                       ArrayFPtr, &
                       [ArraySize])
    else
      Success = .false.
      return
    endif

    if (C_ASSOCIATED(ArrayShape)) then
      call C_F_POINTER(ArrayShape, &
                       ArrayShapeFPtr, &
                       [ArrayShapeSize])
    else
      Success = .false.
      return
    endif

    Success = logical(FortranCrystal%PositionOperator(KPoint = KPointFPtr, &
                                                      Derivative = Derivative, &
                                                      Array = ArrayFPtr, &
                                                      ArrayShape = ArrayShapeFPtr), C_BOOL)

  end procedure TBPositionOperator_C

end submodule TBCI_F_Implementation
