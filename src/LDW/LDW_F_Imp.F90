#include "src/LDW/LapackDeferredWrappers.M90"
#include "src/Core/CoreFortran.M90"

submodule (LapackDeferredWrappers) LDW_F_Implementation

  use, intrinsic :: ISO_C_BINDING, only: C_PTR, &
                                         C_NULL_PTR, &
                                         C_SIZEOF, &
                                         C_ASSOCIATED, &
                                         C_F_POINTER
  use, intrinsic :: IEEE_ARITHMETIC, only: IEEE_IS_FINITE
  use CoreFortran, only: FortranMulOverflow, FortranMalloc, FortranFree, FortranAddOverflow

  implicit none

  contains

  module procedure DiagHermitian
    integer(C_SIZE_T)        :: BufferSize, BufferCopy
    complex(REAL64), pointer :: MatrixCopy(:, :)
    real(REAL64),    pointer :: Eig(:)
    complex(REAL64), pointer :: WORK(:)
    real(REAL64),    pointer :: RWORK(:)
    integer(INT32),  pointer :: IWORK(:)

    type(C_PTR) :: MatrixCopyHandle, &
                   EigHandle, &
                   WORKHandle, &
                   RWORKHandle, &
                   IWORKHandle

    complex(REAL64) :: QueryWORK(1)
    real(REAL64)    :: QueryRWORK(1)
    integer(INT32)  :: QueryIWORK(1)

    integer(INT32)  :: IntDim
    integer(INT32)  :: LWORK, LRWORK, LIWORK
    integer(INT32)  :: WorkspaceQueryStatus, &
                       CalculationStatus

    nullify(MatrixCopy, Eig, WORK, RWORK, IWORK)

    MatrixCopyHandle = C_NULL_PTR
    EigHandle        = C_NULL_PTR
    WORKHandle       = C_NULL_PTR
    RWORKHandle      = C_NULL_PTR
    IWORKHandle      = C_NULL_PTR

    Info = 0_INT32

    if (Dimension == 0_C_SIZE_T) return

    Info = int(huge(1_INT32), C_INT)

    if (Dimension > int(huge(1_INT32), C_SIZE_T)) return

    Body: block

      !Matrix copy buffer.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(cmplx(1.0_REAL64, 1.0_REAL64, REAL64)), C_SIZE_T), &
                                     B   = Dimension, &
                                     Res = BufferSize))) exit Body
      BufferCopy = BufferSize
      if (logical(FortranMulOverflow(A   = BufferCopy, &
                                     B   = Dimension, &
                                     Res = BufferSize))) exit Body

      MatrixCopyHandle = FortranMalloc(BufferSize)
      if (.not.C_ASSOCIATED(MatrixCopyHandle)) exit Body
      call C_F_POINTER(MatrixCopyHandle, &
                       MatrixCopy, &
                       [Dimension, Dimension])
      MatrixCopy = cmplx(Matrix, KIND = REAL64)

      !Eigenvalue buffer.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(1.0_REAL64), C_SIZE_T), &
                                     B   = Dimension, &
                                     Res = BufferSize))) exit Body

      EigHandle = FortranMalloc(BufferSize)
      if (.not.C_ASSOCIATED(EigHandle)) exit Body
      call C_F_POINTER(EigHandle, &
                       Eig, &
                       [Dimension])

      IntDim = int(Dimension, INT32)

      !Workspace query. Initialize the query buffers so that a callback
      !which fails to fill them is detected by the validation below.
      QueryWORK  = cmplx(0.0_REAL64, 0.0_REAL64, REAL64)
      QueryRWORK = 0.0_REAL64
      QueryIWORK = 0_INT32

      call ZHEEVDProcedure('V', &
                           'U', &
                           IntDim, &
                           MatrixCopy, &
                           IntDim, &
                           Eig, &
                           QueryWORK, &
                           -1_INT32, &
                           QueryRWORK, &
                           -1_INT32, &
                           QueryIWORK, &
                           -1_INT32, &
                           WorkspaceQueryStatus)

      if (WorkspaceQueryStatus /= 0_C_INT) then
        Info = int(WorkspaceQueryStatus, C_INT)
        exit Body
      endif

      !From here on Info stays at the failure sentinel until the
      !computational call succeeds.
      Info = int(huge(1_INT32), C_INT)

      if (.not.IEEE_IS_FINITE(real(QueryWORK(1), REAL64)) .or. &
          .not.IEEE_IS_FINITE(QueryRWORK(1)) .or. &
          real(QueryWORK(1), REAL64) < 0.0_REAL64 .or. &
          real(QueryWORK(1), REAL64) > real(huge(1_INT32), REAL64) .or. &
          QueryRWORK(1) < 0.0_REAL64 .or. &
          QueryRWORK(1) > real(huge(1_INT32), REAL64) .or. &
          QueryIWORK(1) < 1_INT32) exit Body

      LWORK  = int(nint(real(QueryWORK(1), REAL64)), INT32)
      LRWORK = int(nint(QueryRWORK(1)), INT32)
      LIWORK = QueryIWORK(1)

      !Complex workspace.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(cmplx(1.0_REAL64, 1.0_REAL64, REAL64)), C_SIZE_T), &
                                     B   = int(LWORK, C_SIZE_T), &
                                     Res = BufferSize))) exit Body

      WORKHandle = FortranMalloc(BufferSize)
      if (.not.C_ASSOCIATED(WORKHandle)) exit Body
      call C_F_POINTER(WORKHandle, &
                       WORK, &
                       [LWORK])

      !Real workspace.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(1.0_REAL64), C_SIZE_T), &
                                     B   = int(LRWORK, C_SIZE_T), &
                                     Res = BufferSize))) exit Body

      RWORKHandle = FortranMalloc(BufferSize)
      if (.not.C_ASSOCIATED(RWORKHandle)) exit Body
      call C_F_POINTER(RWORKHandle, &
                       RWORK, &
                       [LRWORK])

      !Integer workspace.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(1_INT32), C_SIZE_T), &
                                     B   = int(LIWORK, C_SIZE_T), &
                                     Res = BufferSize))) exit Body

      IWORKHandle = FortranMalloc(BufferSize)
      if (.not.C_ASSOCIATED(IWORKHandle)) exit Body
      call C_F_POINTER(IWORKHandle, &
                       IWORK, &
                       [LIWORK])

      !Actual diagonalization.
      call ZHEEVDProcedure('V', &
                           'U', &
                           IntDim, &
                           MatrixCopy, &
                           IntDim, &
                           Eig, &
                           WORK, &
                           LWORK, &
                           RWORK, &
                           LRWORK, &
                           IWORK, &
                           LIWORK, &
                           CalculationStatus)

      if (present(Eigenvalues)) Eigenvalues = real(Eig, C_DOUBLE)
      Unitary = cmplx(MatrixCopy, KIND = C_DOUBLE_COMPLEX)
      Info    = int(CalculationStatus, C_INT)

    end block Body

    if (associated(Eig)) Eig => null()
    if (associated(MatrixCopy)) MatrixCopy => null()
    if (associated(WORK)) WORK => null()
    if (associated(RWORK)) RWORK => null()
    if (associated(IWORK)) IWORK => null()

    call FortranFree(IWORKHandle)
    call FortranFree(RWORKHandle)
    call FortranFree(WORKHandle)
    call FortranFree(EigHandle)
    call FortranFree(MatrixCopyHandle)

  end procedure DiagHermitian

  !< SELECT callback for ZGEES. LDW always requests SORT = 'N', so LAPACK
  !< never references this procedure; the comparison only keeps the dummy
  !< argument used.
  pure logical(LOGICAL32) function SelectUnused(C) result(Selected)
    complex(REAL64), intent(in) :: C

    Selected = .false.
    associate(dummy => C)
    end associate

  end function SelectUnused

  module procedure SchurDecomposition
    integer(C_SIZE_T)           :: BufferSize, BufferCopy
    complex(REAL64),    pointer :: MatrixCopy(:, :)
    complex(REAL64),    pointer :: Eig(:)
    complex(REAL64),    pointer :: LocalUnitary(:, :)
    complex(REAL64),    pointer :: WORK(:)
    real(REAL64),       pointer :: RWORK(:)
    logical(LOGICAL32), pointer :: BWORK(:)

    type(C_PTR) :: MatrixCopyHandle, &
                   EigHandle, &
                   UnitaryHandle, &
                   WORKHandle, &
                   RWORKHandle, &
                   BWORKHandle

    complex(REAL64)    :: QueryWORK(1)
    real(REAL64)       :: QueryRWORK(1)
    logical(LOGICAL32) :: QueryBWORK(1)

    integer(INT32) :: IntDim, SDim
    integer(INT32) :: LWORK
    integer(INT32) :: WorkspaceQueryStatus, &
                      CalculationStatus

    nullify(MatrixCopy, Eig, LocalUnitary, WORK, RWORK, BWORK)

    MatrixCopyHandle = C_NULL_PTR
    EigHandle        = C_NULL_PTR
    UnitaryHandle    = C_NULL_PTR
    WORKHandle       = C_NULL_PTR
    RWORKHandle      = C_NULL_PTR
    BWORKHandle      = C_NULL_PTR

    Info = 0_INT32

    if (Dimension == 0_C_SIZE_T) return

    Info = int(huge(1_INT32), C_INT)

    if (Dimension > int(huge(1_INT32), C_SIZE_T)) return

    Body: block

      !Matrix copy buffer.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(cmplx(1.0_REAL64, 1.0_REAL64, REAL64)), C_SIZE_T), &
                                     B   = Dimension, &
                                     Res = BufferSize))) exit Body
      BufferCopy = BufferSize
      if (logical(FortranMulOverflow(A   = BufferCopy, &
                                     B   = Dimension, &
                                     Res = BufferSize))) exit Body

      MatrixCopyHandle = FortranMalloc(BufferSize)
      if (.not.C_ASSOCIATED(MatrixCopyHandle)) exit Body
      call C_F_POINTER(MatrixCopyHandle, &
                       MatrixCopy, &
                       [Dimension, Dimension])
      MatrixCopy = cmplx(Matrix, KIND = REAL64)

      !Eigenvalue buffer.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(cmplx(1.0_REAL64, 1.0_REAL64, REAL64)), C_SIZE_T), &
                                     B   = Dimension, &
                                     Res = BufferSize))) exit Body

      EigHandle = FortranMalloc(BufferSize)
      if (.not.C_ASSOCIATED(EigHandle)) exit Body
      call C_F_POINTER(EigHandle, &
                       Eig, &
                       [Dimension])

      !Unitary matrix buffer.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(cmplx(1.0_REAL64, 1.0_REAL64, REAL64)), C_SIZE_T), &
                                     B   = Dimension, &
                                     Res = BufferSize))) exit Body
      BufferCopy = BufferSize
      if (logical(FortranMulOverflow(A   = BufferCopy, &
                                     B   = Dimension, &
                                     Res = BufferSize))) exit Body

      UnitaryHandle = FortranMalloc(BufferSize)
      if (.not.C_ASSOCIATED(UnitaryHandle)) exit Body
      call C_F_POINTER(UnitaryHandle, &
                       LocalUnitary, &
                       [Dimension, Dimension])

      IntDim = int(Dimension, INT32)

      !Workspace query. Initialize the query buffers so that a callback
      !which fails to fill them is detected by the validation below.
      QueryWORK  = cmplx(0.0_REAL64, 0.0_REAL64, REAL64)
      QueryRWORK = 0.0_REAL64
      QueryBWORK = .false._LOGICAL32

      call ZGEESProcedure('V', &
                          'N', &
                          SelectUnused, &
                          IntDim, &
                          MatrixCopy, &
                          IntDim, &
                          SDim, &
                          Eig, &
                          LocalUnitary, &
                          IntDim, &
                          QueryWORK, &
                          -1_INT32, &
                          QueryRWORK, &
                          QueryBWORK, &
                          WorkspaceQueryStatus)

      if (WorkspaceQueryStatus /= 0_C_INT) then
        Info = int(WorkspaceQueryStatus, C_INT)
        exit Body
      endif

      !From here on Info stays at the failure sentinel until the
      !computational call succeeds.
      Info = int(huge(1_INT32), C_INT)

      if (.not.IEEE_IS_FINITE(real(QueryWORK(1), REAL64)) .or. &
          real(QueryWORK(1), REAL64) < 0.0_REAL64 .or. &
          real(QueryWORK(1), REAL64) > real(huge(1_INT32), REAL64)) exit Body

      LWORK = int(nint(real(QueryWORK(1), REAL64)), INT32)

      !Complex workspace.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(cmplx(1.0_REAL64, 1.0_REAL64, REAL64)), C_SIZE_T), &
                                     B   = int(LWORK, C_SIZE_T), &
                                     Res = BufferSize))) exit Body

      WORKHandle = FortranMalloc(BufferSize)
      if (.not.C_ASSOCIATED(WORKHandle)) exit Body
      call C_F_POINTER(WORKHandle, &
                       WORK, &
                       [LWORK])

      !Real workspace.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(1.0_REAL64), C_SIZE_T), &
                                     B   = Dimension, &
                                     Res = BufferSize))) exit Body

      RWORKHandle = FortranMalloc(BufferSize)
      if (.not.C_ASSOCIATED(RWORKHandle)) exit Body
      call C_F_POINTER(RWORKHandle, &
                       RWORK, &
                       [Dimension])

      !Logical workspace.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(.false._LOGICAL32), C_SIZE_T), &
                                     B   = Dimension, &
                                     Res = BufferSize))) exit Body

      BWORKHandle = FortranMalloc(BufferSize)
      if (.not.C_ASSOCIATED(BWORKHandle)) exit Body
      call C_F_POINTER(BWORKHandle, &
                       BWORK, &
                       [Dimension])

      !Actual Schur decomposition.
      call ZGEESProcedure('V', &
                          'N', &
                          SelectUnused, &
                          IntDim, &
                          MatrixCopy, &
                          IntDim, &
                          SDim, &
                          Eig, &
                          LocalUnitary, &
                          IntDim, &
                          WORK, &
                          LWORK, &
                          RWORK, &
                          BWORK, &
                          CalculationStatus)

      Unitary = cmplx(LocalUnitary, KIND = C_DOUBLE_COMPLEX)
      if (present(SchurForm)) SchurForm = cmplx(MatrixCopy, KIND = C_DOUBLE_COMPLEX)
      Info = int(CalculationStatus, C_INT)

    end block Body

    if (associated(BWORK)) BWORK => null()
    if (associated(RWORK)) RWORK => null()
    if (associated(WORK)) WORK => null()
    if (associated(Eig)) Eig => null()
    if (associated(LocalUnitary)) LocalUnitary => null()
    if (associated(MatrixCopy)) MatrixCopy => null()

    call FortranFree(BWORKHandle)
    call FortranFree(RWORKHandle)
    call FortranFree(WORKHandle)
    call FortranFree(EigHandle)
    call FortranFree(UnitaryHandle)
    call FortranFree(MatrixCopyHandle)

  end procedure SchurDecomposition

  module procedure SVDDecomposition
    integer(C_SIZE_T)        :: BufferASize, BufferACopy, &
                                BufferBSize, BufferBCopy
    complex(REAL64), pointer :: MatrixCopy(:, :)
    complex(REAL64), pointer :: LocalUnitaryU(:, :)
    real(REAL64),    pointer :: Sig(:)
    complex(REAL64), pointer :: LocalUnitaryVH(:, :)
    complex(REAL64), pointer :: WORK(:)
    real(REAL64),    pointer :: RWORK(:)
    integer(INT32),  pointer :: IWORK(:)

    type(C_PTR) :: MatrixCopyHandle, &
                   UnitaryUHandle, &
                   SigHandle, &
                   UnitaryVHHandle, &
                   WORKHandle, &
                   RWORKHandle, &
                   IWORKHandle

    complex(REAL64) :: QueryWORK(1)
    real(REAL64)    :: QueryRWORK(1)
    integer(INT32)  :: QueryIWORK(1)

    integer(INT32)    :: IntDimensionR, IntDimensionC
    integer(INT32)    :: MinMN, MaxMN
    integer(INT32)    :: LWORK, LRWORK, LIWORK
    integer(C_SIZE_T) :: ValA, ValB
    integer(INT32)    :: WorkspaceQueryStatus, &
                         CalculationStatus

    integer(INT32)  :: i, j, k
    real(REAL64)    :: SigSwap
    complex(REAL64) :: ColumnSwap, RowSwap

    nullify(MatrixCopy, LocalUnitaryU, Sig, LocalUnitaryVH, &
            WORK, RWORK, IWORK)

    MatrixCopyHandle = C_NULL_PTR
    UnitaryUHandle   = C_NULL_PTR
    SigHandle        = C_NULL_PTR
    UnitaryVHHandle  = C_NULL_PTR
    WORKHandle       = C_NULL_PTR
    RWORKHandle      = C_NULL_PTR
    IWORKHandle      = C_NULL_PTR

    Info = 0_INT32

    if ((DimensionR == 0_C_SIZE_T) .or. &
        (DimensionC == 0_C_SIZE_T)) return

    Info = int(huge(1_INT32), C_INT)

    if ((DimensionR > int(huge(1_INT32), C_SIZE_T)) .or. &
        (DimensionC > int(huge(1_INT32), C_SIZE_T))) return

    IntDimensionR = int(DimensionR, INT32)
    IntDimensionC = int(DimensionC, INT32)
    MinMN          = min(IntDimensionR, IntDimensionC)
    MaxMN          = max(IntDimensionR, IntDimensionC)

    Body: block

      !Matrix copy buffer.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(cmplx(1.0_REAL64, 1.0_REAL64, REAL64)), C_SIZE_T), &
                                     B   = DimensionR, &
                                     Res = BufferASize))) exit Body
      BufferACopy = BufferASize
      if (logical(FortranMulOverflow(A   = BufferACopy, &
                                     B   = DimensionC, &
                                     Res = BufferASize))) exit Body

      MatrixCopyHandle = FortranMalloc(BufferASize)
      if (.not.C_ASSOCIATED(MatrixCopyHandle)) exit Body
      call C_F_POINTER(MatrixCopyHandle, &
                       MatrixCopy, &
                       [DimensionR, DimensionC])
      MatrixCopy = cmplx(Matrix, KIND = REAL64)

      !UnitaryU buffer.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(cmplx(1.0_REAL64, 1.0_REAL64, REAL64)), C_SIZE_T), &
                                     B   = DimensionR, &
                                     Res = BufferASize))) exit Body
      BufferACopy = BufferASize
      if (logical(FortranMulOverflow(A   = BufferACopy, &
                                     B   = DimensionR, &
                                     Res = BufferASize))) exit Body

      UnitaryUHandle = FortranMalloc(BufferASize)
      if (.not.C_ASSOCIATED(UnitaryUHandle)) exit Body
      call C_F_POINTER(UnitaryUHandle, &
                       LocalUnitaryU, &
                       [DimensionR, DimensionR])

      !Singular value buffer.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(1.0_REAL64), C_SIZE_T), &
                                     B   = min(DimensionR, DimensionC), &
                                     Res = BufferASize))) exit Body

      SigHandle = FortranMalloc(BufferASize)
      if (.not.C_ASSOCIATED(SigHandle)) exit Body
      call C_F_POINTER(SigHandle, &
                       Sig, &
                       [min(DimensionR, DimensionC)])

      !UnitaryVH buffer.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(cmplx(1.0_REAL64, 1.0_REAL64, REAL64)), C_SIZE_T), &
                                     B   = DimensionC, &
                                     Res = BufferASize))) exit Body
      BufferACopy = BufferASize
      if (logical(FortranMulOverflow(A   = BufferACopy, &
                                     B   = DimensionC, &
                                     Res = BufferASize))) exit Body

      UnitaryVHHandle = FortranMalloc(BufferASize)
      if (.not.C_ASSOCIATED(UnitaryVHHandle)) exit Body
      call C_F_POINTER(UnitaryVHHandle, &
                       LocalUnitaryVH, &
                       [DimensionC, DimensionC])

      !LRWORK calculation. ZGESDD has no RWORK length argument, so the
      !wrapper provides the largest requirement of the reference
      !implementation: max(5*mn*mn + 5*mn, 2*mx*mn + 2*mn*mn + mn).
      if (logical(FortranMulOverflow(A   = int(MinMN, C_SIZE_T), &
                                     B   = int(MinMN, C_SIZE_T), &
                                     Res = BufferASize))) exit Body
      BufferACopy = BufferASize
      if (logical(FortranMulOverflow(A   = BufferACopy, &
                                     B   = 5_C_SIZE_T, &
                                     Res = BufferASize))) exit Body !Holds 5*mn*mn.

      if (logical(FortranMulOverflow(A   = int(MinMN, C_SIZE_T), &
                                     B   = 5_C_SIZE_T, &
                                     Res = BufferBSize))) exit Body !Holds 5*mn.

      if (logical(FortranAddOverflow(A   = BufferASize, &
                                     B   = BufferBSize, &
                                     Res = ValA))) exit Body !Holds 5*mn*mn + 5*mn.

      if (logical(FortranMulOverflow(A   = int(MaxMN, C_SIZE_T), &
                                     B   = int(MinMN, C_SIZE_T), &
                                     Res = BufferASize))) exit Body
      BufferACopy = BufferASize
      if (logical(FortranMulOverflow(A   = BufferACopy, &
                                     B   = 2_C_SIZE_T, &
                                     Res = BufferASize))) exit Body !Holds 2*mx*mn.

      if (logical(FortranMulOverflow(A   = int(MinMN, C_SIZE_T), &
                                     B   = int(MinMN, C_SIZE_T), &
                                     Res = BufferBSize))) exit Body
      BufferBCopy = BufferBSize
      if (logical(FortranMulOverflow(A   = BufferBCopy, &
                                     B   = 2_C_SIZE_T, &
                                     Res = BufferBSize))) exit Body !Holds 2*mn*mn.

      if (logical(FortranAddOverflow(A   = BufferASize, &
                                     B   = BufferBSize, &
                                     Res = BufferACopy))) exit Body !Holds 2*mx*mn + 2*mn*mn.

      if (logical(FortranAddOverflow(A   = BufferACopy, &
                                     B   = int(MinMN, C_SIZE_T), &
                                     Res = ValB))) exit Body !Holds 2*mx*mn + 2*mn*mn + mn.

      BufferASize = max(ValA, ValB)
      if (BufferASize > int(huge(1_INT32), C_SIZE_T)) exit Body
      LRWORK = int(BufferASize, INT32)

      !LIWORK calculation. ZGESDD has no IWORK length argument, so the
      !wrapper provides 8*min(M, N), the requirement of the reference
      !implementation.
      if (logical(FortranMulOverflow(A   = int(MinMN, C_SIZE_T), &
                                     B   = 8_C_SIZE_T, &
                                     Res = BufferASize))) exit Body
      if (BufferASize > int(huge(1_INT32), C_SIZE_T)) exit Body
      LIWORK = int(BufferASize, INT32)

      !Workspace query. Initialize the query buffers so that a callback
      !which fails to fill WORK(1) is detected by the validation below.
      QueryWORK  = cmplx(0.0_REAL64, 0.0_REAL64, REAL64)
      QueryRWORK = 0.0_REAL64
      QueryIWORK = 0_INT32

      call ZGESDDProcedure('A', &
                           IntDimensionR, &
                           IntDimensionC, &
                           MatrixCopy, &
                           IntDimensionR, &
                           Sig, &
                           LocalUnitaryU, &
                           IntDimensionR, &
                           LocalUnitaryVH, &
                           IntDimensionC, &
                           QueryWORK, &
                           -1_INT32, &
                           QueryRWORK, &
                           QueryIWORK, &
                           WorkspaceQueryStatus)

      if (WorkspaceQueryStatus /= 0_C_INT) then
        Info = int(WorkspaceQueryStatus, C_INT)
        exit Body
      endif

      !From here on Info stays at the failure sentinel until the
      !computational call succeeds.
      Info = int(huge(1_INT32), C_INT)

      !The queried LWORK must be a usable buffer length. A callback which
      !leaves WORK(1) untouched reports zero and is rejected here.
      if (.not.IEEE_IS_FINITE(real(QueryWORK(1), REAL64)) .or. &
          real(QueryWORK(1), REAL64) < 1.0_REAL64 .or. &
          real(QueryWORK(1), REAL64) > real(huge(1_INT32), REAL64)) exit Body

      LWORK = int(nint(real(QueryWORK(1), REAL64)), INT32)

      !Complex workspace.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(cmplx(1.0_REAL64, 1.0_REAL64, REAL64)), C_SIZE_T), &
                                     B   = int(LWORK, C_SIZE_T), &
                                     Res = BufferASize))) exit Body

      WORKHandle = FortranMalloc(BufferASize)
      if (.not.C_ASSOCIATED(WORKHandle)) exit Body
      call C_F_POINTER(WORKHandle, &
                       WORK, &
                       [LWORK])

      !Real workspace.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(1.0_REAL64), C_SIZE_T), &
                                     B   = int(LRWORK, C_SIZE_T), &
                                     Res = BufferASize))) exit Body

      RWORKHandle = FortranMalloc(BufferASize)
      if (.not.C_ASSOCIATED(RWORKHandle)) exit Body
      call C_F_POINTER(RWORKHandle, &
                       RWORK, &
                       [LRWORK])

      !Integer workspace.
      if (logical(FortranMulOverflow(A   = int(C_SIZEOF(1_INT32), C_SIZE_T), &
                                     B   = int(LIWORK, C_SIZE_T), &
                                     Res = BufferASize))) exit Body

      IWORKHandle = FortranMalloc(BufferASize)
      if (.not.C_ASSOCIATED(IWORKHandle)) exit Body
      call C_F_POINTER(IWORKHandle, &
                       IWORK, &
                       [LIWORK])

      !Singular value decomposition. JOBZ is 'A' so that U is DimensionR by
      !DimensionR and VT is DimensionC by DimensionC. ZGESDD returns the
      !singular values in descending order.
      call ZGESDDProcedure('A', &
                           IntDimensionR, &
                           IntDimensionC, &
                           MatrixCopy, &
                           IntDimensionR, &
                           Sig, &
                           LocalUnitaryU, &
                           IntDimensionR, &
                           LocalUnitaryVH, &
                           IntDimensionC, &
                           WORK, &
                           LWORK, &
                           RWORK, &
                           IWORK, &
                           CalculationStatus)

      !The other LDW drivers report their spectrum in ascending order, so the
      !descending sequence produced by ZGESDD is reversed. The permutation is
      !applied to the matching columns of U and rows of VT as well, so that
      !A = U*diag(S)*VT still holds and DiagSigma(j) keeps pairing with
      !column j of U and row j of VT.
      do j = 1_INT32, MinMN / 2_INT32
        k = MinMN - j + 1_INT32

        SigSwap = Sig(j)
        Sig(j)  = Sig(k)
        Sig(k)  = SigSwap

        do i = 1_INT32, IntDimensionR
          ColumnSwap          = LocalUnitaryU(i, j)
          LocalUnitaryU(i, j) = LocalUnitaryU(i, k)
          LocalUnitaryU(i, k) = ColumnSwap
        enddo

        do i = 1_INT32, IntDimensionC
          RowSwap              = LocalUnitaryVH(j, i)
          LocalUnitaryVH(j, i) = LocalUnitaryVH(k, i)
          LocalUnitaryVH(k, i) = RowSwap
        enddo

      enddo

      if (present(DiagSigma)) DiagSigma = real(Sig, C_DOUBLE)
      UnitaryU  = cmplx(LocalUnitaryU, KIND = C_DOUBLE_COMPLEX)
      UnitaryVH = cmplx(LocalUnitaryVH, KIND = C_DOUBLE_COMPLEX)
      Info      = int(CalculationStatus, C_INT)

    end block Body

    if (associated(IWORK)) IWORK => null()
    if (associated(RWORK)) RWORK => null()
    if (associated(WORK)) WORK => null()
    if (associated(LocalUnitaryVH)) LocalUnitaryVH => null()
    if (associated(Sig)) Sig => null()
    if (associated(LocalUnitaryU)) LocalUnitaryU => null()
    if (associated(MatrixCopy)) MatrixCopy => null()

    call FortranFree(IWORKHandle)
    call FortranFree(RWORKHandle)
    call FortranFree(WORKHandle)
    call FortranFree(UnitaryVHHandle)
    call FortranFree(SigHandle)
    call FortranFree(UnitaryUHandle)
    call FortranFree(MatrixCopyHandle)

  end procedure SVDDecomposition

  module procedure DegeneracyGroup
    integer(C_SIZE_T) :: i, j

    if (DegeneracyThreshold <= 0.0_C_DOUBLE) return

    Degeneracies = 0_C_SIZE_T

    i = 1_C_SIZE_T
    do while (i <= Dimension)

      j = i

      do while (j < Dimension)
        if (Eigenvalues(j + 1_C_SIZE_T) - Eigenvalues(j) > DegeneracyThreshold) exit
        j = j + 1_C_SIZE_T
      enddo

      Degeneracies(i) = j - i + 1_C_SIZE_T
      i               = j + 1_C_SIZE_T

    enddo

  end procedure DegeneracyGroup

end submodule LDW_F_Implementation
