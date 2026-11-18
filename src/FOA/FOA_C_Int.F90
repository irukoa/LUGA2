#include "src/FOA/FunctionalOptimizationAlgorithms.M90"

submodule (FunctionalOptimizationAlgorithms) FOA_C_Interfaces

  use, intrinsic :: ISO_C_BINDING, only: C_NULL_PTR, &
                                         C_LOC, &
                                         C_ASSOCIATED, &
                                         C_F_POINTER, &
                                         C_F_PROCPOINTER
  use, intrinsic :: IEEE_ARITHMETIC, only: IEEE_QUIET_NAN, &
                                           IEEE_VALUE

  implicit none

  type :: FortranContextWrapper
    procedure(ObjectiveFunctionInterface_C), pointer, nopass :: ObjectiveFunction => null()
    type(C_PTR)                                              :: Context = C_NULL_PTR
  end type FortranContextWrapper

  contains

  !< Evaluates a C objective function on behalf of the Fortran algorithm.
  function FSideObjectiveFunction(EvaluationPoint, UserContext) &
    result(Value)
    real(C_DOUBLE),  intent(in)                       :: EvaluationPoint(:)
    class(*),        intent(inout), optional, pointer :: UserContext
    real(C_DOUBLE)                                    :: Value

    real(C_DOUBLE), target :: LocalEvaluationPoint(size(EvaluationPoint))

    Value = IEEE_VALUE(0.0_C_DOUBLE, IEEE_QUIET_NAN)

    if (.not.present(UserContext)) return
    if (.not.associated(UserContext)) return

    LocalEvaluationPoint = EvaluationPoint
    select type (UserContext)
      type is (FortranContextWrapper)
        Value = UserContext%ObjectiveFunction(size(EvaluationPoint, KIND = C_SIZE_T), &
                                              C_LOC(LocalEvaluationPoint), &
                                              UserContext%Context)
    end select
  end function FSideObjectiveFunction

  module procedure BisectionRootFinding_C
    real(C_DOUBLE), pointer :: LeftPivotFPtr(:), &
                               RightPivotFPtr(:), &
                               RootFPtr(:)

    procedure(ObjectiveFunctionInterface_C), pointer :: CSideObjectiveFunction
    type(FortranContextWrapper),             target  :: LocalWrapper
    class(*),                                pointer :: LocalWrapperPointer

    if (Dimension == 0_C_SIZE_T) return

    if (C_ASSOCIATED(Root)) then
      call C_F_POINTER(Root, &
                       RootFPtr, &
                       [Dimension])
      RootFPtr = IEEE_VALUE(0.0_C_DOUBLE, IEEE_QUIET_NAN)
    else
      return
    endif

    if (C_ASSOCIATED(LeftPivot)) then
      call C_F_POINTER(LeftPivot, &
                       LeftPivotFPtr, &
                       [Dimension])
    else
      return
    endif

    if (C_ASSOCIATED(RightPivot)) then
      call C_F_POINTER(RightPivot, &
                       RightPivotFPtr, &
                       [Dimension])
    else
      return
    endif

    if (C_ASSOCIATED(ObjectiveFunction)) then
      call C_F_PROCPOINTER(ObjectiveFunction, &
                           CSideObjectiveFunction)
    else
      return
    endif
    LocalWrapper%ObjectiveFunction => CSideObjectiveFunction
    LocalWrapper%Context           =  UserContext
    LocalWrapperPointer            => LocalWrapper

    RootFPtr = BisectionRootFinding(LeftPivotFPtr, &
                                    RightPivotFPtr, &
                                    FSideObjectiveFunction, &
                                    MaxIterations, &
                                    Tolerance, &
                                    LocalWrapperPointer)

  end procedure BisectionRootFinding_C

  module procedure GoldenSectionMinimization_C
    real(C_DOUBLE), pointer :: LeftPivotFPtr(:), &
                               RightPivotFPtr(:), &
                               MinimumFPtr(:)

    procedure(ObjectiveFunctionInterface_C), pointer :: CSideObjectiveFunction
    type(FortranContextWrapper),             target  :: LocalWrapper
    class(*),                                pointer :: LocalWrapperPointer

    if (Dimension == 0_C_SIZE_T) return

    if (C_ASSOCIATED(Minimum)) then
      call C_F_POINTER(Minimum, &
                       MinimumFPtr, &
                       [Dimension])
      MinimumFPtr = IEEE_VALUE(0.0_C_DOUBLE, IEEE_QUIET_NAN)
    else
      return
    endif

    if (C_ASSOCIATED(LeftPivot)) then
      call C_F_POINTER(LeftPivot, &
                       LeftPivotFPtr, &
                       [Dimension])
    else
      return
    endif

    if (C_ASSOCIATED(RightPivot)) then
      call C_F_POINTER(RightPivot, &
                       RightPivotFPtr, &
                       [Dimension])
    else
      return
    endif

    if (C_ASSOCIATED(ObjectiveFunction)) then
      call C_F_PROCPOINTER(ObjectiveFunction, &
                           CSideObjectiveFunction)
    else
      return
    endif
    LocalWrapper%ObjectiveFunction => CSideObjectiveFunction
    LocalWrapper%Context           =  UserContext
    LocalWrapperPointer            => LocalWrapper

    MinimumFPtr = GoldenSectionMinimization(LeftPivotFPtr, &
                                            RightPivotFPtr, &
                                            FSideObjectiveFunction, &
                                            MaxIterations, &
                                            Tolerance, &
                                            LocalWrapperPointer)

  end procedure GoldenSectionMinimization_C

end submodule FOA_C_Interfaces
