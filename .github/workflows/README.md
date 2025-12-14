# CI/CD Workflows

This directory contains GitHub Actions workflows for the edfio project.

## Workflows Overview

### 1. CI Workflow (`ci.yml`)

**Trigger**: Push/PR to `main` or `develop` branches, manual dispatch

**Purpose**: Build and test the library across multiple platforms and compilers.

**Jobs**:
- **build-and-test**: Builds and runs tests on:
  - Ubuntu with GCC 13 and Clang 17
  - macOS with default compiler
  - Windows with MSVC
- **sanitizers**: Runs tests with address and undefined behavior sanitizers on Ubuntu/Clang
- **examples**: Builds and runs example programs

**Artifacts**: Test results for each platform/compiler combination

### 2. Code Quality Workflow (`code-quality.yml`)

**Trigger**: Push/PR to `main` or `develop` branches, manual dispatch

**Purpose**: Enforce code quality standards.

**Jobs**:
- **clang-format**: Checks code formatting compliance
- **clang-tidy**: Performs static analysis

**Artifacts**: clang-tidy analysis results

### 3. Security Workflow (`security.yml`)

**Trigger**: Push/PR to `main` or `develop` branches, weekly schedule, manual dispatch

**Purpose**: Scan for security vulnerabilities.

**Jobs**:
- **codeql**: Runs GitHub's CodeQL security analysis

**Schedule**: Weekly on Monday at 00:00 UTC

## Workflow Status Badges

Add these badges to your README.md:

```markdown
![CI](https://github.com/idotta/edfio/workflows/CI/badge.svg)
![Code Quality](https://github.com/idotta/edfio/workflows/Code%20Quality/badge.svg)
![Security](https://github.com/idotta/edfio/workflows/Security/badge.svg)
```

## Local Testing

Before pushing changes, you can test locally:

```bash
# Build and test (same as CI)
cmake --preset debug
cmake --build build/debug
ctest --test-dir build/debug

# Check formatting (same as workflow)
find . -path './build*' -prune -o \( -name '*.cpp' -o -name '*.hpp' \) \
  -print0 | xargs -0 clang-format --dry-run --Werror

# Run with sanitizers
cmake --preset sanitize
cmake --build build/sanitize
ctest --test-dir build/sanitize
```

## Required Secrets

No additional secrets are required. The workflows use the default `GITHUB_TOKEN` provided by GitHub Actions.

## Maintenance

- Update compiler versions in CI as new versions become available
- Update action versions regularly for security and features
- Monitor CodeQL results and address any security findings
