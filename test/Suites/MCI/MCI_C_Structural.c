#include "src/Core/Core.h"
#include "src/MCI/MessageCodingInterface.h"
#include "test/CoreSupport/CoreSupport.h"
#include "test/TSD/TSD.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

// MCICompiledFormat is opaque to callers. These structural tests complete the
// type locally to exercise the defensive validation of malformed records that
// CompileMCIFormat itself cannot produce. Keep this layout in sync with
// src/MCI/MCI_C_Imp.c.
struct MCICompiledFormat {
  size_t  NumFields;
  size_t *BitSize;
  size_t *BitOffset;
  size_t  TotalBits;
  size_t  TotalBytes;
};

CONTEXT_FN(StructuralSetup) {
  (void)ContextData;
  Core_Malloc = FallibleMalloc;
  ResetFallibleMalloc();
  Core_Realloc = FallibleRealloc;
  ResetFallibleRealloc();
  Core_AddOverflow = FallibleAddOverflow;
  ResetFallibleAddOverflow();
  Core_MulOverflow = FallibleMulOverflow;
  ResetFallibleMulOverflow();
}
CONTEXT_FN(StructuralTeardown) {
  (void)ContextData;
  RESET_FN(Core_Malloc);
  RESET_FN(Core_Realloc);
  RESET_FN(Core_AddOverflow);
  RESET_FN(Core_MulOverflow);
}
CONTEXT(StructuralCtx,
        StructuralSetup,
        StructuralTeardown,
        (size_t)0);

static MCICompiledFormat *
CompileForStructuralTest(const char  *Format,
                         const size_t MaxFieldBitSize) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat(Format, MaxFieldBitSize);
  ASSERT(CompiledFormat);
  return CompiledFormat;
}

static void BuildNestedFormat(char        *Buffer,
                              const size_t Depth,
                              const char  *Field) {
  size_t i;
  size_t Position = 0;

  for (i = 0; i < Depth; i++) {
    Buffer[Position++] = '1';
    Buffer[Position++] = 'R';
    Buffer[Position++] = '(';
  }
  while (*Field) {
    Buffer[Position++] = *Field++;
  }
  for (i = 0; i < Depth; i++) {
    Buffer[Position++] = ')';
  }
  Buffer[Position] = '\0';
}

TEST(FreeMCIFormat_BadInput) {
  MCICompiledFormat *CompiledFormat = NULL;

  FreeMCIFormat(NULL);
  FreeMCIFormat(&CompiledFormat);
  ASSERT(!CompiledFormat);
}

TEST(FreeMCIFormat_NominalAndDoubleFree) {
  MCICompiledFormat *CompiledFormat = NULL;

  CompiledFormat = CompileForStructuralTest("8|8", 8);
  if (!CompiledFormat) {
    return;
  }
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 2);
  ASSERT(GetMCIMessageFieldCount(CompiledFormat) == 2);

  FreeMCIFormat(&CompiledFormat);
  ASSERT(!CompiledFormat);

  FreeMCIFormat(&CompiledFormat);
  ASSERT(!CompiledFormat);
}

TEST(GetMCIMessageByteSize_BadInput) {
  ASSERT(GetMCIMessageByteSize(NULL) == 0);
}

TEST(GetMCIMessageFieldCount_BadInput) {
  ASSERT(GetMCIMessageFieldCount(NULL) == 0);
}

TEST(CompileMCIFormat_BadInput) {
  MCICompiledFormat *CompiledFormat = NULL;

  CompiledFormat = CompileMCIFormat(NULL, 8);
  ASSERT(!CompiledFormat);
  FreeMCIFormat(&CompiledFormat);

  CompiledFormat = CompileMCIFormat("", 8);
  ASSERT(!CompiledFormat);
  FreeMCIFormat(&CompiledFormat);

  CompiledFormat = CompileMCIFormat("1", 0);
  ASSERT(!CompiledFormat);
  FreeMCIFormat(&CompiledFormat);

  CompiledFormat = CompileMCIFormat("9", 8);
  ASSERT(!CompiledFormat);
  FreeMCIFormat(&CompiledFormat);
}

TEST(CompileMCIFormat_PartialFailureCleanup) {
  static const char *const Cases[] = {
      "8|9",
      "8|8x",
      "8|18446744073709551616",
      "8|2R(0)",
  };
  MCICompiledFormat *CompiledFormat;
  size_t             i;

  for (i = 0; i < (sizeof(Cases) / sizeof(Cases[0])); i++) {
    CompiledFormat = CompileMCIFormat(Cases[i], 8);
    ASSERT(!CompiledFormat);
    FreeMCIFormat(&CompiledFormat);
    ASSERT(!CompiledFormat);
  }
}

TEST(CompileMCIFormat_RejectsZeroFieldsAndCounts) {
  static const char *const Cases[] = {
      "0", "00", "0|8", "8|0", "0R(8)", "00R(8)", "8|0R(8)", "0R(0)",
  };
  MCICompiledFormat *CompiledFormat;
  size_t             i;

  for (i = 0; i < (sizeof(Cases) / sizeof(Cases[0])); i++) {
    CompiledFormat = CompileMCIFormat(Cases[i], 8);
    ASSERT(!CompiledFormat);
    FreeMCIFormat(&CompiledFormat);
    ASSERT(!CompiledFormat);
  }
}

TEST(CompileMCIFormat_RejectsExcessiveNesting) {
  char               Format[512];
  MCICompiledFormat *CompiledFormat;

  BuildNestedFormat(Format, 64, "8");
  CompiledFormat = CompileMCIFormat(Format, 8);
  ASSERT(CompiledFormat);
  if (CompiledFormat) {
    ASSERT(GetMCIMessageByteSize(CompiledFormat) == 1);
    FreeMCIFormat(&CompiledFormat);
  }

  BuildNestedFormat(Format, 65, "8");
  CompiledFormat = CompileMCIFormat(Format, 8);
  ASSERT(!CompiledFormat);
  FreeMCIFormat(&CompiledFormat);
}

TEST(CompileMCIFormat_RejectsExcessiveRepetition) {
  MCICompiledFormat *CompiledFormat;

  CompiledFormat = CompileMCIFormat("64R(1)", 1);
  ASSERT(CompiledFormat);
  if (CompiledFormat) {
    ASSERT(GetMCIMessageFieldCount(CompiledFormat) == 64);
    ASSERT(GetMCIMessageByteSize(CompiledFormat) == 8);
    FreeMCIFormat(&CompiledFormat);
  }

  CompiledFormat = CompileMCIFormat("65R(1)", 1);
  ASSERT(!CompiledFormat);
  FreeMCIFormat(&CompiledFormat);

  CompiledFormat = CompileMCIFormat("1000R(1)", 1);
  ASSERT(!CompiledFormat);
  FreeMCIFormat(&CompiledFormat);

  CompiledFormat = CompileMCIFormat("1R(65R(1))", 1);
  ASSERT(!CompiledFormat);
  FreeMCIFormat(&CompiledFormat);
}

TEST(CompileMCIFormat_MaxFieldBitSizeLimits) {
  MCICompiledFormat *CompiledFormat;

  CompiledFormat = CompileMCIFormat("1", SIZE_MAX);
  ASSERT(!CompiledFormat);
  FreeMCIFormat(&CompiledFormat);

  CompiledFormat = CompileMCIFormat("1", sizeof(size_t) * CHAR_BIT);
  ASSERT(!CompiledFormat);
  FreeMCIFormat(&CompiledFormat);

  CompiledFormat = CompileMCIFormat("1", (sizeof(size_t) * CHAR_BIT) - 1);
  ASSERT(!CompiledFormat);
  FreeMCIFormat(&CompiledFormat);
}

TEST(EncodeMCIMessage_InvalidArgumentsPreserveMessage) {
  MCICompiledFormat *CompiledFormat;
  uint8_t            Message[2]  = {0xAA, 0xBB};
  const uint8_t      Expected[2] = {0xAA, 0xBB};

  CompiledFormat = CompileForStructuralTest("3|5", 5);
  if (!CompiledFormat) {
    return;
  }

  ASSERT(!EncodeMCIMessage(NULL, 2, Message, 0, MSB_FIRST, 0));
  ASSERT(!EncodeMCIMessage(CompiledFormat, 2, NULL, 0, MSB_FIRST, 0));
  ASSERT(!EncodeMCIMessage(CompiledFormat, 2, Message, 2, MSB_FIRST, 0));
  ASSERT(
      !EncodeMCIMessage(CompiledFormat, 2, Message, 0, (EncodingLayout)0, 0));
  ASSERT(
      !EncodeMCIMessage(CompiledFormat, 2, Message, 0, (EncodingLayout)3, 0));
  ASSERT(
      !EncodeMCIMessage(CompiledFormat, 2, Message, 0, (EncodingLayout)-1, 0));
  ASSERT(!EncodeMCIMessage(CompiledFormat, 0, Message, 0, MSB_FIRST, 0));
  ASSERT(!EncodeMCIMessage(CompiledFormat, 2, Message, 0, MSB_FIRST, 8));

  ASSERT(memcmp(Message, Expected, sizeof(Message)) == 0);
  FreeMCIFormat(&CompiledFormat);
}

TEST(EncodeMCIMessage_RejectsMalformedCompiledFormat) {
  size_t            BitSize[2]     = {3, 5};
  size_t            BitOffset[2]   = {0, 3};
  uint8_t           Message[2]     = {0xAA, 0xBB};
  const uint8_t     Expected[2]    = {0xAA, 0xBB};
  MCICompiledFormat CompiledFormat = {2, BitSize, BitOffset, 8, 1};

  CompiledFormat.BitSize = NULL;
  ASSERT(!EncodeMCIMessage(&CompiledFormat, 2, Message, 0, MSB_FIRST, 0));

  CompiledFormat.BitSize   = BitSize;
  CompiledFormat.BitOffset = NULL;
  ASSERT(!EncodeMCIMessage(&CompiledFormat, 2, Message, 0, MSB_FIRST, 0));

  CompiledFormat.BitOffset  = BitOffset;
  CompiledFormat.TotalBytes = 0;
  ASSERT(!EncodeMCIMessage(&CompiledFormat, 2, Message, 0, MSB_FIRST, 0));

  CompiledFormat.TotalBytes = 1;
  BitSize[0]                = 0;
  ASSERT(!EncodeMCIMessage(&CompiledFormat, 2, Message, 0, MSB_FIRST, 0));

  BitSize[0]   = 3;
  BitOffset[0] = 9;
  ASSERT(!EncodeMCIMessage(&CompiledFormat, 2, Message, 0, MSB_FIRST, 0));

  BitOffset[0] = 1;
  BitSize[0]   = 8;
  ASSERT(!EncodeMCIMessage(&CompiledFormat, 2, Message, 0, MSB_FIRST, 0));

  BitOffset[0]             = 0;
  BitSize[0]               = SIZE_MAX;
  CompiledFormat.TotalBits = SIZE_MAX;
  ASSERT(!EncodeMCIMessage(&CompiledFormat, 2, Message, 0, MSB_FIRST, 0));

  ASSERT(memcmp(Message, Expected, sizeof(Message)) == 0);
}

TEST(DecodeMCIMessage_InvalidArgumentsPreserveValue) {
  MCICompiledFormat *CompiledFormat;
  const uint8_t      Message[2] = {0xFF, 0xFF};
  size_t             Value      = SIZE_MAX;

  CompiledFormat = CompileForStructuralTest("3|5", 5);
  if (!CompiledFormat) {
    return;
  }

  ASSERT(!DecodeMCIMessage(NULL, 2, Message, 0, MSB_FIRST, &Value));
  ASSERT(!DecodeMCIMessage(CompiledFormat, 2, NULL, 0, MSB_FIRST, &Value));
  ASSERT(!DecodeMCIMessage(CompiledFormat, 2, Message, 0, MSB_FIRST, NULL));
  ASSERT(!DecodeMCIMessage(CompiledFormat, 2, Message, 2, MSB_FIRST, &Value));
  ASSERT(!DecodeMCIMessage(CompiledFormat, 2, Message, 0, (EncodingLayout)0,
                           &Value));
  ASSERT(!DecodeMCIMessage(CompiledFormat, 2, Message, 0, (EncodingLayout)3,
                           &Value));
  ASSERT(!DecodeMCIMessage(CompiledFormat, 2, Message, 0, (EncodingLayout)-1,
                           &Value));
  ASSERT(!DecodeMCIMessage(CompiledFormat, 0, Message, 0, MSB_FIRST, &Value));

  ASSERT(Value == SIZE_MAX);
  FreeMCIFormat(&CompiledFormat);
}

TEST(DecodeMCIMessage_RejectsMalformedCompiledFormat) {
  size_t            BitSize[2]     = {3, 5};
  size_t            BitOffset[2]   = {0, 3};
  const uint8_t     Message[2]     = {0xFF, 0xFF};
  size_t            Value          = SIZE_MAX;
  MCICompiledFormat CompiledFormat = {2, BitSize, BitOffset, 8, 1};

  CompiledFormat.BitSize = NULL;
  ASSERT(!DecodeMCIMessage(&CompiledFormat, 2, Message, 0, MSB_FIRST, &Value));

  CompiledFormat.BitSize   = BitSize;
  CompiledFormat.BitOffset = NULL;
  ASSERT(!DecodeMCIMessage(&CompiledFormat, 2, Message, 0, MSB_FIRST, &Value));

  CompiledFormat.BitOffset  = BitOffset;
  CompiledFormat.TotalBytes = 0;
  ASSERT(!DecodeMCIMessage(&CompiledFormat, 2, Message, 0, MSB_FIRST, &Value));

  CompiledFormat.TotalBytes = 1;
  BitSize[0]                = 0;
  ASSERT(!DecodeMCIMessage(&CompiledFormat, 2, Message, 0, MSB_FIRST, &Value));

  BitSize[0] = (sizeof(size_t) * CHAR_BIT) + 1;
  ASSERT(!DecodeMCIMessage(&CompiledFormat, 2, Message, 0, MSB_FIRST, &Value));

  BitSize[0]   = 3;
  BitOffset[0] = 9;
  ASSERT(!DecodeMCIMessage(&CompiledFormat, 2, Message, 0, MSB_FIRST, &Value));

  BitOffset[0] = 1;
  BitSize[0]   = 8;
  ASSERT(!DecodeMCIMessage(&CompiledFormat, 2, Message, 0, MSB_FIRST, &Value));

  ASSERT(Value == SIZE_MAX);
}

TEST(DecodeMCIMessage_DoesNotModifyMessage) {
  MCICompiledFormat *CompiledFormat;
  uint8_t            Message[2]  = {0xA5, 0x5A};
  const uint8_t      Expected[2] = {0xA5, 0x5A};
  size_t             Value;

  CompiledFormat = CompileForStructuralTest("3|5", 5);
  if (!CompiledFormat) {
    return;
  }

  Value = SIZE_MAX;
  ASSERT(DecodeMCIMessage(CompiledFormat, 2, Message, 0, MSB_FIRST, &Value));
  ASSERT(Value == 5);
  ASSERT(memcmp(Message, Expected, sizeof(Message)) == 0);

  FreeMCIFormat(&CompiledFormat);
}

TEST_SUITE(MCIStructuralContractSuite,
           FreeMCIFormat_BadInput,
           FreeMCIFormat_NominalAndDoubleFree,
           GetMCIMessageByteSize_BadInput,
           GetMCIMessageFieldCount_BadInput,
           CompileMCIFormat_BadInput,
           CompileMCIFormat_PartialFailureCleanup,
           CompileMCIFormat_RejectsZeroFieldsAndCounts,
           CompileMCIFormat_RejectsExcessiveNesting,
           CompileMCIFormat_RejectsExcessiveRepetition,
           EncodeMCIMessage_InvalidArgumentsPreserveMessage,
           EncodeMCIMessage_RejectsMalformedCompiledFormat,
           DecodeMCIMessage_InvalidArgumentsPreserveValue,
           DecodeMCIMessage_RejectsMalformedCompiledFormat,
           DecodeMCIMessage_DoesNotModifyMessage);

// Allocation faults. The simple format "8|3|5" has three Core_Malloc calls
// (expanded format, compiled record and bit-offset array) and one Core_Realloc
// call per field. The offset loop and final byte rounding make the additive
// fault-free call the nineteenth one; ipow and the per-field allocation-size
// multiplication make the multiplicative fault-free call the eighth one.
TEST_F(StructuralCtx,
       CompileMCIFormat_MallocFail) {
  MCICompiledFormat *CompiledFormat = NULL;
  size_t             FailAt;

  (void)ContextData;
  for (FailAt = 1; FailAt <= 8; FailAt++) {
    SetFallibleMalloc(FailAt);
    CompiledFormat = CompileMCIFormat("8|3|5", 8);
    if (CompiledFormat) {
      break;
    }
  }
  ASSERT(FailAt == 4);
  ASSERT(CompiledFormat);
  if (CompiledFormat) {
    ASSERT(GetMCIMessageByteSize(CompiledFormat) == 2);
    FreeMCIFormat(&CompiledFormat);
  }
}

TEST_F(StructuralCtx,
       CompileMCIFormat_ReallocFail) {
  MCICompiledFormat *CompiledFormat = NULL;
  size_t             FailAt;

  (void)ContextData;
  for (FailAt = 1; FailAt <= 8; FailAt++) {
    SetFallibleRealloc(FailAt);
    CompiledFormat = CompileMCIFormat("8|3|5", 8);
    if (CompiledFormat) {
      break;
    }
  }
  ASSERT(FailAt == 4);
  ASSERT(CompiledFormat);
  if (CompiledFormat) {
    ASSERT(GetMCIMessageByteSize(CompiledFormat) == 2);
    FreeMCIFormat(&CompiledFormat);
  }
}

TEST_F(StructuralCtx,
       CompileMCIFormat_AddOverflowFail) {
  MCICompiledFormat *CompiledFormat = NULL;
  size_t             FailAt;

  (void)ContextData;
  for (FailAt = 1; FailAt <= 32; FailAt++) {
    SetFallibleAddOverflow(FailAt);
    CompiledFormat = CompileMCIFormat("8|3|5", 8);
    if (CompiledFormat) {
      break;
    }
  }
  ASSERT(FailAt == 19);
  ASSERT(CompiledFormat);
  if (CompiledFormat) {
    ASSERT(GetMCIMessageByteSize(CompiledFormat) == 2);
    FreeMCIFormat(&CompiledFormat);
  }
}

TEST_F(StructuralCtx,
       CompileMCIFormat_MulOverflowFail) {
  MCICompiledFormat *CompiledFormat = NULL;
  size_t             FailAt;

  (void)ContextData;
  for (FailAt = 1; FailAt <= 16; FailAt++) {
    SetFallibleMulOverflow(FailAt);
    CompiledFormat = CompileMCIFormat("8|3|5", 8);
    if (CompiledFormat) {
      break;
    }
  }
  ASSERT(FailAt == 8);
  ASSERT(CompiledFormat);
  if (CompiledFormat) {
    ASSERT(GetMCIMessageByteSize(CompiledFormat) == 2);
    FreeMCIFormat(&CompiledFormat);
  }
}

TEST_F(StructuralCtx,
       CompileMCIFormat_AllocationFailWithRepetition) {
  MCICompiledFormat *CompiledFormat = NULL;
  size_t             FailAt;

  (void)ContextData;
  for (FailAt = 1; FailAt <= 8; FailAt++) {
    SetFallibleMalloc(FailAt);
    CompiledFormat = CompileMCIFormat("2R(8|3)|5", 8);
    if (CompiledFormat) {
      break;
    }
  }
  ASSERT(FailAt == 4);
  ASSERT(CompiledFormat);
  if (CompiledFormat) {
    ASSERT(GetMCIMessageByteSize(CompiledFormat) == 4);
    FreeMCIFormat(&CompiledFormat);
  }

  ResetFallibleMalloc();
  for (FailAt = 1; FailAt <= 8; FailAt++) {
    SetFallibleRealloc(FailAt);
    CompiledFormat = CompileMCIFormat("2R(8|3)|5", 8);
    if (CompiledFormat) {
      break;
    }
  }
  ASSERT(FailAt == 6);
  ASSERT(CompiledFormat);
  if (CompiledFormat) {
    ASSERT(GetMCIMessageByteSize(CompiledFormat) == 4);
    FreeMCIFormat(&CompiledFormat);
  }
}

TEST_F(StructuralCtx,
       CompileMCIFormat_AddOverflowFailWithRepetition) {
  MCICompiledFormat *CompiledFormat = NULL;
  size_t             FailAt;

  (void)ContextData;
  for (FailAt = 1; FailAt <= 64; FailAt++) {
    SetFallibleAddOverflow(FailAt);
    CompiledFormat = CompileMCIFormat("2R(8|3)|5", 8);
    if (CompiledFormat) {
      break;
    }
  }
  ASSERT(FailAt > 1);
  ASSERT(CompiledFormat);
  if (CompiledFormat) {
    ASSERT(GetMCIMessageByteSize(CompiledFormat) == 4);
    FreeMCIFormat(&CompiledFormat);
  }
}

TEST_F(StructuralCtx,
       CompileMCIFormat_AddOverflowFailBeforeRepetition) {
  MCICompiledFormat *CompiledFormat = NULL;
  size_t             FailAt;

  (void)ContextData;
  for (FailAt = 1; FailAt <= 64; FailAt++) {
    SetFallibleAddOverflow(FailAt);
    CompiledFormat = CompileMCIFormat("8|2R(3)", 8);
    if (CompiledFormat) {
      break;
    }
  }
  ASSERT(FailAt > 1);
  ASSERT(CompiledFormat);
  if (CompiledFormat) {
    ASSERT(GetMCIMessageByteSize(CompiledFormat) == 2);
    FreeMCIFormat(&CompiledFormat);
  }
}

TEST_F(StructuralCtx,
       CompileMCIFormat_MulOverflowFailWithRepetition) {
  MCICompiledFormat *CompiledFormat = NULL;
  size_t             FailAt;

  (void)ContextData;
  for (FailAt = 1; FailAt <= 32; FailAt++) {
    SetFallibleMulOverflow(FailAt);
    CompiledFormat = CompileMCIFormat("2R(8|3)|5", 8);
    if (CompiledFormat) {
      break;
    }
  }
  ASSERT(FailAt > 1);
  ASSERT(CompiledFormat);
  if (CompiledFormat) {
    ASSERT(GetMCIMessageByteSize(CompiledFormat) == 4);
    FreeMCIFormat(&CompiledFormat);
  }
}

TEST_SUITE(MCIStructuralAllocationSuite,
           CompileMCIFormat_MallocFail,
           CompileMCIFormat_ReallocFail,
           CompileMCIFormat_AddOverflowFail,
           CompileMCIFormat_MulOverflowFail,
           CompileMCIFormat_AllocationFailWithRepetition,
           CompileMCIFormat_AddOverflowFailWithRepetition,
           CompileMCIFormat_AddOverflowFailBeforeRepetition,
           CompileMCIFormat_MulOverflowFailWithRepetition);

TEST(CompileMCIFormat_DigitOverflow) {
  ASSERT(CompileMCIFormat("18446744073709551616", 8) == NULL);
}

TEST(CompileMCIFormat_RepetitionOverflow) {
  ASSERT(CompileMCIFormat("18446744073709551615R(8)", 8) == NULL);
  ASSERT(CompileMCIFormat("8R(18446744073709551615)", 8) == NULL);
}

TEST_SUITE(MCIStructuralArithmeticSuite,
           CompileMCIFormat_DigitOverflow,
           CompileMCIFormat_RepetitionOverflow,
           CompileMCIFormat_MaxFieldBitSizeLimits);
