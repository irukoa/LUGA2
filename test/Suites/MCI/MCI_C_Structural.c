#include "src/Core/Core.h"
#include "src/MCI/MessageCodingInterface.h"
#include "test/CoreSupport/CoreSupport.h"
#include "test/TSD/TSD.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// MCICompiledFormat is opaque to callers. These structural tests complete the
// type locally to exercise the defensive validation of malformed records that
// CompileMCIFormat itself cannot produce. Keep this layout in sync with
// src/MCI/MCI_C_Imp.c.
struct MCICompiledFormat {
  char   *FormatCopy;
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

static void BuildSemanticPath(char        *Buffer,
                              const size_t ComponentCount) {
  size_t i;
  size_t Position = 0;

  for (i = 0; i < ComponentCount; i++) {
    if (i > 0) {
      Buffer[Position++] = '.';
    }
    Buffer[Position++] = '1';
  }
  Buffer[Position] = '\0';
}

static void AppendText(char       *Buffer,
                       size_t     *Position,
                       const char *Text) {
  while (*Text) {
    Buffer[(*Position)++] = *Text++;
  }
  Buffer[*Position] = '\0';
}

static void AppendDecimal(char        *Buffer,
                          size_t      *Position,
                          const size_t Value) {
  char   Digits[24];
  size_t Count     = 0;
  size_t Remaining = Value;
  size_t i;

  if (Remaining == 0) {
    Digits[Count++] = '0';
  }
  while (Remaining > 0) {
    Digits[Count++] = (char)('0' + (Remaining % 10));
    Remaining /= 10;
  }
  for (i = Count; i > 0; i--) {
    Buffer[(*Position)++] = Digits[i - 1];
  }
  Buffer[*Position] = '\0';
}

// Appends "<Count>R(" followed by InnerDepth copies of "<InnerCount>R(", a
// single field and the matching closing parentheses.
static void AppendNestedRepeatedFormat(char        *Buffer,
                                       size_t      *Position,
                                       const size_t Count,
                                       const size_t InnerCount,
                                       const size_t InnerDepth) {
  size_t i;

  AppendDecimal(Buffer, Position, Count);
  AppendText(Buffer, Position, "R(");
  for (i = 0; i < InnerDepth; i++) {
    AppendDecimal(Buffer, Position, InnerCount);
    AppendText(Buffer, Position, "R(");
  }
  AppendText(Buffer, Position, "1");
  for (i = 0; i <= InnerDepth; i++) {
    AppendText(Buffer, Position, ")");
  }
}

// Builds a sequence whose expanded field count is exactly SIZE_MAX:
// 2^64 - 1 = 15 * 64^10 + 63 * (64^9 + ... + 1).
static void BuildMaximumFieldCountSequence(char   *Buffer,
                                           size_t *GroupCount) {
  size_t Position = 0;
  size_t k;

  for (k = 0; k < 10; k++) {
    AppendNestedRepeatedFormat(Buffer, &Position, 63, 64, k);
    AppendText(Buffer, &Position, "|");
  }
  AppendNestedRepeatedFormat(Buffer, &Position, 15, 64, 10);
  *GroupCount = (10 * 63) + 15;
}

TEST(FreeMCIFormat_BadInput) {
  MCICompiledFormat *CompiledFormat = NULL;

  FreeMCIFormat(NULL);
  FreeMCIFormat(&CompiledFormat);
  ASSERT(!CompiledFormat);
}

TEST(FreeMCIFormat_NominalAndDoubleFree) {
  MCICompiledFormat *CompiledFormat = NULL;

  CompiledFormat = CompileMCIFormat("8|8", 8);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageBitSize(CompiledFormat) == 16);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 2);
  ASSERT(GetMCIMessageFieldCount(CompiledFormat) == 2);

  FreeMCIFormat(&CompiledFormat);
  ASSERT(!CompiledFormat);

  FreeMCIFormat(&CompiledFormat);
  ASSERT(!CompiledFormat);
}

TEST(GetMCIMessageBitSize_BadInput) { ASSERT(GetMCIMessageBitSize(NULL) == 0); }

TEST(GetMCIMessageByteSize_BadInput) {
  ASSERT(GetMCIMessageByteSize(NULL) == 0);
}

TEST(GetMCIMessageFieldCount_BadInput) {
  ASSERT(GetMCIMessageFieldCount(NULL) == 0);
}

TEST(GetMCIMessageFieldId_MaximumDepth) {
  char               Format[512];
  char               Path[256];
  MCICompiledFormat *CompiledFormat;

  BuildNestedFormat(Format, 64, "1");
  BuildSemanticPath(Path, 65);
  CompiledFormat = CompileMCIFormat(Format, 1);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, Path) == 0);

  BuildSemanticPath(Path, 66);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, Path) == SIZE_MAX);
  FreeMCIFormat(&CompiledFormat);
}

TEST(GetMCIMessageFieldId_BadInput) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("7|8", 8);
  ASSERT_X(CompiledFormat);

  ASSERT(GetMCIMessageFieldId(NULL, "1") == SIZE_MAX);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, NULL) == SIZE_MAX);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "") == SIZE_MAX);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "1") == 0);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "2") == 1);

  FreeMCIFormat(&CompiledFormat);
}

TEST(GetMCIMessageFieldId_RejectsMalformedCompiledFormat) {
  MCICompiledFormat CompiledFormat = {
      .FormatCopy = NULL,
      .NumFields  = 1,
      .BitSize    = NULL,
      .BitOffset  = NULL,
      .TotalBits  = 0,
      .TotalBytes = 0,
  };

  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1") == SIZE_MAX);

  CompiledFormat.FormatCopy = (char *)"1";
  CompiledFormat.NumFields  = 0;
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1") == SIZE_MAX);

  CompiledFormat.NumFields = 1;
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1") == 0);

  CompiledFormat.FormatCopy = (char *)"1x";
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1") == SIZE_MAX);

  CompiledFormat.FormatCopy = (char *)"1R";
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1") == SIZE_MAX);

  CompiledFormat.FormatCopy = (char *)"0R(1)";
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1") == SIZE_MAX);

  CompiledFormat.FormatCopy = (char *)"1R(";
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1") == SIZE_MAX);

  CompiledFormat.FormatCopy = (char *)"1R(1";
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1") == SIZE_MAX);

  CompiledFormat.FormatCopy = (char *)"1R(1)2";
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "2.1") == SIZE_MAX);

  CompiledFormat.FormatCopy = (char *)"1|2x";
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "2") == SIZE_MAX);

  CompiledFormat.FormatCopy = (char *)"1R(1)|";
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "2.1") == SIZE_MAX);

  CompiledFormat.FormatCopy = (char *)"1R(2R)";
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1.1") == SIZE_MAX);

  CompiledFormat.FormatCopy = (char *)"1R(0R(1))";
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1.1") == SIZE_MAX);

  CompiledFormat.FormatCopy = (char *)"1R(2R(3";
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1.1") == SIZE_MAX);

  CompiledFormat.FormatCopy = (char *)"1R(2x)";
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1.1") == SIZE_MAX);

  CompiledFormat.FormatCopy = (char *)"1R(2R(3)4)";
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1.1") == SIZE_MAX);

  {
    char Format[512];
    BuildNestedFormat(Format, 65, "1");
    CompiledFormat.FormatCopy = Format;
    ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1.1") == SIZE_MAX);
  }
}

TEST(GetMCIMessageFieldId_RejectsOverflowingFormats) {
  char              A63[512];
  char              A12[256];
  char              Maximum[4096];
  char              Format[8192];
  char              Path[16];
  size_t            GroupCount;
  size_t            PathLength     = 0;
  size_t            Position       = 0;
  MCICompiledFormat CompiledFormat = {
      .FormatCopy = NULL,
      .NumFields  = 1,
      .BitSize    = NULL,
      .BitOffset  = NULL,
      .TotalBits  = 0,
      .TotalBytes = 0,
  };

  // A twelve-level repetition of count 64 overflows while measuring.
  AppendNestedRepeatedFormat(A12, &Position, 64, 64, 12);
  CompiledFormat.FormatCopy = A12;
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1.1") == SIZE_MAX);

  Position = 0;
  AppendNestedRepeatedFormat(A63, &Position, 2, 2, 62);

  // Two 2^63-field groups overflow when their counts are added.
  Position = 0;
  AppendText(Format, &Position, "1R(");
  AppendText(Format, &Position, A63);
  AppendText(Format, &Position, "|");
  AppendText(Format, &Position, A63);
  AppendText(Format, &Position, ")");
  CompiledFormat.FormatCopy = Format;
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1.1") == SIZE_MAX);

  // A maximum-count sequence followed by a plain field overflows the plain
  // field count.
  BuildMaximumFieldCountSequence(Maximum, &GroupCount);
  Position = 0;
  AppendText(Format, &Position, "1R(");
  AppendText(Format, &Position, Maximum);
  AppendText(Format, &Position, "|1)");
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "1.1") == SIZE_MAX);

  // A selected repetition copy overflows the base plus offset sum.
  Position = 0;
  AppendText(Format, &Position, A63);
  AppendText(Format, &Position, "|2R(");
  AppendText(Format, &Position, A63);
  AppendText(Format, &Position, "|2R(1))");
  CompiledFormat.FormatCopy = Format;
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "3.3.1") == SIZE_MAX);

  // A selected repetition copy overflows the candidate plus copy offset sum.
  Position = 0;
  AppendText(Format, &Position, A63);
  AppendText(Format, &Position, "|2R(");
  AppendText(Format, &Position, A63);
  AppendText(Format, &Position, ")");
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "4.1") == SIZE_MAX);

  // Skipping repetition copies overflows the accumulated copy offset.
  Position = 0;
  AppendText(Format, &Position, "2R(");
  AppendText(Format, &Position, A63);
  AppendText(Format, &Position, ")");
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "3") == SIZE_MAX);

  // Skipping a repetition overflows the accumulated flat offset.
  Position = 0;
  AppendText(Format, &Position, A63);
  AppendText(Format, &Position, "|1R(");
  AppendText(Format, &Position, A63);
  AppendText(Format, &Position, ")");
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "4") == SIZE_MAX);

  // Selecting a plain field overflows the base plus offset sum.
  Position = 0;
  AppendText(Format, &Position, A63);
  AppendText(Format, &Position, "|2R(");
  AppendText(Format, &Position, A63);
  AppendText(Format, &Position, "|1)");
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, "3.3") == SIZE_MAX);

  // Skipping a plain field overflows when the offset is already SIZE_MAX.
  Position = 0;
  AppendText(Format, &Position, Maximum);
  AppendText(Format, &Position, "|1");
  CompiledFormat.FormatCopy = Format;
  AppendDecimal(Path, &PathLength, GroupCount + 2);
  ASSERT(GetMCIMessageFieldId(&CompiledFormat, Path) == SIZE_MAX);
}

TEST(GetMCIMessageSemanticId_BadInput) {
  MCICompiledFormat Empty = {
      .FormatCopy = NULL,
      .NumFields  = 1,
      .BitSize    = NULL,
      .BitOffset  = NULL,
      .TotalBits  = 0,
      .TotalBytes = 0,
  };
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("7|8", 8);

  ASSERT(GetMCIMessageSemanticId(NULL, 0) == NULL);
  ASSERT(GetMCIMessageSemanticId(&Empty, 0) == NULL);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageSemanticId(CompiledFormat, 2) == NULL);
  ASSERT(GetMCIMessageSemanticId(CompiledFormat, SIZE_MAX) == NULL);
  FreeMCIFormat(&CompiledFormat);
}

TEST(GetMCIMessageSemanticId_RejectsMalformedCompiledFormat) {
  static const char *const Cases[] = {
      "",         "1x",     "1R",         "1R(",    "1R(1",
      "1R(1))",   "1R(1)2", "1|2x",       "1R(1)|", "1R(2R)",
      "1R(2R(3)", "1R(2x)", "1R(2R(3)4)", "0R(1)",  "1R(0R(1))",
  };
  MCICompiledFormat CompiledFormat = {
      .FormatCopy = NULL,
      .NumFields  = 1,
      .BitSize    = NULL,
      .BitOffset  = NULL,
      .TotalBits  = 0,
      .TotalBytes = 0,
  };
  char  *SemanticId;
  size_t i;

  for (i = 0; i < (sizeof(Cases) / sizeof(Cases[0])); i++) {
    CompiledFormat.FormatCopy = (char *)Cases[i];
    ASSERT(GetMCIMessageSemanticId(&CompiledFormat, 0) == NULL);
  }

  CompiledFormat.FormatCopy = (char *)"1";
  SemanticId                = GetMCIMessageSemanticId(&CompiledFormat, 0);
  ASSERT_X(SemanticId);
  ASSERT(strcmp(SemanticId, "1") == 0);
  free(SemanticId);

  CompiledFormat.NumFields = 2;
  ASSERT(GetMCIMessageSemanticId(&CompiledFormat, 0) == NULL);

  CompiledFormat.NumFields = 0;
  ASSERT(GetMCIMessageSemanticId(&CompiledFormat, 0) == NULL);
}

TEST(GetMCIMessageSemanticId_MaximumDepth) {
  char               Format[512];
  char               Path[256];
  MCICompiledFormat *CompiledFormat;
  MCICompiledFormat  Malformed = {
      .FormatCopy = NULL,
      .NumFields  = 1,
      .BitSize    = NULL,
      .BitOffset  = NULL,
      .TotalBits  = 0,
      .TotalBytes = 0,
  };
  char *SemanticId;

  BuildNestedFormat(Format, 64, "1");
  BuildSemanticPath(Path, 65);
  CompiledFormat = CompileMCIFormat(Format, 1);
  ASSERT_X(CompiledFormat);
  SemanticId = GetMCIMessageSemanticId(CompiledFormat, 0);
  ASSERT_X(SemanticId);
  ASSERT(strcmp(SemanticId, Path) == 0);
  free(SemanticId);
  FreeMCIFormat(&CompiledFormat);

  BuildNestedFormat(Format, 65, "1");
  Malformed.FormatCopy = Format;
  ASSERT(GetMCIMessageSemanticId(&Malformed, 0) == NULL);
}

TEST(GetMCIMessageSemanticId_RejectsOverflowingFormats) {
  char              A63[512];
  char              A12[256];
  char              Maximum[4096];
  char              Format[8192];
  size_t            GroupCount;
  size_t            Position       = 0;
  MCICompiledFormat CompiledFormat = {
      .FormatCopy = NULL,
      .NumFields  = SIZE_MAX,
      .BitSize    = NULL,
      .BitOffset  = NULL,
      .TotalBits  = 0,
      .TotalBytes = 0,
  };

  // A twelve-level repetition of count 64 overflows while measuring.
  AppendNestedRepeatedFormat(A12, &Position, 64, 64, 12);
  Position = 0;
  AppendText(Format, &Position, A12);
  CompiledFormat.FormatCopy = Format;
  ASSERT(GetMCIMessageSemanticId(&CompiledFormat, 0) == NULL);

  // Multiplying a 2^63-field sequence by two overflows the group size.
  Position = 0;
  AppendNestedRepeatedFormat(A63, &Position, 2, 2, 62);
  Position = 0;
  AppendText(Format, &Position, "2R(");
  AppendText(Format, &Position, A63);
  AppendText(Format, &Position, ")");
  CompiledFormat.FormatCopy = Format;
  ASSERT(GetMCIMessageSemanticId(&CompiledFormat, 0) == NULL);

  // A SIZE_MAX-field sequence followed by one more field overflows the sum.
  BuildMaximumFieldCountSequence(Maximum, &GroupCount);
  Position = 0;
  AppendText(Format, &Position, Maximum);
  AppendText(Format, &Position, "|1");
  CompiledFormat.FormatCopy = Format;
  ASSERT(GetMCIMessageSemanticId(&CompiledFormat, 0) == NULL);
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
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 1);
  FreeMCIFormat(&CompiledFormat);

  BuildNestedFormat(Format, 65, "8");
  CompiledFormat = CompileMCIFormat(Format, 8);
  ASSERT(!CompiledFormat);
  FreeMCIFormat(&CompiledFormat);
}

TEST(CompileMCIFormat_RejectsExcessiveRepetition) {
  MCICompiledFormat *CompiledFormat;

  CompiledFormat = CompileMCIFormat("64R(1)", 1);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageFieldCount(CompiledFormat) == 64);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 8);
  FreeMCIFormat(&CompiledFormat);

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

  CompiledFormat = CompileMCIFormat("3|5", 5);
  ASSERT_X(CompiledFormat);

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
  MCICompiledFormat CompiledFormat = {
      .FormatCopy = NULL,
      .NumFields  = 2,
      .BitSize    = BitSize,
      .BitOffset  = BitOffset,
      .TotalBits  = 8,
      .TotalBytes = 1,
  };

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

  CompiledFormat = CompileMCIFormat("3|5", 5);
  ASSERT_X(CompiledFormat);

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
  MCICompiledFormat CompiledFormat = {
      .FormatCopy = NULL,
      .NumFields  = 2,
      .BitSize    = BitSize,
      .BitOffset  = BitOffset,
      .TotalBits  = 8,
      .TotalBytes = 1,
  };

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

  CompiledFormat = CompileMCIFormat("3|5", 5);
  ASSERT_X(CompiledFormat);

  Value = SIZE_MAX;
  ASSERT(DecodeMCIMessage(CompiledFormat, 2, Message, 0, MSB_FIRST, &Value));
  ASSERT(Value == 5);
  ASSERT(memcmp(Message, Expected, sizeof(Message)) == 0);

  FreeMCIFormat(&CompiledFormat);
}

TEST_SUITE(MCIStructuralContractSuite,
           FreeMCIFormat_BadInput,
           FreeMCIFormat_NominalAndDoubleFree,
           GetMCIMessageBitSize_BadInput,
           GetMCIMessageByteSize_BadInput,
           GetMCIMessageFieldCount_BadInput,
           GetMCIMessageFieldId_MaximumDepth,
           GetMCIMessageFieldId_BadInput,
           GetMCIMessageFieldId_RejectsMalformedCompiledFormat,
           GetMCIMessageFieldId_RejectsOverflowingFormats,
           GetMCIMessageSemanticId_BadInput,
           GetMCIMessageSemanticId_RejectsMalformedCompiledFormat,
           GetMCIMessageSemanticId_MaximumDepth,
           GetMCIMessageSemanticId_RejectsOverflowingFormats,
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

// Allocation faults. The simple format "8|3|5" has four Core_Malloc calls
// (expanded format, compiled record, bit-offset array and retained format
// string) and one Core_Realloc call per field. The offset loop, final byte
// rounding and retained-format length calculation make the additive fault-free
// call the twentieth one; ipow and the per-field allocation-size multiplication
// make the multiplicative fault-free call the ninth one.
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
  ASSERT(FailAt == 5);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 2);
  FreeMCIFormat(&CompiledFormat);
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
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 2);
  FreeMCIFormat(&CompiledFormat);
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
  ASSERT(FailAt == 20);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 2);
  FreeMCIFormat(&CompiledFormat);
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
  ASSERT(FailAt == 9);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 2);
  FreeMCIFormat(&CompiledFormat);
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
  ASSERT(FailAt == 5);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 4);
  FreeMCIFormat(&CompiledFormat);

  ResetFallibleMalloc();
  for (FailAt = 1; FailAt <= 8; FailAt++) {
    SetFallibleRealloc(FailAt);
    CompiledFormat = CompileMCIFormat("2R(8|3)|5", 8);
    if (CompiledFormat) {
      break;
    }
  }
  ASSERT(FailAt == 6);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 4);
  FreeMCIFormat(&CompiledFormat);
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
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 4);
  FreeMCIFormat(&CompiledFormat);
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
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 2);
  FreeMCIFormat(&CompiledFormat);
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
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 4);
  FreeMCIFormat(&CompiledFormat);
}

TEST_F(StructuralCtx,
       GetMCIMessageSemanticId_MallocFail) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("7|2R(3|5)", 7);
  char              *SemanticId;

  (void)ContextData;
  ASSERT_X(CompiledFormat);

  SetFallibleMalloc(1);
  ASSERT(GetMCIMessageSemanticId(CompiledFormat, 0) == NULL);

  SemanticId = GetMCIMessageSemanticId(CompiledFormat, 0);
  ASSERT_X(SemanticId);
  ASSERT(strcmp(SemanticId, "1") == 0);
  free(SemanticId);

  FreeMCIFormat(&CompiledFormat);
}

TEST_F(StructuralCtx,
       GetMCIMessageSemanticId_OverflowFail) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("1R(1)", 1);
  char              *SemanticId;
  size_t             FailAt;

  (void)ContextData;
  ASSERT_X(CompiledFormat);

  // Fail every additive overflow check in turn. The successful call only
  // occurs after the validation, path construction, length calculation and
  // allocation-size checks have all completed.
  for (FailAt = 1; FailAt <= 32; FailAt++) {
    SetFallibleAddOverflow(FailAt);
    SemanticId = GetMCIMessageSemanticId(CompiledFormat, 0);
    if (SemanticId) {
      ASSERT(strcmp(SemanticId, "1.1") == 0);
      free(SemanticId);
      break;
    }
  }
  ASSERT(FailAt > 1);
  ASSERT(FailAt <= 32);
  ResetFallibleAddOverflow();

  // The same applies to the two multiplicative checks: the repetition group
  // size calculation and the final byte-size multiplication.
  for (FailAt = 1; FailAt <= 16; FailAt++) {
    SetFallibleMulOverflow(FailAt);
    SemanticId = GetMCIMessageSemanticId(CompiledFormat, 0);
    if (SemanticId) {
      ASSERT(strcmp(SemanticId, "1.1") == 0);
      free(SemanticId);
      break;
    }
  }
  ASSERT(FailAt > 1);
  ASSERT(FailAt <= 16);
  ResetFallibleMulOverflow();

  FreeMCIFormat(&CompiledFormat);
}

TEST_F(StructuralCtx,
       GetMCIMessageSemanticId_SkippedGroupOverflowFail) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("1R(1)|1", 1);
  char              *SemanticId;
  size_t             FailAt;

  (void)ContextData;
  ASSERT_X(CompiledFormat);

  // Skipping the repetition advances GroupIndex. The fault-free call is the
  // tenth additive check after validation and path construction.
  for (FailAt = 1; FailAt <= 10; FailAt++) {
    SetFallibleAddOverflow(FailAt);
    SemanticId = GetMCIMessageSemanticId(CompiledFormat, 1);
    if (SemanticId) {
      ASSERT(strcmp(SemanticId, "2") == 0);
      free(SemanticId);
      break;
    }
  }
  ASSERT(FailAt == 10);
  ResetFallibleAddOverflow();

  FreeMCIFormat(&CompiledFormat);
}

TEST_F(StructuralCtx,
       GetMCIMessageSemanticId_SkippedFieldOverflowFail) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("1|1", 1);
  char              *SemanticId;
  size_t             FailAt;

  (void)ContextData;
  ASSERT_X(CompiledFormat);

  // Skipping a plain field advances GroupIndex. The fault-free call is the
  // sixth additive check after validation and path construction.
  for (FailAt = 1; FailAt <= 6; FailAt++) {
    SetFallibleAddOverflow(FailAt);
    SemanticId = GetMCIMessageSemanticId(CompiledFormat, 1);
    if (SemanticId) {
      ASSERT(strcmp(SemanticId, "2") == 0);
      free(SemanticId);
      break;
    }
  }
  ASSERT(FailAt == 6);
  ResetFallibleAddOverflow();

  FreeMCIFormat(&CompiledFormat);
}

TEST_SUITE(MCIStructuralAllocationSuite,
           CompileMCIFormat_MallocFail,
           CompileMCIFormat_ReallocFail,
           CompileMCIFormat_AddOverflowFail,
           CompileMCIFormat_MulOverflowFail,
           CompileMCIFormat_AllocationFailWithRepetition,
           CompileMCIFormat_AddOverflowFailWithRepetition,
           CompileMCIFormat_AddOverflowFailBeforeRepetition,
           CompileMCIFormat_MulOverflowFailWithRepetition,
           GetMCIMessageSemanticId_MallocFail,
           GetMCIMessageSemanticId_OverflowFail,
           GetMCIMessageSemanticId_SkippedGroupOverflowFail,
           GetMCIMessageSemanticId_SkippedFieldOverflowFail);

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
