#include "src/FOA/FunctionalOptimizationAlgorithms.M90"

submodule (FunctionalOptimizationAlgorithms) FOA_F_Implementation

  use, intrinsic :: IEEE_ARITHMETIC, only: IEEE_IS_FINITE, &
                                           IEEE_IS_NAN, &
                                           IEEE_QUIET_NAN, &
                                           IEEE_VALUE

  implicit none

  real(C_DOUBLE), parameter :: InvGoldenRatio = (sqrt(5.0_C_DOUBLE) - 1.0_C_DOUBLE) / 2.0_C_DOUBLE

  contains

  module procedure BisectionRootFinding
    procedure(ObjectiveFunctionInterface), pointer :: LocalObjective

    real(C_DOUBLE)    :: a(size(LeftPivot, KIND = C_SIZE_T)), &
                         b(size(LeftPivot, KIND = C_SIZE_T))
    real(C_DOUBLE)    :: fa, fb, fRoot
    integer(C_SIZE_T) :: i

    Root = IEEE_VALUE(0.0_C_DOUBLE, IEEE_QUIET_NAN)

    if (size(LeftPivot,  KIND = C_SIZE_T) /= &
        size(RightPivot, KIND = C_SIZE_T)) return

    if (size(LeftPivot,  KIND = C_SIZE_T) == 0_C_SIZE_T) return

    if (.not.associated(ObjectiveFunction)) return
    LocalObjective => ObjectiveFunction

    if (Tolerance < 0.0_C_DOUBLE) return
    if (IEEE_IS_NAN(Tolerance)) return
    if (MaxIterations == 0_C_SIZE_T) return

    if (any(.not.IEEE_IS_FINITE(LeftPivot))) return
    if (any(.not.IEEE_IS_FINITE(RightPivot))) return

    a = LeftPivot
    b = RightPivot

    if (present(UserContext)) then
      fa = LocalObjective(EvaluationPoint = a, &
                          UserContext = UserContext)
      fb = LocalObjective(EvaluationPoint = b, &
                          UserContext = UserContext)
    else
      fa = LocalObjective(EvaluationPoint = a)
      fb = LocalObjective(EvaluationPoint = b)
    endif

    if (abs(fa) <= 0.0_C_DOUBLE) then
      Root = a
      return
    endif

    if (abs(fb) <= 0.0_C_DOUBLE) then
      Root = b
      return
    endif

    if (IEEE_IS_NAN(fa) .or. IEEE_IS_NAN(fb)) return
    if ((fa < 0.0_C_DOUBLE) .eqv. (fb < 0.0_C_DOUBLE)) return

    do i = 1_C_SIZE_T, MaxIterations
      !Half of each end is summed separately, so the midpoint cannot overflow
      !even when the bracket nearly fills the range of the type.
      Root = a / 2.0_C_DOUBLE + b / 2.0_C_DOUBLE

      !The two ends are adjacent representable values,
      !no point lies strictly between them, and
      !the last estimate is the answer.
      if (all(Root <= min(a, b) .or. Root >= max(a, b))) return

      if (present(UserContext)) then
        fRoot = LocalObjective(EvaluationPoint = Root, &
                               UserContext = UserContext)
      else
        fRoot = LocalObjective(EvaluationPoint = Root)
      endif

      if (IEEE_IS_NAN(fRoot)) then
        Root = IEEE_VALUE(0.0_C_DOUBLE, IEEE_QUIET_NAN)
        return
      endif

      if (abs(fRoot) <= Tolerance) return

      if ((fRoot < 0.0_C_DOUBLE) .eqv. (fa < 0.0_C_DOUBLE)) then
        a  = Root
        fa = fRoot
      else
        b = Root
      endif
    enddo

  end procedure BisectionRootFinding

  module procedure GoldenSectionMinimization
    procedure(ObjectiveFunctionInterface), pointer :: LocalObjective

    real(C_DOUBLE)    :: a(size(LeftPivot, KIND = C_SIZE_T)), &
                         b(size(LeftPivot, KIND = C_SIZE_T))
    real(C_DOUBLE)    :: fa, fb, aT, bT, t
    real(C_DOUBLE)    :: Width, Scale, lo, hi
    logical           :: Exhausted
    integer(C_SIZE_T) :: i

    Minimum = IEEE_VALUE(0.0_C_DOUBLE, IEEE_QUIET_NAN)

    if (size(LeftPivot,  KIND = C_SIZE_T) /= &
        size(RightPivot, KIND = C_SIZE_T)) return

    if (size(LeftPivot, KIND = C_SIZE_T) == 0_C_SIZE_T) return

    if (.not.associated(ObjectiveFunction)) return
    LocalObjective => ObjectiveFunction

    if (Tolerance < 0.0_C_DOUBLE) return
    if (IEEE_IS_NAN(Tolerance)) return
    if (MaxIterations == 0_C_SIZE_T) return

    if (any(.not.IEEE_IS_FINITE(LeftPivot))) return
    if (any(.not.IEEE_IS_FINITE(RightPivot))) return

    !Halving keeps the length of a segment wider than the range of the type
    !measurable. Doubling it back is the only operation which can overflow.
    Scale = maxval(abs(LeftPivot / 2.0_C_DOUBLE - &
                       RightPivot / 2.0_C_DOUBLE))

    if (Scale > huge(Scale) / 2.0_C_DOUBLE) then
      Scale = huge(Scale)
    else
      Scale = Scale * 2.0_C_DOUBLE
    endif

    lo    = 0.0_C_DOUBLE
    hi    = 1.0_C_DOUBLE
    Width = 1.0_C_DOUBLE

    Exhausted = (Width * Scale <= Tolerance)

    if (.not.Exhausted) then
      aT = hi - InvGoldenRatio * Width
      bT = lo + InvGoldenRatio * Width

      a  = (1.0_C_DOUBLE - aT) * LeftPivot + aT * RightPivot
      b  = (1.0_C_DOUBLE - bT) * LeftPivot + bT * RightPivot

      Exhausted = all(abs(a - b) <= 0.0_C_DOUBLE)

      if (.not.Exhausted) then
        if (present(UserContext)) then
          fa = LocalObjective(EvaluationPoint = a, &
                              UserContext = UserContext)
          fb = LocalObjective(EvaluationPoint = b, &
                              UserContext = UserContext)
        else
          fa = LocalObjective(EvaluationPoint = a)
          fb = LocalObjective(EvaluationPoint = b)
        endif

        if (IEEE_IS_NAN(fa) .or. IEEE_IS_NAN(fb)) return
      endif
    endif

    do i = 1_C_SIZE_T, MaxIterations
      if (Exhausted) exit

      Width = hi - lo
      if (Width * Scale <= Tolerance) exit

      if (fa < fb) then
        hi = bT
        bT = aT
        b  = a
        fb = fa

        Width = hi - lo
        aT    = hi - InvGoldenRatio * Width
        a     = (1.0_C_DOUBLE - aT) * LeftPivot + aT * RightPivot

        Exhausted = all(abs(a - b) <= 0.0_C_DOUBLE)
        if (Exhausted) exit

        if (present(UserContext)) then
          fa = LocalObjective(EvaluationPoint = a, &
                              UserContext = UserContext)
        else
          fa = LocalObjective(EvaluationPoint = a)
        endif

        if (IEEE_IS_NAN(fa)) return
      else
        lo = aT
        aT = bT
        a  = b
        fa = fb

        Width = hi - lo
        bT    = lo + InvGoldenRatio * Width
        b     = (1.0_C_DOUBLE - bT) * LeftPivot + bT * RightPivot

        Exhausted = all(abs(b - a) <= 0.0_C_DOUBLE)
        if (Exhausted) exit

        if (present(UserContext)) then
          fb = LocalObjective(EvaluationPoint = b, &
                              UserContext = UserContext)
        else
          fb = LocalObjective(EvaluationPoint = b)
        endif

        if (IEEE_IS_NAN(fb)) return
      endif
    enddo

    t       = (lo + hi) / 2.0_C_DOUBLE
    Minimum = (1.0_C_DOUBLE - t) * LeftPivot + t * RightPivot

  end procedure GoldenSectionMinimization

end submodule FOA_F_Implementation
