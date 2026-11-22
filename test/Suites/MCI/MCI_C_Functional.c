#include "src/MCI/MessageCodingInterface.h"
#include "test/TSD/TSD.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

static void ExpectBytes(const char          *Format,
                        const size_t         MaxFieldBitSize,
                        const EncodingLayout Encoding,
                        const size_t         Values[],
                        const size_t         NumFields,
                        const uint8_t        Expected[],
                        const size_t         ExpectedSize) {
  MCICompiledFormat *CompiledFormat;
  uint8_t            Message[32] = {0};
  size_t             i;

  CompiledFormat = CompileMCIFormat(Format, MaxFieldBitSize);
  ASSERT_X(CompiledFormat);
  ASSERT_X(GetMCIMessageByteSize(CompiledFormat) == ExpectedSize);
  ASSERT_X(ExpectedSize <= sizeof(Message));
  for (i = 0; i < NumFields; i++) {
    ASSERT(EncodeMCIMessage(CompiledFormat, ExpectedSize, Message, i, Encoding,
                            Values[i]));
  }
  ASSERT(memcmp(Message, Expected, ExpectedSize) == 0);
  FreeMCIFormat(&CompiledFormat);
}

static void ExpectDecoded(const char          *Format,
                          const size_t         MaxFieldBitSize,
                          const EncodingLayout Encoding,
                          const uint8_t        Message[],
                          const size_t         MessageSize,
                          const size_t         ExpectedValues[],
                          const size_t         NumFields) {
  MCICompiledFormat *CompiledFormat;
  size_t             Value;
  size_t             i;

  CompiledFormat = CompileMCIFormat(Format, MaxFieldBitSize);
  ASSERT_X(CompiledFormat);
  ASSERT_X(GetMCIMessageByteSize(CompiledFormat) == MessageSize);
  for (i = 0; i < NumFields; i++) {
    Value = SIZE_MAX;
    ASSERT(DecodeMCIMessage(CompiledFormat, MessageSize, Message, i, Encoding,
                            &Value));
    ASSERT(Value == ExpectedValues[i]);
  }
  FreeMCIFormat(&CompiledFormat);
}

static void ExpectDecodeFailure(const MCICompiledFormat *CompiledFormat,
                                const size_t             MessageByteSize,
                                const uint8_t            Message[],
                                const size_t             FieldId,
                                const EncodingLayout     Encoding) {
  size_t Value = SIZE_MAX;

  ASSERT(!DecodeMCIMessage(CompiledFormat, MessageByteSize, Message, FieldId,
                           Encoding, &Value));
  ASSERT(Value == SIZE_MAX);
}

static void ExpectRoundTrip(const char          *Format,
                            const size_t         MaxFieldBitSize,
                            const EncodingLayout Encoding,
                            const size_t         Values[],
                            const size_t         NumFields) {
  MCICompiledFormat *CompiledFormat;
  uint8_t            Message[32] = {0};
  size_t             MessageSize;
  size_t             Value;
  size_t             i;

  CompiledFormat = CompileMCIFormat(Format, MaxFieldBitSize);
  ASSERT_X(CompiledFormat);
  MessageSize = GetMCIMessageByteSize(CompiledFormat);
  ASSERT_X(MessageSize > 0);
  ASSERT_X(MessageSize <= sizeof(Message));
  for (i = 0; i < NumFields; i++) {
    ASSERT(EncodeMCIMessage(CompiledFormat, MessageSize, Message, i, Encoding,
                            Values[i]));
  }
  for (i = 0; i < NumFields; i++) {
    Value = SIZE_MAX;
    ASSERT(DecodeMCIMessage(CompiledFormat, MessageSize, Message, i, Encoding,
                            &Value));
    ASSERT(Value == Values[i]);
  }
  FreeMCIFormat(&CompiledFormat);
}

TEST(MCICompile_ValidSizes) {
  static const struct {
    const char *Format;
    size_t      MaxFieldBitSize;
    size_t      BitSize;
    size_t      ByteSize;
  } Cases[] = {
      {"1", 1, 1, 1},
      {"7", 7, 7, 1},
      {"8", 8, 8, 1},
      {"9", 9, 9, 2},
      {"16", 16, 16, 2},
      {"17", 17, 17, 3},
      {"8|16|3", 16, 27, 4},
      {"1R(8)", 8, 8, 1},
      {"2R(4)", 4, 8, 1},
      {"2R(1|2)", 2, 6, 1},
      {"2R(2|3)", 3, 10, 2},
      {"2R(2R(3))", 3, 12, 2},
      {"2R(2R(3|4))", 4, 28, 4},
      {"8|2R(2|3)", 8, 18, 3},
      {"0008|0004", 8, 12, 2},
      {"01", 1, 1, 1},
      {"64R(1)", 1, 64, 8},
  };
  MCICompiledFormat *CompiledFormat;
  size_t             i;

  for (i = 0; i < (sizeof(Cases) / sizeof(Cases[0])); i++) {
    CompiledFormat =
        CompileMCIFormat(Cases[i].Format, Cases[i].MaxFieldBitSize);
    ASSERT_X(CompiledFormat);
    ASSERT(GetMCIMessageBitSize(CompiledFormat) == Cases[i].BitSize);
    ASSERT(GetMCIMessageByteSize(CompiledFormat) == Cases[i].ByteSize);
    FreeMCIFormat(&CompiledFormat);
  }
}

TEST(MCICompile_InvalidFormats) {
  static const char *const Cases[] = {
      NULL,        "",          "0",      "00",       "1|",         "|1",
      "1||2",      "R(1)",      "1R()",   "1R(0)",    "1R(2",       "1R(2))",
      "1r(2)",     " 1",        "1 ",     "1| 2",     "1R(2|)",     "1R(|2)",
      "1R(2)R(3)", "1|0R(2|3)", "1 R(2)", "1R (2)",   "1.0",        "0x10",
      "+1",        "-1",        "65R(1)", "1000R(1)", "1R(65R(1))",
  };
  MCICompiledFormat *CompiledFormat;
  size_t             i;

  for (i = 0; i < (sizeof(Cases) / sizeof(Cases[0])); i++) {
    CompiledFormat = CompileMCIFormat(Cases[i], 8);
    ASSERT(CompiledFormat == NULL);
    FreeMCIFormat(&CompiledFormat);
  }
}

TEST(MCICompile_FieldLimits) {
  MCICompiledFormat *CompiledFormat;

  CompiledFormat = CompileMCIFormat("1", 0);
  ASSERT(CompiledFormat == NULL);
  FreeMCIFormat(&CompiledFormat);

  CompiledFormat = CompileMCIFormat("2", 1);
  ASSERT(CompiledFormat == NULL);
  FreeMCIFormat(&CompiledFormat);

  CompiledFormat = CompileMCIFormat("1", 1);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageBitSize(CompiledFormat) == 1);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 1);
  FreeMCIFormat(&CompiledFormat);

  CompiledFormat = CompileMCIFormat("8", 8);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageBitSize(CompiledFormat) == 8);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 1);
  FreeMCIFormat(&CompiledFormat);

  CompiledFormat = CompileMCIFormat("9", 8);
  ASSERT(CompiledFormat == NULL);
  FreeMCIFormat(&CompiledFormat);

  CompiledFormat = CompileMCIFormat("8|9", 8);
  ASSERT(CompiledFormat == NULL);
  FreeMCIFormat(&CompiledFormat);
}

TEST(MCICompile_OverflowRejected) {
  static const char *const Cases[] = {
      "18446744073709551616",
      "1R(18446744073709551616)",
      "1R(18446744073709551615)",
  };
  size_t i;

  for (i = 0; i < (sizeof(Cases) / sizeof(Cases[0])); i++) {
    ASSERT(CompileMCIFormat(Cases[i], 8) == NULL);
  }
}

TEST(MCICompile_FieldCount) {
  MCICompiledFormat *CompiledFormat;

  ASSERT(GetMCIMessageFieldCount(NULL) == 0);

  CompiledFormat = CompileMCIFormat("8|2R(2|3)", 8);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageFieldCount(CompiledFormat) == 5);
  ASSERT(GetMCIMessageBitSize(CompiledFormat) == 18);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 3);
  FreeMCIFormat(&CompiledFormat);

  CompiledFormat = CompileMCIFormat("64R(1)", 1);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageFieldCount(CompiledFormat) == 64);
  ASSERT(GetMCIMessageBitSize(CompiledFormat) == 64);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 8);
  FreeMCIFormat(&CompiledFormat);
}

TEST(MCICompile_ExpandedFieldLimit) {
  static const char *const OverLimitFormats[] = {
      "64R(16R(1))|1", "64R(16R(1)|1)",    "64R(17R(1))",
      "64R(64R(1))",   "64R(64R(64R(1)))", "64R(16R(1))|64R(16R(1))",
  };
  MCICompiledFormat *CompiledFormat;
  size_t             i;

  // The limit is inclusive: 1024 expanded fields are accepted.
  CompiledFormat = CompileMCIFormat("64R(16R(1))", 1);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageFieldCount(CompiledFormat) == 1024);
  ASSERT(GetMCIMessageBitSize(CompiledFormat) == 1024);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 128);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "64.16.1") == 1023);
  FreeMCIFormat(&CompiledFormat);

  for (i = 0; i < (sizeof(OverLimitFormats) / sizeof(OverLimitFormats[0]));
       i++) {
    CompiledFormat = CompileMCIFormat(OverLimitFormats[i], 1);
    ASSERT(CompiledFormat == NULL);
    FreeMCIFormat(&CompiledFormat);
  }
}

TEST(MCICompile_AccessorsAndFree) {
  MCICompiledFormat *CompiledFormat = NULL;

  ASSERT(GetMCIMessageBitSize(NULL) == 0);
  ASSERT(GetMCIMessageByteSize(NULL) == 0);
  FreeMCIFormat(NULL);
  FreeMCIFormat(&CompiledFormat);
  ASSERT(CompiledFormat == NULL);

  CompiledFormat = CompileMCIFormat("8", 8);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageBitSize(CompiledFormat) == 8);
  ASSERT(GetMCIMessageByteSize(CompiledFormat) == 1);
  FreeMCIFormat(&CompiledFormat);
  ASSERT(CompiledFormat == NULL);
  FreeMCIFormat(&CompiledFormat);
  ASSERT(CompiledFormat == NULL);
}

TEST(MCISemanticFieldId_RepeatedGroups) {
  static const struct {
    const char *SemanticId;
    size_t      FieldId;
  } Cases[] = {
      {"1", 0}, {"2.1", 1}, {"2.2", 2}, {"3.1", 3}, {"3.2", 4},
  };
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("7|2R(3|5)", 7);
  size_t             i;
  ASSERT_X(CompiledFormat);

  for (i = 0; i < (sizeof(Cases) / sizeof(Cases[0])); i++) {
    ASSERT(GetMCIMessageFieldId(CompiledFormat, Cases[i].SemanticId) ==
           Cases[i].FieldId);
  }

  ASSERT(GetMCIMessageFieldId(CompiledFormat, "1.1") == SIZE_MAX);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "2") == SIZE_MAX);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "2.3") == SIZE_MAX);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "2.1.1") == SIZE_MAX);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "4.1") == SIZE_MAX);

  FreeMCIFormat(&CompiledFormat);
}

TEST(MCISemanticFieldId_TopLevelAndSingleFieldGroups) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("7|8", 8);
  ASSERT_X(CompiledFormat);

  ASSERT(GetMCIMessageFieldId(CompiledFormat, "1") == 0);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "2") == 1);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "1.1") == SIZE_MAX);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "3") == SIZE_MAX);
  FreeMCIFormat(&CompiledFormat);

  CompiledFormat = CompileMCIFormat("2R(7)", 7);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "1.1") == 0);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "2.1") == 1);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "1") == SIZE_MAX);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "3.1") == SIZE_MAX);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "1.2") == SIZE_MAX);
  FreeMCIFormat(&CompiledFormat);
}

TEST(MCISemanticFieldId_NestedAndTrailingGroups) {
  static const struct {
    const char *SemanticId;
    size_t      FieldId;
  } NestedCases[] = {
      {"1.1.1", 0}, {"1.1.2", 1}, {"1.2.1", 2},  {"1.2.2", 3},
      {"1.3.1", 4}, {"1.3.2", 5}, {"2.1.1", 6},  {"2.1.2", 7},
      {"2.2.1", 8}, {"2.2.2", 9}, {"2.3.1", 10}, {"2.3.2", 11},
  };
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("2R(3R(4|5))", 5);
  size_t             i;
  ASSERT_X(CompiledFormat);

  for (i = 0; i < (sizeof(NestedCases) / sizeof(NestedCases[0])); i++) {
    ASSERT(GetMCIMessageFieldId(CompiledFormat, NestedCases[i].SemanticId) ==
           NestedCases[i].FieldId);
  }
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "1.1") == SIZE_MAX);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "3.1.1") == SIZE_MAX);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "1.4.1") == SIZE_MAX);
  FreeMCIFormat(&CompiledFormat);

  CompiledFormat = CompileMCIFormat("2R(3|5)|9", 9);
  ASSERT_X(CompiledFormat);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "1.1") == 0);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "1.2") == 1);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "2.1") == 2);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "2.2") == 3);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "3") == 4);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "4") == SIZE_MAX);
  FreeMCIFormat(&CompiledFormat);
}

TEST(MCISemanticFieldId_InvalidIdentifiers) {
  static const char *const Cases[] = {
      "",
      "0",
      "01",
      "1.",
      ".1",
      "1..2",
      "1.0",
      "1x",
      " 1",
      "+1",
      "-1",
      "1 2",
      "18446744073709551616",
      "99999999999999999999",
  };
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("7|2R(3|5)", 7);
  size_t             i;
  ASSERT_X(CompiledFormat);

  for (i = 0; i < (sizeof(Cases) / sizeof(Cases[0])); i++) {
    ASSERT(GetMCIMessageFieldId(CompiledFormat, Cases[i]) == SIZE_MAX);
  }
  FreeMCIFormat(&CompiledFormat);
}

TEST(MCISemanticFieldId_OwnsFormatCopy) {
  char               Format[]       = "7|2R(3|5)";
  MCICompiledFormat *CompiledFormat = CompileMCIFormat(Format, 7);
  ASSERT_X(CompiledFormat);

  memset(Format, '\0', sizeof(Format));
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "2.2") == 2);
  ASSERT(GetMCIMessageFieldId(CompiledFormat, "3.2") == 4);
  FreeMCIFormat(&CompiledFormat);
}

TEST(MCISemanticFieldId_IntegratesWithCodec) {
  static const struct {
    const char *SemanticId;
    size_t      Value;
  } Cases[] = {
      {"1", 0x55}, {"2.1", 2}, {"2.2", 0x11}, {"3.1", 5}, {"3.2", 0x1E},
  };
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("7|2R(3|5)", 7);
  uint8_t            Message[3]     = {0};
  size_t             FieldId;
  size_t             Decoded;
  size_t             i;
  ASSERT_X(CompiledFormat);

  for (i = 0; i < (sizeof(Cases) / sizeof(Cases[0])); i++) {
    FieldId = GetMCIMessageFieldId(CompiledFormat, Cases[i].SemanticId);
    ASSERT(FieldId < GetMCIMessageFieldCount(CompiledFormat));
    ASSERT(EncodeMCIMessage(CompiledFormat, sizeof(Message), Message, FieldId,
                            MSB_FIRST, Cases[i].Value));
    Decoded = SIZE_MAX;
    ASSERT(DecodeMCIMessage(CompiledFormat, sizeof(Message), Message, FieldId,
                            MSB_FIRST, &Decoded));
    ASSERT(Decoded == Cases[i].Value);
  }
  FreeMCIFormat(&CompiledFormat);
}

TEST_SUITE(MCICompileFunctionalSuite,
           MCICompile_ValidSizes,
           MCICompile_InvalidFormats,
           MCICompile_FieldLimits,
           MCICompile_OverflowRejected,
           MCICompile_FieldCount,
           MCICompile_ExpandedFieldLimit,
           MCICompile_AccessorsAndFree,
           MCISemanticFieldId_RepeatedGroups,
           MCISemanticFieldId_TopLevelAndSingleFieldGroups,
           MCISemanticFieldId_NestedAndTrailingGroups,
           MCISemanticFieldId_InvalidIdentifiers,
           MCISemanticFieldId_OwnsFormatCopy,
           MCISemanticFieldId_IntegratesWithCodec);

TEST(MCIEncode_MSB_FIRST) {
  static const size_t  One[]           = {1};
  static const uint8_t OneMSB[]        = {0x80};
  static const size_t  Nibbles[]       = {0xA, 0x5};
  static const uint8_t NibblesMSB[]    = {0xA5};
  static const size_t  Mixed[]         = {5, 13};
  static const uint8_t MixedMSB[]      = {0xAD};
  static const size_t  Word[]          = {0x1234};
  static const uint8_t WordMSB[]       = {0x12, 0x34};
  static const size_t  TwelveFour[]    = {0xABC, 0xD};
  static const uint8_t TwelveFourMSB[] = {0xAB, 0xCD};
  static const size_t  Padded[]        = {0xAB, 0xC};
  static const uint8_t PaddedMSB[]     = {0xAB, 0xC0};
  static const size_t  Repeated[]      = {0xAB, 1, 2, 3, 4};
  static const uint8_t RepeatedMSB[]   = {0xAB, 0x57, 0x00};
  static const size_t  Wide[]          = {0x123456};
  static const uint8_t WideMSB[]       = {0x12, 0x34, 0x56};
  static const size_t  TwoTwelve[]     = {0xABC, 0xDEF};
  static const uint8_t TwoTwelveMSB[]  = {0xAB, 0xCD, 0xEF};

  ExpectBytes("1", 1, MSB_FIRST, One, 1, OneMSB, sizeof(OneMSB));
  ExpectBytes("01", 1, MSB_FIRST, One, 1, OneMSB, sizeof(OneMSB));
  ExpectBytes("4|4", 4, MSB_FIRST, Nibbles, 2, NibblesMSB, sizeof(NibblesMSB));
  ExpectBytes("3|5", 5, MSB_FIRST, Mixed, 2, MixedMSB, sizeof(MixedMSB));
  ExpectBytes("16", 16, MSB_FIRST, Word, 1, WordMSB, sizeof(WordMSB));
  ExpectBytes("12|4", 12, MSB_FIRST, TwelveFour, 2, TwelveFourMSB,
              sizeof(TwelveFourMSB));
  ExpectBytes("008|004", 8, MSB_FIRST, Padded, 2, PaddedMSB, sizeof(PaddedMSB));
  ExpectBytes("8|2R(2|3)", 8, MSB_FIRST, Repeated, 5, RepeatedMSB,
              sizeof(RepeatedMSB));
  ExpectBytes("24", 24, MSB_FIRST, Wide, 1, WideMSB, sizeof(WideMSB));
  ExpectBytes("12|12", 12, MSB_FIRST, TwoTwelve, 2, TwoTwelveMSB,
              sizeof(TwoTwelveMSB));
}

TEST(MCIEncode_LSB_FIRST) {
  static const size_t  One[]           = {1};
  static const uint8_t OneLSB[]        = {0x01};
  static const size_t  Nibbles[]       = {0xA, 0x5};
  static const uint8_t NibblesLSB[]    = {0x5A};
  static const size_t  Mixed[]         = {5, 13};
  static const uint8_t MixedLSB[]      = {0x6D};
  static const size_t  Word[]          = {0x1234};
  static const uint8_t WordLSB[]       = {0x34, 0x12};
  static const size_t  TwelveFour[]    = {0xABC, 0xD};
  static const uint8_t TwelveFourLSB[] = {0xBC, 0xDA};
  static const size_t  Padded[]        = {0xAB, 0xC};
  static const uint8_t PaddedLSB[]     = {0xAB, 0x0C};
  static const size_t  Repeated[]      = {0xAB, 1, 2, 3, 4};
  static const uint8_t RepeatedLSB[]   = {0xAB, 0x69, 0x02};
  static const size_t  Wide[]          = {0x123456};
  static const uint8_t WideLSB[]       = {0x56, 0x34, 0x12};
  static const size_t  TwoTwelve[]     = {0xABC, 0xDEF};
  static const uint8_t TwoTwelveLSB[]  = {0xBC, 0xFA, 0xDE};

  ExpectBytes("1", 1, LSB_FIRST, One, 1, OneLSB, sizeof(OneLSB));
  ExpectBytes("01", 1, LSB_FIRST, One, 1, OneLSB, sizeof(OneLSB));
  ExpectBytes("4|4", 4, LSB_FIRST, Nibbles, 2, NibblesLSB, sizeof(NibblesLSB));
  ExpectBytes("3|5", 5, LSB_FIRST, Mixed, 2, MixedLSB, sizeof(MixedLSB));
  ExpectBytes("16", 16, LSB_FIRST, Word, 1, WordLSB, sizeof(WordLSB));
  ExpectBytes("12|4", 12, LSB_FIRST, TwelveFour, 2, TwelveFourLSB,
              sizeof(TwelveFourLSB));
  ExpectBytes("008|004", 8, LSB_FIRST, Padded, 2, PaddedLSB, sizeof(PaddedLSB));
  ExpectBytes("8|2R(2|3)", 8, LSB_FIRST, Repeated, 5, RepeatedLSB,
              sizeof(RepeatedLSB));
  ExpectBytes("24", 24, LSB_FIRST, Wide, 1, WideLSB, sizeof(WideLSB));
  ExpectBytes("12|12", 12, LSB_FIRST, TwoTwelve, 2, TwoTwelveLSB,
              sizeof(TwoTwelveLSB));
}

TEST(MCIEncode_PreservesBits) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("3", 3);
  uint8_t            Message[1];
  ASSERT_X(CompiledFormat);

  Message[0] = 0xFF;
  ASSERT(EncodeMCIMessage(CompiledFormat, 1, Message, 0, MSB_FIRST, 0));
  ASSERT(Message[0] == 0x1F);

  Message[0] = 0xFF;
  ASSERT(EncodeMCIMessage(CompiledFormat, 1, Message, 0, LSB_FIRST, 0));
  ASSERT(Message[0] == 0xF8);

  Message[0] = 0x00;
  ASSERT(EncodeMCIMessage(CompiledFormat, 1, Message, 0, MSB_FIRST, 7));
  ASSERT(Message[0] == 0xE0);

  Message[0] = 0x00;
  ASSERT(EncodeMCIMessage(CompiledFormat, 1, Message, 0, LSB_FIRST, 7));
  ASSERT(Message[0] == 0x07);

  FreeMCIFormat(&CompiledFormat);
}

TEST(MCIEncode_PreservesOtherFields) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("8|8", 8);
  uint8_t            Message[3]     = {0xAA, 0xBB, 0xCC};
  ASSERT_X(CompiledFormat);

  ASSERT(EncodeMCIMessage(CompiledFormat, 3, Message, 0, MSB_FIRST, 0x12));
  ASSERT(Message[0] == 0x12);
  ASSERT(Message[1] == 0xBB);
  ASSERT(Message[2] == 0xCC);

  ASSERT(EncodeMCIMessage(CompiledFormat, 3, Message, 1, MSB_FIRST, 0x34));
  ASSERT(Message[0] == 0x12);
  ASSERT(Message[1] == 0x34);
  ASSERT(Message[2] == 0xCC);

  FreeMCIFormat(&CompiledFormat);
}

TEST(MCIEncode_RejectsSmallBuffer) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("8|8", 8);
  uint8_t            Message[2]     = {0xAA, 0xBB};
  ASSERT_X(CompiledFormat);

  ASSERT(!EncodeMCIMessage(CompiledFormat, 1, Message, 0, MSB_FIRST, 0x12));
  ASSERT(Message[0] == 0xAA);
  ASSERT(Message[1] == 0xBB);

  ASSERT(!EncodeMCIMessage(CompiledFormat, 0, Message, 0, MSB_FIRST, 0x12));
  ASSERT(Message[0] == 0xAA);
  ASSERT(Message[1] == 0xBB);

  FreeMCIFormat(&CompiledFormat);
}

TEST(MCIEncode_InvalidInputs) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("4", 4);
  uint8_t            Message[1]     = {0xAA};
  ASSERT_X(CompiledFormat);

  ASSERT(!EncodeMCIMessage(NULL, 1, Message, 0, MSB_FIRST, 0));
  ASSERT(!EncodeMCIMessage(CompiledFormat, 1, NULL, 0, MSB_FIRST, 0));
  ASSERT(!EncodeMCIMessage(CompiledFormat, 1, Message, 1, MSB_FIRST, 0));
  ASSERT(
      !EncodeMCIMessage(CompiledFormat, 1, Message, 0, (EncodingLayout)0, 0));
  ASSERT(
      !EncodeMCIMessage(CompiledFormat, 1, Message, 0, (EncodingLayout)3, 0));
  ASSERT(
      !EncodeMCIMessage(CompiledFormat, 1, Message, 0, (EncodingLayout)-1, 0));
  ASSERT(!EncodeMCIMessage(CompiledFormat, 1, Message, 0, MSB_FIRST, 16));
  ASSERT(Message[0] == 0xAA);

  FreeMCIFormat(&CompiledFormat);
}

TEST(MCIEncode_ValueRange) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("4", 4);
  uint8_t            MSBMessage[1]  = {0};
  uint8_t            LSBMessage[1]  = {0};
  ASSERT_X(CompiledFormat);

  ASSERT(EncodeMCIMessage(CompiledFormat, 1, MSBMessage, 0, MSB_FIRST, 15));
  ASSERT(MSBMessage[0] == 0xF0);

  ASSERT(EncodeMCIMessage(CompiledFormat, 1, LSBMessage, 0, LSB_FIRST, 15));
  ASSERT(LSBMessage[0] == 0x0F);

  MSBMessage[0] = 0xAA;
  LSBMessage[0] = 0xAA;
  ASSERT(!EncodeMCIMessage(CompiledFormat, 1, MSBMessage, 0, MSB_FIRST, 16));
  ASSERT(!EncodeMCIMessage(CompiledFormat, 1, LSBMessage, 0, LSB_FIRST, 16));
  ASSERT(MSBMessage[0] == 0xAA);
  ASSERT(LSBMessage[0] == 0xAA);

  FreeMCIFormat(&CompiledFormat);
}

TEST(MCIEncode_WideRange) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("24", 24);
  uint8_t            Message[3]     = {0};
  ASSERT_X(CompiledFormat);

  ASSERT(EncodeMCIMessage(CompiledFormat, 3, Message, 0, MSB_FIRST, 0xFFFFFF));
  ASSERT(Message[0] == 0xFF);
  ASSERT(Message[1] == 0xFF);
  ASSERT(Message[2] == 0xFF);

  Message[0] = 0xAA;
  Message[1] = 0xBB;
  Message[2] = 0xCC;
  ASSERT(
      !EncodeMCIMessage(CompiledFormat, 3, Message, 0, MSB_FIRST, 0x1000000));
  ASSERT(Message[0] == 0xAA);
  ASSERT(Message[1] == 0xBB);
  ASSERT(Message[2] == 0xCC);

  FreeMCIFormat(&CompiledFormat);
}

TEST_SUITE(MCIEncodeFunctionalSuite,
           MCIEncode_MSB_FIRST,
           MCIEncode_LSB_FIRST,
           MCIEncode_PreservesBits,
           MCIEncode_PreservesOtherFields,
           MCIEncode_RejectsSmallBuffer,
           MCIEncode_InvalidInputs,
           MCIEncode_ValueRange,
           MCIEncode_WideRange);

TEST(MCIDecode_MSB_FIRST) {
  static const uint8_t One[]              = {0x80};
  static const size_t  OneValues[]        = {1};
  static const uint8_t Nibbles[]          = {0xA5};
  static const size_t  NibblesValues[]    = {0xA, 0x5};
  static const uint8_t Mixed[]            = {0xAD};
  static const size_t  MixedValues[]      = {5, 13};
  static const uint8_t Word[]             = {0x12, 0x34};
  static const size_t  WordValues[]       = {0x1234};
  static const uint8_t TwelveFour[]       = {0xAB, 0xCD};
  static const size_t  TwelveFourValues[] = {0xABC, 0xD};
  static const uint8_t Padded[]           = {0xAB, 0xC0};
  static const size_t  PaddedValues[]     = {0xAB, 0xC};
  static const uint8_t Repeated[]         = {0xAB, 0x57, 0x00};
  static const size_t  RepeatedValues[]   = {0xAB, 1, 2, 3, 4};
  static const uint8_t Wide[]             = {0x12, 0x34, 0x56};
  static const size_t  WideValues[]       = {0x123456};
  static const uint8_t TwoTwelve[]        = {0xAB, 0xCD, 0xEF};
  static const size_t  TwoTwelveValues[]  = {0xABC, 0xDEF};

  ExpectDecoded("1", 1, MSB_FIRST, One, sizeof(One), OneValues, 1);
  ExpectDecoded("01", 1, MSB_FIRST, One, sizeof(One), OneValues, 1);
  ExpectDecoded("4|4", 4, MSB_FIRST, Nibbles, sizeof(Nibbles), NibblesValues,
                2);
  ExpectDecoded("3|5", 5, MSB_FIRST, Mixed, sizeof(Mixed), MixedValues, 2);
  ExpectDecoded("16", 16, MSB_FIRST, Word, sizeof(Word), WordValues, 1);
  ExpectDecoded("12|4", 12, MSB_FIRST, TwelveFour, sizeof(TwelveFour),
                TwelveFourValues, 2);
  ExpectDecoded("008|004", 8, MSB_FIRST, Padded, sizeof(Padded), PaddedValues,
                2);
  ExpectDecoded("8|2R(2|3)", 8, MSB_FIRST, Repeated, sizeof(Repeated),
                RepeatedValues, 5);
  ExpectDecoded("24", 24, MSB_FIRST, Wide, sizeof(Wide), WideValues, 1);
  ExpectDecoded("12|12", 12, MSB_FIRST, TwoTwelve, sizeof(TwoTwelve),
                TwoTwelveValues, 2);
}

TEST(MCIDecode_LSB_FIRST) {
  static const uint8_t One[]              = {0x01};
  static const size_t  OneValues[]        = {1};
  static const uint8_t Nibbles[]          = {0x5A};
  static const size_t  NibblesValues[]    = {0xA, 0x5};
  static const uint8_t Mixed[]            = {0x6D};
  static const size_t  MixedValues[]      = {5, 13};
  static const uint8_t Word[]             = {0x34, 0x12};
  static const size_t  WordValues[]       = {0x1234};
  static const uint8_t TwelveFour[]       = {0xBC, 0xDA};
  static const size_t  TwelveFourValues[] = {0xABC, 0xD};
  static const uint8_t Padded[]           = {0xAB, 0x0C};
  static const size_t  PaddedValues[]     = {0xAB, 0xC};
  static const uint8_t Repeated[]         = {0xAB, 0x69, 0x02};
  static const size_t  RepeatedValues[]   = {0xAB, 1, 2, 3, 4};
  static const uint8_t Wide[]             = {0x56, 0x34, 0x12};
  static const size_t  WideValues[]       = {0x123456};
  static const uint8_t TwoTwelve[]        = {0xBC, 0xFA, 0xDE};
  static const size_t  TwoTwelveValues[]  = {0xABC, 0xDEF};

  ExpectDecoded("1", 1, LSB_FIRST, One, sizeof(One), OneValues, 1);
  ExpectDecoded("01", 1, LSB_FIRST, One, sizeof(One), OneValues, 1);
  ExpectDecoded("4|4", 4, LSB_FIRST, Nibbles, sizeof(Nibbles), NibblesValues,
                2);
  ExpectDecoded("3|5", 5, LSB_FIRST, Mixed, sizeof(Mixed), MixedValues, 2);
  ExpectDecoded("16", 16, LSB_FIRST, Word, sizeof(Word), WordValues, 1);
  ExpectDecoded("12|4", 12, LSB_FIRST, TwelveFour, sizeof(TwelveFour),
                TwelveFourValues, 2);
  ExpectDecoded("008|004", 8, LSB_FIRST, Padded, sizeof(Padded), PaddedValues,
                2);
  ExpectDecoded("8|2R(2|3)", 8, LSB_FIRST, Repeated, sizeof(Repeated),
                RepeatedValues, 5);
  ExpectDecoded("24", 24, LSB_FIRST, Wide, sizeof(Wide), WideValues, 1);
  ExpectDecoded("12|12", 12, LSB_FIRST, TwoTwelve, sizeof(TwoTwelve),
                TwoTwelveValues, 2);
}

TEST(MCIDecode_IgnoresPadding) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("3", 3);
  uint8_t            Message[1];
  size_t             Value;
  ASSERT_X(CompiledFormat);

  Message[0] = 0x1F;
  Value      = SIZE_MAX;
  ASSERT(DecodeMCIMessage(CompiledFormat, 1, Message, 0, MSB_FIRST, &Value));
  ASSERT(Value == 0);

  Message[0] = 0xE0;
  Value      = SIZE_MAX;
  ASSERT(DecodeMCIMessage(CompiledFormat, 1, Message, 0, MSB_FIRST, &Value));
  ASSERT(Value == 7);

  Message[0] = 0xFF;
  Value      = SIZE_MAX;
  ASSERT(DecodeMCIMessage(CompiledFormat, 1, Message, 0, MSB_FIRST, &Value));
  ASSERT(Value == 7);

  Message[0] = 0xF8;
  Value      = SIZE_MAX;
  ASSERT(DecodeMCIMessage(CompiledFormat, 1, Message, 0, LSB_FIRST, &Value));
  ASSERT(Value == 0);

  Message[0] = 0x07;
  Value      = SIZE_MAX;
  ASSERT(DecodeMCIMessage(CompiledFormat, 1, Message, 0, LSB_FIRST, &Value));
  ASSERT(Value == 7);

  Message[0] = 0xFF;
  Value      = SIZE_MAX;
  ASSERT(DecodeMCIMessage(CompiledFormat, 1, Message, 0, LSB_FIRST, &Value));
  ASSERT(Value == 7);

  FreeMCIFormat(&CompiledFormat);
}

TEST(MCIDecode_AcceptsExtraBytes) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("8", 8);
  const uint8_t      Message[]      = {0x12, 0x34};
  size_t             Value;
  ASSERT_X(CompiledFormat);

  Value = SIZE_MAX;
  ASSERT(DecodeMCIMessage(CompiledFormat, 2, Message, 0, MSB_FIRST, &Value));
  ASSERT(Value == 0x12);

  Value = SIZE_MAX;
  ASSERT(DecodeMCIMessage(CompiledFormat, 2, Message, 0, LSB_FIRST, &Value));
  ASSERT(Value == 0x12);

  FreeMCIFormat(&CompiledFormat);
}

TEST(MCIDecode_RejectsSmallBuffer) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("8|8", 8);
  const uint8_t      Message[]      = {0x12, 0x34};
  ASSERT_X(CompiledFormat);

  ExpectDecodeFailure(CompiledFormat, 1, Message, 0, MSB_FIRST);
  ExpectDecodeFailure(CompiledFormat, 0, Message, 0, MSB_FIRST);

  FreeMCIFormat(&CompiledFormat);
}

TEST(MCIDecode_InvalidInputs) {
  MCICompiledFormat *CompiledFormat = CompileMCIFormat("4", 4);
  const uint8_t      Message[]      = {0x0F};
  ASSERT_X(CompiledFormat);

  ExpectDecodeFailure(NULL, 1, Message, 0, MSB_FIRST);
  ExpectDecodeFailure(CompiledFormat, 1, NULL, 0, MSB_FIRST);
  ExpectDecodeFailure(CompiledFormat, 0, Message, 0, MSB_FIRST);
  ExpectDecodeFailure(CompiledFormat, 1, Message, 1, MSB_FIRST);
  ExpectDecodeFailure(CompiledFormat, 1, Message, 0, (EncodingLayout)0);
  ExpectDecodeFailure(CompiledFormat, 1, Message, 0, (EncodingLayout)3);
  ExpectDecodeFailure(CompiledFormat, 1, Message, 0, (EncodingLayout)-1);
  ASSERT(!DecodeMCIMessage(CompiledFormat, 1, Message, 0, MSB_FIRST, NULL));

  FreeMCIFormat(&CompiledFormat);
}

TEST_SUITE(MCIDecodeFunctionalSuite,
           MCIDecode_MSB_FIRST,
           MCIDecode_LSB_FIRST,
           MCIDecode_IgnoresPadding,
           MCIDecode_AcceptsExtraBytes,
           MCIDecode_RejectsSmallBuffer,
           MCIDecode_InvalidInputs);

TEST(MCIRoundTrip_BothLayouts) {
  static const EncodingLayout Layouts[]   = {MSB_FIRST, LSB_FIRST};
  static const size_t         Bits[]      = {0, 1, 0, 1, 0, 1, 0, 1};
  static const size_t         Nibbles[]   = {0xA, 0x5};
  static const size_t         Zero[]      = {0, 0};
  static const size_t         Max[]       = {7, 31};
  static const size_t         Mixed[]     = {5, 13};
  static const size_t         Zero16[]    = {0};
  static const size_t         One16[]     = {1};
  static const size_t         Max16[]     = {0xFFFF};
  static const size_t         Word16[]    = {0x1234};
  static const size_t         Repeated[]  = {0xAB, 1, 2, 3, 4};
  static const size_t         Padded[]    = {0xAB, 0xC};
  static const size_t         Wide[]      = {0x123456};
  static const size_t         TwoTwelve[] = {0xABC, 0xDEF};
  size_t                      i;

  for (i = 0; i < (sizeof(Layouts) / sizeof(Layouts[0])); i++) {
    ExpectRoundTrip("1|1|1|1|1|1|1|1", 1, Layouts[i], Bits, 8);
    ExpectRoundTrip("4|4", 4, Layouts[i], Nibbles, 2);
    ExpectRoundTrip("3|5", 5, Layouts[i], Zero, 2);
    ExpectRoundTrip("3|5", 5, Layouts[i], Max, 2);
    ExpectRoundTrip("3|5", 5, Layouts[i], Mixed, 2);
    ExpectRoundTrip("16", 16, Layouts[i], Zero16, 1);
    ExpectRoundTrip("16", 16, Layouts[i], One16, 1);
    ExpectRoundTrip("16", 16, Layouts[i], Max16, 1);
    ExpectRoundTrip("16", 16, Layouts[i], Word16, 1);
    ExpectRoundTrip("8|2R(2|3)", 8, Layouts[i], Repeated, 5);
    ExpectRoundTrip("008|004", 8, Layouts[i], Padded, 2);
    ExpectRoundTrip("24", 24, Layouts[i], Wide, 1);
    ExpectRoundTrip("12|12", 12, Layouts[i], TwoTwelve, 2);
  }
}

TEST_SUITE(MCIRoundTripFunctionalSuite,
           MCIRoundTrip_BothLayouts);
