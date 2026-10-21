# Drop the spurious coverage records of Fortran (sub)module terminators from an
# lcov '.info' stream.
#
# For every submodule gfortran generates a one-line '__copy_...' routine which
# spans nothing but the 'end submodule' line.  It is never called, so gcov
# reports it (and the terminator line) as missed, even though no code can reach
# that line: the unit terminator holds no logic.
#
# lcov cannot drop it on its own: '--omit-lines' - like every other source
# filter - is only applied to c, perl, python and java sources (see
# 'applyFilters' in lcovutil.pm).  The records are therefore removed here, right
# after the capture, and the 'FNF'/'LF' totals are corrected so that the result
# is still self-consistent.
#
# Only zero-count records are removed: the terminator of a *module* can carry a
# real count (e.g. GraphUtilities.M90) and must not be touched.
#
# Usage: awk -f OmitTerminatorLines.awk < coverage.info > filtered.info

BEGIN {
    # 'end module', 'end submodule', 'endmodule' and 'endsubmodule', optionally
    # followed by the unit name: nothing but whitespace may follow 'module'.
    terminator = "^[ \t]*end[ \t]*(sub)?module([ \t]|$)"
    reset()
}

# A new source file starts: settle the previous block, then collect the line
# numbers of its terminators.
/^SF:/ {
    flush()

    source = substr($0, 4)
    line = 0
    while ((getline text < source) > 0) {
        ++line
        if (text ~ terminator) {
            terminatorLine[line] = 1
        }
    }
    close(source)

    block[++blockLines] = $0
    next
}

# Functions generated on a terminator: 'FNL:index,start,end' followed by one
# 'FNA:index,count,name' per alias.
/^FNL:/ {
    split(substr($0, 5), field, ",")
    fnStart[field[1]] = field[2] + 0
    fnEnd[field[1]]   = field[3] + 0
    bufferFn(field[1])
    next
}

/^FNA:/ {
    split(substr($0, 5), field, ",")
    fnSeen[field[1]] = 1
    if ((field[2] + 0) != 0) {
        fnHit[field[1]] = 1
    }
    bufferFn(field[1])
    next
}

/^DA:/ {
    split(substr($0, 4), field, ",")
    isDaLine[blockLines + 1] = 1
    daLine[blockLines + 1]   = field[1] + 0
    daCount[blockLines + 1]  = field[2] + 0
}

/^BRDA:/ {
    split(substr($0, 6), field, ",")
    isBrLine[blockLines + 1] = 1
    brLine[blockLines + 1]   = field[1] + 0
}

# Per-testcase function data: removing a function would need more bookkeeping
# than it is worth, so leave such a record untouched.
/^FNDA:/ { hasFnda = 1 }

{ block[++blockLines] = $0 }

END { flush() }

# Buffer a function record (FNL/FNA) together with the index it belongs to.
function bufferFn(key) {
    isFnLine[blockLines + 1] = 1
    fnKey[blockLines + 1]    = key
    block[++blockLines]      = $0
}

# Print the buffered '.info' block, dropping the generated functions which cover
# a terminator and correcting the function and line totals.
function flush(   i, key, dropped, removed) {
    dropped = 0
    removed = 0

    if (!hasFnda) {
        # a function is dropped when it covers exactly one line, that line is a
        # terminator, and the function was never hit
        for (key in fnStart) {
            if ((key in fnSeen) && fnEnd[key] == fnStart[key] &&
                !(key in fnHit) && (fnStart[key] in terminatorLine)) {
                dropFn[key] = 1
                ++dropped
            }
        }

        # a terminator keeps its data when a surviving function starts there
        for (key in fnStart) {
            if (!(key in dropFn)) {
                keepLine[fnStart[key]] = 1
            }
        }

        for (i = 1; i <= blockLines; ++i) {
            if ((i in isDaLine) && daCount[i] == 0 &&
                (daLine[i] in terminatorLine) && !(daLine[i] in keepLine)) {
                removedLine[daLine[i]] = 1
                ++removed
            }
        }
    }

    for (i = 1; i <= blockLines; ++i) {
        if ((i in isFnLine) && (fnKey[i] in dropFn)) {
            continue
        }
        if ((i in isDaLine) && (daLine[i] in removedLine)) {
            continue
        }
        if ((i in isBrLine) && (brLine[i] in removedLine)) {
            continue
        }
        if (block[i] ~ /^FNF:/) {
            print "FNF:" (substr(block[i], 5) - dropped)
        } else if (block[i] ~ /^LF:/) {
            print "LF:" (substr(block[i], 4) - removed)
        } else {
            print block[i]
        }
    }

    reset()
}

# Clear the state of the block being read.
function reset() {
    blockLines = 0
    hasFnda    = 0
    split("", block)
    split("", terminatorLine)
    split("", fnStart)
    split("", fnEnd)
    split("", fnSeen)
    split("", fnHit)
    split("", dropFn)
    split("", keepLine)
    split("", removedLine)
    split("", isFnLine)
    split("", fnKey)
    split("", isDaLine)
    split("", daLine)
    split("", daCount)
    split("", isBrLine)
    split("", brLine)
}
