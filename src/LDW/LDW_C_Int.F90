#include "src/LDW/LapackDeferredWrappers.M90"

submodule (LapackDeferredWrappers) LDW_C_Interfaces

  use, intrinsic :: ISO_C_BINDING, only: C_F_PROCPOINTER, &
                                         C_ASSOCIATED, &
                                         C_F_POINTER

  implicit none

  contains

  module procedure DiagHermitian_C
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

  end procedure DiagHermitian_C

  module procedure SchurDecomposition_C
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

  end procedure SchurDecomposition_C

  module procedure SVDDecomposition_C
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

  end procedure SVDDecomposition_C

  module procedure GeneralizedSchurDecomposition_C
    complex(C_DOUBLE_COMPLEX),  pointer :: MatrixAFPtr(:, :)
    complex(C_DOUBLE_COMPLEX),  pointer :: MatrixBFPtr(:, :)
    complex(C_DOUBLE_COMPLEX),  pointer :: UnitaryUFPtr(:, :)
    complex(C_DOUBLE_COMPLEX),  pointer :: EigenvaluesAFPtr(:)
    complex(C_DOUBLE_COMPLEX),  pointer :: EigenvaluesBFPtr(:)
    complex(C_DOUBLE_COMPLEX),  pointer :: UnitaryVHFPtr(:, :)
    procedure(ZGGESInterface),  pointer :: ZGGESProcedureFPtr
    integer(C_INT),             pointer :: InfoFPtr

    logical :: StoreEigenvaluesA, &
               StoreEigenvaluesB

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

    if (C_ASSOCIATED(MatrixA)) then
      call C_F_POINTER(MatrixA, &
                       MatrixAFPtr, &
                       [Dimension, Dimension])
    else
      return
    endif

    if (C_ASSOCIATED(MatrixB)) then
      call C_F_POINTER(MatrixB, &
                       MatrixBFPtr, &
                       [Dimension, Dimension])
    else
      return
    endif

    if (C_ASSOCIATED(UnitaryU)) then
      call C_F_POINTER(UnitaryU, &
                       UnitaryUFPtr, &
                       [Dimension, Dimension])
    else
      return
    endif

    if (C_ASSOCIATED(EigenvaluesA)) then
      call C_F_POINTER(EigenvaluesA, &
                       EigenvaluesAFPtr, &
                       [Dimension])
      StoreEigenvaluesA = .true.
    else
      StoreEigenvaluesA = .false.
    endif

    if (C_ASSOCIATED(EigenvaluesB)) then
      call C_F_POINTER(EigenvaluesB, &
                       EigenvaluesBFPtr, &
                       [Dimension])
      StoreEigenvaluesB = .true.
    else
      StoreEigenvaluesB = .false.
    endif

    if (C_ASSOCIATED(UnitaryVH)) then
      call C_F_POINTER(UnitaryVH, &
                       UnitaryVHFPtr, &
                       [Dimension, Dimension])
    else
      return
    endif

    if (C_ASSOCIATED(ZGGESProcedure)) then
      call C_F_PROCPOINTER(ZGGESProcedure, &
                           ZGGESProcedureFPtr)
    else
      return
    endif

    InfoFPtr = 0_C_INT

    if (StoreEigenvaluesA) then
      if (StoreEigenvaluesB) then
        call GeneralizedSchurDecomposition(Dimension = Dimension, &
                                           MatrixA = MatrixAFPtr, &
                                           MatrixB = MatrixBFPtr, &
                                           UnitaryU = UnitaryUFPtr, &
                                           EigenvaluesA = EigenvaluesAFPtr, &
                                           EigenvaluesB = EigenvaluesBFPtr, &
                                           UnitaryVH = UnitaryVHFPtr, &
                                           ZGGESProcedure = ZGGESProcedureFPtr, &
                                           Info = InfoFPtr)
      else
        call GeneralizedSchurDecomposition(Dimension = Dimension, &
                                           MatrixA = MatrixAFPtr, &
                                           MatrixB = MatrixBFPtr, &
                                           UnitaryU = UnitaryUFPtr, &
                                           EigenvaluesA = EigenvaluesAFPtr, &
                                           UnitaryVH = UnitaryVHFPtr, &
                                           ZGGESProcedure = ZGGESProcedureFPtr, &
                                           Info = InfoFPtr)
      endif
    else
      if (StoreEigenvaluesB) then
        call GeneralizedSchurDecomposition(Dimension = Dimension, &
                                           MatrixA = MatrixAFPtr, &
                                           MatrixB = MatrixBFPtr, &
                                           UnitaryU = UnitaryUFPtr, &
                                           EigenvaluesB = EigenvaluesBFPtr, &
                                           UnitaryVH = UnitaryVHFPtr, &
                                           ZGGESProcedure = ZGGESProcedureFPtr, &
                                           Info = InfoFPtr)
      else
        call GeneralizedSchurDecomposition(Dimension = Dimension, &
                                           MatrixA = MatrixAFPtr, &
                                           MatrixB = MatrixBFPtr, &
                                           UnitaryU = UnitaryUFPtr, &
                                           UnitaryVH = UnitaryVHFPtr, &
                                           ZGGESProcedure = ZGGESProcedureFPtr, &
                                           Info = InfoFPtr)
      endif
    endif

  end procedure GeneralizedSchurDecomposition_C

  module procedure DegeneracyGroup_C
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

  end procedure DegeneracyGroup_C

end submodule LDW_C_Interfaces
