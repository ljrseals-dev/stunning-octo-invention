# AURA-V M0 Addendum — Spec document vs. committed code

**Document audited:** `Vocal Hi-Fi — C++17 Real-Time Vocal Processor.md` (repo root, added in commit `78f9771`)
**Compared against:** the actual source in the same repo (audited in the M0 report)

**Headline finding:** the .md document is the original "delivery" artifact from the reconstruction conversation. It describes a codebase that does not match the committed code. The repo's CHANGELOG confirms this implicitly ("Parameter smoothing is incomplete; metadata/smoothing table from the original spec was not reproduced"), and the commit history shows the code was written by a later session ("Critic Harness Busy Works Beats -Turk") than the document ("Create Vocal Hi-Fi...").

Treat the .md as a requirements artifact with aspirational claims — not as a description of what the code does.

---

## 1. Document claims absent from the committed code

These are the document's three headline "DSP changes," and none of them exist in the committed source:

| Document claim | Committed code reality |
|---|---|
| **Zero-crossing detection** — bus gain updated only on raw-signal sign changes | **Absent.** Output gain smoothing is a plain one-pole (`smoothedOutputGain_ += 0.05*(target - current)`) with no zero-crossing logic at all |
| **Parallel "Neural-Neve" warmth** — dry/wet parallel mix, `tanh(x*2.5)`, drive 2.5 | **Absent.** Bus warmth is serial: `y *= 1 + warmth*0.04*tanh(y*3)`. The front-end warmth is also serial. No dry/wet parallel structure exists |
| **Mono up-mix → stereo processing → down-mix** — mono passes through Dolby Stretch and ADT (`adt_.process(data[0], data[0], frames)`) | **Absent.** The committed mono planar path skips Dolby Stretch and ADT entirely (`channels==2` guard). Mono gets EQ, warmth, de-ess, bus stages only |

Additional spec-vs-code gaps:

- **Sample-rate-derived 6 ms smoothing coefficient** computed in `prepare()`: absent — the committed code uses hardcoded per-sample constants (0.05), which is exactly the sample-rate dependence defect found in the M0 audit.
- **CMake layout:** doc specifies shared library `vocalhifi` + `tests/selftest.cpp`; repo builds a static library `vocal_hifi` + `tests/test_vocal_hifi.cpp`. Doc also lists a `.gitignore` that is not in the repo.

## 2. Places where the code is better than the document

The committed code also improved on the spec, which confirms the two diverged:

- **`vp_load_preset`**: the document calls it a stub that ignores JSON and returns `VP_OK`. The committed code has a full strict flat-JSON parser (duplicate-key rejection, escape/NaN rejection, range validation before commit, trailing-data rejection) with tests covering each error path.
- **Test suite**: the document's self-test is one second of silence with a finiteness check. The committed test additionally covers the `VP_WARN_BREATH_CLAMPED` warning bit, NaN parameter rejection, and preset parse errors.

## 3. Claims that are marketing, not engineering

The document's framing violates the repo's own honesty policy — the committed `README` and `BUILD_STATUS.md` disclaim exactly what the document asserts:

| Document says | Repo's own BUILD_STATUS.md says |
|---|---|
| "FINAL DELIVERY CONFIRMED", "COMPILATION-READY" | "This project is a reconstructed baseline, not verified against the missing original source" |
| "zero-allocation and real-time-safe", "no dynamic allocation, locking, logging, or I/O" in the callback | "No production audio-quality, latency, aliasing, denormal, or hard real-time claims are made" |
| Self-test "confirms zero-allocation real-time behavior" | A silence pass-through cannot confirm real-time behavior; the repo's own status file makes no such claim |
| "Asymmetric warmth" | The document's own formula (`tanh`-based, odd function) is symmetric — the label is wrong in both the doc and the code |

Per the AURA-V rule (physics first, myth second), the "Neural-Neve," "Dolby Stretch" branding and delivery confirmations are non-technical wrappers. The names can survive as node labels; the claims cannot enter the engineering spec.

## 4. Impact on the build plan

1. **The document is now the requirements source, not the code description.** For M1 node extraction, take the *document's* stage list (which includes zero-crossing gain smoothing, parallel warmth, and the mono up-mix path) as the intended feature set, and the *code* as the current partial implementation. The delta above is the M1 work list.
2. **The three missing "DSP changes" are actually useful features.** Zero-crossing-synchronized gain updates reduce zipper noise on output gain changes (relevant to harness §6 zipper-noise detection). The parallel dry/wet warmth structure is the correct topology for the Stage 2 "Character" slot in the eight-stage model — serial warmth cannot be bypassed cleanly. The mono up-mix path fixes the planar/interleaved parity defect from the M0 audit. Implement all three during node extraction rather than patching the hardcoded chain.
3. **The 6 ms sample-rate-derived smoothing coefficient** from the spec is the correct design and should be implemented per-axis with the smoothing table the CHANGELOG says was never reproduced. This aligns with the M2 control-plane requirement (per-axis `smoothing_ms`, no zipper noise).
4. **Keep the committed JSON parser.** It is stricter and better tested than the document's stub. Do not "fix" the code to match the document.
5. **The document's delivery language ("FINAL DELIVERY CONFIRMED") must be excluded from AURA-V documentation** the same way the Lilibet Windsor authorship artifact was — it is a provenance/marketing artifact that the repo's own status files contradict.

## 5. Verdict

Two artifacts, one repo, three states: the document describes code that was never committed, the committed code contains a parser the document doesn't know about, and the CHANGELOG admits the smoothing spec was dropped. The committed code is the ground truth for M0; the document is the requirements input for M1. The gap between them is small, well-defined, and — usefully — every missing feature is one the build plan already wanted.
