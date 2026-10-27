#include "src/EINT/ExtrapolationIntegration.M90"

submodule (ExtrapolationIntegration) EINT_C_Interfaces

  use, intrinsic :: ISO_C_BINDING, only: C_ASSOCIATED, &
                                         C_F_POINTER

  implicit none

  contains

  module procedure EINT_C
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
  end procedure EINT_C

  module procedure EINT_PC
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
  end procedure EINT_PC

end submodule EINT_C_Interfaces
