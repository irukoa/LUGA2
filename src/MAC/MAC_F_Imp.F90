#include "src/MAC/MAC_C_Int.M90"

submodule (MultidimensionalArrayContainers:MAC_C_Interfaces) MAC_F_Implementation

  use, intrinsic :: ISO_C_BINDING, only: C_ASSOCIATED, C_F_POINTER

  implicit none

  contains

  module procedure FortranFreeMAC
    call FreeMAC(Container%OpaqueHandle)
    Container%Init = .false.
  end procedure FortranFreeMAC

  module procedure FortranFreeMAC_Final
    call FreeMAC(Container%OpaqueHandle)
    Container%Init = .false.
  end procedure FortranFreeMAC_Final

  module procedure FortranNewMAC
    integer(MACLayout) :: LocalLayout

    if (present(Layout)) then
      if ((Layout == COLUMN_MAJOR) .or. &
          (Layout == ROW_MAJOR)) then
        LocalLayout = Layout
      else
        LocalLayout = COLUMN_MAJOR
      end if
    else
      LocalLayout = COLUMN_MAJOR
    end if

    if (C_ASSOCIATED(Container%OpaqueHandle)) call FortranFreeMAC(Container)

    Container%OpaqueHandle = NewMAC(Rank   = int(size(Shape), C_SIZE_T), &
                                    Shape  = Shape, &
                                    Layout = LocalLayout)

    if (C_ASSOCIATED(Container%OpaqueHandle)) Container%Init = .true.
  end procedure FortranNewMAC

  module procedure FortranInitialized
    Initialized = Container%Init
  end procedure FortranInitialized

  module procedure FortranMACGetRank
    Rank = MACGetRank(Container%OpaqueHandle)
  end procedure FortranMACGetRank

  module procedure FortranMACGetShape
    type(C_PTR)                :: ShapeCPtr
    integer(C_SIZE_T), pointer :: ShapeFPtr(:)

    ShapeCPtr = MACGetShape(Container%OpaqueHandle)
    call C_F_POINTER(ShapeCPtr, &
                     ShapeFPtr, &
                     [MACGetRank(Container%OpaqueHandle)])
    Shape = ShapeFPtr
  end procedure FortranMACGetShape

  module procedure FortranMACGetElementCount
    ElementCount = MACGetElementCount(Container%OpaqueHandle)
  end procedure FortranMACGetElementCount

  module procedure FortranMACGetLayout
    Layout = MACGetLayout(Container%OpaqueHandle)
  end procedure FortranMACGetLayout

  module procedure FortranMACFlatToMultidimensional
    call MACFlatToMultidimensional(OpaqueHandle          = Container%OpaqueHandle, &
                                   Rank                  = MACGetRank(Container%OpaqueHandle), &
                                   FlatIndex             = FlatIndex, &
                                   MultidimensionalIndex = MultidimensionalIndex)
  end procedure FortranMACFlatToMultidimensional

  module procedure FortranMACMultidimensionalToFlat
    call MACMultidimensionalToFlat(OpaqueHandle          = Container%OpaqueHandle, &
                                   Rank                  = MACGetRank(Container%OpaqueHandle), &
                                   MultidimensionalIndex = MultidimensionalIndex, &
                                   FlatIndex             = FlatIndex)
  end procedure FortranMACMultidimensionalToFlat

end submodule MAC_F_Implementation
