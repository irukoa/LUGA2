#include "src/LDW/LapackDeferredWrappers.M90"

submodule (LapackDeferredWrappers) LDW_C_Interfaces

  use, intrinsic :: ISO_C_BINDING, only: C_PTR, &
                                         C_FUNPTR, &
                                         C_F_PROCPOINTER, &
                                         C_ASSOCIATED, &
                                         C_F_POINTER

  implicit none

  contains

  subroutine DiagHermitian_C(Dimension, Matrix, Unitary, Eigenvalues, ZHEEVDProcedure, Info) &
    bind(C, name="DiagHermitian")
    integer(C_SIZE_T), intent(in), value :: Dimension
    type(C_PTR),       intent(in), value :: Matrix
    type(C_PTR),       intent(in), value :: Unitary
    type(C_PTR),       intent(in), value :: Eigenvalues
    type(C_FUNPTR),    intent(in), value :: ZHEEVDProcedure
    type(C_PTR),       intent(in), value :: Info

    complex(C_DOUBLE_COMPLEX),  pointer :: MatrixFPtr(:, :)
    complex(C_DOUBLE_COMPLEX),  pointer :: UnitaryFPtr(:, :)
    real(C_DOUBLE),             pointer :: EigenvaluesFPtr(:)
    procedure(ZHEEVDInterface), pointer :: ZHEEVDProcedureFPtr
    integer(C_INT),             pointer :: InfoFPtr

    if (C_ASSOCIATED(Info)) then
      call C_F_POINTER(Info, &
                       InfoFPtr)
      if (Dimension > int(huge(1_INT32), C_SIZE_T)) then
        InfoFPtr = int(huge(1_INT32), C_INT)
        return
      endif
    else
      return
    endif

    if (C_ASSOCIATED(Matrix)) then
      call C_F_POINTER(Matrix, &
                       MatrixFPtr, &
                       [Dimension, Dimension])
    else
      return
    endif

    if (C_ASSOCIATED(Unitary)) then
      call C_F_POINTER(Unitary, &
                       UnitaryFPtr, &
                       [Dimension, Dimension])
    else
      return
    endif

    if (C_ASSOCIATED(Eigenvalues)) then
      call C_F_POINTER(Eigenvalues, &
                       EigenvaluesFPtr, &
                       [Dimension])
    else
      return
    endif

    if (C_ASSOCIATED(ZHEEVDProcedure)) then
      call C_F_PROCPOINTER(ZHEEVDProcedure, &
                           ZHEEVDProcedureFPtr)
    else
      return
    endif

    InfoFPtr = 0_C_INT

    call DiagHermitian(Dimension = Dimension, &
                       Matrix = MatrixFPtr, &
                       Unitary = UnitaryFPtr, &
                       Eigenvalues = EigenvaluesFPtr, &
                       ZHEEVDProcedure = ZHEEVDProcedureFPtr, &
                       Info = InfoFPtr)

  end subroutine

  subroutine DegeneracyGroup_C(Dimension, Eigenvalues, DegeneracyThreshold, Degeneracies) &
    bind(C, name="DegeneracyGroup")
    integer(C_SIZE_T), intent(in), value :: Dimension
    type(C_PTR),       intent(in), value :: Eigenvalues
    real(C_DOUBLE),    intent(in), value :: DegeneracyThreshold
    type(C_PTR),       intent(in), value :: Degeneracies

    real(C_DOUBLE),    pointer :: EigenvaluesFPtr(:)
    integer(C_SIZE_T), pointer :: DegeneraciesFPtr(:)

    if (C_ASSOCIATED(Eigenvalues)) then
      call C_F_POINTER(Eigenvalues, &
                       EigenvaluesFPtr, &
                       [Dimension])
    else
      return
    endif

    if (C_ASSOCIATED(Degeneracies)) then
      call C_F_POINTER(Degeneracies, &
                       DegeneraciesFPtr, &
                       [Dimension])
    else
      return
    endif

    call DegeneracyGroup(Dimension = Dimension, &
                         Eigenvalues = EigenvaluesFPtr, &
                         DegeneracyThreshold = DegeneracyThreshold, &
                         Degeneracies = DegeneraciesFPtr)

  end subroutine DegeneracyGroup_C

end submodule LDW_C_Interfaces
