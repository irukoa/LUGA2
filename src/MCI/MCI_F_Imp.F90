#include "src/MCI/MCI_C_Int.M90"
#include "src/Core/CoreFortran.M90"

submodule (MessageCodingInterface:MCI_C_Interfaces) MCI_F_Implementation

  use, intrinsic :: ISO_C_BINDING, only: C_ASSOCIATED, C_F_POINTER, &
                                         C_NULL_CHAR

  use CoreFortran, only: FortranFree

  implicit none

  contains

  module procedure FortranCopyMCICompiledFormat
    if (C_ASSOCIATED(Copy%OpaqueHandle, Original%OpaqueHandle)) return
    if (C_ASSOCIATED(Copy%OpaqueHandle)) call FortranFreeMCIFormat(Copy)
    Copy%OpaqueHandle = Original%OpaqueHandle
    Copy%Init         = Original%Init
    Copy%Owner        = .false.
  end procedure FortranCopyMCICompiledFormat

  module procedure FortranFreeMCIFormat
    if (CompiledFormat%Owner) then
      call FreeMCIFormat(CompiledFormat%OpaqueHandle)
    else
      CompiledFormat%OpaqueHandle = C_NULL_PTR
    endif
    CompiledFormat%Init = .false.
  end procedure FortranFreeMCIFormat

  module procedure FortranFreeMCIFormat_Final
    if (CompiledFormat%Owner) then
      call FreeMCIFormat(CompiledFormat%OpaqueHandle)
    else
      CompiledFormat%OpaqueHandle = C_NULL_PTR
    endif
    CompiledFormat%Init = .false.
  end procedure FortranFreeMCIFormat_Final

  module procedure FortranCompileMCIFormat
    if (C_ASSOCIATED(CompiledFormat%OpaqueHandle)) call FortranFreeMCIFormat(CompiledFormat)

    CompiledFormat%OpaqueHandle = CompileMCIFormat(Format = trim(Format) // C_NULL_CHAR, &
                                                   MaxFieldBitSize = MaxFieldBitSize)

    if (C_ASSOCIATED(CompiledFormat%OpaqueHandle)) CompiledFormat%Init  = .true.
    if (C_ASSOCIATED(CompiledFormat%OpaqueHandle)) CompiledFormat%Owner = .true.
  end procedure FortranCompileMCIFormat

  module procedure FortranInitialized
    Initialized = CompiledFormat%Init
  end procedure FortranInitialized

  module procedure FortranGetMCIMessageBitSize
    BitSize = GetMCIMessageBitSize(CompiledFormat%OpaqueHandle)
  end procedure FortranGetMCIMessageBitSize

  module procedure FortranGetMCIMessageByteSize
    ByteSize = GetMCIMessageByteSize(CompiledFormat%OpaqueHandle)
  end procedure FortranGetMCIMessageByteSize

  module procedure FortranGetMCIMessageFieldCount
    FieldCount = GetMCIMessageFieldCount(CompiledFormat%OpaqueHandle)
  end procedure FortranGetMCIMessageFieldCount

  module procedure FortranGetMCIMessageFieldId
    FieldId = GetMCIMessageFieldId(CompiledFormat = CompiledFormat%OpaqueHandle, &
                                   SemanticId = trim(SemanticId) // C_NULL_CHAR)
  end procedure FortranGetMCIMessageFieldId

  module procedure FortranGetMCIMessageSemanticId
    character(C_CHAR), pointer :: Characters(:)
    type(C_PTR)                :: SemanticIdPtr
    integer(C_SIZE_T)          :: SemanticIdLength

    SemanticIdPtr = GetMCIMessageSemanticId(CompiledFormat = CompiledFormat%OpaqueHandle, &
                                            FieldId = FieldId)
    if (.not.C_ASSOCIATED(SemanticIdPtr)) then
      SemanticId = ''
      return
    endif

    SemanticIdLength = CStrlen(SemanticIdPtr)
    call C_F_POINTER(SemanticIdPtr, Characters, [SemanticIdLength])
    SemanticId = transfer(Characters, repeat(' ', SemanticIdLength))
    nullify(Characters)
    call FortranFree(SemanticIdPtr)
    SemanticIdPtr = C_NULL_PTR
  end procedure FortranGetMCIMessageSemanticId

  module procedure FortranEncodeMCIMessage
    Success = logical(EncodeMCIMessage(CompiledFormat = CompiledFormat%OpaqueHandle, &
                                       MessageByteSize = size(Message, KIND = C_SIZE_T), &
                                       Message = Message, &
                                       FieldId = FieldId, &
                                       Encoding = Encoding, &
                                       Val = Value))
  end procedure FortranEncodeMCIMessage

  module procedure FortranDecodeMCIMessage
    Success = logical(DecodeMCIMessage(CompiledFormat = CompiledFormat%OpaqueHandle, &
                                       MessageByteSize = size(Message, KIND = C_SIZE_T), &
                                       Message = Message, &
                                       FieldId = FieldId, &
                                       Encoding = Encoding, &
                                       Val = Value))
  end procedure FortranDecodeMCIMessage

end submodule MCI_F_Implementation
