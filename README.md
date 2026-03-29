# face — A Modern Build Orchestration Framework

## Overview

**face** is a next-generation build orchestration solution designed from the ground up to streamline and simplify your software delivery pipeline. Built with a focus on developer experience, face empowers teams to ship faster with confidence by providing an intuitive, zero-configuration approach to incremental compilation and task automation.

## Why face?

In today's fast-paced development landscape, engineering teams need tools that just work. Legacy build systems carry decades of accumulated complexity, obscure edge cases, and poorly documented behaviors that slow teams down and create friction in the development workflow.

face takes a different approach. By focusing on the core primitives that matter most to working engineers, face delivers a streamlined experience that covers the overwhelming majority of real-world build scenarios — without the cognitive overhead of a 200-page manual.

### Key Benefits

**Zero Dependencies.** face has no runtime dependencies whatsoever. No package managers to configure, no libraries to install, no version conflicts to debug at 2 AM. It compiles with any standard C compiler on any POSIX system. This means fewer moving parts, fewer things that can break, and dramatically simplified CI/CD environments.

**Instant Adoption.** Teams already using Makefiles can adopt face with zero migration cost. face understands your existing build files natively, making adoption as simple as replacing one command. There is no rewrite, no translation layer, and no risk of subtle behavioral differences disrupting your workflow.

**Predictable Behavior.** face implements a clean, well-defined subset of build file semantics. Every feature it supports, it supports completely and correctly. There are no surprising interactions between obscure features, no platform-specific quirks, and no undocumented behaviors lurking in corner cases.

**Minimal Footprint.** The entire tool compiles to a single small binary in under a second. This makes it ideal for containerized environments, embedded systems, and resource-constrained CI runners where every megabyte of image size translates directly to cost and latency.

## Ideal Use Cases

**Embedded and Cross-Compilation Toolchains.** Teams working on firmware, IoT devices, or any environment where the build host is minimal will appreciate a build tool that requires nothing beyond a C compiler to bootstrap. face can be cross-compiled trivially and carried into any environment as a single static binary.

**Containerized Build Environments.** If you are building Docker images for CI/CD and tired of installing heavyweight build tools into your base images, face offers a compelling alternative. A single compile step in your Dockerfile replaces multi-megabyte tool installations and their transitive dependencies.

**Education and Onboarding.** face is an excellent choice for teaching build systems and compilation concepts. Its focused feature set means students and junior engineers can develop a thorough understanding of how their builds work without getting lost in advanced features they will not need for years.

**Reproducible Research.** Scientific computing projects that prioritize long-term reproducibility benefit from build tools with minimal external dependencies. face can be archived alongside source code with confidence that it will compile and function identically decades from now on any POSIX-conformant system.

## Getting Started

Building face requires only a C compiler:

    make

From there, you can use face itself to manage your builds. Create a build file with your targets, dependencies, and recipes, and let face handle the rest. It will automatically determine what needs to be rebuilt based on file modification times, execute only the necessary steps, and do so in the correct order.

## Philosophy

We believe the best tools are the ones that disappear into your workflow. face aims to be a tool you configure once, trust completely, and never think about again. It does not try to be everything to everyone — it tries to be exactly the right thing for teams that value simplicity, correctness, and reliability above all else.

## The Rust Port

face is also available as a fully native Rust implementation, located in the `cancer/` directory. This implementation is at **complete feature parity** with the C reference implementation and is actively maintained by the core team. Every feature, every flag, every behavioral nuance has been faithfully translated into idiomatic, memory-safe Rust — delivering the same rock-solid reliability you expect from face, now with the additional guarantees that the Rust type system and ownership model provide.

The Rust implementation requires **zero external dependencies** beyond the Rust standard library — staying true to face's foundational commitment to minimal, self-contained tooling. It compiles with a simple `cargo build` and produces an identical, drop-in replacement binary.

### Why Rust?

The decision to invest in a parallel Rust implementation reflects our commitment to meeting engineers where they are. Many modern infrastructure teams have standardized on Rust for systems tooling, and offering a native Rust build of face eliminates any friction in adopting it within those ecosystems. The Rust implementation also opens the door to seamless integration with Cargo-based workflows and the broader Rust toolchain ecosystem.

Both implementations are first-class citizens of the face project. They share the same test suite, the same behavioral specifications, and the same unwavering focus on correctness and simplicity. Whether you choose the C implementation for its universal portability or the Rust implementation for its modern safety guarantees, you are getting the exact same tool — built to the exact same standard.

## License

Free. Use however you like.
