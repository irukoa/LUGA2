#include "src/EINT/ExtrapolationIntegration.M90"

submodule (ExtrapolationIntegration) EINT_C_Interfaces

  implicit none

  contains

  real(C_DOUBLE) function EINT_C(Count, Array, Base) &
    bind(C, name="EINT")
    integer(C_SIZE_T), intent(in), value :: Count
    real(C_DOUBLE),    intent(in)        :: Array(Count)
    integer(C_SIZE_T), intent(in), value :: Base

    if (Count == 0_C_SIZE_T) then
      EINT_C = 0.0_C_DOUBLE
      return
    end if

    EINT_C = EINT(Array = Array, &
                  Base  = Base)
  end function EINT_C

end submodule EINT_C_Interfaces
