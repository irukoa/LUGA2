#include "src/EINT/ExtrapolationIntegration.M90"

submodule (ExtrapolationIntegration) EINT_C_Interfaces

  use, intrinsic :: ISO_C_BINDING, only: C_PTR, &
                                         C_ASSOCIATED, &
                                         C_F_POINTER

  implicit none

  contains

  real(C_DOUBLE) function EINT_C(Count, Array, Base) &
    bind(C, name="EINT")
    integer(C_SIZE_T), intent(in), value :: Count
    type(C_PTR),       intent(in), value :: Array
    integer(C_SIZE_T), intent(in), value :: Base

    real(C_DOUBLE), pointer :: ArrayFPtr(:)

    if (C_ASSOCIATED(Array)) then
      call C_F_POINTER(Array, &
                       ArrayFPtr, &
                       [Count])
    else
      EINT_C = 0.0_C_DOUBLE
      return
    endif

    if (Count == 0_C_SIZE_T) then
      EINT_C = 0.0_C_DOUBLE
      return
    endif

    EINT_C = EINT(Array = ArrayFPtr, &
                  Base  = Base)
  end function EINT_C

  real(C_DOUBLE) function EINT_PC(Count, Array, Base) &
    bind(C, name="EINT_P")
    integer(C_SIZE_T), intent(in), value :: Count
    type(C_PTR),       intent(in), value :: Array
    integer(C_SIZE_T), intent(in), value :: Base

    real(C_DOUBLE), pointer :: ArrayFPtr(:)

    if (C_ASSOCIATED(Array)) then
      call C_F_POINTER(Array, &
                       ArrayFPtr, &
                       [Count])
    else
      EINT_PC = 0.0_C_DOUBLE
      return
    endif

    if (Count == 0_C_SIZE_T) then
      EINT_PC = 0.0_C_DOUBLE
      return
    endif

    EINT_PC = EINT_P(Array = ArrayFPtr, &
                     Base  = Base)
  end function EINT_PC

end submodule EINT_C_Interfaces
