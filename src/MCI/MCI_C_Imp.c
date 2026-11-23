#include "src/Core/Core.h"
#include "src/MCI/MessageCodingInterface.h"
#include <limits.h>
#include <string.h>

struct MCICompiledFormat {
  char   *FormatCopy;
  size_t  NumFields;
  size_t *BitSize;
  size_t *BitOffset;
  size_t  TotalBits;
  size_t  TotalBytes;
};

// Bounds a single repetition count, the recursion depth of nested repetition
// groups, and the total number of expanded fields so runaway formats are
// rejected before their expansion is materialized. MCISemanticIdMaxComponents
// is the largest possible path through an expanded format: one component for
// the top-level group plus one per repetition level.
enum {
#ifdef TESTABLE
  MCIExpansionCountLimit = 64,
  MCIExpansionDepthLimit = 64,
  MCIExpandedFieldLimit  = 1024
#else
#include "ModuleConfiguration/MCI.cfg"
#endif
      ,
  MCISemanticIdMaxComponents = MCIExpansionDepthLimit + 1
};

void FreeMCIFormat(MCICompiledFormat **CompiledFormatReference) {
  if (!CompiledFormatReference) {
    return;
  }
  if (*CompiledFormatReference) {
    free((*CompiledFormatReference)->FormatCopy);
    free((*CompiledFormatReference)->BitSize);
    free((*CompiledFormatReference)->BitOffset);
    free(*CompiledFormatReference);
    *CompiledFormatReference = NULL;
  }
}

typedef struct {
  char  *Buffer;
  size_t Length;
  size_t FieldCount;
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
// ends the sequence, or NULL when the sequence is malformed or would expand to
// more than MCIExpandedFieldLimit fields. A NULL Buffer measures the expansion
// without writing it.
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
  size_t       FieldContribution;
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
        InnerExpansion.Buffer     = NULL;
        InnerExpansion.Length     = 0;
        InnerExpansion.FieldCount = 0;
        InnerEnd =
            ExpandFormatSequence(InnerStart, &InnerExpansion, NestingDepth + 1);
        if ((!InnerEnd) || (*InnerEnd != ')')) {
          return NULL;
        }
        if (InnerExpansion.FieldCount >
            ((size_t)MCIExpandedFieldLimit / Count)) {
          return NULL;
        }
        FieldContribution = Count * InnerExpansion.FieldCount;
        if (FieldContribution >
            ((size_t)MCIExpandedFieldLimit - Expansion->FieldCount)) {
          return NULL;
        }
        Expansion->FieldCount += FieldContribution;
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
      if (!Expansion->Buffer) {
        if (Expansion->FieldCount >= (size_t)MCIExpandedFieldLimit) {
          return NULL;
        }
        Expansion->FieldCount++;
      }
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
  Measure.Buffer     = NULL;
  Measure.Length     = 0;
  Measure.FieldCount = 0;
  End                = ExpandFormatSequence(Format, &Measure, 0);
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
  Write.Buffer     = Expanded;
  Write.Length     = 0;
  Write.FieldCount = 0;
  End              = ExpandFormatSequence(Format, &Write, 0);
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
  size_t             FormatLength;
  void              *TmpAlloc = NULL;
  size_t             i;
  size_t             MaxRepresentable;

  char *ExpandedFormat = NULL;
  char *Saveptr        = NULL;
  char *Token          = NULL;
  long  ReturnedValue;

  bool AllocatedExpandedFormat = false;
  bool AllocatedCompiledFormat = false;

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
  AllocatedCompiledFormat    = true;
  CompiledFormat->FormatCopy = NULL;
  CompiledFormat->BitOffset  = NULL;
  CompiledFormat->BitSize    = NULL;

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

  FormatLength = strlen(Format);
  if (Core_AddOverflow(FormatLength, 1, &FormatLength)) {
    goto cleanup;
  }
  if (Core_MulOverflow(FormatLength, sizeof(char), &AllocSize)) {
    goto cleanup;
  }
  CompiledFormat->FormatCopy = (char *)Core_Malloc(AllocSize);
  if (!CompiledFormat->FormatCopy) {
    goto cleanup;
  }
  memcpy(CompiledFormat->FormatCopy, Format, FormatLength);

  return CompiledFormat;
cleanup:
  if (AllocatedCompiledFormat) {
    free(CompiledFormat->FormatCopy);
    free(CompiledFormat->BitOffset);
    free(CompiledFormat->BitSize);
    free(CompiledFormat);
  }
  if (AllocatedExpandedFormat)
    free(ExpandedFormat);
  return NULL;
}

size_t GetMCIMessageBitSize(const MCICompiledFormat *CompiledFormat) {
  if (!CompiledFormat) {
    return 0;
  }
  return CompiledFormat->TotalBits;
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

static bool ParseSemanticId(const char *SemanticId,
                            size_t      Components[],
                            size_t     *ComponentCount) {
  const char *ComponentStart;
  const char *Cursor;
  size_t      Count = 0;
  size_t      Digit;
  size_t      Value;

  if (*SemanticId == '\0') {
    return false;
  }

  Cursor = SemanticId;
  for (;;) {
    ComponentStart = Cursor;
    Value          = 0;
    while (('0' <= *Cursor) && (*Cursor <= '9')) {
      Digit = (size_t)(*Cursor - '0');
      if (Core_MulOverflow(Value, 10, &Value)) {
        return false;
      }
      if (Core_AddOverflow(Value, Digit, &Value)) {
        return false;
      }
      Cursor++;
    }
    if (Cursor == ComponentStart) {
      return false;
    }
    if (((size_t)(Cursor - ComponentStart) > 1) && (ComponentStart[0] == '0')) {
      return false;
    }
    if (Value == 0) {
      return false;
    }
    if (Count >= MCISemanticIdMaxComponents) {
      return false;
    }
    Components[Count++] = Value;

    if (*Cursor == '\0') {
      break;
    }
    if (*Cursor != '.') {
      return false;
    }
    Cursor++;
  }

  *ComponentCount = Count;
  return true;
}

// Measures one format sequence without expanding it. End points at the ')' or
// '\0' that terminates the sequence on success.
static bool MeasureFormatSequence(const char  *Cursor,
                                  size_t      *FieldCount,
                                  const char **End,
                                  const size_t NestingDepth) {
  const char *FieldStart;
  const char *InnerEnd;
  size_t      FieldLength;
  size_t      Count;
  size_t      InnerCount;
  size_t      Product;
  size_t      Total = 0;

  if (NestingDepth > MCIExpansionDepthLimit) {
    return false;
  }

  for (;;) {
    FieldStart = Cursor;
    while (('0' <= *Cursor) && (*Cursor <= '9')) {
      Cursor++;
    }
    if (Cursor == FieldStart) {
      return false;
    }
    FieldLength = (size_t)(Cursor - FieldStart);

    if (*Cursor == 'R') {
      if (Cursor[1] != '(') {
        return false;
      }
      if (!ParseRepetitionCount(FieldStart, FieldLength, &Count)) {
        return false;
      }
      if (!MeasureFormatSequence(Cursor + 2, &InnerCount, &InnerEnd,
                                 NestingDepth + 1)) {
        return false;
      }
      if (*InnerEnd != ')') {
        return false;
      }
      if (Core_MulOverflow(Count, InnerCount, &Product)) {
        return false;
      }
      if (Core_AddOverflow(Total, Product, &Total)) {
        return false;
      }
      Cursor = InnerEnd + 1;
    } else if ((*Cursor == '\0') || (*Cursor == ')') || (*Cursor == '|')) {
      if (Core_AddOverflow(Total, 1, &Total)) {
        return false;
      }
    } else {
      return false;
    }

    if (*Cursor == '|') {
      Cursor++;
      continue;
    }
    if ((*Cursor != '\0') && (*Cursor != ')')) {
      return false;
    }
    *FieldCount = Total;
    *End        = Cursor;
    return true;
  }
}

// Resolves one group at the current format level, descending into a repetition
// copy when Components[0] selects it. BaseFlatIndex is the flat index of the
// first field in Cursor.
static bool ResolveSemanticSequence(const char  *Cursor,
                                    const size_t Components[],
                                    size_t       ComponentCount,
                                    size_t       BaseFlatIndex,
                                    size_t      *FieldId,
                                    const size_t NestingDepth) {
  const char *FieldStart;
  const char *InnerEnd;
  size_t      FieldLength;
  size_t      Count;
  size_t      InnerCount;
  size_t      CopyIndex;
  size_t      CopyOffset;
  size_t      Candidate;
  size_t      Offset     = 0;
  size_t      GroupIndex = 1;

  for (;;) {
    FieldStart = Cursor;
    while (('0' <= *Cursor) && (*Cursor <= '9')) {
      Cursor++;
    }
    if (Cursor == FieldStart) {
      return false;
    }
    FieldLength = (size_t)(Cursor - FieldStart);

    if (*Cursor == 'R') {
      if (Cursor[1] != '(') {
        return false;
      }
      if (!ParseRepetitionCount(FieldStart, FieldLength, &Count)) {
        return false;
      }
      if (!MeasureFormatSequence(Cursor + 2, &InnerCount, &InnerEnd,
                                 NestingDepth + 1)) {
        return false;
      }
      if (*InnerEnd != ')') {
        return false;
      }

      CopyOffset = 0;
      for (CopyIndex = 0; CopyIndex < Count; CopyIndex++) {
        if (GroupIndex == Components[0]) {
          if (ComponentCount < 2) {
            return false;
          }
          if (Core_AddOverflow(BaseFlatIndex, Offset, &Candidate)) {
            return false;
          }
          if (Core_AddOverflow(Candidate, CopyOffset, &Candidate)) {
            return false;
          }
          if (ResolveSemanticSequence(Cursor + 2, Components + 1,
                                      ComponentCount - 1, Candidate, FieldId,
                                      NestingDepth + 1)) {
            return true;
          }
          return false;
        }
        GroupIndex++;
        if (Core_AddOverflow(CopyOffset, InnerCount, &CopyOffset)) {
          return false;
        }
      }
      if (Core_AddOverflow(Offset, CopyOffset, &Offset)) {
        return false;
      }
      Cursor = InnerEnd + 1;
    } else if ((*Cursor == '\0') || (*Cursor == ')') || (*Cursor == '|')) {
      if (GroupIndex == Components[0]) {
        if (ComponentCount != 1) {
          return false;
        }
        if (Core_AddOverflow(BaseFlatIndex, Offset, FieldId)) {
          return false;
        }
        return true;
      }
      GroupIndex++;
      if (Core_AddOverflow(Offset, 1, &Offset)) {
        return false;
      }
    } else {
      return false;
    }

    if (*Cursor == '|') {
      Cursor++;
      continue;
    }
    return false;
  }
}

// Rejects a retained format that does not describe exactly NumFields fields.
// This completes the prefix-only validation performed by
// ResolveSemanticSequence so the inverse lookup never accepts a malformed
// structural record.
static bool ValidateFormatCopy(const MCICompiledFormat *CompiledFormat) {
  const char *End;
  size_t      FieldCount;

  if (!MeasureFormatSequence(CompiledFormat->FormatCopy, &FieldCount, &End,
                             0)) {
    return false;
  }
  return (*End == '\0') && (FieldCount == CompiledFormat->NumFields);
}

// Appends the semantic path of FieldId to Components. Remaining is the
// zero-based field index relative to the first field in Cursor. FormatCopy has
// already passed ValidateFormatCopy, so field syntax, nesting, and the
// MCISemanticIdMaxComponents bound are known to be valid; this function only
// walks the selected path.
static bool BuildSemanticSequence(const char *Cursor,
                                  size_t      Remaining,
                                  size_t      Components[],
                                  size_t     *ComponentCount) {
  const char *FieldStart;
  const char *InnerEnd;
  size_t      FieldLength;
  size_t      Count;
  size_t      InnerCount;
  size_t      GroupSize;
  size_t      CopyIndex;
  size_t      InnerOffset;
  size_t      Next;
  size_t      GroupIndex = 1;

  for (;;) {
    FieldStart = Cursor;
    while (('0' <= *Cursor) && (*Cursor <= '9')) {
      Cursor++;
    }
    FieldLength = (size_t)(Cursor - FieldStart);

    if (*Cursor == 'R') {
      if (!ParseRepetitionCount(FieldStart, FieldLength, &Count)) {
        return false;
      }
      if (!MeasureFormatSequence(Cursor + 2, &InnerCount, &InnerEnd, 0)) {
        return false;
      }
      if (Core_MulOverflow(Count, InnerCount, &GroupSize)) {
        return false;
      }

      if (Remaining < GroupSize) {
        CopyIndex   = Remaining / InnerCount;
        InnerOffset = Remaining % InnerCount;
        if (Core_AddOverflow(GroupIndex, CopyIndex, &Next)) {
          return false;
        }
        Components[(*ComponentCount)++] = Next;
        return BuildSemanticSequence(Cursor + 2, InnerOffset, Components,
                                     ComponentCount);
      }

      Remaining -= GroupSize;
      if (Core_AddOverflow(GroupIndex, Count, &GroupIndex)) {
        return false;
      }
      Cursor = InnerEnd + 1;
    } else {
      if (Remaining == 0) {
        Components[(*ComponentCount)++] = GroupIndex;
        return true;
      }
      Remaining--;
      if (Core_AddOverflow(GroupIndex, 1, &GroupIndex)) {
        return false;
      }
    }

    // A validated sequence that is not selected here must continue with
    // another element after the '|' separator.
    Cursor++;
  }
}

static size_t DecimalDigitCount(size_t Value) {
  size_t Digits = 1;

  while (Value >= 10) {
    Value /= 10;
    Digits++;
  }
  return Digits;
}

// Writes Value in canonical decimal form at Buffer[*Position]. The caller has
// reserved DecimalDigitCount(Value) bytes for the digits.
static void WriteDecimal(char   *Buffer,
                         size_t *Position,
                         size_t  Value) {
  size_t Cursor = *Position + DecimalDigitCount(Value);

  *Position = Cursor;
  do {
    Buffer[--Cursor] = (char)('0' + (Value % 10));
    Value /= 10;
  } while (Value > 0);
}

size_t GetMCIMessageFieldId(const MCICompiledFormat *CompiledFormat,
                            const char               SemanticId[]) {
  size_t Components[MCISemanticIdMaxComponents];
  size_t ComponentCount;
  size_t FieldId;

  if (!CompiledFormat) {
    return SIZE_MAX;
  }
  if (!SemanticId) {
    return SIZE_MAX;
  }
  if (!CompiledFormat->FormatCopy) {
    return SIZE_MAX;
  }
  if (!ParseSemanticId(SemanticId, Components, &ComponentCount)) {
    return SIZE_MAX;
  }
  if (!ResolveSemanticSequence(CompiledFormat->FormatCopy, Components,
                               ComponentCount, 0, &FieldId, 0)) {
    return SIZE_MAX;
  }
  if (FieldId >= CompiledFormat->NumFields) {
    return SIZE_MAX;
  }
  return FieldId;
}

char *GetMCIMessageSemanticId(const MCICompiledFormat *CompiledFormat,
                              const size_t             FieldId) {
  size_t Components[MCISemanticIdMaxComponents];
  size_t ComponentCount = 0;
  size_t Length         = 0;
  size_t AllocationSize;
  size_t Position = 0;
  size_t i;
  char  *SemanticId;

  if ((!CompiledFormat) || (!CompiledFormat->FormatCopy) ||
      (FieldId >= CompiledFormat->NumFields)) {
    return NULL;
  }
  if (!ValidateFormatCopy(CompiledFormat)) {
    return NULL;
  }
  if (!BuildSemanticSequence(CompiledFormat->FormatCopy, FieldId, Components,
                             &ComponentCount)) {
    return NULL;
  }

  for (i = 0; i < ComponentCount; i++) {
    if (Core_AddOverflow(Length, DecimalDigitCount(Components[i]), &Length)) {
      return NULL;
    }
    if ((i > 0) && Core_AddOverflow(Length, 1, &Length)) {
      return NULL;
    }
  }

  AllocationSize = Length;
  if (Core_AddOverflow(AllocationSize, 1, &AllocationSize)) {
    return NULL;
  }
  if (Core_MulOverflow(AllocationSize, sizeof(char), &AllocationSize)) {
    return NULL;
  }

  SemanticId = (char *)Core_Malloc(AllocationSize);
  if (!SemanticId) {
    return NULL;
  }

  for (i = 0; i < ComponentCount; i++) {
    if (i > 0) {
      SemanticId[Position++] = '.';
    }
    WriteDecimal(SemanticId, &Position, Components[i]);
  }
  SemanticId[Position] = '\0';

  return SemanticId;
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
