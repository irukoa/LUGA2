#!/usr/bin/env python3
"""Enforce ``style/FortranFormatting.md`` on the Doxygen blocks of the C headers.

``clang-format`` owns the code of a header and the physical shape of a comment:
it rewraps the prose at the column limit, keeps one space after ``*`` and keeps
the ``*`` column aligned.  It deliberately does not re-indent what it finds
*inside* a comment, so the rules of section 9 that describe the interior of a
block -- the column where a tag and its prose start, the column of a description
under its tag, the rows of a section banner, the order of the tags -- are left to
the reviewer.  This script decides them, so that a lapse is caught before the
review.

Only the blocks that open with ``/**`` are read; the include guards, the
declarations and the implementation comments in between are ignored.

Every violation carries the identifier of the rule it breaks; the identifiers are
listed by ``--rules``.  The verdict that needs judgement (the length of a
``@brief``) is reported as a warning and does not change the exit status.  The
script never edits a file: the layout of a block is a decision, not a rewrite.

Usage::

    python3 style/CDocStyleCheck.py                    # every header of src
    python3 style/CDocStyleCheck.py src/LDW            # a directory
    python3 style/CDocStyleCheck.py --rules            # list the rules
    python3 style/CDocStyleCheck.py --quiet            # report the counts only

The exit status is 0 when no violation is found, 1 otherwise.
"""

from __future__ import annotations

import argparse
import re
import sys
from collections.abc import Iterator
from dataclasses import dataclass
from pathlib import Path

REPOSITORY = Path(__file__).resolve().parent.parent

HEADER_SUFFIX = ".h"
BLOCK_OPEN = "/**"
BLOCK_CLOSE = "*/"

#: The tags of section 9, in the order they must appear in a block.
TAG_ORDER = ("file", "brief", "details", "param", "return", "retval", "note", "see")

#: ``@return`` and ``@retval`` document the same topic, so they share a rank.
TAG_RANK = {
    "file": 0,
    "brief": 1,
    "details": 2,
    "param": 3,
    "return": 4,
    "retval": 4,
    "note": 5,
    "see": 6,
}

#: Width of the dash row of a section banner (`` * `` plus this many dashes).
BANNER_WIDTH = 73

#: Column of a tag, of the prose of ``@brief`` and of the prose of ``@details``.
TAG_COLUMN = 3
PROSE_COLUMN = 3

#: Columns allowed for the body of a section: paragraph or bullet, continuation
#: of a bullet, display block (equations, array shapes).
BODY_COLUMNS = {
    "": (PROSE_COLUMN,),
    "file": (PROSE_COLUMN,),
    "brief": (PROSE_COLUMN,),
    "details": (PROSE_COLUMN, PROSE_COLUMN + 2, PROSE_COLUMN + 4),
    "note": (PROSE_COLUMN, PROSE_COLUMN + 2),
    "param": (TAG_COLUMN + 4,),
    "return": (TAG_COLUMN + 4,),
    "retval": (TAG_COLUMN + 4,),
}

#: The longest ``@brief`` that is still a brief.
MAX_BRIEF_SENTENCES = 2

#: Spelling that ends in a period without ending a sentence.
ABBREVIATIONS = ("e.g.", "i.e.", "etc.", "cf.", "vs.", "resp.")

#: A sentence ends at one of these characters followed by a new sentence.
SENTENCE_END = re.compile(r"[.!?](?=\s+[A-Z(\[])")

TAG_PATTERN = re.compile(r"^@([A-Za-z]+)")
DASH_PATTERN = re.compile(r"^-+$")

WARNING_RULES = frozenset({"doc/brief-length"})

RULE_SUMMARY = [
    ("doc/unterminated", "the block is not closed by */ (section 9)"),
    ("doc/star-column", "a line of a block does not start with * after the opening /** (section 9)"),
    ("doc/star-space", "exactly one space separates * from the text (section 9)"),
    ("doc/tag-known", "a tag is not one of the tags of the guide (section 9)"),
    ("doc/file-tag", "the argument of @file is not the file name, or the tag is not first (section 9)"),
    ("doc/tag-column", "a tag does not start after ` * ` (section 9)"),
    ("doc/tag-order", "the tags of a block are out of order (section 9)"),
    ("doc/tag-blank", "a tag is not preceded by a blank ` *` line (section 9)"),
    ("doc/see-inline", "the target of @see is not on the tag line (section 9)"),
    ("doc/body-column", "a line of a section starts in the wrong column (section 9)"),
    ("doc/banner", "a section banner is not a pair of dash rows around an upper-case title (section 9)"),
    ("doc/tail-blank", "a block ends with a blank ` *` line (section 9)"),
    ("doc/brief-length", "a @brief holds more than two sentences (section 9) (warning)"),
]


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


@dataclass
class Entry:
    """One line inside a comment block, split into what the rules look at."""

    line: int
    kind: str
    """``blank``, ``tag``, ``dash``, ``text`` or ``broken``."""
    section: str
    """The tag the line belongs to, ``""`` before the first tag."""
    name: str
    """The tag name when ``kind`` is ``tag``, ``""`` otherwise."""
    column: int
    """Zero-based column of the first character of the content."""
    text: str
    """The content of the line, without the leading ``*``."""


def entry_of(text: str, number: int, section: str) -> Entry:
    """Split one line of a block into the fields the rules compare."""
    star = len(text) - len(text.lstrip())
    if not text.lstrip().startswith("*"):
        return Entry(number, "broken", section, "", star, text.strip())
    body = text[star + 1 :]
    if body and not body.startswith(" "):
        return Entry(number, "tight", section, "", star + 1, body.rstrip())
    content = body[1:] if body else ""
    if not content.rstrip():
        return Entry(number, "blank", section, "", 0, "")
    column = star + 2 + len(content) - len(content.lstrip())
    stripped = content.strip()
    match = TAG_PATTERN.match(stripped)
    if match:
        return Entry(number, "tag", section, match.group(1), column, stripped)
    if DASH_PATTERN.match(stripped):
        return Entry(number, "dash", section, "", column, stripped)
    return Entry(number, "text", section, "", column, stripped)


def blocks(lines: list[str]) -> Iterator[tuple[int, int, list[Entry], bool]]:
    """Yield ``(opener, column, entries, closed)`` for every ``/**`` block."""
    index = 0
    while index < len(lines):
        if lines[index].strip() != BLOCK_OPEN:
            index += 1
            continue
        column = len(lines[index]) - len(lines[index].lstrip())
        entries: list[Entry] = []
        section = ""
        cursor = index + 1
        closed = False
        while cursor < len(lines):
            if lines[cursor].strip() == BLOCK_CLOSE:
                closed = True
                break
            entry = entry_of(lines[cursor], cursor + 1, section)
            if entry.kind == "tag":
                section = entry.name
                entry.section = entry.name
            entries.append(entry)
            cursor += 1
        yield index + 1, column, entries, closed
        index = cursor + 1


def sentences(text: str) -> int:
    """Count the sentences of a paragraph, ignoring the usual abbreviations."""
    scrubbed = text.strip()
    if not scrubbed:
        return 0
    for abbreviation in ABBREVIATIONS:
        scrubbed = scrubbed.replace(abbreviation, abbreviation.replace(".", "\u2024"))
    body = re.sub(r"[.!?]+$", "", scrubbed)
    return 1 + len(SENTENCE_END.findall(body))


def columns_of(offsets: tuple[int, ...], comment_column: int) -> list[int]:
    """Turn the offsets allowed in a section into the columns of this block."""
    return [comment_column + offset for offset in offsets]


def describe(columns: list[int]) -> str:
    """Name the columns a rule expected, for the message of a violation."""
    if len(columns) == 1:
        return f"column {columns[0]}"
    return "one of the columns " + ", ".join(str(column) for column in columns)


class Checker:
    """Report the broken rules of one header."""

    def __init__(self, path: Path, lines: list[str]) -> None:
        self.path = path
        self.lines = lines
        self.violations: list[Violation] = []

    def report(self, line: int, rule: str, message: str) -> None:
        self.violations.append(Violation(self.path, line, rule, message))

    def check(self) -> list[Violation]:
        for index, (opener, column, entries, closed) in enumerate(blocks(self.lines)):
            if not closed:
                self.report(opener, "doc/unterminated", "the block is not closed by */")
            self.check_shape(entries, column)
            self.check_file_tag(entries, index == 0)
            self.check_tags(entries, column)
            self.check_bodies(entries, column)
            self.check_banners(entries)
            self.check_tail(entries)
            self.check_brief(entries)
        return self.violations

    def check_file_tag(self, entries: list[Entry], first: bool) -> None:
        """``@file`` names the header and opens the first block of the file."""
        for entry in entries:
            if entry.kind != "tag" or entry.name != "file":
                continue
            if not first:
                self.report(entry.line, "doc/file-tag", "`@file` documents the file, so it opens the first block")
            argument = entry.text[len("@file") :].strip()
            if argument and Path(argument).name != self.path.name:
                self.report(
                    entry.line,
                    "doc/file-tag",
                    f"`@file` names `{argument}`, expected `{self.path.name}`",
                )

    def check_shape(self, entries: list[Entry], column: int) -> None:
        """Every line of a block opens with ``*`` in the column after ``/**``."""
        for entry in entries:
            if entry.kind == "broken":
                self.report(
                    entry.line,
                    "doc/star-column",
                    f"the line does not start with `*` in column {column + 1}",
                )
            elif entry.kind == "tight":
                self.report(entry.line, "doc/star-space", "exactly one space separates `*` from the text")

    def check_tags(self, entries: list[Entry], column: int) -> None:
        """The tags are known, ordered, indented and separated by a blank line."""
        previous: Entry | None = None
        last_tag: Entry | None = None
        for entry in entries:
            if entry.kind != "tag":
                previous = entry
                continue
            if entry.name not in TAG_RANK:
                self.report(entry.line, "doc/tag-known", f"`@{entry.name}` is not a tag of section 9")
            if last_tag is not None and entry.name in TAG_RANK and last_tag.name in TAG_RANK:
                if TAG_RANK[entry.name] < TAG_RANK[last_tag.name]:
                    self.report(
                        entry.line,
                        "doc/tag-order",
                        f"`@{entry.name}` follows `@{last_tag.name}`, which must come after it",
                    )
            if previous is not None and previous.kind != "blank":
                if not (previous.kind == "tag" and previous.name == entry.name):
                    self.report(
                        entry.line,
                        "doc/tag-blank",
                        f"`@{entry.name}` is not preceded by a blank ` *` line",
                    )
            if entry.column != column + TAG_COLUMN:
                self.report(
                    entry.line,
                    "doc/tag-column",
                    f"the tag starts in column {entry.column}, expected {column + TAG_COLUMN}",
                )
            if entry.name == "see" and entry.text == "@see":
                self.report(entry.line, "doc/see-inline", "the target of `@see` is written on the tag line")
            previous = entry
            last_tag = entry

    def check_bodies(self, entries: list[Entry], column: int) -> None:
        """The body of a section starts in the column the section prescribes."""
        for entry in entries:
            if entry.kind != "text":
                continue
            allowed = BODY_COLUMNS.get(entry.section)
            if allowed is None:
                self.report(
                    entry.line,
                    "doc/see-inline",
                    "the target of `@see` is written on the tag line, not under it",
                )
                continue
            expected = columns_of(allowed, column)
            if entry.column not in expected:
                self.report(
                    entry.line,
                    "doc/body-column",
                    f"a line of `@{entry.section}` starts in column {entry.column}, expected {describe(expected)}",
                )


    def check_banners(self, entries: list[Entry]) -> None:
        """A banner is a pair of dash rows around an upper-case title."""
        consumed: set[int] = set()
        for index, entry in enumerate(entries):
            if index in consumed or entry.kind != "dash":
                continue
            if len(entry.text) != BANNER_WIDTH:
                self.report(
                    entry.line,
                    "doc/banner",
                    f"the dash row is {len(entry.text)} dashes long, expected {BANNER_WIDTH}",
                )
            title = entries[index + 1] if index + 1 < len(entries) else None
            bottom = entries[index + 2] if index + 2 < len(entries) else None
            if title is None or bottom is None or title.kind != "text" or bottom.kind != "dash":
                self.report(
                    entry.line,
                    "doc/banner",
                    "a banner is a dash row, an upper-case title and a dash row",
                )
                continue
            consumed.update({index + 1, index + 2})
            letters = re.sub(r"[^A-Za-z]", "", re.sub(r"\([^)]*\)", "", title.text))
            if not letters or letters != letters.upper():
                self.report(title.line, "doc/banner", "the title of a banner is in upper case")
            before = entries[index - 1] if index > 0 else None
            if before is not None and before.kind != "blank":
                if not (before.kind == "tag" and before.name == "details"):
                    self.report(entry.line, "doc/banner", "a banner is preceded by a blank ` *` line")
            after = entries[index + 3] if index + 3 < len(entries) else None
            if after is not None and after.kind != "blank":
                self.report(bottom.line, "doc/banner", "a banner is followed by a blank ` *` line")

    def check_tail(self, entries: list[Entry]) -> None:
        """A block ends on its last line of content, not on a blank ` *` line."""
        if entries and entries[-1].kind == "blank":
            self.report(entries[-1].line, "doc/tail-blank", "the block ends with a blank ` *` line")

    def check_brief(self, entries: list[Entry]) -> None:
        """The ``@brief`` of a block holds one or two sentences."""
        paragraph: list[str] = []
        opener: int | None = None
        for entry in entries:
            if entry.kind == "tag":
                if paragraph:
                    break
                if entry.name == "brief":
                    opener = entry.line
                    rest = entry.text[len("@brief") :].strip()
                    if rest:
                        paragraph.append(rest)
                continue
            if opener is None:
                continue
            if entry.kind == "text":
                paragraph.append(entry.text)
        if opener is None or not paragraph:
            return
        count = sentences(" ".join(paragraph))
        if count > MAX_BRIEF_SENTENCES:
            self.report(
                opener,
                "doc/brief-length",
                f"the `@brief` holds {count} sentences, {MAX_BRIEF_SENTENCES} at most",
            )


def collect(paths: list[Path]) -> list[Path]:
    """Return the headers below ``paths``."""
    files: list[Path] = []
    for path in paths:
        if path.is_dir():
            files.extend(sorted(candidate for candidate in path.rglob("*") if candidate.suffix == HEADER_SUFFIX))
        elif path.suffix == HEADER_SUFFIX:
            files.append(path)
    return [path for path in files if path.is_file()]


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("paths", nargs="*", help="files or directories (default: src)")
    parser.add_argument("--rules", action="store_true", help="list the rules and exit")
    parser.add_argument("--quiet", action="store_true", help="report the counts only")
    options = parser.parse_args(argv)
    if options.rules:
        for rule, summary in RULE_SUMMARY:
            print(f"{rule:<24} {summary}")
        return 0
    paths = [Path(path).resolve() for path in options.paths]
    files = collect(paths or [REPOSITORY / "src"])
    violations: list[Violation] = []
    for path in files:
        violations.extend(Checker(path, path.read_text(encoding="utf-8").splitlines()).check())
    warnings = [violation for violation in violations if violation.warning]
    errors = [violation for violation in violations if not violation.warning]
    if not options.quiet:
        for violation in violations:
            print(violation)
    print(f"{len(files)} files, {len(errors)} violations, {len(warnings)} warnings")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
