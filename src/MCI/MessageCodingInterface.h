#ifndef MessageCodingInterface_CH
#define MessageCodingInterface_CH

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Bit ordering used when packing fields into a message.
 *
 * @details
 * With MSB_FIRST, packing starts at the most significant bit of the first
 * message byte and advances toward the least significant bit; each field is
 * stored most significant bit first. With LSB_FIRST, packing starts at the
 * least significant bit of the first message byte and advances toward the most
 * significant bit; each field is stored least significant bit first. In both
 * layouts, fields are packed consecutively in the order in which they appear
 * in the compiled format.
 *
 * @note
 * - The layout is selected independently for each encode or decode operation.
 * - Bytes are always interpreted in memory order.
 */
typedef enum {
  MSB_FIRST = 1,
  LSB_FIRST = 2
} EncodingLayout;

/**
 * @brief Opaque description of a compiled message format.
 *
 * @details
 * Stores the number of fields, the bit width and bit offset of each field, and
 * the total message size derived from the format string. Repetition groups are
 * expanded when the format is compiled, so each repeated element receives its
 * own field index.
 *
 * @note
 * - The internal representation is hidden from callers.
 * - Instances are created by CompileMCIFormat and released by FreeMCIFormat.
 * - A compiled format is immutable and may be reused for any number of
 *   messages.
 *
 * @see CompileMCIFormat
 * @see FreeMCIFormat
 */
typedef struct MCICompiledFormat MCICompiledFormat;

/**
 * @brief Releases a compiled message format.
 *
 * @details
 * Releases the field metadata and the MCICompiledFormat object. The pointer
 * referenced by CompiledFormatReference is set to NULL after a successful
 * release.
 *
 * @param[in,out] CompiledFormatReference
 *     Reference to the compiled format pointer to release.
 *
 * @note
 * - Safe to call with NULL.
 * - Safe to call when the referenced pointer is NULL.
 * - The caller must not use the released object after this call.
 */
void FreeMCIFormat(MCICompiledFormat **CompiledFormatReference);

/**
 * @brief Compiles a textual field format into a reusable description.
 *
 * @details
 * Parses Format and records the bit width and bit offset of every field. The
 * accepted grammar is:
 *
 *     Format  ::= Element ('|' Element)*
 *     Element ::= Digits ['R' '(' Format ')']
 *
 * Digits is a non-empty decimal integer. Each number is a field width in bits.
 * For example, "8|4|2R(3)" compiles to four fields of widths 8, 4, 3, and 3.
 * The syntax is exact: R is uppercase, whitespace is not allowed, and empty
 * groups are rejected. Field widths and repetition counts must be greater than
 * zero, field widths must not exceed MaxFieldBitSize, repetition counts must
 * not exceed 64, and repetition nesting is limited to 64 levels. Leading zeros
 * in a field width are accepted and do not change its value. A repetition
 * expands to Count copies of its inner sequence, separated by '|'. Arithmetic
 * overflow and allocation failure are reported as errors.
 *
 * @param[in] Format
 *     NUL-terminated format string to compile. The string is not modified and
 *     is not retained by the returned object.
 *
 * @param[in] MaxFieldBitSize
 *     Maximum permitted width, in bits, of any expanded field. It must leave
 *     2^MaxFieldBitSize representable in both size_t and long.
 *
 * @return
 *     Pointer to a newly allocated compiled format on success, or NULL when
 *     Format is invalid, an expansion or size overflows, or allocation fails.
 *
 * @note
 * - The returned object is owned by the caller and must be released with
 *   FreeMCIFormat.
 * - A field width of one bit represents values 0 and 1; an n-bit field
 *   represents values from 0 through 2^n - 1.
 * - The cost of compiling is proportional to the expanded format length, which
 *   may be much larger than the source string when repetition is used.
 *
 * @see FreeMCIFormat
 * @see GetMCIMessageByteSize
 * @see GetMCIMessageFieldCount
 */
MCICompiledFormat *CompileMCIFormat(const char   Format[],
                                    const size_t MaxFieldBitSize);

/**
 * @brief Returns the minimum message size required by a compiled format.
 *
 * @details
 * The size is the total number of field bits rounded up to a whole number of
 * bytes. It is the smallest value that may be passed as MessageByteSize to
 * EncodeMCIMessage or DecodeMCIMessage for this format.
 *
 * @param[in] CompiledFormat
 *     Pointer to the compiled format.
 *
 * @return
 *     Minimum message size in bytes, or 0 when CompiledFormat is NULL.
 *
 * @note
 * - A valid compiled format always requires at least one byte.
 * - Larger buffers are accepted by the encode and decode operations; only the
 *   first GetMCIMessageByteSize bytes participate in the message.
 *
 * @see GetMCIMessageFieldCount
 * @see EncodeMCIMessage
 * @see DecodeMCIMessage
 */
size_t GetMCIMessageByteSize(const MCICompiledFormat *CompiledFormat);

/**
 * @brief Returns the number of fields in a compiled format.
 *
 * @details
 * Counts expanded fields, so every element produced by a repetition group is
 * counted separately. The returned value is the exclusive upper bound for the
 * FieldId argument of EncodeMCIMessage and DecodeMCIMessage.
 *
 * @param[in] CompiledFormat
 *     Pointer to the compiled format.
 *
 * @return
 *     Number of fields, or 0 when CompiledFormat is NULL.
 *
 * @note
 * - A valid compiled format always contains at least one field.
 * - The value is fixed for the lifetime of the compiled format.
 *
 * @see EncodeMCIMessage
 * @see DecodeMCIMessage
 * @see GetMCIMessageFieldId
 */
size_t GetMCIMessageFieldCount(const MCICompiledFormat *CompiledFormat);

/**
 * @brief Resolves a semantic field identifier to a flat field index.
 *
 * @details
 * Semantic identifiers address a field through the expanded group structure of
 * the source format, using one-based decimal components separated by periods.
 * At each format level, every plain field and every repetition copy is one
 * group, numbered from one in expanded left-to-right order. A plain field is
 * selected by its group number alone. A repetition copy is selected by its
 * group number followed by the path to a field inside its inner sequence.
 *
 * For example, "7|2R(3|5)" expands to five fields with flat indices 0 through
 * 4. The semantic identifiers are:
 *
 *     "1"   -> 0    (the field of width 7)
 *     "2.1" -> 1    (first copy of the group, first inner field)
 *     "2.2" -> 2    (first copy of the group, second inner field)
 *     "3.1" -> 3    (second copy of the group, first inner field)
 *     "3.2" -> 4    (second copy of the group, second inner field)
 *
 * Nested repetitions extend the path with one component per level. For
 * example, "2R(3R(4|5))" contains identifiers such as "1.1.1" and "1.2.2".
 *
 * @param[in] CompiledFormat
 *     Pointer to the compiled format.
 *
 * @param[in] SemanticId
 *     NUL-terminated semantic field identifier. Components must be positive
 *     decimal integers without leading zeros.
 *
 * @return
 *     Zero-based flat field index on success, or SIZE_MAX when an argument is
 *     invalid, the identifier is malformed, or no field matches the identifier.
 *
 * @note
 * - Semantic components are one-based; the returned field index is zero-based.
 * - A repeated group is not a leaf, so a field inside "2R(7)" is addressed as
 *   "1.1" or "2.1", not as "1" or "2".
 * - The lookup parses the retained source format and does not allocate. Callers
 *   that resolve the same identifier repeatedly may cache the result.
 * - A successful result is always smaller than GetMCIMessageFieldCount.
 *
 * @see GetMCIMessageFieldCount
 * @see EncodeMCIMessage
 * @see DecodeMCIMessage
 */
size_t GetMCIMessageFieldId(const MCICompiledFormat *CompiledFormat,
                            const char               SemanticId[]);

/**
 * @brief Encodes one field value into a message buffer.
 *
 * @details
 * Writes Value into the bit range assigned to FieldId. The field range and
 * value are validated before any bit is written. Fields are numbered from zero
 * in expanded format order, so repeated elements have distinct identifiers.
 * Bits outside the selected field are left unchanged.
 *
 * @param[in] CompiledFormat
 *     Pointer to the compiled format.
 *
 * @param[in] MessageByteSize
 *     Number of bytes available in Message. It must be at least
 *     GetMCIMessageByteSize(CompiledFormat).
 *
 * @param[in,out] Message
 *     Message buffer receiving the field. The selected bits are overwritten;
 *     all other bits, including the unused padding bits of the final byte, are
 *     preserved.
 *
 * @param[in] FieldId
 *     Zero-based index of the field to encode. It must be smaller than the
 *     number of fields in CompiledFormat.
 *
 * @param[in] Encoding
 *     Bit ordering used for the field.
 *
 * @param[in] Value
 *     Unsigned value to encode. It must be smaller than 2^BitSize, where
 *     BitSize is the width of the selected field.
 *
 * @return
 *     true on success, or false when an argument is invalid, FieldId is out of
 *     range, the buffer is too small, or Value does not fit the field. In that
 *     case Message is not modified.
 *
 * @note
 * - The function writes exactly the bits of the selected field.
 * - Message must point to at least MessageByteSize writable bytes.
 * - The format is immutable, so concurrent calls that use distinct message
 *   buffers are safe.
 *
 * @see DecodeMCIMessage
 */
bool EncodeMCIMessage(const MCICompiledFormat *CompiledFormat,
                      size_t                   MessageByteSize,
                      uint8_t                  Message[MessageByteSize],
                      size_t                   FieldId,
                      EncodingLayout           Encoding,
                      size_t                   Value);

/**
 * @brief Decodes one field value from a message buffer.
 *
 * @details
 * Reads the bits assigned to FieldId and returns their unsigned value. The
 * message is not modified. Bits outside the selected field, including padding
 * bits, are ignored.
 *
 * @param[in] CompiledFormat
 *     Pointer to the compiled format.
 *
 * @param[in] MessageByteSize
 *     Number of bytes available in Message. It must be at least
 *     GetMCIMessageByteSize(CompiledFormat).
 *
 * @param[in] Message
 *     Message buffer containing the encoded field.
 *
 * @param[in] FieldId
 *     Zero-based index of the field to decode. It must be smaller than the
 *     number of fields in CompiledFormat.
 *
 * @param[in] Encoding
 *     Bit ordering used by the field.
 *
 * @param[out] Value
 *     Pointer receiving the decoded unsigned value. It is written only on
 *     success; otherwise it is left unchanged.
 *
 * @return
 *     true on success, or false when an argument is invalid, FieldId is out of
 *     range, the buffer is too small, or the field cannot be represented by
 *     size_t.
 *
 * @note
 * - Message must point to at least MessageByteSize readable bytes.
 * - A field wider than size_t is rejected rather than truncated.
 * - The format is immutable, so concurrent calls that use distinct message
 *   buffers are safe.
 *
 * @see EncodeMCIMessage
 */
bool DecodeMCIMessage(const MCICompiledFormat *CompiledFormat,
                      size_t                   MessageByteSize,
                      const uint8_t            Message[MessageByteSize],
                      size_t                   FieldId,
                      EncodingLayout           Encoding,
                      size_t                  *Value);

#endif
