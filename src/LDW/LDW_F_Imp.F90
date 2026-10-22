#include "src/LDW/LapackDeferredWrappers.M90"
#include "src/Core/CoreFortran.M90"

submodule (LapackDeferredWrappers) LDW_F_Implementation

  use, intrinsic :: ISO_C_BINDING, only: C_PTR, &
                                         C_NULL_PTR, &
                                         C_SIZEOF, &
                                         C_ASSOCIATED, &
                                         C_F_POINTER
  use, intrinsic :: IEEE_ARITHMETIC, only: IEEE_IS_FINITE
  use CoreFortran, only: FortranMulOverflow, FortranMalloc, FortranFree

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
