#!/usr/bin/env python3
"""Enforce ``style/FortranFormatting.md`` on the Fortran sources of LUGA2.

The formatting guide is written for human reviewers; this script encodes the
part of it that a machine can decide, so that a violation is caught before the
review.  Two kinds of file are understood:

* ``*.M90`` and ``*.F90`` are Fortran with preprocessor lines that start in
  column one;
* ``*.cf`` are C suites that embed Fortran test bodies inside
  ``#ifdef TSD_FORTRAN_IMPL`` regions.  Only those regions are checked, and the
  framework macros whose arguments are not Fortran (``TEST_E``,
  ``TSD_TEST_E_INTERFACES``, ...) are skipped.

Every violation carries the identifier of the rule it breaks; the identifiers
are listed by ``--rules``.  Rules whose verdict needs judgement (the weak
80-column limit, the period of a comment) are reported as warnings and do not
change the exit status.

Usage::

    python3 style/FortranStyleCheck.py                       # whole corpus
    python3 style/FortranStyleCheck.py src/TBCI              # a directory
    python3 style/FortranStyleCheck.py --fix src/TBCI test   # apply the fixes
    python3 style/FortranStyleCheck.py --rules               # list the rules

The exit status is 0 when nothing is reported, 1 when a violation is found.
"""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

REPOSITORY = Path(__file__).resolve().parent.parent

FORTRAN_SUFFIXES = (".M90", ".F90")
SUITE_SUFFIX = ".cf"

#: Framework macros of ``test/TSD`` that expand to C or to a whole program unit.
EXEMPT_MACROS = frozenset(
    {
        "ASSERT",
        "ASSERT_X",
        "ASSERT_NOEXIT_WRAP",
        "ASSERT_EXIT_WRAP",
        "CONTEXT",
        "CONTEXT_FN",
        "END_TEST_E",
        "END_TEST_EF",
        "TEST",
        "TEST_E",
        "TEST_EF",
        "TEST_F",
        "TEST_SUITE",
        "TSD_TEST_E_INTERFACES",
    }
)

#: Macros of the framework whose argument is Fortran code and is therefore
#: formatted like code; every other macro of ``EXEMPT_MACROS`` is left alone.
CODE_MACROS = frozenset({"ASSERT", "ASSERT_X"})

#: Keywords that take a space before their opening parenthesis.
SPACED_PAREN_KEYWORDS = frozenset(
    {
        "if",
        "elseif",
        "while",
        "where",
        "elsewhere",
        "forall",
        "case",
        "select",
        "associate",
        "do",
    }
)

#: Fortran keywords that are never a call or an array reference, so a space
#: before their parenthesis is not the ``Name (`` form the guide forbids.
#: ``is`` belongs here for ``type is (real(rk))``, the guard of ``select type``.
TIGHT_PAREN_KEYWORDS = frozenset(
    {
        "allocate",
        "backspace",
        "bind",
        "block",
        "call",
        "character",
        "class",
        "close",
        "complex",
        "default",
        "deallocate",
        "dimension",
        "double",
        "elemental",
        "end",
        "enum",
        "enumerator",
        "extends",
        "external",
        "final",
        "flush",
        "function",
        "generic",
        "implicit",
        "import",
        "impure",
        "inquire",
        "integer",
        "intent",
        "interface",
        "intrinsic",
        "is",
        "kind",
        "len",
        "logical",
        "module",
        "namelist",
        "nullify",
        "open",
        "optional",
        "parameter",
        "pass",
        "pointer",
        "print",
        "procedure",
        "program",
        "pure",
        "read",
        "real",
        "recursive",
        "result",
        "return",
        "rewind",
        "save",
        "submodule",
        "subroutine",
        "target",
        "type",
        "use",
        "value",
        "volatile",
        "wait",
        "write",
    }
)

#: Rules reported as warnings: their verdict needs a human.
WARNING_RULES = frozenset({"len/80", "doc/period"})

RULE_SUMMARY = (
    ("chars/tab", "a tab character occurs (section 1)"),
    ("chars/trailing-space", "a line ends with blanks (section 1)"),
    ("chars/eol", "the file is not CRLF-free (section 1)"),
    ("chars/final-newline", "the file does not end with one newline (section 1)"),
    ("chars/ascii", "a non-ASCII character occurs outside a comment (section 1)"),
    ("chars/semicolon", "a semicolon separates statements (section 1)"),
    ("chars/preproc-column", "a preprocessor line is indented (section 1)"),
    ("blank/leading", "the file starts with a blank line (section 5)"),
    ("blank/consecutive", "two blank lines follow each other (section 5)"),
    ("preproc/tail", "a .M90 header does not end with a blank line and #endif (section 1)"),
    ("preproc/guard", "the include guard of a .M90 header is malformed (section 1)"),
    ("suite/tail", "a .cf suite does not end with its TEST_SUITE and #endif (section 1)"),
    ("indent/odd", "a statement starts in an odd column (section 2)"),
    ("cont/leading-amp", "a continuation line starts with & (section 4)"),
    ("cont/align", "a continuation is not aligned after the bracket it continues (section 4)"),
    ("cont/keyword-eq", "the keyword arguments of a continued call do not align (section 4)"),
    ("decl/align-colons", "the :: column is not aligned within a declaration block (section 6)"),
    ("decl/align-attrs", "the attribute columns are not aligned within a block (section 6)"),
    ("decl/align-values", "the = of an initializer is not aligned within a block (section 6)"),
    ("decl/access-list", "an access statement carries more than one name (section 6)"),
    ("decl/import-colons", "import carries :: (section 6)"),
    ("decl/use-intrinsic", "ISO_C_BINDING is used without intrinsic (section 6)"),
    ("stmt/tight-paren", "a call or an array reference is separated from its bracket (section 7)"),
    ("stmt/spaced-paren", "a control statement is tight against its condition (section 7)"),
    ("stmt/assign-align", "consecutive assignments do not align their = (section 7)"),
    ("blk/end-if", "end if or end do is used instead of endif or enddo (section 7)"),
    ("blk/end-name", "a named program unit is closed without its name (section 7)"),
    ("blk/end-anonymous", "end interface or end enum carries a name (section 7)"),
    ("ops/binary-space", "a binary operator is not surrounded by one space (section 8)"),
    ("ops/comma", "a comma is not followed by exactly one space (section 8)"),
    ("ops/colons", ":: is not surrounded by one space (section 8)"),
    ("ops/equal", "= is not surrounded by one space (section 8)"),
    ("ops/percent", "the component selector % is not tight (section 8)"),
    ("ops/not", ".not. is not tight against its operand (section 8)"),
    ("doc/dash-space", "an implementation comment has a space after ! (section 9)"),
    ("doc/doc-space", "a documentation comment has no space after !< (section 9)"),
    (
        "doc/note-continuation",
        "the continuation of a !Note: is not indented under its text (section 9)",
    ),
    ("doc/period", "a comment does not end with a period (section 9)"),
    ("len/80", "the line is longer than 80 columns (section 3, weak)"),
)

#: A declaration: ``type specifier, attributes :: entities``. ``generic`` and
#: ``final`` are the type-bound statements that share a block with ``procedure``.
DECLARATION = re.compile(
    r"^(?P<type>integer|real|complex|logical|character|type|class|procedure|enumerator|double|"
    r"generic|final)\b(?P<rest>.*)$",
    re.IGNORECASE,
)

#: An assignment or a pointer assignment, with the left-hand side bracketed.
ASSIGNMENT = re.compile(
    r"^(?P<lhs>[A-Za-z_]\w*(?:%[A-Za-z_]\w*|\([^()]*\))*)\s*(?P<op>=|=>)\s*(?P<rhs>.*)$"
)

#: A one-line ``if`` guard, whose body is aligned only within equal guards.
GUARDED = re.compile(r"^(?P<guard>if\s*\(.*\))\s+(?P<body>\S.*)$")

#: A unit header, used to learn the name an ``end`` statement must repeat.
UNIT_HEADER = re.compile(
    r"^(?:(?:pure|elemental|impure|recursive|module|abstract)\s+)*"
    r"(?P<unit>module|submodule|program|subroutine|function|procedure|type|interface|enum)\b"
    r"(?:\s*\([^()]*\))?\s*(?:,\s*\w+)*\s*(?P<name>\w+)?",
    re.IGNORECASE,
)

#: Program units that the guide requires to be closed with their name.
NAMED_UNITS = ("module", "submodule", "program", "subroutine", "function", "procedure", "type")

#: Every token the spacing rules of section 8 talk about.
SPACING_TOKEN = re.compile(
    r"\.(?:not|and|or|eqv|neqv|lt|le|gt|ge|eq|ne|xor)\.|\*\*|//|==|/=|<=|>=|=>|::|[+\-*/<>=%,]",
    re.IGNORECASE,
)

#: A control statement, which takes one space before its condition.
CONTROL_PAREN = re.compile(
    r"\b(if|elseif|where|elsewhere|forall|case|critical|while)\(",
    re.IGNORECASE,
)

#: The ``assignment(...)``, ``operator(...)`` and ``bind(...)`` spans.
PROTECTED_CALL = re.compile(r"\b(?:assignment|operator|bind)\s*\([^)]*\)", re.IGNORECASE)

#: Spans the operator rules leave alone: array constructors, ``assignment(=)``,
#: ``operator(<)`` and the keyword arguments of ``bind``.
PROTECTED = re.compile(r"\[[^\]]*\]|" + PROTECTED_CALL.pattern, re.IGNORECASE)

#: An initializer or a type-bound binding: ``=`` or ``=>``, never ``==``.
INITIALIZER = re.compile(r"=>|(?<![=<>/])=(?!=)")


def protected_spans(code: str) -> list[tuple[int, int]]:
    """Return the character spans that the operator rules do not police."""
    return [(match.start(), match.end()) for match in PROTECTED.finditer(code)]


def previous_nonspace(code: str, index: int) -> str:
    """Return the last character before ``index`` that is neither blank nor EOL."""
    position = index - 1
    while position >= 0 and code[position] in " \n":
        position -= 1
    return code[position] if position >= 0 else ""


@dataclass


class Violation:
    """One broken rule at one line."""

    path: Path
    line: int
    rule: str
    message: str

    @property
    def warning(self) -> bool:
        return self.rule in WARNING_RULES

    def __str__(self) -> str:
        return f"{self.path}:{self.line}: {self.rule}: {self.message}"


def split_code_comment(text: str) -> tuple[str, str]:
    """Split a Fortran line into its code and its comment (both raw)."""
    quote = ""
    index = 0
    while index < len(text):
        char = text[index]
        if quote:
            if char == quote:
                if index + 1 < len(text) and text[index + 1] == quote:
                    index += 2
                    continue
                quote = ""
        elif char in "'\"":
            quote = char
        elif char == "!":
            return text[:index], text[index:]
        index += 1
    return text, ""


def bracket_stack(code: str, columns: list[int], depth: list[tuple[str, int]]) -> None:
    """Update ``depth`` with the parentheses opened and closed by ``code``.

    Square brackets are ignored: the items of an array constructor are not
    aligned under the ``[`` but indented two columns, as the dense expressions
    of section 8 do.
    """
    for offset, char in enumerate(code):
        if char == "(":
            depth.append((char, columns[offset]))
        elif char == ")" and depth:
            depth.pop()


def first_token(code: str) -> str:
    """Return the leading identifier of a statement, upper case."""
    match = re.match(r"\s*([A-Za-z_]\w*)", code)
    return match.group(1).upper() if match else ""


def indentation(text: str) -> int:
    """Return the number of leading spaces of ``text``."""
    return len(text) - len(text.lstrip(" "))


@dataclass


class Statement:
    """A logical statement: its lines, its code and its nesting."""

    first: int
    last: int
    indent: int
    codes: list[str] = field(default_factory=list)
    lines: list[int] = field(default_factory=list)

    @property
    def code(self) -> str:
        """The statement with its continuations joined by one space."""
        return " ".join(part.strip().rstrip("&").strip() for part in self.codes)

    @property
    def macro(self) -> bool:
        """True when the statement is a framework macro."""
        return first_token(self.codes[0]) in EXEMPT_MACROS

    @property
    def exempt(self) -> bool:
        """True when the statement is a macro that the guide does not format."""
        return self.macro and first_token(self.codes[0]) not in CODE_MACROS


@dataclass


class Source:
    """A file split into its lines, with the Fortran region of a .cf known."""

    path: Path
    lines: list[str]
    fortran: list[bool]
    text: str

    @classmethod
    def load(cls, path: Path) -> "Source":
        raw = path.read_bytes()
        text = raw.decode("utf-8", errors="replace")
        if "\r\n" in text:
            text = text.replace("\r\n", "\n")
        lines = text.split("\n")
        if lines and lines[-1] == "":
            lines.pop()
        fortran = cls._fortran_mask(path, lines)
        return cls(path=path, lines=lines, fortran=fortran, text=text)

    @staticmethod
    def _fortran_mask(path: Path, lines: list[str]) -> list[bool]:
        """Mark the lines that hold Fortran, skipping the C parts of a suite."""
        if path.suffix in FORTRAN_SUFFIXES:
            return [True] * len(lines)
        mask: list[bool] = []
        conditions: list[str | None] = []
        for line in lines:
            directive = re.match(r"\s*#\s*(ifdef|ifndef|if|elif|else|endif)\b\s*(\w*)", line)
            if directive:
                kind, name = directive.group(1), directive.group(2)
                if kind in ("ifdef", "ifndef", "if"):
                    conditions.append(name if kind != "if" else None)
                elif kind == "else" and conditions:
                    conditions[-1] = None
                elif kind == "endif" and conditions:
                    conditions.pop()
                mask.append(False)
                continue
            mask.append("TSD_FORTRAN_IMPL" in [c for c in conditions if c])
        return mask

    def is_code(self, index: int) -> bool:
        """True when the one-based line index holds Fortran code or comment."""
        return 0 < index <= len(self.lines) and self.fortran[index - 1]


@dataclass


class LineView:
    """One source line, split into Fortran code and comment."""

    number: int
    text: str
    code: str
    comment: str
    fortran: bool

    @property
    def blank(self) -> bool:
        return not self.text.strip()

    @property
    def comment_only(self) -> bool:
        return bool(self.text.strip()) and not self.code.strip()

    @property
    def preproc(self) -> bool:
        return self.text.lstrip().startswith("#")

    @property
    def indent(self) -> int:
        return indentation(self.text)


def mask_strings(code: str) -> str:
    """Blank the contents of the string literals of ``code``."""
    out: list[str] = []
    quote = ""
    index = 0
    while index < len(code):
        char = code[index]
        if quote:
            if char == quote:
                if index + 1 < len(code) and code[index + 1] == quote:
                    out.append("xx")
                    index += 2
                    continue
                quote = ""
                out.append(char)
            else:
                out.append("x")
        elif char in "'\"":
            quote = char
            out.append(char)
        else:
            out.append(char)
        index += 1
    return "".join(out)


def group_statements(views: list[LineView]) -> list[Statement]:
    """Group the Fortran lines into logical statements."""
    statements: list[Statement] = []
    index = 0
    while index < len(views):
        view = views[index]
        if not view.fortran or view.blank or view.comment_only or view.preproc:
            index += 1
            continue
        statement = Statement(first=view.number, last=view.number, indent=view.indent)
        statement.codes.append(view.code)
        statement.lines.append(view.number)
        if first_token(view.code) in EXEMPT_MACROS:
            depth = 0
            while True:
                depth += view.code.count("(") - view.code.count(")")
                if depth <= 0 or index + 1 >= len(views):
                    break
                index += 1
                view = views[index]
                statement.codes.append(view.code)
                statement.lines.append(view.number)
        else:
            while view.code.rstrip().endswith("&") and index + 1 < len(views):
                index += 1
                view = views[index]
                if not view.fortran:
                    break
                statement.codes.append(view.code)
                statement.lines.append(view.number)
        statement.last = statement.lines[-1]
        statements.append(statement)
        index += 1
    return statements


def continuation_column(codes: list[str], indent: int, position: int) -> int:
    """Return the one-based column where continuation ``position`` must start.

    The guide aligns a continuation one column after the opening bracket it
    continues, under the first entity of a declaration or of a ``use ... only:``
    list, and two columns right of the statement when neither applies.
    """
    depth: list[tuple[str, int]] = []
    for code in codes[:position]:
        masked = mask_strings(code)
        columns = [offset + 1 for offset in range(len(masked))]
        bracket_stack(masked, columns, depth)
    if depth:
        return depth[-1][1] + 1
    first = mask_strings(codes[0]).rstrip()
    if first.endswith("&"):
        first = first[:-1].rstrip()
    if first.endswith(","):
        only = re.search(r"only\s*:\s*\S", first)
        if only is not None:
            return only.end()
        declarations = re.search(r"::\s*\S", first)
        if declarations is not None:
            return declarations.end()
    return indent + 3


def is_exponent(code: str, index: int) -> bool:
    """True when the sign at ``index`` belongs to a real literal exponent."""
    if index == 0 or code[index - 1] not in "eEdD":
        return False
    return index >= 2 and (code[index - 2].isdigit() or code[index - 2] == ".")


def gaps(text: str, start: int, end: int) -> tuple[int, int, bool, bool]:
    """Return the blanks around ``text[start:end]`` and whether a side exists.

    A side that runs into the end of the line does not exist: the indentation of
    a continuation line is not the left gap of the operator that starts it.
    """
    left = 0
    while start - left - 1 >= 0 and text[start - left - 1] == " ":
        left += 1
    right = 0
    while end + right < len(text) and text[end + right] == " ":
        right += 1
    has_left = start - left > 0 and text[start - left - 1] != "\n"
    has_right = end + right < len(text) and text[end + right] != "\n"
    return left, right, has_left, has_right


def satisfies(count: int, desired: int, exact: bool) -> bool:
    """Tell whether ``count`` blanks satisfy the wanted count."""
    return count == desired if exact else count >= desired


def gap_target(current: int, desired: int | None, exact: bool) -> int:
    """Return the blanks the fixer writes around an operator.

    ``desired`` of ``None`` keeps the blanks the line already has; an exact
    target replaces them, a minimum only widens them.
    """
    if desired is None:
        return current
    return desired if exact else max(current, desired)


def pad_to(text: str, column: int) -> str:
    """Pad ``text`` so that the next character starts in ``column``.

    A block whose prefix is already past ``column`` keeps one blank, so that the
    comma or the ``::`` that follows still separates two tokens.
    """
    return text.ljust(column - 1) if len(text) < column - 1 else text + " "


def spacing_rule(operator: str) -> tuple[str, str]:
    """Return the identifier and the spelling of the rule of an operator."""
    if operator == "=":
        return "ops/equal", "="
    if operator == ",":
        return "ops/comma", "the comma"
    if operator == "::":
        return "ops/colons", "::"
    if operator == "%":
        return "ops/percent", "%"
    if operator.lower() == ".not.":
        return "ops/not", ".not."
    return "ops/binary-space", operator


def operator_gaps(text: str, match: re.Match[str]) -> tuple[int | None, int | None] | None:
    """Return the blanks the operator needs on its left and on its right.

    ``None`` on one side means that the side is not policed, ``None`` as the
    whole result means that the token is not an operator at all.
    """
    operator = match.group()
    index = match.start()
    lowered = operator.lower()
    after = text[match.end() : match.end() + 1]
    before = previous_nonspace(text, index)
    if lowered == ".not.":
        return None, 0
    if lowered.startswith(".") and lowered.endswith("."):
        return 1, 1
    if operator in ("**", "%"):
        return 0, 0
    if operator == ",":
        return 0, 1
    if operator in ("=", "::", "==", "/=", "<=", ">=", "=>", "//", "<", ">"):
        return 1, 1
    if operator in ("+", "-"):
        if is_exponent(text, index):
            return None
        if before in ("", "(", "[", ",", "=", "+", "-", "*", "/", "<", ">", ":", "&"):
            return None, 0
        return 1, 1
    if operator == "/":
        if before == "(" or after == ")":
            return None
        return 1, 1
    if operator == "*":
        if before in ("(", ",") and after == ")":
            return None
        return 1, 1
    return 1, 1


def split_top_level(text: str, separator: str) -> list[tuple[str, int]]:
    """Split ``text`` on ``separator`` outside brackets, keeping the offsets."""
    parts: list[tuple[str, int]] = []
    depth = 0
    start = 0
    for offset, char in enumerate(text):
        if char in "([":
            depth += 1
        elif char in ")]":
            depth -= 1
        elif char == separator and depth == 0:
            parts.append((text[start:offset], start))
            start = offset + 1
    parts.append((text[start:], start))
    return parts


def split_declaration(code: str) -> tuple[str, list[tuple[str, int]], tuple[str, int]]:
    """Return the type specifier, the attributes and the entities of a line."""
    head, separator, entities = code.partition("::")
    if not separator:
        return head.strip(), [], ("", 0)
    segments = split_top_level(head, ",")
    type_spec = segments[0][0].strip()
    attributes = []
    for part, offset in segments[1:]:
        lead = len(part) - len(part.lstrip(" "))
        attributes.append((part.strip(), offset + lead))
    offset = len(head) + 2
    while offset < len(code) and code[offset] == " ":
        offset += 1
    return type_spec, attributes, (entities.strip(), offset)


def initializer_span(entities: str) -> tuple[int, int] | None:
    """Return the span of the ``=`` or ``=>`` of an entity list, if any.

    A binding named ``assignment(=)`` carries an ``=`` that is not an
    initializer, so the protected calls are masked before the search.
    """
    masked = PROTECTED_CALL.sub(lambda match: "x" * len(match.group()), entities)
    match = INITIALIZER.search(masked)
    return (match.start(), match.end()) if match is not None else None


def statement_kind(statement: Statement) -> tuple[str, str]:
    """Classify a statement for the alignment blocks of sections 6 and 7."""
    if statement.macro:
        return "other", ""
    code = mask_strings(statement.codes[0]).strip()
    if not code:
        return "other", ""
    if re.match(r"^(public|private)\s*::", code):
        return "access", ""
    if re.match(r"^use\b", code):
        return "use", ""
    if DECLARATION.match(code) and "::" in code:
        return "declaration", ""
    guard = GUARDED.match(code)
    if guard is not None and ASSIGNMENT.match(guard.group("body").strip()):
        return "assignment", guard.group("guard")
    if ASSIGNMENT.match(code):
        return "assignment", ""
    return "other", ""


def operator_column(code: str) -> int:
    """Return the one-based column of the assignment operator of ``code``."""
    depth = 0
    index = 0
    while index < len(code):
        char = code[index]
        if char in "([":
            depth += 1
        elif char in ")]":
            depth -= 1
        elif depth == 0 and char == "=" and code[index + 1 : index + 2] != "=":
            return index + 1
        index += 1
    return 0


#: A declaration whose type specifier is longer than this starts its own
#: alignment block (section 6): the ``procedure(...)`` pointer of a callback
#: would otherwise push the declarations around it to the right.
LONG_TYPE_SPECIFIER = 20


def starts_own_block(statement: Statement) -> bool:
    """Tell whether a declaration is isolated in its alignment block."""
    type_spec, _, _ = split_declaration(mask_strings(statement.codes[0]))
    return len(type_spec) > LONG_TYPE_SPECIFIER


def alignment_blocks(statements: list[Statement]) -> list[list[Statement]]:
    """Group the adjacent statements that share one alignment block."""
    blocks: list[list[Statement]] = []
    current: list[Statement] = []
    for statement in statements:
        kind, key = statement_kind(statement)
        if kind in ("declaration", "assignment"):
            if current:
                last = current[-1]
                last_kind, last_key = statement_kind(last)
                adjacent = statement.first == last.last + 1
                same_indent = last.indent == statement.indent
                continuous = adjacent and last_kind == kind and last_key == key
                isolated = kind == "declaration" and (
                    starts_own_block(statement) or starts_own_block(last)
                )
                if not (continuous and same_indent) or isolated:
                    blocks.append(current)
                    current = []
            current.append(statement)
        elif current:
            blocks.append(current)
            current = []
    if current:
        blocks.append(current)
    return blocks


class Checker:
    """Report the violations of the formatting guide in one file."""

    def __init__(self, source: Source) -> None:
        self.source = source
        self.path = source.path
        self.views: list[LineView] = []
        for number, text in enumerate(source.lines, start=1):
            code, comment = split_code_comment(text)
            self.views.append(LineView(number, text, code, comment, source.fortran[number - 1]))
        self.by_number = {view.number: view for view in self.views}
        self.statements = group_statements(self.views)
        self.violations: list[Violation] = []

    def report(self, line: int, rule: str, message: str) -> None:
        self.violations.append(Violation(self.path, line, rule, message))

    def check(self) -> list[Violation]:
        self.characters()
        self.blank_lines()
        self.header()
        self.indentation()
        self.continuations()
        self.alignments()
        self.blocks()
        self.operators()
        self.comments()
        self.line_length()
        self.violations.sort(key=lambda violation: (violation.line, violation.rule))
        return self.violations

    def characters(self) -> None:
        raw = self.path.read_bytes()
        if b"\r" in raw:
            self.report(1, "chars/eol", "the file contains a carriage return")
        if not raw.endswith(b"\n"):
            self.report(
                len(self.views), "chars/final-newline", "the file does not end with a newline"
            )
        elif raw.endswith(b"\n\n"):
            self.report(len(self.views), "chars/final-newline", "the file ends with a blank line")
        for view in self.views:
            if "\t" in view.text:
                self.report(view.number, "chars/tab", "a tab character occurs")
            if view.text != view.text.rstrip(" "):
                self.report(view.number, "chars/trailing-space", "the line ends with a blank")
            if not view.fortran or view.blank:
                continue
            if view.preproc and view.text != view.text.lstrip():
                self.report(
                    view.number,
                    "chars/preproc-column",
                    "a preprocessor line starts in column one",
                )
            if ";" in mask_strings(view.code):
                self.report(view.number, "chars/semicolon", "a semicolon separates two statements")
            if any(ord(char) > 127 for char in view.code):
                self.report(
                    view.number,
                    "chars/ascii",
                    "a non-ASCII character occurs outside a comment",
                )

    def blank_lines(self) -> None:
        if self.views and self.views[0].blank:
            self.report(1, "blank/leading", "the file starts with a blank line")
        previous = False
        for view in self.views:
            if not view.fortran:
                previous = False
                continue
            if view.blank and previous:
                self.report(view.number, "blank/consecutive", "two blank lines follow each other")
            previous = view.blank

    def header(self) -> None:
        """Check the include guard and the tail of a header or of a suite."""
        if not self.views:
            return
        last = self.views[-1]
        if self.path.suffix == ".M90":
            first = self.views[0].text.strip()
            second = self.views[1].text.strip() if len(self.views) > 1 else ""
            guard = re.match(r"^#ifndef (\w+_(?:F)?H)$", first)
            if guard is None:
                self.report(1, "preproc/guard", "the guard must open with #ifndef <Name>_FH")
            elif second != f"#define {guard.group(1)}":
                self.report(
                    2,
                    "preproc/guard",
                    f"the guard must continue with #define {guard.group(1)}",
                )
            if last.text.strip() != "#endif":
                self.report(last.number, "preproc/tail", "the header must end with #endif")
                return
            if len(self.views) < 3 or self.views[-2].text.strip():
                self.report(
                    last.number,
                    "preproc/tail",
                    "a blank line must precede the closing #endif",
                )
            terminator = self.views[-3].text.strip() if len(self.views) > 2 else ""
            if not terminator.startswith(("end module", "end submodule")):
                self.report(
                    last.number,
                    "preproc/tail",
                    "the module terminator must precede the closing #endif",
                )
        elif self.path.suffix == SUITE_SUFFIX:
            if last.text.strip() != "#endif":
                self.report(last.number, "suite/tail", "the suite must end with #endif")
            elif len(self.views) < 2 or not self.views[-2].text.rstrip().endswith(");"):
                self.report(
                    last.number,
                    "suite/tail",
                    "TEST_SUITE(...) must close directly before #endif",
                )

    def indentation(self) -> None:
        for statement in self.statements:
            if statement.macro:
                continue
            if statement.indent % 2:
                self.report(
                    statement.first,
                    "indent/odd",
                    f"the statement starts in column {statement.indent + 1}",
                )

    def continuations(self) -> None:
        for statement in self.statements:
            if statement.exempt or len(statement.codes) < 2:
                continue
            for position in range(1, len(statement.codes)):
                view = self.by_number[statement.lines[position]]
                if view.text.lstrip().startswith("&"):
                    self.report(
                        view.number,
                        "cont/leading-amp",
                        "a continuation line never starts with &",
                    )
                    continue
                expected = continuation_column(statement.codes, statement.indent, position)
                if view.indent + 1 != expected:
                    self.report(
                        view.number,
                        "cont/align",
                        f"the continuation starts in column {view.indent + 1} "
                        f"instead of {expected}",
                    )
            self.keyword_arguments(statement)

    def keyword_arguments(self, statement: Statement) -> None:
        """Check that the keywords of a continued call align their ``=``.

        Two layouts are compliant, and a call must not mix them: the plain list,
        where every keyword keeps one space before its ``=`` (the example of
        section 4), and the padded list, where the ``=`` are aligned as a block
        (``FortranNewMAC`` in `src/MAC/MAC_F_Imp.F90`).
        """
        keywords = []
        for position in range(1, len(statement.codes)):
            code = mask_strings(statement.codes[position])
            match = re.match(r"\s*([A-Za-z_]\w*)(\s*)=(?!=)", code)
            if match is not None:
                keywords.append((statement.lines[position], match.end(), len(match.group(2))))
        if len(keywords) < 2 or max(gap for _, _, gap in keywords) == 1:
            return
        target = max(column for _, column, _ in keywords)
        for line, column, _ in keywords:
            if column != target:
                self.report(
                    line,
                    "cont/keyword-eq",
                    f"the keyword = is in column {column} instead of {target}",
                )

    def alignments(self) -> None:
        for block in alignment_blocks(self.statements):
            kind, _ = statement_kind(block[0])
            if kind == "declaration":
                self.align_declarations(block)
            else:
                self.align_assignments(block)
        self.access_statements()

    def align_assignments(self, block: list[Statement]) -> None:
        columns = {}
        for statement in block:
            column = operator_column(mask_strings(statement.codes[0]))
            if column:
                columns[statement.first] = column
        if len(columns) < 2:
            return
        target = max(columns.values())
        for statement in block:
            column = columns.get(statement.first)
            if column and column != target:
                self.report(
                    statement.first,
                    "stmt/assign-align",
                    f"the = is in column {column} instead of {target}",
                )

    def align_declarations(self, block: list[Statement]) -> None:
        parsed = []
        for statement in block:
            code = mask_strings(statement.codes[0])
            parsed.append((statement, code, *split_declaration(code)))
        colons = {statement.first: code.index("::") + 1 for statement, code, _, _, _ in parsed}
        if len(set(colons.values())) > 1:
            target = max(colons.values())
            for statement, code, _, _, _ in parsed:
                if colons[statement.first] != target:
                    self.report(
                        statement.first,
                        "decl/align-colons",
                        f"the :: is in column {colons[statement.first]} instead of {target}",
                    )
        depth = max(len(attributes) for _, _, _, attributes, _ in parsed)
        for index in range(depth):
            columns = {
                statement.first: attributes[index][1] + 1
                for statement, _, _, attributes, _ in parsed
                if len(attributes) > index
            }
            if len(set(columns.values())) > 1:
                target = max(columns.values())
                for statement, _, _, attributes, _ in parsed:
                    if len(attributes) > index and columns[statement.first] != target:
                        self.report(
                            statement.first,
                            "decl/align-attrs",
                            f"attribute {index + 1} is in column "
                            f"{columns[statement.first]} instead of {target}",
                        )
        values = {}
        for statement, code, _, _, (entities, offset) in parsed:
            span = initializer_span(entities)
            if span is not None:
                values[statement.first] = offset + span[0] + 1
        if len(set(values.values())) > 1:
            target = max(values.values())
            for statement in block:
                column = values.get(statement.first)
                if column and column != target:
                    self.report(
                        statement.first,
                        "decl/align-values",
                        f"the initializer = is in column {column} instead of {target}",
                    )

    def access_statements(self) -> None:
        for statement in self.statements:
            if statement.macro:
                continue
            code = mask_strings(statement.codes[0]).strip()
            access = re.match(r"^(public|private)\s*::\s*(?P<names>.+)$", code)
            if access is not None and re.search(r"(?<![(\[]),(?![)\]])", access.group("names")):
                self.report(
                    statement.first,
                    "decl/access-list",
                    "an access statement carries one name",
                )
            if re.match(r"^import\s*::", code):
                self.report(statement.first, "decl/import-colons", "import takes no ::")
            if re.match(r"^use\s+ISO_C_BINDING\b", code, re.IGNORECASE):
                self.report(
                    statement.first,
                    "decl/use-intrinsic",
                    "ISO_C_BINDING is used with intrinsic",
                )

    def blocks(self) -> None:
        """Check the closing statement of every block and program unit."""
        stacks: dict[str, list[str]] = {}
        for statement in self.statements:
            if statement.macro:
                continue
            code = mask_strings(statement.codes[0]).strip()
            if re.match(r"^end\s+(if|do)\b", code):
                self.report(
                    statement.first,
                    "blk/end-if",
                    "the two-word form end if / end do is never used",
                )
                continue
            closure = re.match(r"^end\s*(?P<unit>[A-Za-z]+)?\s*(?P<name>[A-Za-z_]\w*)?$", code)
            if closure is not None:
                unit = (closure.group("unit") or "").lower()
                name = closure.group("name")
                if unit in ("interface", "enum"):
                    if name:
                        self.report(
                            statement.first,
                            "blk/end-anonymous",
                            f"end {unit} carries no name",
                        )
                elif unit in NAMED_UNITS:
                    stack = stacks.setdefault(unit, [])
                    expected = stack.pop() if stack else ""
                    if name is None:
                        self.report(
                            statement.first,
                            "blk/end-name",
                            f"the unit must be closed with its name: end {unit} "
                            f"{expected}".rstrip(),
                        )
                    elif expected and name != expected:
                        self.report(
                            statement.first,
                            "blk/end-name",
                            f"the unit closes {name} but end {unit} {expected} opened it",
                        )
                continue
            header = UNIT_HEADER.match(code)
            if header is None:
                continue
            if header.group("name"):
                stacks.setdefault(header.group("unit").lower(), []).append(header.group("name"))

    def operators(self) -> None:
        constructors = self.constructor_lines()
        for view in self.views:
            if not view.fortran or view.blank or not view.code.strip():
                continue
            code = mask_strings(view.code)
            spans = protected_spans(code)
            if view.number not in constructors:
                self.operator_spacing(view, code, spans)
            self.parentheses(view, code)

    def constructor_lines(self) -> set[int]:
        """Return the lines that sit inside an array constructor."""
        inside: set[int] = set()
        for statement in self.statements:
            depth = 0
            for number, code in zip(statement.lines, statement.codes):
                if depth:
                    inside.add(number)
                depth += mask_strings(code).count("[") - mask_strings(code).count("]")
        return inside

    def operator_spacing(self, view: LineView, code: str, spans: list[tuple[int, int]]) -> None:
        for match in SPACING_TOKEN.finditer(code):
            if any(start <= match.start() < end for start, end in spans):
                continue
            wanted = operator_gaps(code, match)
            if wanted is None:
                continue
            left, right, has_left, has_right = gaps(code, match.start(), match.end())
            rule, symbol = spacing_rule(match.group())
            exact = match.group() not in ("=", "::", ",", "=>")
            desired_left, desired_right = wanted
            if desired_left is not None and has_left and not satisfies(left, desired_left, exact):
                self.report(view.number, rule, f"{symbol} needs one space before it, found {left}")
            checked = desired_right is not None and has_right
            if checked and not satisfies(right, desired_right, exact):
                self.report(view.number, rule, f"{symbol} needs one space after it, found {right}")

    def parentheses(self, view: LineView, code: str) -> None:
        for match in re.finditer(r"\b([A-Za-z_]\w*)\s+\(", code):
            name = match.group(1)
            if name.lower() in SPACED_PAREN_KEYWORDS or name.lower() in TIGHT_PAREN_KEYWORDS:
                continue
            if name.isupper():
                continue
            self.report(view.number, "stmt/tight-paren", f"{name} stays tight against its bracket")
        for match in CONTROL_PAREN.finditer(code):
            self.report(
                view.number,
                "stmt/spaced-paren",
                f"{match.group(1)} takes one space before its condition",
            )

    def comments(self) -> None:
        anchor = 0
        block: list[LineView] = []
        for view in self.views:
            if not view.fortran or view.blank or not view.comment:
                if block:
                    self.comment_period(block)
                    block = []
                anchor = 0
                continue
            block.append(view)
            comment = view.comment
            if comment.startswith("!$"):
                continue
            if comment.startswith("!<"):
                if len(comment) > 2 and comment[2] not in " <":
                    self.report(
                        view.number,
                        "doc/doc-space",
                        "a documentation comment keeps one space after !<",
                    )
            elif len(comment) > 1 and comment[1] == " " and not comment.startswith("!  "):
                self.report(
                    view.number,
                    "doc/dash-space",
                    "an implementation comment has no space after !",
                )
            note = view.text.find("Note:")
            if note >= 0 and view.comment.strip().startswith("Note:"):
                anchor = note + len("Note: ")
                while anchor < len(view.text) and view.text[anchor] == " ":
                    anchor += 1
                continue
            if anchor:
                body = comment[1:]
                lead = len(body) - len(body.lstrip(" "))
                column = view.indent + 1 + lead
                if column != anchor:
                    self.report(
                        view.number,
                        "doc/note-continuation",
                        f"the note continues in column {column + 1} instead of {anchor + 1}",
                    )
        if block:
            self.comment_period(block)

    def comment_period(self, block: list[LineView]) -> None:
        """Warn when the last line of a prose comment has no final period."""
        text = " ".join(view.comment[1:].lstrip("<").strip() for view in block)
        if "-----" in text or "|" in text:
            return
        last = block[-1].comment[1:].lstrip("<").strip()
        if not last or last.endswith((".", ":", ",", ";", ")", "]", ">", "-", "=", '"', "'")):
            return
        if not re.search(r"[A-Za-z]{2,}", last):
            return
        if len(last.split()) < 2:
            return
        if last.isupper():
            return
        self.report(block[-1].number, "doc/period", "the comment does not end with a period")

    def line_length(self) -> None:
        for view in self.views:
            if view.fortran and len(view.text) > 80:
                self.report(view.number, "len/80", f"the line is {len(view.text)} columns long")


def protected_spans_multiline(text: str) -> list[tuple[int, int]]:
    """Return the spans of a whole statement that the operator rules skip."""
    spans: list[tuple[int, int]] = []
    depth = 0
    start = 0
    for index, char in enumerate(text):
        if char == "[":
            if depth == 0:
                start = index
            depth += 1
        elif char == "]" and depth:
            depth -= 1
            if depth == 0:
                spans.append((start, index + 1))
    if depth:
        spans.append((start, len(text)))
    spans.extend(
        (match.start(), match.end())
        for match in PROTECTED_CALL.finditer(text)
    )
    return spans


def fix_operator_spacing(lines: list[str], statements: list[Statement]) -> bool:
    """Give every operator the blanks of sections 4 and 8."""
    changed = False
    for statement in statements:
        if statement.exempt:
            continue
        text = "\n".join(statement.codes)
        masked = mask_strings(text)
        spans = protected_spans_multiline(masked)
        edits: list[tuple[int, int, str]] = []
        claimed: list[tuple[int, int]] = []
        for match in SPACING_TOKEN.finditer(masked):
            if any(start <= match.start() < end for start, end in spans):
                continue
            wanted = operator_gaps(masked, match)
            if wanted is None:
                continue
            left, right, _, _ = gaps(text, match.start(), match.end())
            exact = match.group() not in ("=", "::", ",", "=>")
            desired_left, desired_right = wanted
            new_left = gap_target(left, desired_left, exact)
            new_right = gap_target(right, desired_right, exact)
            if (new_left, new_right) == (left, right):
                continue
            start = match.start() - left
            end = match.end() + right
            if any(start < other_end and other_start < end for other_start, other_end in claimed):
                continue
            claimed.append((start, end))
            edits.append((start, end, " " * new_left + match.group() + " " * new_right))
        if not edits:
            continue
        for start, end, replacement in sorted(edits, reverse=True):
            text = text[:start] + replacement + text[end:]
        parts = text.split("\n")
        if len(parts) != len(statement.codes):
            continue
        for offset, number in enumerate(statement.lines):
            lines[number - 1] = parts[offset] + split_code_comment(lines[number - 1])[1]
            statement.codes[offset] = parts[offset]
        changed = True
    return changed


def fix_alignments(lines: list[str], statements: list[Statement]) -> bool:
    """Align the ``=``, the attributes and the ``::`` of every block."""
    changed = False
    for block in alignment_blocks(statements):
        kind, _ = statement_kind(block[0])
        if kind == "declaration":
            changed = fix_declaration_block(lines, block) or changed
        else:
            changed = fix_assignment_block(lines, block) or changed
    return changed


def fix_assignment_block(lines: list[str], block: list[Statement]) -> bool:
    """Pad the left-hand sides so that the ``=`` of a block line up."""
    columns = {}
    for statement in block:
        column = operator_column(mask_strings(statement.codes[0]))
        if column:
            columns[statement.first] = column
    if len(columns) < 2:
        return False
    target = max(columns.values())
    changed = False
    for statement in block:
        column = columns.get(statement.first)
        if not column or column == target:
            continue
        index = statement.first - 1
        code = statement.codes[0]
        padded = code[: column - 1].rstrip().ljust(target - 1) + code[column - 1 :]
        lines[index] = padded + split_code_comment(lines[index])[1]
        statement.codes[0] = padded
        changed = True
    return changed


def fix_declaration_block(lines: list[str], block: list[Statement]) -> bool:
    """Pad the attributes and the ``::`` of a declaration block."""
    parsed = []
    for statement in block:
        code = mask_strings(statement.codes[0])
        parsed.append((statement, code, *split_declaration(code)))
    colon_target = max(code.index("::") + 1 for _, code, _, _, _ in parsed)
    depth = max(len(attributes) for _, _, _, attributes, _ in parsed)
    attribute_targets = [
        max(
            attributes[index][1] + 1
            for _, _, _, attributes, _ in parsed
            if len(attributes) > index
        )
        for index in range(depth)
    ]
    values = []
    for _, _, _, _, (entities, offset) in parsed:
        span = initializer_span(entities)
        if span is not None:
            values.append(offset + span[0])
    value_target = max(values, default=0) + 1
    changed = False
    for statement, code, type_spec, attributes, (entities, offset) in parsed:
        rebuilt = " " * statement.indent + type_spec
        for index, (attribute, _) in enumerate(attributes):
            rebuilt = pad_to(rebuilt + ",", attribute_targets[index]) + attribute
        rebuilt = pad_to(rebuilt, colon_target) + ":: "
        span = initializer_span(entities)
        if span is not None and value_target:
            rebuilt += entities[: span[0]].rstrip()
            rebuilt = pad_to(rebuilt, value_target)
            rebuilt += entities[span[0] : span[1]] + " " + entities[span[1] :].lstrip()
        else:
            rebuilt += entities
        if rebuilt == code:
            continue
        index = statement.first - 1
        lines[index] = rebuilt + split_code_comment(lines[index])[1]
        statement.codes[0] = rebuilt
        changed = True
    return changed


def fix_continuations(lines: list[str], statements: list[Statement]) -> bool:
    """Re-indent the continuations and align the keywords of a call."""
    changed = False
    for statement in statements:
        if statement.exempt or len(statement.codes) < 2:
            continue
        for position in range(1, len(statement.codes)):
            expected = continuation_column(statement.codes, statement.indent, position)
            index = statement.lines[position] - 1
            if indentation(lines[index]) + 1 != expected:
                lines[index] = " " * (expected - 1) + lines[index].lstrip()
                aligned = statement.codes[position].lstrip()
                statement.codes[position] = " " * (expected - 1) + aligned
                changed = True
        changed = fix_keyword_equalities(lines, statement) or changed
    return changed


def fix_keyword_equalities(lines: list[str], statement: Statement) -> bool:
    """Pad the keywords of a continued call so that their ``=`` align.

    A call whose keywords keep one space before their ``=`` is the plain list of
    section 4 and is left alone; a call that pads at least one keyword gets all
    of its ``=`` aligned.
    """
    matches = []
    for position in range(1, len(statement.codes)):
        code = statement.codes[position]
        match = re.match(r"(\s*)([A-Za-z_]\w*)(\s*)=(?!=)", code)
        if match is not None:
            matches.append((position, match))
    if len(matches) < 2 or max(len(match.group(3)) for _, match in matches) == 1:
        return False
    target = max(match.end() for _, match in matches)
    changed = False
    for position, match in matches:
        if match.end() == target:
            continue
        index = statement.lines[position] - 1
        code = statement.codes[position]
        comment = split_code_comment(lines[index])[1]
        prefix = match.group(1) + match.group(2)
        rebuilt = prefix.ljust(target - 1) + "= " + code[match.end() :].lstrip()
        lines[index] = rebuilt + comment
        statement.codes[position] = rebuilt
        changed = True
    return changed


def fix_lines(lines: list[str], fortran: list[bool]) -> bool:
    """Apply the mechanical rewrites of the guide to the Fortran lines."""
    changed = False
    for index, text in enumerate(lines):
        if not fortran[index]:
            continue
        fixed = text.rstrip(" ")
        code, comment = split_code_comment(fixed)
        rewritten = re.sub(r"\bend\s+if\b", "endif", code)
        rewritten = re.sub(r"\bend\s+do\b", "enddo", rewritten)
        rewritten = CONTROL_PAREN.sub(r"\1 (", rewritten)
        if comment.startswith("!") and not comment.startswith(("!$", "!<", "!  ")):
            if comment.startswith("! "):
                comment = "!" + comment[2:]
        elif comment.startswith("!<") and len(comment) > 2 and comment[2] not in " <":
            comment = "!< " + comment[2:]
        if rewritten + comment != text:
            lines[index] = rewritten + comment
            changed = True
    return changed


def tidy(lines: list[str], fortran: list[bool]) -> bool:
    """Drop the leading and repeated blank lines and the blank tail."""
    original = list(lines)
    while lines and not lines[0].strip() and fortran[0]:
        del lines[0]
        del fortran[0]
    index = 1
    while index < len(lines):
        if fortran[index] and not lines[index].strip() and not lines[index - 1].strip():
            del lines[index]
            del fortran[index]
            continue
        index += 1
    while lines and not lines[-1].strip():
        lines.pop()
        fortran.pop()
    return lines != original


def views_of(lines: list[str], fortran: list[bool]) -> list[LineView]:
    """Build the views of the current state of a file."""
    views: list[LineView] = []
    for index, text in enumerate(lines):
        code, comment = split_code_comment(text)
        views.append(LineView(index + 1, text, code, comment, fortran[index]))
    return views


def statements_of(lines: list[str], fortran: list[bool]) -> list[Statement]:
    """Group the statements of the current state of a file."""
    return group_statements(views_of(lines, fortran))


def fix_file(path: Path) -> bool:
    """Rewrite ``path`` with the mechanical fixes; report whether it changed."""
    source = Source.load(path)
    lines = list(source.lines)
    fortran = list(source.fortran)
    changed = fix_lines(lines, fortran)
    changed = fix_operator_spacing(lines, statements_of(lines, fortran)) or changed
    changed = fix_alignments(lines, statements_of(lines, fortran)) or changed
    changed = fix_continuations(lines, statements_of(lines, fortran)) or changed
    changed = tidy(lines, fortran) or changed
    rewritten = "\n".join(lines) + "\n"
    if not changed or rewritten == source.text:
        return False
    path.write_text(rewritten, encoding="utf-8")
    return True


def collect(paths: list[Path]) -> list[Path]:
    """Return the Fortran files below ``paths``."""
    files: list[Path] = []
    for path in paths:
        if path.is_dir():
            files.extend(
                candidate
                for candidate in sorted(path.rglob("*"))
                if candidate.suffix in FORTRAN_SUFFIXES or candidate.suffix == SUITE_SUFFIX
            )
        elif path.suffix in FORTRAN_SUFFIXES or path.suffix == SUITE_SUFFIX:
            files.append(path)
    return [path for path in files if path.is_file()]


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("paths", nargs="*", help="files or directories (default: src and test)")
    parser.add_argument("--fix", action="store_true", help="apply the mechanical fixes in place")
    parser.add_argument("--rules", action="store_true", help="list the rules and exit")
    parser.add_argument("--quiet", action="store_true", help="report the counts only")
    options = parser.parse_args(argv)
    if options.rules:
        for rule, summary in RULE_SUMMARY:
            suffix = " (warning)" if rule in WARNING_RULES else ""
            print(f"{rule:<24} {summary}{suffix}")
        return 0
    paths = [Path(path).resolve() for path in options.paths]
    files = collect(paths or [REPOSITORY / "src", REPOSITORY / "test"])
    if options.fix:
        for path in files:
            if fix_file(path):
                print(f"fixed {path}")
    violations: list[Violation] = []
    for path in files:
        violations.extend(Checker(Source.load(path)).check())
    warnings = [violation for violation in violations if violation.warning]
    errors = [violation for violation in violations if not violation.warning]
    if not options.quiet:
        for violation in violations:
            print(violation)
    print(f"{len(files)} files, {len(errors)} violations, {len(warnings)} warnings")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
