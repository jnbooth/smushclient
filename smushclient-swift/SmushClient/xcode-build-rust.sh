#!/bin/sh -e

export PATH="$HOME/.cargo/bin:$PATH"

if [ $CONFIGURATION = "Release" ]; then
    echo "BUIlDING FOR RELEASE"

    cargo build --release --manifest-path ../Cargo.toml
else
    echo "BUIlDING FOR DEBUG"

    cargo build --manifest-path ../Cargo.toml
fi
