# Fortran formatting style

Style used by the Fortran sources (`src/**/*.M90`, `src/**/*.F90`, `test/**/*.M90`, `test/**/*.cf`). The C sources are formatted by `clang-format` from `.clang-format` through `.pre-commit-config.yaml`, which also trims trailing whitespace and fixes the end of file for every text file; no Fortran formatter is configured, so the layout below is applied by hand and a change is compliant when a reviewer can check it against these rules. The rules a machine can decide are encoded in `style/FortranStyleCheck.py`, which reports them and applies the mechanical fixes (section 11).

The rules are derived from the corpus itself and mirror the options of `.clang-format` (`IndentWidth: 2`, `AlignConsecutiveDeclarations`, `AlignConsecutiveAssignments`, `AlignAfterOpenBracket: Align`, `AlignOperands: Align`, `MaxEmptyLinesToKeep: 1`, `SpaceBeforeParens: ControlStatements`, `SpacesBeforeTrailingComments: 1`, `UseTab: Never`), extended to the Fortran constructs that have no C equivalent (continuations, `::`, `enddo`).

## Quick reference

| Item | Rule |
| --- | --- |
| Indent unit | 2 spaces, one level per block |
| Hard column limit | none enforced for Fortran; 80 columns is a soft target |
| Continuation | trailing `&`, aligned after the opening bracket it continues |
| Declarations | type, attributes, `::`, entities; attribute and `::` columns aligned |
| Assignments | runs of assignments align `=` and `=>` |
| Keywords | lower case; C-interoperability names keep their C spelling |
| `end if` / `end do` | never; write `endif` / `enddo` |
| Comments | `!` + text, `!Note:` for notes, `!<` for documentation |
| Closed by | `end <unit> <name>` (the name is never omitted) |

## 1. Files and characters

- One statement per line. `;` never separates statements (it only occurs in comments).
- Plain ASCII inside code. Non-ASCII characters are allowed in comment prose (`src/EINT/ExtrapolationIntegration.M90` writes `∫` in a formula).
- No tab characters, no trailing whitespace, LF line endings.
- The file ends with exactly one newline. A `.M90` header ends with the module terminator, one blank line and `#endif`; a `.cf` suite file ends with the closing `TEST_SUITE(...);` macro directly followed by `#endif`.
- Preprocessor lines start in column 1 and are never re-indented; the `#ifndef` / `#define` / `#endif` guard of a `.M90` header wraps the whole module.

## 2. Indentation

- Two spaces per nesting level: `module` body, `interface` body, procedure body, `if` / `do` / `select` blocks.
- Every statement starts in an even column. An odd column belongs to a continuation line only.
- Statements nested in a block body are indented with respect to the block opener, which is indented with respect to its own container:

```fortran
  module procedure FortranNewMAC
    integer(MACLayout) :: LocalLayout

    if (present(Layout)) then
      if ((Layout == COLUMN_MAJOR) .or. &
          (Layout == ROW_MAJOR)) then
        LocalLayout = Layout
      else
        LocalLayout = COLUMN_MAJOR
      endif
    endif
```

- `contains` sits at the same indentation as the declarations of the program unit, and the contained procedures are indented one level below it.
- `case` labels inside `select case` sit at the indentation of `select case` itself (see `test/Suites/LDW/LDW_F_Structural.cf`).

## 3. Line length

`ColumnLimit: 80` is the one option of `.clang-format` that the Fortran corpus does not follow: the C headers and sources are at zero violations, while Fortran lines may exceed the limit. In Fortran, 80 columns is therefore a **weak rule**. Legibility wins: a continued call, a long `bind(C, name="...")` or a descriptive comment may exceed 80 columns instead of being broken. Do not re-break a line that is already readable, and never pad a line to reach column 80.

## 4. Continuation lines

- `&` closes the line, and a comma precedes it when the statement is a list:

```fortran
    call DiagHermitian(Dimension = Dimension, &
                       Matrix = MatrixFPtr, &
                       Unitary = UnitaryFPtr, &
                       Eigenvalues = EigenvaluesFPtr, &
                       ZHEEVDProcedure = ZHEEVDProcedureFPtr, &
                       Info = InfoFPtr)
```

- A continued line is aligned one column after the opening bracket it continues, so the items line up under the first item of the list.
- A continued declaration list is aligned under the first entity, and a `use ... only:` list is aligned under the first imported name:

```fortran
  use, intrinsic :: ISO_C_BINDING, only: C_PTR, &
                                         C_SIZE_T, &
                                         C_BOOL
```

```fortran
    type(C_PTR) :: MatrixCopyHandle, &
                   EigHandle, &
                   WORKHandle
```

- When no bracket or list is open, the continuation is indented two columns to the right of the statement. This covers broken expressions and the structural clauses of a procedure header (`bind`, `result`):

```fortran
  integer(sk), parameter :: MaxPowerOfTwo = &
    int(ceiling(log(real(huge(1_sk), rk)) / log(2.0_rk)), sk)

    recursive function FortranWeightCallback(FSideCSupport, FSideCOrigin, FSideCDestination, &
                                             FSideCUserContext) &
      bind(C, name="LUGA2_GRTP_WeightCallback") &
      result(FSideCWeight)
```

- A leading `&` on the continuation line is never used.
- Keyword arguments of a continued call align their `=` with each other; the list is aligned as a block, so inserting a longer keyword re-aligns the block.

## 5. Blank lines

- Exactly one blank line separates sections: program units, procedures, type definitions, interface bodies, declarations from executable statements.
- Two consecutive blank lines never occur, and a file never starts or ends with a blank line.
- In a procedure header the dummy arguments, the function result and the local variables form three blocks separated by blank lines:

```fortran
    pure integer(sk) function PowerFactor(Number, PrimeFactor)
      integer(sk), intent(in) :: Number
      integer(sk), intent(in) :: PrimeFactor

      integer(sk) :: Aux

      PowerFactor = 0_sk
```

- The same rule isolates declarations that cannot be aligned with the dummies without a huge gap: the unlimited polymorphic context of a callback and the returned weight get their own block.
- A blank line also separates `import` / `use` statements from the declarations that follow them, and the `contains` statement from the procedure that follows it.

## 6. Declarations

- Shape: type specifier, attributes, `::`, entity list. Attributes are separated by `, ` and never span lines unless the entity list itself is continued.
- Inside one block (no blank line, no comment, no other statement in between) the attribute columns are aligned by index and the `::` column is aligned. Attributes keep a single space after their comma unless the alignment requires more:

```fortran
      class(MAC),        intent(inout) :: Container
      integer(C_SIZE_T), intent(in)    :: Shape(:)

      integer(MACLayout), intent(in), optional :: Layout
```

```fortran
      type(C_PTR),       intent(in), value :: Ptr
      integer(C_SIZE_T), intent(in), value :: NewSize

      type(C_PTR) :: CPointer
```

- When the lines of a block declare initial values, `=` is aligned too:

```fortran
    type(C_PTR) :: OpaqueHandle = C_NULL_PTR
    logical     :: Init         = .false.
    logical     :: Owner        = .true.
```

- Type-bound procedure blocks align `::` and `=>`; `generic` and `final` take part in the same block:

```fortran
    procedure, public, pass(Container) :: Multidimensional => FortranMACFlatToMultidimensional
    procedure                          :: FortranCopyMAC
    generic,   public                  :: assignment(=)    => FortranCopyMAC
    final                              :: FortranFreeMAC_Final
```

- A declaration whose type specifier is a `procedure(...)` pointer that is much longer than the specifiers around it starts its own alignment block, so that it does not push the other declarations to the right. The declaration that follows such a pointer (typically the `class(*)` context of a callback) keeps interior padding when that padding lines its `optional` and `pointer` attributes up with the pointer declaration.
- `import` takes no `::` (`import C_PTR, C_SIZE_T`), and the intrinsic module is named explicitly: `use, intrinsic :: ISO_C_BINDING, only: ...`.
- Access statements keep one name per statement: `public :: C_PTR`, `public :: C_SIZE_T`, ... (not a comma-separated list).
- Kind parameters are lowercase (`rk` for the working real, `sk` for the size kind); C-interoperability kinds keep their standard spelling (`C_SIZE_T`, `C_DOUBLE`, `C_BOOL`).
- Literals carry an explicit kind suffix (`1_sk`, `0.0_rk`, `0_C_SIZE_T`, `.false._C_BOOL`).

## 7. Executable statements

- Consecutive assignments at the same indentation align `=` and `=>`. A run is broken by a blank line, a comment, a different statement, or a change of the guard prefix; it is *not* broken by the continued part of an assignment:

```fortran
  Path             = 42_C_SIZE_T
  Count            = 99_C_SIZE_T
  ShortestDistance = -1.0_C_DOUBLE
  Success          = GraphBidirectionalDijkstra(Forward, Backward, 0_C_SIZE_T, 3_C_SIZE_T, &
                                                ShortestPath = Path, &
                                                ShortestPathVertexCount = Count, &
                                                ShortestDistance = ShortestDistance)
```

```fortran
  Weight  => MirroredTollWeight
  Path    = 42_C_SIZE_T
  Count   = 99_C_SIZE_T
  Success = GraphBidirectionalDijkstra(...)
```

- A one-line `if` keeps its body on the same line, and a run of such lines aligns the assignment of the bodies when the guards are identical:

```fortran
    if (C_ASSOCIATED(Container%OpaqueHandle)) Container%Init  = .true.
    if (C_ASSOCIATED(Container%OpaqueHandle)) Container%Owner = .true.
```

- `if` is separated from its condition by one space (`if (present(Base))`), while a call or an array reference stays tight against its bracket (`EINT(Array = ArrayFPtr)`, `Arr(Index)`).
- Blocks are closed with `endif`, `enddo`, `end select`, `end associate`, `end block <name>`: the two-word forms `end if` / `end do` are never used.
- Every named program unit is closed with its name: `end subroutine EINT_C`, `end function IsPrime`, `end procedure EINT`, `end module CoreFortran`, `end submodule EINT_C_Interfaces`, `end type MAC`. `end interface` and `end enum` carry no name.

## 8. Operators and separators

- Binary operators are surrounded by one space: `+`, `-`, `*`, `/`, `//`, `==`, `/=`, `<`, `<=`, `>`, `>=`, `.and.`, `.or.`, `.eqv.`, `.neqv.`.
- Operators that bind tighter than their neighbours stay tight: `**`, unary `-`, `%`, and `.not.` in front of its operand (`.not.C_ASSOCIATED(Handle)`).
- `,` is followed by exactly one space (or by the line end); it is never preceded by a space. `, &` closes a continued list item.
- `::` has exactly one space on each side. Keyword arguments are written `Keyword = Value`, including the kind of an intrinsic call (`cmplx(Matrix, KIND = REAL64)`).
- Array constructors, matrices and dense expressions may use extra spaces when they carry visual structure; those blocks are written by hand and only the `=` columns are aligned:

```fortran
  H(1, :)   = [ 3*Cmplx1,  1*CmplxI, 1*Cmplx1]
  H(2, :)   = [-1*CmplxI,  3*Cmplx1, 1*CmplxI]
  H(3, :)   = [ 1*Cmplx1, -1*CmplxI, 3*Cmplx1]
  HOriginal = H
```

## 9. Comments and documentation

Three kinds of comment exist; the marker identifies the kind.

- `!<` — documentation. It documents the entity that follows (module constants, types, interfaces, procedures, sections of a file). One sentence per line, a space after `!<`, and a blank `!<` line between paragraphs:

```fortran
  !< Sentinel value representing the distance to an unreachable vertex.
  !< Used by shortest-path algorithms to indicate that no parent vertex
  !< exists for a particular vertex.
  !< Values are provided by the C implementation.
  real(C_DOUBLE), protected, bind(C, name="C_INFINITY"), public :: INFINITY
```

- `!Note:` — a behavioural note attached to an interface or a procedure. The continuation of a note is indented so that its text starts under the text of the `!Note:` line:

```fortran
    !Note: UserContext must be declared CLASS(*), POINTER: a less polymorphic
    !      actual is rejected by some compilers for an INTENT(INOUT) unlimited
    !      polymorphic dummy.
```

- `!` — implementation comment or section label. No space after `!`: `!Fast return.`, `!Compute the trapezoidal sums.`, `!Kind aliases.`
- Section banners inside a submodule use `!<` and are surrounded by blank lines, for example `!< GRAPH ALGORITHMS - WORKSPACE.` in `src/GRTP/GRTP_C_Int.M90`.

Other rules:

- A comment line is indented like the code it belongs to.
- A trailing comment is separated from the code by exactly one space: `CCtx = ContextData !Unused.`
- Comments end with a period and use sentence case.
- Comment blocks never contain a line with a trailing space, and the `!` column of a block never moves.

## 10. Exemptions

The following constructs are intentionally left as they are; a reviewer must not "fix" them:

- C macro invocations of the test framework (`TEST_SUITE(...)`, `TSD_TEST_E_INTERFACES`): their arguments use C layout, at the column where the macro opened its parenthesis.
- `!$OMP` directives: their clauses keep the OpenMP spelling, and the directive indent follows the code it applies to.
- ASCII diagrams in comments (`test/Suites/GRTP/GRTP_Algorithms_F_Functional.cf` draws graphs with spaces inside the comment).
- Lines wider than 80 columns (weak rule, see section 3).
- The hand-laid matrix and array-constructor blocks of the LDW tests, where the spacing encodes the mathematical structure.

## 11. Checking

`style/FortranStyleCheck.py` encodes the rules a machine can decide and reports every violation with the identifier of the rule and the column it expected:

```sh
python3 style/FortranStyleCheck.py                    # the whole corpus
python3 style/FortranStyleCheck.py src/TBCI           # one module
python3 style/FortranStyleCheck.py --fix src/TBCI     # apply the mechanical fixes
python3 style/FortranStyleCheck.py --rules            # list the rules
```

The checker reads `.M90` and `.F90` as Fortran; in a `.cf` suite it reads the Fortran bodies of the `#ifdef TSD_FORTRAN_IMPL` regions and leaves the C part and the framework macros to `clang-format`. The weak rules (the 80-column limit, the period of a comment) are warnings: they are reported but do not change the exit status. The exit status is 0 when no violation is found.

`--fix` applies the rewrites that have a single answer: the alignment of the continuations, of the `::` and attribute columns, of the assignment operators and of the keywords of a continued call; the spacing of the operators and separators; the one-line form of `if (...)`; `endif` and `enddo`; the comment markers; the trailing whitespace and the blank lines. It never renames an identifier, never re-breaks a line and never touches a construct of section 10, so running it twice changes nothing; review the diff anyway, because a fix can move a comment or expose a line that a human should re-wrap.
