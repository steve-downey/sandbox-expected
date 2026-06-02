/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "beman::expected", "index.html", [
    [ "Conformance Audit: beman::expected vs std::expected (C++26)", "md_docs_conformance_audit.html", [
      [ "1. unexpected<E> [expected.unexpected]", "md_docs_conformance_audit.html#autotoc_md2", [
        [ "1.1 Static assertions [expected.un.general] para 2", "md_docs_conformance_audit.html#autotoc_md3", null ],
        [ "1.2 Constructors [expected.un.cons]", "md_docs_conformance_audit.html#autotoc_md4", null ],
        [ "1.3 Observers [expected.un.obs]", "md_docs_conformance_audit.html#autotoc_md5", null ],
        [ "1.4 Swap [expected.un.swap]", "md_docs_conformance_audit.html#autotoc_md6", null ],
        [ "1.5 Equality operator [expected.un.eq]", "md_docs_conformance_audit.html#autotoc_md7", null ],
        [ "1.6 CTAD", "md_docs_conformance_audit.html#autotoc_md8", null ]
      ] ],
      [ "2. bad_expected_access<void> [expected.bad.void]", "md_docs_conformance_audit.html#autotoc_md10", null ],
      [ "3. bad_expected_access<E> [expected.bad]", "md_docs_conformance_audit.html#autotoc_md12", null ],
      [ "4. expected<T, E> Primary Template [expected.expected]", "md_docs_conformance_audit.html#autotoc_md14", [
        [ "4.1 Type aliases and rebind", "md_docs_conformance_audit.html#autotoc_md15", null ],
        [ "4.2 Static assertions [expected.object.general] para 2-3", "md_docs_conformance_audit.html#autotoc_md16", null ],
        [ "4.3 Constructors [expected.object.cons]", "md_docs_conformance_audit.html#autotoc_md17", [
          [ "Default constructor", "md_docs_conformance_audit.html#autotoc_md18", null ],
          [ "Copy constructor", "md_docs_conformance_audit.html#autotoc_md19", null ],
          [ "Move constructor", "md_docs_conformance_audit.html#autotoc_md20", null ],
          [ "Converting constructors from expected<U, G>", "md_docs_conformance_audit.html#autotoc_md21", null ],
          [ "Value constructor <tt>expected(U&& v)</tt>", "md_docs_conformance_audit.html#autotoc_md22", null ],
          [ "unexpected<G> constructors", "md_docs_conformance_audit.html#autotoc_md23", null ],
          [ "in_place_t constructors", "md_docs_conformance_audit.html#autotoc_md24", null ],
          [ "unexpect_t constructors", "md_docs_conformance_audit.html#autotoc_md25", null ]
        ] ],
        [ "4.4 Destructor [expected.object.dtor]", "md_docs_conformance_audit.html#autotoc_md26", null ],
        [ "4.5 Assignment [expected.object.assign]", "md_docs_conformance_audit.html#autotoc_md27", [
          [ "Copy assignment", "md_docs_conformance_audit.html#autotoc_md28", null ],
          [ "Move assignment", "md_docs_conformance_audit.html#autotoc_md29", null ],
          [ "Value assignment <tt>operator=(U&&)</tt>", "md_docs_conformance_audit.html#autotoc_md30", null ],
          [ "unexpected<G> assignment", "md_docs_conformance_audit.html#autotoc_md31", null ],
          [ "emplace", "md_docs_conformance_audit.html#autotoc_md32", null ]
        ] ],
        [ "4.6 Swap [expected.object.swap]", "md_docs_conformance_audit.html#autotoc_md33", null ],
        [ "4.7 Observers [expected.object.obs]", "md_docs_conformance_audit.html#autotoc_md34", [
          [ "operator->", "md_docs_conformance_audit.html#autotoc_md35", null ],
          [ "operator*", "md_docs_conformance_audit.html#autotoc_md36", null ],
          [ "operator bool / has_value()", "md_docs_conformance_audit.html#autotoc_md37", null ],
          [ "value()", "md_docs_conformance_audit.html#autotoc_md38", null ],
          [ "error()", "md_docs_conformance_audit.html#autotoc_md39", null ],
          [ "value_or()", "md_docs_conformance_audit.html#autotoc_md40", null ],
          [ "error_or()", "md_docs_conformance_audit.html#autotoc_md41", null ]
        ] ],
        [ "4.8 Monadic operations [expected.object.monadic]", "md_docs_conformance_audit.html#autotoc_md42", [
          [ "and_then (all 4 overloads)", "md_docs_conformance_audit.html#autotoc_md43", null ],
          [ "or_else (all 4 overloads)", "md_docs_conformance_audit.html#autotoc_md44", null ],
          [ "transform (all 4 overloads)", "md_docs_conformance_audit.html#autotoc_md45", null ],
          [ "transform_error (all 4 overloads)", "md_docs_conformance_audit.html#autotoc_md46", null ]
        ] ],
        [ "4.9 Equality operators [expected.object.eq]", "md_docs_conformance_audit.html#autotoc_md47", [
          [ "operator==(expected, expected<T2, E2>)", "md_docs_conformance_audit.html#autotoc_md48", null ],
          [ "operator==(expected, T2)", "md_docs_conformance_audit.html#autotoc_md49", null ],
          [ "operator==(expected, unexpected<E2>)", "md_docs_conformance_audit.html#autotoc_md50", null ]
        ] ]
      ] ],
      [ "5. expected<void, E> Specialization [expected.void]", "md_docs_conformance_audit.html#autotoc_md52", [
        [ "5.1 Static assertions", "md_docs_conformance_audit.html#autotoc_md53", null ],
        [ "5.2 Constructors [expected.void.cons]", "md_docs_conformance_audit.html#autotoc_md54", null ],
        [ "5.3 Destructor [expected.void.dtor]", "md_docs_conformance_audit.html#autotoc_md55", null ],
        [ "5.4 Assignment [expected.void.assign]", "md_docs_conformance_audit.html#autotoc_md56", null ],
        [ "5.5 Swap [expected.void.swap]", "md_docs_conformance_audit.html#autotoc_md57", null ],
        [ "5.6 Observers [expected.void.obs]", "md_docs_conformance_audit.html#autotoc_md58", null ],
        [ "5.7 Monadic operations [expected.void.monadic]", "md_docs_conformance_audit.html#autotoc_md59", [
          [ "and_then", "md_docs_conformance_audit.html#autotoc_md60", null ],
          [ "or_else", "md_docs_conformance_audit.html#autotoc_md61", null ],
          [ "transform", "md_docs_conformance_audit.html#autotoc_md62", null ],
          [ "transform_error", "md_docs_conformance_audit.html#autotoc_md63", null ]
        ] ],
        [ "5.8 Equality operators [expected.void.eq]", "md_docs_conformance_audit.html#autotoc_md64", null ]
      ] ],
      [ "Summary", "md_docs_conformance_audit.html#autotoc_md66", [
        [ "Extensions (not in standard, kept as conforming)", "md_docs_conformance_audit.html#autotoc_md67", null ]
      ] ]
    ] ],
    [ "Fix 1: Constructor, Assignment, and Equality Constraints", "md_docs_conformance_fixes_fix1_constraints.html", [
      [ "Goal", "md_docs_conformance_fixes_fix1_constraints.html#autotoc_md70", null ],
      [ "Changes", "md_docs_conformance_fixes_fix1_constraints.html#autotoc_md71", [
        [ "1. Add <tt>converts_from_any_cvref</tt> helper", "md_docs_conformance_fixes_fix1_constraints.html#autotoc_md72", null ],
        [ "2. Fix converting constructors from expected<U, G>", "md_docs_conformance_fixes_fix1_constraints.html#autotoc_md73", null ],
        [ "3. Fix value constructor <tt>expected(U&&)</tt> constraints", "md_docs_conformance_fixes_fix1_constraints.html#autotoc_md74", null ],
        [ "4. Fix value assignment <tt>operator=(U&&)</tt>", "md_docs_conformance_fixes_fix1_constraints.html#autotoc_md75", null ],
        [ "5. Fix move assignment constraint", "md_docs_conformance_fixes_fix1_constraints.html#autotoc_md76", null ],
        [ "6. Fix <tt>operator==(const expected&, const T2&)</tt>", "md_docs_conformance_fixes_fix1_constraints.html#autotoc_md77", null ]
      ] ],
      [ "Tests", "md_docs_conformance_fixes_fix1_constraints.html#autotoc_md78", [
        [ "New tests (beman-only target — these test constraint behavior)", "md_docs_conformance_fixes_fix1_constraints.html#autotoc_md79", null ],
        [ "Negative compile tests", "md_docs_conformance_fixes_fix1_constraints.html#autotoc_md80", null ],
        [ "Existing tests", "md_docs_conformance_fixes_fix1_constraints.html#autotoc_md81", null ]
      ] ],
      [ "Verification", "md_docs_conformance_fixes_fix1_constraints.html#autotoc_md82", null ],
      [ "Handoff", "md_docs_conformance_fixes_fix1_constraints.html#autotoc_md83", null ]
    ] ],
    [ "Fix 2: Trivial Special Member Functions", "md_docs_conformance_fixes_fix2_trivial_smfs.html", [
      [ "Goal", "md_docs_conformance_fixes_fix2_trivial_smfs.html#autotoc_md86", null ],
      [ "Background", "md_docs_conformance_fixes_fix2_trivial_smfs.html#autotoc_md87", null ],
      [ "Changes", "md_docs_conformance_fixes_fix2_trivial_smfs.html#autotoc_md88", [
        [ "Primary template: trivial copy constructor", "md_docs_conformance_fixes_fix2_trivial_smfs.html#autotoc_md89", null ],
        [ "Primary template: trivial move constructor", "md_docs_conformance_fixes_fix2_trivial_smfs.html#autotoc_md90", null ],
        [ "Primary template: trivial copy assignment", "md_docs_conformance_fixes_fix2_trivial_smfs.html#autotoc_md91", null ],
        [ "Primary template: trivial move assignment", "md_docs_conformance_fixes_fix2_trivial_smfs.html#autotoc_md92", null ],
        [ "Void specialization: trivial copy assignment", "md_docs_conformance_fixes_fix2_trivial_smfs.html#autotoc_md93", null ],
        [ "Void specialization: trivial move assignment", "md_docs_conformance_fixes_fix2_trivial_smfs.html#autotoc_md94", null ]
      ] ],
      [ "Tests", "md_docs_conformance_fixes_fix2_trivial_smfs.html#autotoc_md95", null ],
      [ "Verification", "md_docs_conformance_fixes_fix2_trivial_smfs.html#autotoc_md96", null ],
      [ "Handoff", "md_docs_conformance_fixes_fix2_trivial_smfs.html#autotoc_md97", null ]
    ] ],
    [ "Fix 3: Monadic Operation Constraints", "md_docs_conformance_fixes_fix3_monadic_constraints.html", [
      [ "Goal", "md_docs_conformance_fixes_fix3_monadic_constraints.html#autotoc_md100", null ],
      [ "Which operations need constraints", "md_docs_conformance_fixes_fix3_monadic_constraints.html#autotoc_md101", [
        [ "Primary template <tt>expected<T, E></tt>", "md_docs_conformance_fixes_fix3_monadic_constraints.html#autotoc_md102", null ],
        [ "Void specialization <tt>expected<void, E></tt>", "md_docs_conformance_fixes_fix3_monadic_constraints.html#autotoc_md103", null ]
      ] ],
      [ "Implementation", "md_docs_conformance_fixes_fix3_monadic_constraints.html#autotoc_md104", null ],
      [ "Tests", "md_docs_conformance_fixes_fix3_monadic_constraints.html#autotoc_md105", null ],
      [ "Verification", "md_docs_conformance_fixes_fix3_monadic_constraints.html#autotoc_md106", null ],
      [ "Handoff", "md_docs_conformance_fixes_fix3_monadic_constraints.html#autotoc_md107", null ]
    ] ],
    [ "Fix 4: Mandates static_asserts", "md_docs_conformance_fixes_fix4_mandates.html", [
      [ "Goal", "md_docs_conformance_fixes_fix4_mandates.html#autotoc_md110", null ],
      [ "Changes", "md_docs_conformance_fixes_fix4_mandates.html#autotoc_md111", [
        [ "1. Primary template static_assert: T not a specialization of unexpected", "md_docs_conformance_fixes_fix4_mandates.html#autotoc_md112", null ],
        [ "2. value() Mandates — primary template", "md_docs_conformance_fixes_fix4_mandates.html#autotoc_md113", null ],
        [ "3. value() Mandates — void specialization", "md_docs_conformance_fixes_fix4_mandates.html#autotoc_md114", null ],
        [ "4. value_or() Mandates — primary template", "md_docs_conformance_fixes_fix4_mandates.html#autotoc_md115", null ],
        [ "5. error_or() Mandates — both primary and void", "md_docs_conformance_fixes_fix4_mandates.html#autotoc_md116", null ],
        [ "6. transform() Mandates — both primary and void", "md_docs_conformance_fixes_fix4_mandates.html#autotoc_md117", null ],
        [ "7. transform_error() Mandates — both primary and void", "md_docs_conformance_fixes_fix4_mandates.html#autotoc_md118", null ]
      ] ],
      [ "Tests", "md_docs_conformance_fixes_fix4_mandates.html#autotoc_md119", null ],
      [ "Verification", "md_docs_conformance_fixes_fix4_mandates.html#autotoc_md120", null ],
      [ "Handoff", "md_docs_conformance_fixes_fix4_mandates.html#autotoc_md121", null ]
    ] ],
    [ "Fix 5: Hardened Preconditions and Minor Fixes", "md_docs_conformance_fixes_fix5_preconditions_and_minor.html", [
      [ "Goal", "md_docs_conformance_fixes_fix5_preconditions_and_minor.html#autotoc_md124", null ],
      [ "Changes", "md_docs_conformance_fixes_fix5_preconditions_and_minor.html#autotoc_md125", [
        [ "1. Hardened preconditions (<tt>include/beman/expected/expected.hpp</tt>)", "md_docs_conformance_fixes_fix5_preconditions_and_minor.html#autotoc_md126", null ],
        [ "2. unexpected friend swap constraint (<tt>include/beman/expected/unexpected.hpp</tt>)", "md_docs_conformance_fixes_fix5_preconditions_and_minor.html#autotoc_md127", null ],
        [ "3. Void specialization <tt>or_else</tt>: use <tt>is_same_v</tt> not <tt>is_void_v</tt>", "md_docs_conformance_fixes_fix5_preconditions_and_minor.html#autotoc_md128", null ],
        [ "4. Void specialization <tt>transform_error</tt>: use <tt>expected<T, G></tt> not <tt>expected<void, G></tt>", "md_docs_conformance_fixes_fix5_preconditions_and_minor.html#autotoc_md129", null ]
      ] ],
      [ "Tests", "md_docs_conformance_fixes_fix5_preconditions_and_minor.html#autotoc_md130", [
        [ "Hardened precondition tests", "md_docs_conformance_fixes_fix5_preconditions_and_minor.html#autotoc_md131", null ],
        [ "unexpected swap constraint test", "md_docs_conformance_fixes_fix5_preconditions_and_minor.html#autotoc_md132", null ],
        [ "or_else / transform_error minor fix tests", "md_docs_conformance_fixes_fix5_preconditions_and_minor.html#autotoc_md133", null ]
      ] ],
      [ "Verification", "md_docs_conformance_fixes_fix5_preconditions_and_minor.html#autotoc_md134", null ],
      [ "Handoff", "md_docs_conformance_fixes_fix5_preconditions_and_minor.html#autotoc_md135", null ]
    ] ],
    [ "Plan: Conformance Fixes for expected<T, E> and expected<void, E>", "md_docs_conformance_fixes_index.html", [
      [ "Motivation", "md_docs_conformance_fixes_index.html#autotoc_md137", null ],
      [ "Reference Materials", "md_docs_conformance_fixes_index.html#autotoc_md138", null ],
      [ "Phase Overview", "md_docs_conformance_fixes_index.html#autotoc_md139", null ],
      [ "Standing Conventions", "md_docs_conformance_fixes_index.html#autotoc_md140", [
        [ "std cross-check rule", "md_docs_conformance_fixes_index.html#autotoc_md141", null ]
      ] ],
      [ "Step Details", "md_docs_conformance_fixes_index.html#autotoc_md142", null ],
      [ "Checklist", "md_docs_conformance_fixes_index.html#autotoc_md143", null ],
      [ "After All Fixes", "md_docs_conformance_fixes_index.html#autotoc_md144", null ]
    ] ],
    [ "Instructions for Fix Steps", "md_docs_conformance_fixes_instructions.html", [
      [ "Prompt", "md_docs_conformance_fixes_instructions.html#autotoc_md147", null ]
    ] ],
    [ "Changes Since Last Version", "md_docs_optional_references.html", [
      [ "Comparison table", "md_docs_optional_references.html#autotoc_md149", [
        [ "Using a raw pointer result for an element search function", "md_docs_optional_references.html#autotoc_md150", null ],
        [ "returning result of an element search function via a (smart) pointer", "md_docs_optional_references.html#autotoc_md151", null ],
        [ "returning result of an element search function via an iterator", "md_docs_optional_references.html#autotoc_md152", null ],
        [ "Using an optional<T*> as a substitute for optional<T&>", "md_docs_optional_references.html#autotoc_md153", null ]
      ] ],
      [ "Motivation", "md_docs_optional_references.html#autotoc_md154", null ],
      [ "Design", "md_docs_optional_references.html#autotoc_md155", [
        [ "Relational Operations", "md_docs_optional_references.html#autotoc_md156", null ],
        [ "make_optional", "md_docs_optional_references.html#autotoc_md157", null ],
        [ "Trivial construction", "md_docs_optional_references.html#autotoc_md158", null ],
        [ "Value Category Affects value()", "md_docs_optional_references.html#autotoc_md159", null ],
        [ "Shallow vs Deep const", "md_docs_optional_references.html#autotoc_md160", null ],
        [ "Conditional Explicit", "md_docs_optional_references.html#autotoc_md161", null ],
        [ "value_or", "md_docs_optional_references.html#autotoc_md162", null ],
        [ "in_place_t construction", "md_docs_optional_references.html#autotoc_md163", null ],
        [ "Converting assignment", "md_docs_optional_references.html#autotoc_md164", null ],
        [ "Compiler Explorer Playground", "md_docs_optional_references.html#autotoc_md165", null ]
      ] ],
      [ "Principles for Reification of Design", "md_docs_optional_references.html#autotoc_md166", [
        [ "Construction from temporary", "md_docs_optional_references.html#autotoc_md167", null ],
        [ "Deleting dangling overloads", "md_docs_optional_references.html#autotoc_md168", null ],
        [ "Assignment of optional<T&>", "md_docs_optional_references.html#autotoc_md169", null ],
        [ "Copy and Assignment of optional<U&>&& to optional<T>", "md_docs_optional_references.html#autotoc_md170", null ]
      ] ],
      [ "Proposal", "md_docs_optional_references.html#autotoc_md171", null ],
      [ "Wording", "md_docs_optional_references.html#autotoc_md172", null ],
      [ "Impact on the standard", "md_docs_optional_references.html#autotoc_md173", null ],
      [ "Acknowledgments", "md_docs_optional_references.html#autotoc_md174", null ],
      [ "Document history", "md_docs_optional_references.html#autotoc_md175", null ],
      [ "Implementation", "md_docs_optional_references.html#autotoc_md176", null ]
    ] ],
    [ "Handoff: After Fix 5 (All Conformance Fixes Complete)", "md_docs_plan_handoff_next.html", [
      [ "What Was Done", "md_docs_plan_handoff_next.html#autotoc_md178", [
        [ "Changes in Fix 5", "md_docs_plan_handoff_next.html#autotoc_md179", null ],
        [ "Test count", "md_docs_plan_handoff_next.html#autotoc_md180", null ]
      ] ],
      [ "Build Commands", "md_docs_plan_handoff_next.html#autotoc_md181", null ],
      [ "Conformance Fix Checklist", "md_docs_plan_handoff_next.html#autotoc_md182", null ],
      [ "All Conformance Fixes Complete", "md_docs_plan_handoff_next.html#autotoc_md183", null ],
      [ "State of the Implementation", "md_docs_plan_handoff_next.html#autotoc_md184", null ],
      [ "Upstream Merge (2026-06-02)", "md_docs_plan_handoff_next.html#autotoc_md185", null ]
    ] ],
    [ "Handoff: Starting State", "md_docs_plan_handoff.html", [
      [ "Repository", "md_docs_plan_handoff.html#autotoc_md187", null ],
      [ "Current State", "md_docs_plan_handoff.html#autotoc_md188", [
        [ "Files", "md_docs_plan_handoff.html#autotoc_md189", null ],
        [ "Build System", "md_docs_plan_handoff.html#autotoc_md190", null ],
        [ "Coding Rules", "md_docs_plan_handoff.html#autotoc_md191", null ],
        [ "Reference Implementation", "md_docs_plan_handoff.html#autotoc_md192", null ]
      ] ],
      [ "Plan", "md_docs_plan_handoff.html#autotoc_md193", null ],
      [ "Test Plan Documents", "md_docs_plan_handoff.html#autotoc_md194", null ],
      [ "What Comes Next", "md_docs_plan_handoff.html#autotoc_md195", null ]
    ] ],
    [ "Plan: Implement expected<T&, E> and expected<T, E&> Reference Specializations", "md_docs_plan_index.html", [
      [ "Proposal", "md_docs_plan_index.html#autotoc_md197", null ],
      [ "Reference Materials", "md_docs_plan_index.html#autotoc_md198", null ],
      [ "Phase Overview", "md_docs_plan_index.html#autotoc_md199", null ],
      [ "Standing Conventions", "md_docs_plan_index.html#autotoc_md200", null ],
      [ "Step Details", "md_docs_plan_index.html#autotoc_md201", null ],
      [ "Checklist", "md_docs_plan_index.html#autotoc_md202", null ]
    ] ],
    [ "Step 1: Implement unexpected<E>", "md_docs_plan_step1_unexpected.html", [
      [ "Goal", "md_docs_plan_step1_unexpected.html#autotoc_md205", null ],
      [ "Context for Executing Agent", "md_docs_plan_step1_unexpected.html#autotoc_md206", [
        [ "Key files", "md_docs_plan_step1_unexpected.html#autotoc_md207", null ],
        [ "Standard reference", "md_docs_plan_step1_unexpected.html#autotoc_md208", null ],
        [ "Constraints on the converting constructor", "md_docs_plan_step1_unexpected.html#autotoc_md209", null ],
        [ "Constraints on E", "md_docs_plan_step1_unexpected.html#autotoc_md210", null ]
      ] ],
      [ "Deliverables", "md_docs_plan_step1_unexpected.html#autotoc_md211", null ],
      [ "Procedure", "md_docs_plan_step1_unexpected.html#autotoc_md212", null ],
      [ "Verification", "md_docs_plan_step1_unexpected.html#autotoc_md213", null ],
      [ "Handoff to Step 2", "md_docs_plan_step1_unexpected.html#autotoc_md214", null ]
    ] ],
    [ "Step 10: Implement expected<void, E&> Specialization", "md_docs_plan_step10_expected_void_ref_e.html", [
      [ "Goal", "md_docs_plan_step10_expected_void_ref_e.html#autotoc_md217", null ],
      [ "Context for Executing Agent", "md_docs_plan_step10_expected_void_ref_e.html#autotoc_md218", [
        [ "Storage model", "md_docs_plan_step10_expected_void_ref_e.html#autotoc_md219", null ],
        [ "Key properties", "md_docs_plan_step10_expected_void_ref_e.html#autotoc_md220", null ],
        [ "Constructors", "md_docs_plan_step10_expected_void_ref_e.html#autotoc_md221", null ],
        [ "Assignment", "md_docs_plan_step10_expected_void_ref_e.html#autotoc_md222", null ],
        [ "Observers", "md_docs_plan_step10_expected_void_ref_e.html#autotoc_md223", null ],
        [ "Monadic operations", "md_docs_plan_step10_expected_void_ref_e.html#autotoc_md224", null ]
      ] ],
      [ "Deliverables", "md_docs_plan_step10_expected_void_ref_e.html#autotoc_md225", null ],
      [ "Procedure", "md_docs_plan_step10_expected_void_ref_e.html#autotoc_md226", null ],
      [ "Verification", "md_docs_plan_step10_expected_void_ref_e.html#autotoc_md227", null ],
      [ "Completion", "md_docs_plan_step10_expected_void_ref_e.html#autotoc_md228", null ]
    ] ],
    [ "Step 2: Implement bad_expected_access", "md_docs_plan_step2_bad_expected_access.html", [
      [ "Goal", "md_docs_plan_step2_bad_expected_access.html#autotoc_md231", null ],
      [ "Context for Executing Agent", "md_docs_plan_step2_bad_expected_access.html#autotoc_md232", [
        [ "Key files", "md_docs_plan_step2_bad_expected_access.html#autotoc_md233", null ],
        [ "Standard reference", "md_docs_plan_step2_bad_expected_access.html#autotoc_md234", null ],
        [ "Notes", "md_docs_plan_step2_bad_expected_access.html#autotoc_md235", null ]
      ] ],
      [ "Deliverables", "md_docs_plan_step2_bad_expected_access.html#autotoc_md236", null ],
      [ "Procedure", "md_docs_plan_step2_bad_expected_access.html#autotoc_md237", null ],
      [ "Verification", "md_docs_plan_step2_bad_expected_access.html#autotoc_md238", null ],
      [ "Handoff to Step 3", "md_docs_plan_step2_bad_expected_access.html#autotoc_md239", null ]
    ] ],
    [ "Step 3: Implement expected<T, E> Primary Template", "md_docs_plan_step3_expected_primary.html", [
      [ "Goal", "md_docs_plan_step3_expected_primary.html#autotoc_md242", null ],
      [ "Context for Executing Agent", "md_docs_plan_step3_expected_primary.html#autotoc_md243", [
        [ "Key files", "md_docs_plan_step3_expected_primary.html#autotoc_md244", null ],
        [ "Standard reference (non-monadic subset)", "md_docs_plan_step3_expected_primary.html#autotoc_md245", null ],
        [ "Storage", "md_docs_plan_step3_expected_primary.html#autotoc_md246", null ],
        [ "Constraints", "md_docs_plan_step3_expected_primary.html#autotoc_md247", null ],
        [ "Exception safety in assignment", "md_docs_plan_step3_expected_primary.html#autotoc_md248", null ]
      ] ],
      [ "Deliverables", "md_docs_plan_step3_expected_primary.html#autotoc_md249", null ],
      [ "Procedure", "md_docs_plan_step3_expected_primary.html#autotoc_md250", null ],
      [ "Verification", "md_docs_plan_step3_expected_primary.html#autotoc_md251", null ],
      [ "Handoff to Step 4", "md_docs_plan_step3_expected_primary.html#autotoc_md252", null ]
    ] ],
    [ "Step 4: Implement expected<void, E> Specialization", "md_docs_plan_step4_expected_void.html", [
      [ "Goal", "md_docs_plan_step4_expected_void.html#autotoc_md255", null ],
      [ "Context for Executing Agent", "md_docs_plan_step4_expected_void.html#autotoc_md256", [
        [ "Key differences from primary template", "md_docs_plan_step4_expected_void.html#autotoc_md257", null ],
        [ "Constructors [expected.void.cons]", "md_docs_plan_step4_expected_void.html#autotoc_md258", null ],
        [ "Storage", "md_docs_plan_step4_expected_void.html#autotoc_md259", null ]
      ] ],
      [ "Deliverables", "md_docs_plan_step4_expected_void.html#autotoc_md260", null ],
      [ "Procedure", "md_docs_plan_step4_expected_void.html#autotoc_md261", null ],
      [ "Verification", "md_docs_plan_step4_expected_void.html#autotoc_md262", null ],
      [ "Handoff to Step 5", "md_docs_plan_step4_expected_void.html#autotoc_md263", null ]
    ] ],
    [ "Step 5: Monadic Operations for expected<T, E>", "md_docs_plan_step5_expected_primary_monadic.html", [
      [ "Goal", "md_docs_plan_step5_expected_primary_monadic.html#autotoc_md266", null ],
      [ "Context for Executing Agent", "md_docs_plan_step5_expected_primary_monadic.html#autotoc_md267", [
        [ "Monadic operations overview", "md_docs_plan_step5_expected_primary_monadic.html#autotoc_md268", null ],
        [ "Key constraints per [expected.object.monadic]", "md_docs_plan_step5_expected_primary_monadic.html#autotoc_md269", null ],
        [ "Ref-qualification pattern", "md_docs_plan_step5_expected_primary_monadic.html#autotoc_md270", null ]
      ] ],
      [ "Deliverables", "md_docs_plan_step5_expected_primary_monadic.html#autotoc_md271", null ],
      [ "Procedure", "md_docs_plan_step5_expected_primary_monadic.html#autotoc_md272", null ],
      [ "Verification", "md_docs_plan_step5_expected_primary_monadic.html#autotoc_md273", null ],
      [ "Handoff to Step 6", "md_docs_plan_step5_expected_primary_monadic.html#autotoc_md274", null ]
    ] ],
    [ "Step 6: Monadic Operations for expected<void, E>", "md_docs_plan_step6_expected_void_monadic.html", [
      [ "Goal", "md_docs_plan_step6_expected_void_monadic.html#autotoc_md277", null ],
      [ "Context for Executing Agent", "md_docs_plan_step6_expected_void_monadic.html#autotoc_md278", [
        [ "Differences from primary template monadic ops", "md_docs_plan_step6_expected_void_monadic.html#autotoc_md279", null ],
        [ "Constraints per [expected.void.monadic]", "md_docs_plan_step6_expected_void_monadic.html#autotoc_md280", null ]
      ] ],
      [ "Deliverables", "md_docs_plan_step6_expected_void_monadic.html#autotoc_md281", null ],
      [ "Procedure", "md_docs_plan_step6_expected_void_monadic.html#autotoc_md282", null ],
      [ "Verification", "md_docs_plan_step6_expected_void_monadic.html#autotoc_md283", null ],
      [ "Handoff to Step 7", "md_docs_plan_step6_expected_void_monadic.html#autotoc_md284", null ]
    ] ],
    [ "Step 7: Implement expected<T&, E> Reference Specialization", "md_docs_plan_step7_expected_ref_t.html", [
      [ "Goal", "md_docs_plan_step7_expected_ref_t.html#autotoc_md287", null ],
      [ "Context for Executing Agent", "md_docs_plan_step7_expected_ref_t.html#autotoc_md288", [
        [ "Design principles (from optional<T&>)", "md_docs_plan_step7_expected_ref_t.html#autotoc_md289", null ],
        [ "Storage model", "md_docs_plan_step7_expected_ref_t.html#autotoc_md290", null ],
        [ "Reference binding helper", "md_docs_plan_step7_expected_ref_t.html#autotoc_md291", null ],
        [ "Constructors", "md_docs_plan_step7_expected_ref_t.html#autotoc_md292", null ],
        [ "Assignment (rebind semantics)", "md_docs_plan_step7_expected_ref_t.html#autotoc_md293", null ],
        [ "Observers", "md_docs_plan_step7_expected_ref_t.html#autotoc_md294", null ],
        [ "Monadic operations", "md_docs_plan_step7_expected_ref_t.html#autotoc_md295", null ],
        [ "reference_constructs_from_temporary_v", "md_docs_plan_step7_expected_ref_t.html#autotoc_md296", null ]
      ] ],
      [ "Deliverables", "md_docs_plan_step7_expected_ref_t.html#autotoc_md297", null ],
      [ "Procedure", "md_docs_plan_step7_expected_ref_t.html#autotoc_md298", null ],
      [ "Verification", "md_docs_plan_step7_expected_ref_t.html#autotoc_md299", null ],
      [ "Handoff to Step 8", "md_docs_plan_step7_expected_ref_t.html#autotoc_md300", null ]
    ] ],
    [ "Step 8: Implement expected<T, E&> Error-Reference Specialization", "md_docs_plan_step8_expected_ref_e.html", [
      [ "Goal", "md_docs_plan_step8_expected_ref_e.html#autotoc_md303", null ],
      [ "Context for Executing Agent", "md_docs_plan_step8_expected_ref_e.html#autotoc_md304", [
        [ "Storage model", "md_docs_plan_step8_expected_ref_e.html#autotoc_md305", null ],
        [ "Design principles", "md_docs_plan_step8_expected_ref_e.html#autotoc_md306", null ],
        [ "Constructors", "md_docs_plan_step8_expected_ref_e.html#autotoc_md307", null ],
        [ "Assignment (rebind on error side)", "md_docs_plan_step8_expected_ref_e.html#autotoc_md308", null ],
        [ "Observers", "md_docs_plan_step8_expected_ref_e.html#autotoc_md309", null ],
        [ "Monadic operations", "md_docs_plan_step8_expected_ref_e.html#autotoc_md310", null ]
      ] ],
      [ "Deliverables", "md_docs_plan_step8_expected_ref_e.html#autotoc_md311", null ],
      [ "Procedure", "md_docs_plan_step8_expected_ref_e.html#autotoc_md312", null ],
      [ "Verification", "md_docs_plan_step8_expected_ref_e.html#autotoc_md313", null ],
      [ "Handoff to Step 9", "md_docs_plan_step8_expected_ref_e.html#autotoc_md314", null ]
    ] ],
    [ "Step 9: Implement expected<T&, E&> Both-Reference Specialization", "md_docs_plan_step9_expected_ref_both.html", [
      [ "Goal", "md_docs_plan_step9_expected_ref_both.html#autotoc_md317", null ],
      [ "Context for Executing Agent", "md_docs_plan_step9_expected_ref_both.html#autotoc_md318", [
        [ "Storage model", "md_docs_plan_step9_expected_ref_both.html#autotoc_md319", null ],
        [ "Key properties", "md_docs_plan_step9_expected_ref_both.html#autotoc_md320", null ],
        [ "Constructors", "md_docs_plan_step9_expected_ref_both.html#autotoc_md321", null ],
        [ "Assignment", "md_docs_plan_step9_expected_ref_both.html#autotoc_md322", null ],
        [ "Observers", "md_docs_plan_step9_expected_ref_both.html#autotoc_md323", null ],
        [ "Monadic operations", "md_docs_plan_step9_expected_ref_both.html#autotoc_md324", null ]
      ] ],
      [ "Deliverables", "md_docs_plan_step9_expected_ref_both.html#autotoc_md325", null ],
      [ "Procedure", "md_docs_plan_step9_expected_ref_both.html#autotoc_md326", null ],
      [ "Verification", "md_docs_plan_step9_expected_ref_both.html#autotoc_md327", null ],
      [ "Handoff to Step 10", "md_docs_plan_step9_expected_ref_both.html#autotoc_md328", null ]
    ] ],
    [ "Test Plan Overview — beman::expected", "md_docs_plan_tests_overview.html", [
      [ "Test Framework", "md_docs_plan_tests_overview.html#autotoc_md331", null ],
      [ "Standard Testing Conventions", "md_docs_plan_tests_overview.html#autotoc_md333", [
        [ "1. Header idempotence", "md_docs_plan_tests_overview.html#autotoc_md334", null ],
        [ "2. Three tiers of negative testing", "md_docs_plan_tests_overview.html#autotoc_md335", null ],
        [ "3. Negative compile test pattern (from transcode)", "md_docs_plan_tests_overview.html#autotoc_md336", null ],
        [ "4. Type-trait / static_assert tests", "md_docs_plan_tests_overview.html#autotoc_md337", null ],
        [ "5. Hardened precondition tests", "md_docs_plan_tests_overview.html#autotoc_md338", null ]
      ] ],
      [ "Files per Step", "md_docs_plan_tests_overview.html#autotoc_md340", null ],
      [ "Standard Reference Summary", "md_docs_plan_tests_overview.html#autotoc_md342", null ],
      [ "CMakeLists Structure", "md_docs_plan_tests_overview.html#autotoc_md344", null ]
    ] ],
    [ "Test Plan: Step 1 — unexpected<E>", "md_docs_plan_tests_step1.html", [
      [ "Testing Strategy", "md_docs_plan_tests_step1.html#autotoc_md347", null ],
      [ "Testable Statements from the Standard", "md_docs_plan_tests_step1.html#autotoc_md349", [
        [ "[expected.un.general] para 2 — ill-formed instantiations", "md_docs_plan_tests_step1.html#autotoc_md350", null ],
        [ "[expected.un.cons] — constructors", "md_docs_plan_tests_step1.html#autotoc_md351", null ],
        [ "[expected.un.obs] — observers", "md_docs_plan_tests_step1.html#autotoc_md352", null ],
        [ "[expected.un.swap] — swap", "md_docs_plan_tests_step1.html#autotoc_md353", null ],
        [ "[expected.un.eq] — equality", "md_docs_plan_tests_step1.html#autotoc_md354", null ]
      ] ],
      [ "Test Outline", "md_docs_plan_tests_step1.html#autotoc_md356", [
        [ "Normal (positive) tests", "md_docs_plan_tests_step1.html#autotoc_md357", null ],
        [ "Constraint / type-trait tests (in normal test file)", "md_docs_plan_tests_step1.html#autotoc_md358", null ]
      ] ],
      [ "Negative Compile Tests", "md_docs_plan_tests_step1.html#autotoc_md360", [
        [ "<tt>unexpected_array_fail.cpp</tt>", "md_docs_plan_tests_step1.html#autotoc_md361", null ],
        [ "<tt>unexpected_cvref_fail.cpp</tt>", "md_docs_plan_tests_step1.html#autotoc_md362", null ],
        [ "<tt>unexpected_self_fail.cpp</tt>", "md_docs_plan_tests_step1.html#autotoc_md363", null ],
        [ "<tt>unexpected_swap_nonswappable_fail.cpp</tt>", "md_docs_plan_tests_step1.html#autotoc_md364", null ]
      ] ],
      [ "CMakeLists additions", "md_docs_plan_tests_step1.html#autotoc_md366", null ]
    ] ],
    [ "Test Plan: Step 10 — expected<void, E&> Void+Error-Reference Specialization", "md_docs_plan_tests_step10.html", [
      [ "Testing Strategy", "md_docs_plan_tests_step10.html#autotoc_md369", null ],
      [ "Type-Level Tests (static_assert)", "md_docs_plan_tests_step10.html#autotoc_md371", null ],
      [ "Test Outline", "md_docs_plan_tests_step10.html#autotoc_md373", null ],
      [ "Negative Compile Tests", "md_docs_plan_tests_step10.html#autotoc_md375", [
        [ "<tt>expected_void_ref_e_temporary_fail.cpp</tt>", "md_docs_plan_tests_step10.html#autotoc_md376", null ],
        [ "<tt>expected_void_ref_e_const_lvalue_fail.cpp</tt>", "md_docs_plan_tests_step10.html#autotoc_md377", null ],
        [ "<tt>expected_void_ref_e_no_value_or_fail.cpp</tt>", "md_docs_plan_tests_step10.html#autotoc_md378", null ]
      ] ],
      [ "CMakeLists additions", "md_docs_plan_tests_step10.html#autotoc_md380", null ]
    ] ],
    [ "Test Plan: Step 2 — bad_expected_access<E>", "md_docs_plan_tests_step2.html", [
      [ "Testing Strategy", "md_docs_plan_tests_step2.html#autotoc_md383", null ],
      [ "Testable Statements from the Standard", "md_docs_plan_tests_step2.html#autotoc_md385", [
        [ "[expected.bad.void] — base specialization", "md_docs_plan_tests_step2.html#autotoc_md386", null ],
        [ "[expected.bad] — primary template", "md_docs_plan_tests_step2.html#autotoc_md387", null ]
      ] ],
      [ "Test Outline", "md_docs_plan_tests_step2.html#autotoc_md389", null ],
      [ "Return value conventions", "md_docs_plan_tests_step2.html#autotoc_md391", null ],
      [ "No negative compile tests", "md_docs_plan_tests_step2.html#autotoc_md393", null ]
    ] ],
    [ "Test Plan: Step 3 — expected<T, E> Primary Template", "md_docs_plan_tests_step3.html", [
      [ "Testing Strategy", "md_docs_plan_tests_step3.html#autotoc_md396", null ],
      [ "Ill-Formed Instantiations [expected.object.general] para 2–3", "md_docs_plan_tests_step3.html#autotoc_md398", null ],
      [ "Constructors [expected.object.cons]", "md_docs_plan_tests_step3.html#autotoc_md400", [
        [ "Default constructor", "md_docs_plan_tests_step3.html#autotoc_md401", null ],
        [ "Copy constructor", "md_docs_plan_tests_step3.html#autotoc_md402", null ],
        [ "Move constructor", "md_docs_plan_tests_step3.html#autotoc_md403", null ],
        [ "Converting constructor from expected<U, G>", "md_docs_plan_tests_step3.html#autotoc_md404", null ],
        [ "Converting constructor from value U&&", "md_docs_plan_tests_step3.html#autotoc_md405", null ],
        [ "Constructors from unexpected<G>", "md_docs_plan_tests_step3.html#autotoc_md406", null ],
        [ "In-place constructors", "md_docs_plan_tests_step3.html#autotoc_md407", null ]
      ] ],
      [ "Destructor [expected.object.dtor]", "md_docs_plan_tests_step3.html#autotoc_md409", null ],
      [ "Assignment [expected.object.assign]", "md_docs_plan_tests_step3.html#autotoc_md411", [
        [ "Copy assignment", "md_docs_plan_tests_step3.html#autotoc_md412", null ],
        [ "Move assignment", "md_docs_plan_tests_step3.html#autotoc_md413", null ],
        [ "Assign from value U&&", "md_docs_plan_tests_step3.html#autotoc_md414", null ],
        [ "Assign from unexpected<G>", "md_docs_plan_tests_step3.html#autotoc_md415", null ]
      ] ],
      [ "Emplace [expected.object.assign] para 18–21", "md_docs_plan_tests_step3.html#autotoc_md417", null ],
      [ "Swap [expected.object.swap]", "md_docs_plan_tests_step3.html#autotoc_md419", null ],
      [ "Observers [expected.object.obs]", "md_docs_plan_tests_step3.html#autotoc_md421", [
        [ "operator->() — Hardened precondition: has_value()", "md_docs_plan_tests_step3.html#autotoc_md422", null ],
        [ "operator*() — Hardened precondition: has_value()", "md_docs_plan_tests_step3.html#autotoc_md423", null ],
        [ "has_value() / operator bool()", "md_docs_plan_tests_step3.html#autotoc_md424", null ],
        [ "value() — throws bad_expected_access when empty", "md_docs_plan_tests_step3.html#autotoc_md425", null ],
        [ "error() — Hardened precondition: !has_value()", "md_docs_plan_tests_step3.html#autotoc_md426", null ],
        [ "value_or()", "md_docs_plan_tests_step3.html#autotoc_md427", null ],
        [ "error_or()", "md_docs_plan_tests_step3.html#autotoc_md428", null ]
      ] ],
      [ "Equality Operators [expected.object.eq]", "md_docs_plan_tests_step3.html#autotoc_md430", [
        [ "expected == expected (T2 not void)", "md_docs_plan_tests_step3.html#autotoc_md431", null ],
        [ "expected == T2 (comparison with value)", "md_docs_plan_tests_step3.html#autotoc_md432", null ],
        [ "expected == unexpected<E2>", "md_docs_plan_tests_step3.html#autotoc_md433", null ]
      ] ],
      [ "Negative Compile Tests", "md_docs_plan_tests_step3.html#autotoc_md435", [
        [ "<tt>expected_t_ref_fail.cpp</tt>", "md_docs_plan_tests_step3.html#autotoc_md436", null ],
        [ "<tt>expected_e_ref_fail.cpp</tt>", "md_docs_plan_tests_step3.html#autotoc_md437", null ],
        [ "<tt>expected_t_array_fail.cpp</tt>", "md_docs_plan_tests_step3.html#autotoc_md438", null ],
        [ "<tt>expected_value_mandate_fail.cpp</tt>", "md_docs_plan_tests_step3.html#autotoc_md439", null ],
        [ "<tt>expected_emplace_throwing_fail.cpp</tt>", "md_docs_plan_tests_step3.html#autotoc_md440", null ]
      ] ]
    ] ],
    [ "Test Plan: Step 4 — expected<void, E> Specialization", "md_docs_plan_tests_step4.html", [
      [ "Testing Strategy", "md_docs_plan_tests_step4.html#autotoc_md443", null ],
      [ "Ill-Formed Instantiations [expected.void.general] para 2", "md_docs_plan_tests_step4.html#autotoc_md445", null ],
      [ "Constructors [expected.void.cons]", "md_docs_plan_tests_step4.html#autotoc_md447", [
        [ "Default constructor", "md_docs_plan_tests_step4.html#autotoc_md448", null ],
        [ "Copy constructor", "md_docs_plan_tests_step4.html#autotoc_md449", null ],
        [ "Move constructor", "md_docs_plan_tests_step4.html#autotoc_md450", null ],
        [ "Converting constructor from expected<U, G> (void case)", "md_docs_plan_tests_step4.html#autotoc_md451", null ],
        [ "Constructors from unexpected<G>", "md_docs_plan_tests_step4.html#autotoc_md452", null ],
        [ "in_place_t constructor", "md_docs_plan_tests_step4.html#autotoc_md453", null ],
        [ "unexpect_t constructors", "md_docs_plan_tests_step4.html#autotoc_md454", null ]
      ] ],
      [ "Destructor [expected.void.dtor]", "md_docs_plan_tests_step4.html#autotoc_md456", null ],
      [ "Assignment [expected.void.assign]", "md_docs_plan_tests_step4.html#autotoc_md458", [
        [ "Copy assignment", "md_docs_plan_tests_step4.html#autotoc_md459", null ],
        [ "Move assignment", "md_docs_plan_tests_step4.html#autotoc_md460", null ],
        [ "Assign from unexpected<G>", "md_docs_plan_tests_step4.html#autotoc_md461", null ],
        [ "emplace()", "md_docs_plan_tests_step4.html#autotoc_md462", null ]
      ] ],
      [ "Swap [expected.void.swap]", "md_docs_plan_tests_step4.html#autotoc_md464", null ],
      [ "Observers [expected.void.obs]", "md_docs_plan_tests_step4.html#autotoc_md466", [
        [ "operator bool() / has_value()", "md_docs_plan_tests_step4.html#autotoc_md467", null ],
        [ "operator*() — void, noexcept, Hardened precondition: has_value()", "md_docs_plan_tests_step4.html#autotoc_md468", null ],
        [ "value() — returns void, throws when empty", "md_docs_plan_tests_step4.html#autotoc_md469", null ],
        [ "error() — Hardened precondition: !has_value()", "md_docs_plan_tests_step4.html#autotoc_md470", null ],
        [ "error_or()", "md_docs_plan_tests_step4.html#autotoc_md471", null ]
      ] ],
      [ "Equality Operators [expected.void.eq]", "md_docs_plan_tests_step4.html#autotoc_md473", [
        [ "expected<void> == expected<void, E2>", "md_docs_plan_tests_step4.html#autotoc_md474", null ],
        [ "expected<void> == unexpected<E2>", "md_docs_plan_tests_step4.html#autotoc_md475", null ]
      ] ],
      [ "No-value members (confirm absence)", "md_docs_plan_tests_step4.html#autotoc_md477", null ]
    ] ],
    [ "Test Plan: Step 5 — Monadic Operations for expected<T, E>", "md_docs_plan_tests_step5.html", [
      [ "Testing Strategy", "md_docs_plan_tests_step5.html#autotoc_md480", null ],
      [ "and_then [expected.object.monadic] para 1–8", "md_docs_plan_tests_step5.html#autotoc_md482", [
        [ "<tt>and_then(F)</tt> — lvalue and const-lvalue overloads (para 1–4)", "md_docs_plan_tests_step5.html#autotoc_md483", null ],
        [ "<tt>and_then(F)</tt> — rvalue and const-rvalue overloads (para 5–8)", "md_docs_plan_tests_step5.html#autotoc_md484", null ]
      ] ],
      [ "or_else [expected.object.monadic] para 9–16", "md_docs_plan_tests_step5.html#autotoc_md486", null ],
      [ "transform [expected.object.monadic] para 17–24", "md_docs_plan_tests_step5.html#autotoc_md488", null ],
      [ "transform_error [expected.object.monadic] para 25–32", "md_docs_plan_tests_step5.html#autotoc_md490", null ],
      [ "Test Outline", "md_docs_plan_tests_step5.html#autotoc_md492", null ],
      [ "Negative Compile Tests", "md_docs_plan_tests_step5.html#autotoc_md494", [
        [ "<tt>and_then_wrong_error_type_fail.cpp</tt>", "md_docs_plan_tests_step5.html#autotoc_md495", null ],
        [ "<tt>and_then_not_expected_fail.cpp</tt>", "md_docs_plan_tests_step5.html#autotoc_md496", null ],
        [ "<tt>or_else_wrong_value_type_fail.cpp</tt>", "md_docs_plan_tests_step5.html#autotoc_md497", null ],
        [ "<tt>transform_error_not_valid_unexpected_arg_fail.cpp</tt>", "md_docs_plan_tests_step5.html#autotoc_md498", null ]
      ] ]
    ] ],
    [ "Test Plan: Step 6 — Monadic Operations for expected<void, E>", "md_docs_plan_tests_step6.html", [
      [ "Testing Strategy", "md_docs_plan_tests_step6.html#autotoc_md501", null ],
      [ "and_then [expected.void.monadic] para 1–8", "md_docs_plan_tests_step6.html#autotoc_md503", [
        [ "Lvalue overloads (para 1–4)", "md_docs_plan_tests_step6.html#autotoc_md504", null ],
        [ "Rvalue overloads (para 5–8)", "md_docs_plan_tests_step6.html#autotoc_md505", null ]
      ] ],
      [ "or_else [expected.void.monadic] para 9–14", "md_docs_plan_tests_step6.html#autotoc_md507", null ],
      [ "transform [expected.void.monadic] para 15–22", "md_docs_plan_tests_step6.html#autotoc_md509", null ],
      [ "transform_error [expected.void.monadic] para 23–28", "md_docs_plan_tests_step6.html#autotoc_md511", null ],
      [ "Test Outline", "md_docs_plan_tests_step6.html#autotoc_md513", null ],
      [ "Negative Compile Tests", "md_docs_plan_tests_step6.html#autotoc_md515", [
        [ "<tt>void_and_then_wrong_error_type_fail.cpp</tt>", "md_docs_plan_tests_step6.html#autotoc_md516", null ],
        [ "<tt>void_or_else_wrong_value_type_fail.cpp</tt>", "md_docs_plan_tests_step6.html#autotoc_md517", null ]
      ] ]
    ] ],
    [ "Test Plan: Step 7 — expected<T&, E> Reference Specialization", "md_docs_plan_tests_step7.html", [
      [ "Testing Strategy", "md_docs_plan_tests_step7.html#autotoc_md520", null ],
      [ "Type-Level Tests (static_assert)", "md_docs_plan_tests_step7.html#autotoc_md522", null ],
      [ "Test Outline", "md_docs_plan_tests_step7.html#autotoc_md524", null ],
      [ "Negative Compile Tests", "md_docs_plan_tests_step7.html#autotoc_md526", [
        [ "<tt>expected_ref_temporary_fail.cpp</tt>", "md_docs_plan_tests_step7.html#autotoc_md527", null ],
        [ "<tt>expected_ref_no_default_fail.cpp</tt>", "md_docs_plan_tests_step7.html#autotoc_md528", null ],
        [ "<tt>expected_ref_inplace_value_fail.cpp</tt>", "md_docs_plan_tests_step7.html#autotoc_md529", null ]
      ] ],
      [ "CMakeLists additions", "md_docs_plan_tests_step7.html#autotoc_md531", null ]
    ] ],
    [ "Test Plan: Step 8 — expected<T, E&> Error-Reference Specialization", "md_docs_plan_tests_step8.html", [
      [ "Testing Strategy", "md_docs_plan_tests_step8.html#autotoc_md534", null ],
      [ "Type-Level Tests (static_assert)", "md_docs_plan_tests_step8.html#autotoc_md536", null ],
      [ "Test Outline", "md_docs_plan_tests_step8.html#autotoc_md538", null ],
      [ "Negative Compile Tests", "md_docs_plan_tests_step8.html#autotoc_md540", [
        [ "<tt>expected_ref_e_temporary_error_fail.cpp</tt>", "md_docs_plan_tests_step8.html#autotoc_md541", null ],
        [ "<tt>expected_ref_e_const_lvalue_assignment_fail.cpp</tt>", "md_docs_plan_tests_step8.html#autotoc_md542", null ]
      ] ]
    ] ],
    [ "Test Plan: Step 9 — expected<T&, E&> Both-Reference Specialization", "md_docs_plan_tests_step9.html", [
      [ "Testing Strategy", "md_docs_plan_tests_step9.html#autotoc_md545", null ],
      [ "Type-Level Tests (static_assert)", "md_docs_plan_tests_step9.html#autotoc_md547", null ],
      [ "Test Outline", "md_docs_plan_tests_step9.html#autotoc_md549", null ],
      [ "Negative Compile Tests", "md_docs_plan_tests_step9.html#autotoc_md551", [
        [ "<tt>expected_ref_both_temp_value_fail.cpp</tt>", "md_docs_plan_tests_step9.html#autotoc_md552", null ],
        [ "<tt>expected_ref_both_temp_error_fail.cpp</tt>", "md_docs_plan_tests_step9.html#autotoc_md553", null ],
        [ "<tt>expected_ref_both_no_default_fail.cpp</tt>", "md_docs_plan_tests_step9.html#autotoc_md554", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"md_docs_plan_step3_expected_primary.html#autotoc_md250"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';