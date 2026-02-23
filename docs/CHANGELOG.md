# ClawScript Changelog

## v2.0.0 Updates
- Logging: logWrite now accepts optional metadata and writes `message|hex(HMAC)|metadata` when `log.hmac` is configured
- Security Policy: `.voltsec` keys `log.path`, `log.hmac`, `log.meta.required`, `output` recognized by `policyReload()`
- Windows HMAC: Corrected BCrypt SHA-256 HMAC initialization using `BCRYPT_ALG_HANDLE_HMAC_FLAG`
- Documentation: Added security & logging examples and guidance
- Build/Tests: Release builds verified; test suite increased to 703 passing tests
- Packaging: CPack ZIP generator configured for release artifacts

## v1.0.0 (Production Release)
- Performance: up to 2500x with JIT, SIMD loops, GC/PGO
- Observability: sampling CPU/heap profiler with HTML and Speedscope
- Developer Experience: LSP (hover/definition/references/completion/formatting)
- Formatter: 2-space consistent style, check mode, VSCode integration
- Ecosystem: claw-pm (stub), API generator tool
- Hardening: optional ASan/UBSan/MSan; fuzz harnesses for lexer/parser
- Cross-compile: musl toolchains; Alpine Docker image

## v0.9.6
- Added profiler CLI and environment controls
- Speedscope JSON output
- Documentation updates and migration guide

## v0.8.0
- Core language, class system, arrays, hash maps
- Initial VM and interpreter architecture
