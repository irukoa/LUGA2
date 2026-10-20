#include "src/LDW/LapackDeferredWrappers.M90"

submodule (LapackDeferredWrappers) LDW_F_Implementation

  implicit none

  contains

  module procedure DiagHermitian
    complex(REAL64) :: MatrixCopy(Dimension, Dimension)
    real(REAL64)    :: Eig(Dimension)
    integer(INT32)  :: IntDim
    complex(REAL64) :: WORKHarness(1)
    integer(INT32)  :: LWORK
    real(REAL64)    :: RWORKHarness(1)
    integer(INT32)  :: LRWORK
    integer(INT32)  :: IWORKHarness(1)
    integer(INT32)  :: LIWORK
    integer(INT32)  :: WorkspaceQueryStatus, &
                       CalculationStatus

    Info = 0_INT32

    if (Dimension == 0_C_SIZE_T) return

    Info = int(huge(1_INT32), C_INT)

    if (Dimension > int(huge(1_INT32), C_SIZE_T)) return

    IntDim = int(Dimension, INT32)

    MatrixCopy = cmplx(Matrix, KIND=REAL64)
    !Workspace query.
    call ZHEEVDProcedure('V', &
                         'U', &
                         IntDim, &
                         MatrixCopy, &
                         IntDim, &
                         Eig, &
                         WORKHarness, &
                         -1_INT32, &
                         RWORKHarness, &
                         -1_INT32, &
                         IWORKHarness, &
                         -1_INT32, &
                         WorkspaceQueryStatus)

    Info = int(WorkspaceQueryStatus, C_INT)

    if (WorkspaceQueryStatus == 0_C_INT) then
      LWORK  = nint(abs(WORKHarness(1_INT32)))
      LRWORK = nint(RWORKHarness(1_INT32))
      LIWORK = IWORKHarness(1_INT32)
      !Actual diagonalization with stack-allocated work arrays.
      block
        complex(REAL64) :: WORK(max(1_INT32, LWORK))
        real(REAL64)    :: RWORK(max(1_INT32, LRWORK))
        integer(INT32)  :: IWORK(max(1_INT32, LIWORK))

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
        Unitary = cmplx(MatrixCopy, KIND=C_DOUBLE_COMPLEX)
        Info    = int(CalculationStatus, C_INT)
      end block
    else
      return
    endif

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
      i = j + 1_C_SIZE_T

    enddo

  end procedure DegeneracyGroup

end submodule LDW_F_Implementation
