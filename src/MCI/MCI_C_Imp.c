#include "src/Core/Core.h"
#include "src/MCI/MessageCodingInterface.h"
#include <limits.h>
#include <string.h>

struct MCICompiledFormat {
  size_t  NumFields;
  size_t *BitSize;
  size_t *BitOffset;
  size_t  TotalBits;
  size_t  TotalBytes;
};

// Bounds a single repetition count and the recursion depth of nested
// repetition groups so straightforward runaway formats are rejected early.
static const size_t MCIExpansionCountLimit = 64;
static const size_t MCIExpansionDepthLimit = 64;

void FreeMCIFormat(MCICompiledFormat **CompiledFormatReference) {
  if (!CompiledFormatReference) {
    return;
  }
  if (*CompiledFormatReference) {
    free((*CompiledFormatReference)->BitSize);
    free((*CompiledFormatReference)->BitOffset);
    free(*CompiledFormatReference);
    *CompiledFormatReference = NULL;
  }
}

typedef struct {
  char  *Buffer;
  size_t Length;
} MCIExpansion;

static bool AddExpansionLength(MCIExpansion *Expansion,
                               const size_t  Amount) {
  return !Core_AddOverflow(Expansion->Length, Amount, &(Expansion->Length));
}

static bool AppendExpansionCharacter(MCIExpansion *Expansion,
                                     const char    Character) {
  size_t OldLength = Expansion->Length;
  if (!AddExpansionLength(Expansion, 1)) {
    return false;
  }
  if (Expansion->Buffer) {
    Expansion->Buffer[OldLength] = Character;
  }
  return true;
}

static bool AppendExpansionSpan(MCIExpansion *Expansion,
                                const char   *Span,
                                const size_t  SpanLength) {
  size_t OldLength = Expansion->Length;
  if (!AddExpansionLength(Expansion, SpanLength)) {
    return false;
  }
  if (Expansion->Buffer) {
    memcpy(Expansion->Buffer + OldLength, Span, SpanLength);
  }
  return true;
}

static bool ParseRepetitionCount(const char  *Digits,
                                 const size_t DigitCount,
                                 size_t      *Count) {
  size_t Digit;
  size_t i;
  size_t Value = 0;

  for (i = 0; i < DigitCount; i++) {
    Digit = (size_t)(Digits[i] - '0');
    if (Core_MulOverflow(Value, 10, &Value)) {
      return false;
    }
    if (Core_AddOverflow(Value, Digit, &Value)) {
      return false;
    }
  }
  if (Value == 0) {
    return false;
  }
  if (Value > MCIExpansionCountLimit) {
    return false;
  }
  *Count = Value;
  return true;
}

// Expands Format ('|' Element)*. Returns the address of the ')' or '\0' which
// ends the sequence, or NULL when the sequence is malformed. A NULL Buffer
// measures the expansion without writing it.
static const char *ExpandFormatSequence(const char   *Cursor,
                                        MCIExpansion *Expansion,
                                        const size_t  NestingDepth) {
  MCIExpansion InnerExpansion;
  const char  *FieldStart;
  const char  *InnerStart;
  const char  *InnerEnd;
  size_t       FieldLength;
  size_t       Count;
  size_t       CopyLength;
  size_t       Start;
  size_t       Product;
  size_t       Contribution;
  size_t       CopyIndex;
  bool         FirstElement = true;

  if (NestingDepth > MCIExpansionDepthLimit) {
    return NULL;
  }

  for (;;) {
    // Scan one digit run.
    FieldStart = Cursor;
    while (('0' <= *Cursor) && (*Cursor <= '9')) {
      Cursor++;
    }
    if (Cursor == FieldStart) {
      return NULL;
    }
    FieldLength = (size_t)(Cursor - FieldStart);

    if (*Cursor == 'R') {
      // Repetition: <n>R(Format).
      if (Cursor[1] != '(') {
        return NULL;
      }
      if (!ParseRepetitionCount(FieldStart, FieldLength, &Count)) {
        return NULL;
      }
      InnerStart = Cursor + 2;

      if ((!FirstElement) && (!AppendExpansionCharacter(Expansion, '|'))) {
        return NULL;
      }

      if (!Expansion->Buffer) {
        // Measure the inner sequence exactly once.
        InnerExpansion.Buffer = NULL;
        InnerExpansion.Length = 0;
        InnerEnd =
            ExpandFormatSequence(InnerStart, &InnerExpansion, NestingDepth + 1);
        if ((!InnerEnd) || (*InnerEnd != ')')) {
          return NULL;
        }
        if (Core_MulOverflow(Count, InnerExpansion.Length, &Product)) {
          return NULL;
        }
        if (Core_AddOverflow(Product, Count - 1, &Contribution)) {
          return NULL;
        }
        if (!AddExpansionLength(Expansion, Contribution)) {
          return NULL;
        }
      } else {
        // Write the first copy, then replicate it.
        Start = Expansion->Length;
        InnerEnd =
            ExpandFormatSequence(InnerStart, Expansion, NestingDepth + 1);
        // The measuring pass has already validated the closing parenthesis.
        if (!InnerEnd) {
          return NULL;
        }
        CopyLength = Expansion->Length - Start;
        for (CopyIndex = 1; CopyIndex < Count; CopyIndex++) {
          if (!AppendExpansionCharacter(Expansion, '|')) {
            return NULL;
          }
          if (!AppendExpansionSpan(Expansion, Expansion->Buffer + Start,
                                   CopyLength)) {
            return NULL;
          }
        }
      }
      FirstElement = false;
      Cursor       = InnerEnd + 1;
    } else if ((*Cursor == '\0') || (*Cursor == ')') || (*Cursor == '|')) {
      // Plain field: copy the digits verbatim.
      if ((!FirstElement) && (!AppendExpansionCharacter(Expansion, '|'))) {
        return NULL;
      }
      if (!AppendExpansionSpan(Expansion, FieldStart, FieldLength)) {
        return NULL;
      }
      FirstElement = false;
    } else {
      return NULL;
    }

    // Sequence separator.
    if (*Cursor == '|') {
      Cursor++;
      continue;
    }
    return Cursor;
  }
}

static char *ExpandFormat(const char Format[]) {
  MCIExpansion Measure;
  MCIExpansion Write;
  const char  *End;
  size_t       AllocationSize;
  char        *Expanded;

  // Measure the repetitionless format.
  Measure.Buffer = NULL;
  Measure.Length = 0;
  End            = ExpandFormatSequence(Format, &Measure, 0);
  if ((!End) || (*End != '\0')) {
    return NULL;
  }

  if (Core_AddOverflow(Measure.Length, 1, &AllocationSize)) {
    return NULL;
  }
  Expanded = (char *)Core_Malloc(AllocationSize);
  if (!Expanded) {
    return NULL;
  }

  // Write the repetitionless format.
  Write.Buffer = Expanded;
  Write.Length = 0;
  End          = ExpandFormatSequence(Format, &Write, 0);
  // The measuring pass has already validated the terminating NUL.
  if (!End) {
    free(Expanded);
    return NULL;
  }
  Expanded[Write.Length] = '\0';
  return Expanded;
}

static size_t ipow(size_t Base,
                   size_t Exp) {
  size_t Result = 1;
  while (Exp > 0) {
    if (Exp & 1) {
      if (Core_MulOverflow(Result, Base, &Result)) {
        return 0;
      }
    }
    Exp >>= 1;
    if (Exp > 0) {
      if (Core_MulOverflow(Base, Base, &Base)) {
        return 0;
      }
    }
  }
  return Result;
}

MCICompiledFormat *CompileMCIFormat(const char   Format[],
                                    const size_t MaxFieldBitSize) {
  MCICompiledFormat *CompiledFormat = NULL;
  size_t             NumFields;
  size_t             AllocSize = 0;
  size_t             BitSize;
  void              *TmpAlloc = NULL;
  size_t             i;
  size_t             MaxRepresentable;

  char *ExpandedFormat = NULL;
  char *Saveptr        = NULL;
  char *Token          = NULL;
  long  ReturnedValue;

  bool AllocatedExpandedFormat = false;
  bool AllocatedCompiledFormat = false;
  bool AllocatedBitOffset      = false;
  bool AllocatedBitSize        = false;

  if (!Format) {
    return NULL;
  }

  // Too large field size.
  MaxRepresentable = ipow(2, MaxFieldBitSize);
  if (MaxRepresentable == 0) {
    return NULL;
  }
  if (MaxRepresentable > LONG_MAX) {
    return NULL;
  }

  // Get the expanded format.
  ExpandedFormat = ExpandFormat(Format);
  if (!ExpandedFormat) {
    goto cleanup;
  }
  AllocatedExpandedFormat = true;

  // Tokenize. ExpandFormat guarantees at least one non-empty field.
  Token = strtok_r(ExpandedFormat, "|", &Saveptr);

  // Allocate the compiled format.
  CompiledFormat = (MCICompiledFormat *)Core_Malloc(sizeof(MCICompiledFormat));
  if (!CompiledFormat) {
    goto cleanup;
  }
  AllocatedCompiledFormat   = true;
  CompiledFormat->BitOffset = NULL;
  CompiledFormat->BitSize   = NULL;

  NumFields = 0;
  while (Token) {
    ReturnedValue = strtol(Token, NULL, 10);
    if (ReturnedValue <= 0) {
      goto cleanup;
    }
    BitSize = (size_t)ReturnedValue;
    if (BitSize > MaxFieldBitSize) {
      goto cleanup;
    }

    // Get NumFields and AllocSize.
    if (Core_AddOverflow(NumFields, 1, &NumFields)) {
      goto cleanup;
    }
    if (Core_MulOverflow(NumFields, sizeof(size_t), &AllocSize)) {
      goto cleanup;
    }

    // Reallocate bit sizes.
    TmpAlloc = Core_Realloc(CompiledFormat->BitSize, AllocSize);
    if (!TmpAlloc) {
      goto cleanup;
    }
    CompiledFormat->BitSize = (size_t *)TmpAlloc;
    AllocatedBitSize        = true;

    // Write bit size.
    CompiledFormat->BitSize[NumFields - 1] = BitSize;

    // Get next token.
    Token = strtok_r(NULL, "|", &Saveptr);
  }
  free(ExpandedFormat);
  AllocatedExpandedFormat = false;

  // Allocate bit offsets.
  CompiledFormat->BitOffset = Core_Malloc(AllocSize);
  if (!CompiledFormat->BitOffset) {
    goto cleanup;
  }
  AllocatedBitOffset = true;

  CompiledFormat->TotalBits = 0;
  for (i = 0; i < NumFields; i++) {
    CompiledFormat->BitOffset[i] = CompiledFormat->TotalBits;
    if (Core_AddOverflow(CompiledFormat->TotalBits, CompiledFormat->BitSize[i],
                         &(CompiledFormat->TotalBits))) {
      goto cleanup;
    }
  }
  CompiledFormat->NumFields = NumFields;
  if (Core_AddOverflow(CompiledFormat->TotalBits, 7,
                       &(CompiledFormat->TotalBytes))) {
    goto cleanup;
  }
  CompiledFormat->TotalBytes = CompiledFormat->TotalBytes / 8;

  return CompiledFormat;
cleanup:
  if (AllocatedBitSize)
    free(CompiledFormat->BitSize);
  if (AllocatedBitOffset)
    free(CompiledFormat->BitOffset);
  if (AllocatedCompiledFormat)
    free(CompiledFormat);
  if (AllocatedExpandedFormat)
    free(ExpandedFormat);
  return NULL;
}

size_t GetMCIMessageByteSize(const MCICompiledFormat *CompiledFormat) {
  if (!CompiledFormat) {
    return 0;
  }
  return CompiledFormat->TotalBytes;
}

size_t GetMCIMessageFieldCount(const MCICompiledFormat *CompiledFormat) {
  if (!CompiledFormat) {
    return 0;
  }
  return CompiledFormat->NumFields;
}

static bool IsEncodingValid(const EncodingLayout Encoding) {
  return (Encoding == MSB_FIRST) || (Encoding == LSB_FIRST);
}

static uint8_t MessageBitMask(const size_t         BitInByte,
                              const EncodingLayout Encoding) {
  if (Encoding == MSB_FIRST) {
    return (uint8_t)(0x80u >> BitInByte);
  }
  return (uint8_t)(0x01u << BitInByte);
}

static void WriteMessageBit(uint8_t              Message[],
                            const size_t         BitIndex,
                            const EncodingLayout Encoding,
                            const bool           BitValue) {
  const size_t  ByteIndex = BitIndex / CHAR_BIT;
  const size_t  BitInByte = BitIndex % CHAR_BIT;
  const uint8_t Mask      = MessageBitMask(BitInByte, Encoding);

  if (BitValue) {
    Message[ByteIndex] |= Mask;
  } else {
    Message[ByteIndex] &= (uint8_t)~Mask;
  }
}

static bool ReadMessageBit(const uint8_t        Message[],
                           const size_t         BitIndex,
                           const EncodingLayout Encoding) {
  const size_t ByteIndex = BitIndex / CHAR_BIT;
  const size_t BitInByte = BitIndex % CHAR_BIT;

  return (Message[ByteIndex] & MessageBitMask(BitInByte, Encoding)) != 0;
}

bool EncodeMCIMessage(const MCICompiledFormat *CompiledFormat,
                      size_t                   MessageByteSize,
                      uint8_t                  Message[MessageByteSize],
                      size_t                   FieldId,
                      EncodingLayout           Encoding,
                      size_t                   Value) {
  size_t BitOffset;
  size_t BitSize;
  size_t MaxRepresentable;
  size_t SourceBit;
  size_t i;

  if (!CompiledFormat) {
    return false;
  }
  if (!IsEncodingValid(Encoding)) {
    return false;
  }
  if (FieldId >= CompiledFormat->NumFields) {
    return false;
  }
  if ((!CompiledFormat->BitSize) || (!CompiledFormat->BitOffset)) {
    return false;
  }
  if (CompiledFormat->TotalBytes == 0) {
    return false;
  }
  if (MessageByteSize < CompiledFormat->TotalBytes) {
    return false;
  }
  if (!Message) {
    return false;
  }

  BitSize   = CompiledFormat->BitSize[FieldId];
  BitOffset = CompiledFormat->BitOffset[FieldId];
  if (BitSize == 0) {
    return false;
  }
  if (BitOffset > CompiledFormat->TotalBits) {
    return false;
  }
  if (BitSize > (CompiledFormat->TotalBits - BitOffset)) {
    return false;
  }

  MaxRepresentable = ipow(2, BitSize);
  if (MaxRepresentable == 0) {
    return false;
  }
  if (Value >= MaxRepresentable) {
    return false;
  }

  for (i = 0; i < BitSize; i++) {
    SourceBit = (Encoding == MSB_FIRST) ? (BitSize - 1 - i) : i;
    WriteMessageBit(Message, BitOffset + i, Encoding,
                    ((Value >> SourceBit) & 1u) != 0);
  }

  return true;
}

bool DecodeMCIMessage(const MCICompiledFormat *CompiledFormat,
                      size_t                   MessageByteSize,
                      const uint8_t            Message[MessageByteSize],
                      size_t                   FieldId,
                      EncodingLayout           Encoding,
                      size_t                  *Value) {
  size_t BitOffset;
  size_t BitSize;
  size_t Decoded = 0;
  size_t SourceBit;
  size_t i;

  if (!CompiledFormat) {
    return false;
  }
  if (!Value) {
    return false;
  }
  if (!IsEncodingValid(Encoding)) {
    return false;
  }
  if (FieldId >= CompiledFormat->NumFields) {
    return false;
  }
  if ((!CompiledFormat->BitSize) || (!CompiledFormat->BitOffset)) {
    return false;
  }
  if (CompiledFormat->TotalBytes == 0) {
    return false;
  }
  if (MessageByteSize < CompiledFormat->TotalBytes) {
    return false;
  }
  if (!Message) {
    return false;
  }

  BitSize   = CompiledFormat->BitSize[FieldId];
  BitOffset = CompiledFormat->BitOffset[FieldId];
  if (BitSize == 0) {
    return false;
  }
  if (BitSize > (sizeof(size_t) * CHAR_BIT)) {
    return false;
  }
  if (BitOffset > CompiledFormat->TotalBits) {
    return false;
  }
  if (BitSize > (CompiledFormat->TotalBits - BitOffset)) {
    return false;
  }

  for (i = 0; i < BitSize; i++) {
    SourceBit = (Encoding == MSB_FIRST) ? (BitSize - 1 - i) : i;
    if (ReadMessageBit(Message, BitOffset + i, Encoding)) {
      Decoded |= (size_t)1 << SourceBit;
    }
  }

  *Value = Decoded;
  return true;
}
