#include "src/FOA/FunctionalOptimizationAlgorithms.M90"

submodule (FunctionalOptimizationAlgorithms) FOA_F_Implementation

  use, intrinsic :: IEEE_ARITHMETIC, only: IEEE_IS_FINITE, &
                                           IEEE_IS_NAN, &
                                           IEEE_QUIET_NAN, &
                                           IEEE_VALUE

  implicit none

  real(C_DOUBLE), parameter :: InvGoldenRatio = (sqrt(5.0_C_DOUBLE) - 1.0_C_DOUBLE) / 2.0_C_DOUBLE
  real(C_DOUBLE), parameter :: CGolden        = (3.0_C_DOUBLE - sqrt(5.0_C_DOUBLE)) / 2.0_C_DOUBLE
  real(C_DOUBLE), parameter :: EpsDouble      = epsilon(0.0_C_DOUBLE)

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
    if (MaxIterations <= 0_C_SIZE_T) return

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
    if (MaxIterations <= 0_C_SIZE_T) return

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

  module procedure BrentRootFinding
    procedure(ObjectiveFunctionInterface), pointer :: LocalObjective

    real(C_DOUBLE)    :: a(size(LeftPivot, KIND = C_SIZE_T)), &
                         b(size(LeftPivot, KIND = C_SIZE_T)), &
                         c(size(LeftPivot, KIND = C_SIZE_T))
    real(C_DOUBLE)    :: fa, fb, fc
    real(C_DOUBLE)    :: d, e, min1, min2, p, q, r, s
    real(C_DOUBLE)    :: ta, tb, tc, Tol1, xm
    integer(C_SIZE_T) :: i

    Root = IEEE_VALUE(0.0_C_DOUBLE, IEEE_QUIET_NAN)

    if (size(LeftPivot,  KIND = C_SIZE_T) /= &
        size(RightPivot, KIND = C_SIZE_T)) return

    if (size(LeftPivot, KIND = C_SIZE_T) == 0_C_SIZE_T) return

    if (.not.associated(ObjectiveFunction)) return
    LocalObjective => ObjectiveFunction

    if (Tolerance < 0.0_C_DOUBLE) return
    if (IEEE_IS_NAN(Tolerance)) return
    if (MaxIterations <= 0_C_SIZE_T) return

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

    !The bracket is kept in the parameter of the segment, between ta and tc,
    !with b the estimate at tb. The first estimate is the right pivot, and the
    !loop opens with the change of sign which places c at the other one.
    ta = 0.0_C_DOUBLE
    tb = 1.0_C_DOUBLE
    tc = tb
    c  = b
    fc = fb

    d = tb - ta
    e = d

    do i = 1_C_SIZE_T, MaxIterations
      !The bracket is the pair of the two most recent estimates: when they lie
      !on the same side of zero, the older end is dropped in favour of a.
      if ((fb < 0.0_C_DOUBLE) .eqv. (fc < 0.0_C_DOUBLE)) then
        tc = ta
        c  = a
        fc = fa
        d  = tb - ta
        e  = d
      endif

      !The estimate with the smaller value moves to b, so that the
      !interpolation runs from it towards the other end of the bracket.
      if (abs(fc) < abs(fb)) then
        ta = tb
        tb = tc
        tc = ta

        a  = b
        b  = c
        c  = a

        fa = fb
        fb = fc
        fc = fa
      endif

      !Tol1 is the spacing of the type around the estimate: half a bracket
      !below it cannot be split any further.
      Tol1 = 2.0_C_DOUBLE * EpsDouble * abs(tb)
      xm   = 0.5_C_DOUBLE * (tc - tb)

      if (abs(xm) <= Tol1) exit

      !The step is the inverse quadratic interpolation through the three most
      !recent estimates, or the secant through the two most recent ones when
      !the first and the third coincide. It is replaced by bisection whenever
      !it falls outside the bracket or is not shorter than the step which
      !produced a, so a bracket which shrinks too slowly is halved.
      if (abs(e) >= Tol1 .and. abs(fa) > abs(fb)) then
        s = fb / fa

        if (abs(ta - tc) <= 0.0_C_DOUBLE) then
          p = 2.0_C_DOUBLE * xm * s
          q = 1.0_C_DOUBLE - s
        else
          q = fa / fc
          r = fb / fc

          p = s * (2.0_C_DOUBLE * xm * q * (q - r) - &
                   (tb - ta) * (r - 1.0_C_DOUBLE))
          q = (q - 1.0_C_DOUBLE) * (r - 1.0_C_DOUBLE) * &
            (s - 1.0_C_DOUBLE)
        endif

        if (p > 0.0_C_DOUBLE) q = -q
        p    = abs(p)
        min1 = 3.0_C_DOUBLE * xm * q - abs(Tol1 * q)
        min2 = abs(e * q)

        if (2.0_C_DOUBLE * p < min(min1, min2)) then
          e = d
          d = p / q
        else
          d = xm
          e = d
        endif
      else
        d = xm
        e = d
      endif

      ta = tb
      a  = b
      fa = fb

      !A step which does not clear Tol1 is replaced by Tol1 itself: at the
      !resolution of the parameter a shorter step is lost in the rounding.
      if (abs(d) > Tol1) then
        tb = tb + d
      else
        tb = tb + sign(Tol1, xm)
      endif

      b = (1.0_C_DOUBLE - tb) * LeftPivot + tb * RightPivot

      if (present(UserContext)) then
        fb = LocalObjective(EvaluationPoint = b, &
                            UserContext = UserContext)
      else
        fb = LocalObjective(EvaluationPoint = b)
      endif

      if (IEEE_IS_NAN(fb)) then
        Root = IEEE_VALUE(0.0_C_DOUBLE, IEEE_QUIET_NAN)
        return
      endif

      if (abs(fb) <= Tolerance) then
        Root = b
        return
      endif
    enddo

    Root = b

  end procedure BrentRootFinding

  module procedure BrentMinimization
    procedure(ObjectiveFunctionInterface), pointer :: LocalObjective

    real(C_DOUBLE)    :: a(size(LeftPivot, KIND = C_SIZE_T)), &
                         Trial(size(LeftPivot, KIND = C_SIZE_T))
    real(C_DOUBLE)    :: fu, fv, fw, fx
    real(C_DOUBLE)    :: Center, d, e, etemp, lo, hi, p, q, r
    real(C_DOUBLE)    :: Scale, ToleranceT, u, v, w, Width, x
    integer(C_SIZE_T) :: i
    logical           :: Exhausted

    Minimum = IEEE_VALUE(0.0_C_DOUBLE, IEEE_QUIET_NAN)

    if (size(LeftPivot,  KIND = C_SIZE_T) /= &
        size(RightPivot, KIND = C_SIZE_T)) return

    if (size(LeftPivot, KIND = C_SIZE_T) == 0_C_SIZE_T) return

    if (.not.associated(ObjectiveFunction)) return
    LocalObjective => ObjectiveFunction

    if (Tolerance < 0.0_C_DOUBLE) return
    if (IEEE_IS_NAN(Tolerance)) return
    if (MaxIterations <= 0_C_SIZE_T) return

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

    !The best estimate and the two points which precede it, all three at the
    !midpoint of the segment: the vertex of their parabola is the midpoint
    !again, so the first step is a golden-section one.
    x = (lo + hi) / 2.0_C_DOUBLE
    w = x
    v = x

    Exhausted = (Width * Scale <= 2.0_C_DOUBLE * Tolerance)

    if (.not.Exhausted) then
      a = (1.0_C_DOUBLE - x) * LeftPivot + x * RightPivot

      if (present(UserContext)) then
        fx = LocalObjective(EvaluationPoint = a, &
                            UserContext = UserContext)
      else
        fx = LocalObjective(EvaluationPoint = a)
      endif

      if (IEEE_IS_NAN(fx)) return
      fw = fx
      fv = fx
    endif

    !The tolerance in the parameter of the segment. A call which reaches this
    !point has a segment longer than twice the tolerance, so the quotient is a
    !number of [0, 0.5) and cannot overflow.
    ToleranceT = 0.0_C_DOUBLE
    if (.not.Exhausted) ToleranceT = Tolerance / Scale

    d = 0.0_C_DOUBLE
    e = 0.0_C_DOUBLE

    do i = 1_C_SIZE_T, MaxIterations
      if (Exhausted) exit

      Center = (lo + hi) / 2.0_C_DOUBLE
      Width  = hi - lo

      if (Width * Scale <= 2.0_C_DOUBLE * Tolerance) exit

      !The step is the vertex of the parabola through (x, fx), (w, fw) and
      !(v, fv) when that vertex lies inside the bracket and is closer to x
      !than the step which produced w; otherwise it is the golden-section step
      !towards the larger part of the bracket.
      if (abs(e) > ToleranceT) then
        r     = (x - w) * (fx - fv)
        q     = (x - v) * (fx - fw)
        p     = (x - v) * q - (x - w) * r
        q     = 2.0_C_DOUBLE * (q - r)
        if (q > 0.0_C_DOUBLE) p = -p
        q     = abs(q)
        etemp = e
        e     = d

        if (IEEE_IS_NAN(p) .or. IEEE_IS_NAN(q) .or. &
            abs(p) >= abs(0.5_C_DOUBLE * q * etemp) .or. &
            p <= q * (lo - x) .or. &
            p >= q * (hi - x)) then
          if (x >= Center) then
            e = lo - x
          else
            e = hi - x
          endif
          d = CGolden * e
        else
          d = p / q
          u = x + d

          !A parabolic step which lands next to an end of the bracket is
          !replaced by the smallest step worth taking, away from that end.
          if (u - lo < 2.0_C_DOUBLE * ToleranceT .or. &
              hi - u < 2.0_C_DOUBLE * ToleranceT) then
            d = sign(ToleranceT, Center - x)
          endif
        endif
      else
        if (x >= Center) then
          e = lo - x
        else
          e = hi - x
        endif
        d = CGolden * e
      endif

      if (abs(d) >= ToleranceT) then
        u = x + d
      else
        u = x + sign(ToleranceT, d)
      endif

      !The step cannot move the estimate: the bracket is at the resolution of
      !the type and the last estimate is the answer.
      if (abs(u - x) <= 0.0_C_DOUBLE) exit

      if (u - lo <= 0.0_C_DOUBLE .or. hi - u <= 0.0_C_DOUBLE) exit

      Trial = (1.0_C_DOUBLE - u) * LeftPivot + u * RightPivot

      if (present(UserContext)) then
        fu = LocalObjective(EvaluationPoint = Trial, &
                            UserContext = UserContext)
      else
        fu = LocalObjective(EvaluationPoint = Trial)
      endif

      if (IEEE_IS_NAN(fu)) return

      if (fu <= fx) then
        if (u >= x) then
          lo = x
        else
          hi = x
        endif

        v  = w
        w  = x
        x  = u
        fv = fw
        fw = fx
        fx = fu
      else
        if (u < x) then
          lo = u
        else
          hi = u
        endif

        if (fu <= fw .or. abs(w - x) <= 0.0_C_DOUBLE) then
          v  = w
          w  = u
          fv = fw
          fw = fu
        elseif (fu <= fv .or. &
                abs(v - x) <= 0.0_C_DOUBLE .or. &
                abs(v - w) <= 0.0_C_DOUBLE) then
          v  = u
          fv = fu
        endif
      endif
    enddo

    Minimum = (1.0_C_DOUBLE - x) * LeftPivot + x * RightPivot

  end procedure BrentMinimization

end submodule FOA_F_Implementation
