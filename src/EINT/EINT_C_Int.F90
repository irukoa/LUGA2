#include "src/EINT/ExtrapolationIntegration.M90"

submodule (ExtrapolationIntegration) EINT_C_Interfaces

  use, intrinsic :: ISO_C_BINDING, only: C_PTR, C_F_POINTER

  implicit none

  contains

  real(C_DOUBLE) function EINT_C(Count, Array, Base) &
    bind(C, name="Eint")
    integer(C_SIZE_T), intent(in), value :: Count
    type(C_PTR),       intent(in), value :: Array
    integer(C_SIZE_T), intent(in), value :: Base

    real(C_DOUBLE), pointer :: ArrayF(:)

    if (Count == 0_C_SIZE_T) then
      EINT_C = 0.0_C_DOUBLE
      return
    end if

    call C_F_POINTER(Array, &
                     ArrayF, &
                     [Count])

    EINT_C = EINT(Array = ArrayF, &
                  Base  = Base)
  end function EINT_C

end submodule EINT_C_Interfaces
