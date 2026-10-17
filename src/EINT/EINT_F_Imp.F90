#include "src/EINT/ExtrapolationIntegration.M90"

submodule (ExtrapolationIntegration) EINT_F_Implementation

  implicit none

  !Kind aliases.
  integer, parameter :: rk = C_DOUBLE
  integer, parameter :: sk = C_SIZE_T

  !Largest power of 2 of the largest number of the model of the type sk.
  integer(sk), parameter :: MaxPowerOfTwo = &
    int(ceiling(log(real(huge(1_sk), rk))/log(2.0_rk)), sk)

  contains

  pure logical function IsPrime(Number)
    integer(sk), intent(in) :: Number

    integer(sk) :: i

    IsPrime = .false.
    if (Number <= 1_sk) return

    do i = 2_sk, floor(sqrt(real(Number, rk)), sk)
      if (modulo(Number, i) == 0_sk) return
    end do

    IsPrime = .true.
  end function IsPrime

  pure integer(sk) function PowerFactor(Number, PrimeFactor)
    integer(sk), intent(in) :: Number
    integer(sk), intent(in) :: PrimeFactor

    integer(sk) :: Aux

    PowerFactor = 0_sk
    if (PrimeFactor < 2_sk) return

    Aux = Number
    do while (modulo(Aux, PrimeFactor) == 0_sk)
      Aux = Aux/PrimeFactor
      PowerFactor = PowerFactor + 1_sk
    end do
  end function PowerFactor

  module procedure EINT
#ifdef _OPENMP
    use, intrinsic :: OMP_LIB
#endif

    integer(sk) :: ArraySize
    integer(sk) :: LocalBase
    integer(sk) :: Power

    real(rk)    :: Work(MaxPowerOfTwo + 1_sk)
    integer(sk) :: SummandCount
    integer(sk) :: i, j, Index
    real(rk)    :: TrapezoidalSum

    ArraySize = size(Array, kind = sk)

    !Fast return.
    if (ArraySize <= 1_sk) then
      if (ArraySize == 1_sk) EINT = Array(1_sk)
      if (ArraySize == 0_sk) EINT = 0.0_rk
      return
    end if

    if (present(Base)) then
      if (IsPrime(Base)) then
        LocalBase = Base
      else
        LocalBase = 2_sk
      end if
    else
      LocalBase = 2_sk
    end if

    Power = PowerFactor(ArraySize - 1_sk, LocalBase)

    !Compute the trapezoidal sums.
    do i = 1_sk, Power + 1_sk
      SummandCount = &
        ((ArraySize - 1_sk) / LocalBase**(i - 1_sk)) + 1_sk
      TrapezoidalSum = 0.5_rk * (Array(1) + Array(ArraySize))
      do j = 2_sk, SummandCount - 1_sk
        Index = (LocalBase**(i - 1_sk)) * (j-1_sk) + 1_sk
        TrapezoidalSum = TrapezoidalSum + Array(Index)
      end do
      Work(Power + 2_sk - i) = TrapezoidalSum / real(SummandCount - 1_sk, rk)
    end do

    !Apply the extrapolation scheme.
    do i = 2_sk, (Power + 1_sk)
      do j = 1_sk, (Power + 1_sk) - i + 1_sk
        Work(j) = &
        ((real(LocalBase, rk)**2_sk)**(i - 1_sk)) * Work(j + 1_sk) - Work(j)
        Work(j) = &
        Work(j) / (-1.0_rk + (real(LocalBase, rk)**2_sk)**(i - 1_sk))
      enddo
    enddo

    EINT = Work(1)
  end procedure EINT

  module procedure EINT_P
#ifdef _OPENMP
    use, intrinsic :: OMP_LIB
#endif

    integer(sk) :: ArraySize
    integer(sk) :: LocalBase
    integer(sk) :: Power

    real(rk)    :: Work(MaxPowerOfTwo + 1_sk)
    integer(sk) :: SummandCount
    integer(sk) :: i, j, Index
    real(rk)    :: TrapezoidalSum

    ArraySize = size(Array, kind = sk)

    !Fast return.
    if (ArraySize <= 1_sk) then
      if (ArraySize == 1_sk) EINT_P = Array(1_sk)
      if (ArraySize == 0_sk) EINT_P = 0.0_rk
      return
    end if

    if (present(Base)) then
      if (IsPrime(Base)) then
        LocalBase = Base
      else
        LocalBase = 2_sk
      end if
    else
      LocalBase = 2_sk
    end if

    Power = PowerFactor(ArraySize - 1_sk, LocalBase)

    !Compute the trapezoidal sums.
    do i = 1_sk, Power + 1_sk
      SummandCount = &
        ((ArraySize - 1_sk) / LocalBase**(i - 1_sk)) + 1_sk
      TrapezoidalSum = 0.5_rk * (Array(1) + Array(ArraySize))
      !$OMP PARALLEL DO SIMD PRIVATE(j, Index) &
      !$OMP SHARED(SummandCount, LocalBase, i, Array) &
      !$OMP REDUCTION(+: TrapezoidalSum) if(.not. omp_in_parallel())
      do j = 2_sk, SummandCount - 1_sk
        Index = (LocalBase**(i - 1_sk)) * (j-1_sk) + 1_sk
        TrapezoidalSum = TrapezoidalSum + Array(Index)
      end do
      !$OMP END PARALLEL DO SIMD
      Work(Power + 2_sk - i) = TrapezoidalSum / real(SummandCount - 1_sk, rk)
    end do

    !Apply the extrapolation scheme.
    do i = 2_sk, (Power + 1_sk)
      do j = 1_sk, (Power + 1_sk) - i + 1_sk
        Work(j) = &
        ((real(LocalBase, rk)**2_sk)**(i - 1_sk)) * Work(j + 1_sk) - Work(j)
        Work(j) = &
        Work(j) / (-1.0_rk + (real(LocalBase, rk)**2_sk)**(i - 1_sk))
      enddo
    enddo

    EINT_P = Work(1)
  end procedure EINT_P

end submodule EINT_F_Implementation
