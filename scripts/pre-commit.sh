#!/bin/sh -e
# shellcheck disable=SC2086

CHANGED_RUST_FILES="$(git diff --name-only --staged --diff-filter=AM -- '*.rs')"
if [ -n "$CHANGED_RUST_FILES" ]; then
  rustfmt +nightly --check $CHANGED_RUST_FILES
fi

git clang-format -f

CHANGED_SHELL_FILES="$(git diff --name-only --staged --diff-filter=AM -- '*.sh')"
if [ -n "$CHANGED_SHELL_FILES" ]; then
  shellcheck $CHANGED_SHELL_FILES
fi

cargo clippy --all-targets --all-features -- -D warnings
