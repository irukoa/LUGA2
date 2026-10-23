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

    logical :: StoreValues

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
      StoreValues = .true.
    else
      StoreValues = .false.
    endif

    if (C_ASSOCIATED(ZHEEVDProcedure)) then
      call C_F_PROCPOINTER(ZHEEVDProcedure, &
                           ZHEEVDProcedureFPtr)
    else
      return
    endif

    InfoFPtr = 0_C_INT

    if (StoreValues) then
      call DiagHermitian(Dimension = Dimension, &
                         Matrix = MatrixFPtr, &
                         Unitary = UnitaryFPtr, &
                         Eigenvalues = EigenvaluesFPtr, &
                         ZHEEVDProcedure = ZHEEVDProcedureFPtr, &
                         Info = InfoFPtr)
    else
      call DiagHermitian(Dimension = Dimension, &
                         Matrix = MatrixFPtr, &
                         Unitary = UnitaryFPtr, &
                         ZHEEVDProcedure = ZHEEVDProcedureFPtr, &
                         Info = InfoFPtr)
    endif

  end subroutine DiagHermitian_C

  subroutine SchurDecomposition_C(Dimension, Matrix, Unitary, SchurForm, ZGEESProcedure, Info) &
    bind(C, name="SchurDecomposition")
    integer(C_SIZE_T), intent(in), value :: Dimension
    type(C_PTR),       intent(in), value :: Matrix
    type(C_PTR),       intent(in), value :: Unitary
    type(C_PTR),       intent(in), value :: SchurForm
    type(C_FUNPTR),    intent(in), value :: ZGEESProcedure
    type(C_PTR),       intent(in), value :: Info

    complex(C_DOUBLE_COMPLEX), pointer :: MatrixFPtr(:, :)
    complex(C_DOUBLE_COMPLEX), pointer :: UnitaryFPtr(:, :)
    complex(C_DOUBLE_COMPLEX), pointer :: SchurFormFPtr(:, :)
    procedure(ZGEESInterface), pointer :: ZGEESProcedureFPtr
    integer(C_INT),            pointer :: InfoFPtr

    logical :: StoreValues

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

    if (C_ASSOCIATED(SchurForm)) then
      call C_F_POINTER(SchurForm, &
                       SchurFormFPtr, &
                       [Dimension, Dimension])
      StoreValues = .true.
    else
      StoreValues = .false.
    endif

    if (C_ASSOCIATED(ZGEESProcedure)) then
      call C_F_PROCPOINTER(ZGEESProcedure, &
                           ZGEESProcedureFPtr)
    else
      return
    endif

    InfoFPtr = 0_C_INT

    if (StoreValues) then
      call SchurDecomposition(Dimension = Dimension, &
                              Matrix = MatrixFPtr, &
                              Unitary = UnitaryFPtr, &
                              SchurForm = SchurFormFPtr, &
                              ZGEESProcedure = ZGEESProcedureFPtr, &
                              Info = InfoFPtr)
    else
      call SchurDecomposition(Dimension = Dimension, &
                              Matrix = MatrixFPtr, &
                              Unitary = UnitaryFPtr, &
                              ZGEESProcedure = ZGEESProcedureFPtr, &
                              Info = InfoFPtr)
    endif

  end subroutine SchurDecomposition_C

  subroutine SVDDecomposition_C(DimensionR, DimensionC, Matrix, UnitaryU, DiagSigma, UnitaryVH, ZGESDDProcedure, Info) &
    bind(C, name="SVDDecomposition")
    integer(C_SIZE_T), intent(in), value :: DimensionR
    integer(C_SIZE_T), intent(in), value :: DimensionC
    type(C_PTR),       intent(in), value :: Matrix
    type(C_PTR),       intent(in), value :: UnitaryU
    type(C_PTR),       intent(in), value :: DiagSigma
    type(C_PTR),       intent(in), value :: UnitaryVH
    type(C_FUNPTR),    intent(in), value :: ZGESDDProcedure
    type(C_PTR),       intent(in), value :: Info

    complex(C_DOUBLE_COMPLEX),  pointer :: MatrixFPtr(:, :)
    complex(C_DOUBLE_COMPLEX),  pointer :: UnitaryUFPtr(:, :)
    real(C_DOUBLE),             pointer :: DiagSigmaFPtr(:)
    complex(C_DOUBLE_COMPLEX),  pointer :: UnitaryVHFPtr(:, :)
    procedure(ZGESDDInterface), pointer :: ZGESDDProcedureFPtr
    integer(C_INT),             pointer :: InfoFPtr

    logical :: StoreValues

    if (C_ASSOCIATED(Info)) then
      call C_F_POINTER(Info, &
                       InfoFPtr)
      if ((DimensionR > int(huge(1_INT32), C_SIZE_T)) .or. &
          (DimensionC > int(huge(1_INT32), C_SIZE_T))) then
        InfoFPtr = int(huge(1_INT32), C_INT)
        return
      endif
    else
      return
    endif

    if (C_ASSOCIATED(Matrix)) then
      call C_F_POINTER(Matrix, &
                       MatrixFPtr, &
                       [DimensionR, DimensionC])
    else
      return
    endif

    if (C_ASSOCIATED(UnitaryU)) then
      call C_F_POINTER(UnitaryU, &
                       UnitaryUFPtr, &
                       [DimensionR, DimensionR])
    else
      return
    endif

    if (C_ASSOCIATED(DiagSigma)) then
      call C_F_POINTER(DiagSigma, &
                       DiagSigmaFPtr, &
                       [min(DimensionR, DimensionC)])
      StoreValues = .true.
    else
      StoreValues = .false.
    endif

    if (C_ASSOCIATED(UnitaryVH)) then
      call C_F_POINTER(UnitaryVH, &
                       UnitaryVHFPtr, &
                       [DimensionC, DimensionC])
    else
      return
    endif

    if (C_ASSOCIATED(ZGESDDProcedure)) then
      call C_F_PROCPOINTER(ZGESDDProcedure, &
                           ZGESDDProcedureFPtr)
    else
      return
    endif

    InfoFPtr = 0_C_INT

    if (StoreValues) then
      call SVDDecomposition(DimensionR = DimensionR, &
                            DimensionC = DimensionC, &
                            Matrix = MatrixFPtr, &
                            UnitaryU = UnitaryUFPtr, &
                            DiagSigma = DiagSigmaFPtr, &
                            UnitaryVH = UnitaryVHFPtr, &
                            ZGESDDProcedure = ZGESDDProcedureFPtr, &
                            Info = InfoFPtr)
    else
      call SVDDecomposition(DimensionR = DimensionR, &
                            DimensionC = DimensionC, &
                            Matrix = MatrixFPtr, &
                            UnitaryU = UnitaryUFPtr, &
                            UnitaryVH = UnitaryVHFPtr, &
                            ZGESDDProcedure = ZGESDDProcedureFPtr, &
                            Info = InfoFPtr)
    endif

  end subroutine SVDDecomposition_C

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
