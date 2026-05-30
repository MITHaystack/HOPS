#!/bin/bash
#!/usr/bin/env bash

INFO_FILE="$1"

if [[ -z "$INFO_FILE" ]]; then
    echo "Usage: $0 <coverage.info>"
    exit 1
fi

BASENAME=$(basename "$INFO_FILE")

if [[ "$BASENAME" == *hops4* ]]; then
    LABEL="HOPS4 coverage"
elif [[ "$BASENAME" == *hops3* ]]; then
    LABEL="HOPS3 coverage"
else
    LABEL="coverage"
fi

COVERAGE=$(lcov --summary "$INFO_FILE" 2>&1 \
  | grep "lines......:" \
  | sed -E 's/.*: ([0-9]+\.[0-9]+)%.*/\1/')

if [[ -z "$COVERAGE" ]]; then
    echo "Failed to extract coverage from $INFO_FILE"
    exit 1
fi

if awk "BEGIN {exit !($COVERAGE >= 90)}"; then
    COLOR="#4c1"
elif awk "BEGIN {exit !($COVERAGE >= 75)}"; then
    COLOR="#97CA00"
elif awk "BEGIN {exit !($COVERAGE >= 60)}"; then
    COLOR="#dfb317"
else
    COLOR="#e05d44"
fi

LABEL_WIDTH=$(( ${#LABEL} * 7 + 10 ))
VALUE_WIDTH=55
TOTAL_WIDTH=$(( LABEL_WIDTH + VALUE_WIDTH ))

OUTPUT="${BASENAME%.info}.svg"

cat > "$OUTPUT" <<EOF
<svg xmlns="http://www.w3.org/2000/svg"
     width="${TOTAL_WIDTH}"
     height="20"
     role="img"
     aria-label="${LABEL}: ${COVERAGE}%">

  <linearGradient id="s" x2="0" y2="100%">
    <stop offset="0" stop-color="#bbb" stop-opacity=".1"/>
    <stop offset="1" stop-opacity=".1"/>
  </linearGradient>

  <clipPath id="r">
    <rect width="${TOTAL_WIDTH}" height="20" rx="3" fill="#fff"/>
  </clipPath>

  <g clip-path="url(#r)">
    <rect width="${LABEL_WIDTH}" height="20" fill="#555"/>
    <rect x="${LABEL_WIDTH}" width="${VALUE_WIDTH}" height="20" fill="${COLOR}"/>
    <rect width="${TOTAL_WIDTH}" height="20" fill="url(#s)"/>
  </g>

  <g fill="#fff"
     text-anchor="middle"
     font-family="Verdana,Geneva,DejaVu Sans,sans-serif"
     font-size="11">

    <text x="$((LABEL_WIDTH / 2))" y="15">${LABEL}</text>

    <text x="$((LABEL_WIDTH + VALUE_WIDTH / 2))"
          y="15">${COVERAGE}%</text>
  </g>
</svg>
EOF

echo "Generated ${OUTPUT} (${LABEL}: ${COVERAGE}%)"
