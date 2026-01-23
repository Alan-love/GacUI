# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.2: Make FileDialogViewModel testable (options + message box + factory) and add empty test file

Refactor the file dialog view model to enable unit tests to control message box outcomes and to observe confirmation results via `IFileDialogViewModel`, without exposing `FileDialogViewModel`’s concrete type in headers.

### what to be done

- Introduce `IFileSystemViewModelMessageBox` with a `ShowMessageBox` signature identical to the current usage inside `TryConfirm`.
	- Store `Ptr<IFileSystemViewModelMessageBox>` in `FileSystemViewModel` (or the component that currently calls message boxes during confirmation).
	- Ensure `TryConfirm` routes its single message-box decision through this interface (and only calls it once).
- Introduce `FileSystemViewModelOptions` containing (at least) the fields specified in the request, plus the message box implementation pointer.
	- Update `FileDialogViewModel` to store the options object instead of duplicating these fields as separate members.
	- Fix all resulting build breaks in the places that construct/configure the view model.
- Update fake services to wire a real message-box implementation in production/fake-dialog contexts:
	- `Source/Utilities/FakeServices/GuiFakeDialogServiceBase.h`
	- `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`
- Add test-only hooks to `IFileDialogViewModel` and update reflection:
	- `bool IsConfirmed();`
	- `const ...& GetConfirmedSelection();`
	- `void ResetConfirmation();`
- Add a cpp-only factory function (defined in the implementation `.cpp`) that returns `Ptr<IFileDialogViewModel>` to create a `FileDialogViewModel` instance for tests.
	- Keep the concrete `FileDialogViewModel` type hidden from headers; unit tests only see `IFileDialogViewModel`.
- Create an empty `TestFileDialogViewModel.cpp` placed alongside `Test/GacUISrc/UnitTest/TestDocumentConfig.cpp` and in the same solution explorer folder.
	- Add it to the correct project file (`*.vcxproj` / `*.vcxitems`) and its `*.filters` entry so it builds (even if it contains no tests yet).

### rationale

- `TryConfirm` behavior depends on user prompts; injecting a message-box interface is the minimal way to make these branches unit-testable without driving UI.
- Consolidating options into `FileSystemViewModelOptions` prevents the test factory from needing to reach into a growing list of constructor parameters or ad-hoc setters, and reduces future churn.
- Exposing confirmation state and selection through the interface is necessary because the tests exercise `IFileDialogViewModel` directly.
- Creating the test `.cpp` early ensures the plumbing (project inclusion, linkage for the factory function) is correct before investing in multiple test categories.

# UPDATES

# INSIGHTS AND REASONING

## Current State / Problem Analysis

`vl::presentation::IFileDialogViewModel` is declared in `Source/Utilities/FakeServices/GuiFakeDialogServiceBase.h` and implemented by a private `FileDialogViewModel` class in `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`.

The current `FileDialogViewModel::TryConfirm` implementation calls `GetCurrentController()->DialogService()->ShowMessageBox(...)` directly (see `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`), which makes:

- Message-box driven branches untestable (tests cannot inject a deterministic return value without UI).
- The “confirmed” state (`confirmed` + `confirmedSelection`) unobservable via `vl::presentation::IFileDialogViewModel` (only used after the modal dialog returns in `FakeDialogServiceBase::ShowFileDialog`).

## Design Goals

- Unit tests can drive `TryConfirm` deterministically by injecting a mock message box.
- Unit tests can assert confirmation state and selections via the interface (`IFileDialogViewModel`) without depending on the concrete `FileDialogViewModel` type.
- Production behavior remains unchanged: message boxes still go through the current dialog service and the UI still uses the existing view-model surface.

## Proposed Design

### 1) Inject message box behavior behind a small interface

Add `vl::presentation::IFileSystemViewModelMessageBox` (name per request) with a `ShowMessageBox` method whose signature matches the current call sites in `FileDialogViewModel::TryConfirm` (currently: native window + text + title + buttons + default button + icon).

Implementation strategy:

- `FileDialogViewModel` no longer calls `GetCurrentController()->DialogService()->ShowMessageBox(...)` directly.
- It calls `options.messageBox->ShowMessageBox(...)` (or an equivalent pointer stored where `TryConfirm` lives).
- The default production implementation is provided by fake dialog services and simply forwards to the current controller’s dialog service, preserving the existing UI behavior.

Rationale:

- This is the minimal seam to make the “prompt/decision” part of `TryConfirm` deterministic in unit tests while leaving all file-system classification and path normalization logic intact.
- The existing logic already only shows one message box per call (each branch returns immediately), so enforcing “only called once” is naturally satisfied by the control flow; the injected interface becomes the single call surface to assert against in tests.

### 2) Consolidate configuration into `FileSystemViewModelOptions`

Add a `vl::presentation::FileSystemViewModelOptions` struct to contain the configuration currently stored as individual `FileDialogViewModel` members:

- `WString title;`
- `bool enabledMultipleSelection;`
- `bool fileMustExist;`
- `bool folderMustExist;`
- `bool promptCreateFile;`
- `bool promptOverriteFile;`
- `WString defaultExtension;`
- `Ptr<IFileSystemViewModelMessageBox> messageBox;`

Update `FileDialogViewModel` to store a single options object (by value or `Ptr`, whichever best matches existing coding patterns) instead of duplicating these fields.

Update all construction/configuration sites (notably `FakeDialogServiceBase::ShowFileDialog` in `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`) to populate the struct once.

Rationale:

- This stabilizes the creation API for tests and prevents option sprawl across ad-hoc members.
- It also sets up later tasks (multiple test categories) by making it easy to set a message-box mock and toggle individual flags without touching internal members.

### 3) Expose confirmation state via `IFileDialogViewModel` (+ reflection)

Extend `vl::presentation::IFileDialogViewModel` in `Source/Utilities/FakeServices/GuiFakeDialogServiceBase.h` with:

- `bool IsConfirmed();`
- `const ...& GetConfirmedSelection();`
- `void ResetConfirmation();`

Update the reflection registration in `Source/Reflection/TypeDescriptors/GuiReflectionBasic.cpp` (the `BEGIN_INTERFACE_MEMBER_NOPROXY(IFileDialogViewModel)` block) to include these members so they remain visible to reflection-based consumers.

Rationale:

- Unit tests are explicitly required to test via the interface, and the existing confirmation results are otherwise private implementation details.
- `ResetConfirmation()` keeps each test case independent without re-creating the view model if later tests want to reuse it.

### 4) Provide a cpp-only factory returning `Ptr<IFileDialogViewModel>`

Add an exported/free function (declared `extern` in unit tests, defined in `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`) that creates and returns `Ptr<vl::presentation::IFileDialogViewModel>`.

This factory should:

- Construct `FileDialogViewModel` (concrete type stays in the `.cpp`).
- Apply `FileSystemViewModelOptions` (including message box implementation).
- Initialize any required internal invariants that production code normally sets up before the dialog UI starts interacting with the view model (e.g. creating `rootFolder`, applying `selectToSave`, preparing `initialDirectory` behavior).

Rationale:

- Tests should not need to include or know about `FileDialogViewModel` (which is intentionally private to the `.cpp`).
- A stable factory is the narrowest API surface to enable unit tests while keeping headers clean.

### 5) Add an empty `TestFileDialogViewModel.cpp` early (plumbing)

Create an empty test source file next to `Test/GacUISrc/UnitTest/TestDocumentConfig.cpp`, added to the appropriate test project (`*.vcxproj` / `*.vcxitems`) and its `*.filters` file to ensure:

- The test build picks up the compilation unit.
- The new factory function can be referenced and linked by unit tests.

## Evidence / Related APIs

- `vl::filesystem::FilePath`, `vl::filesystem::File`, `vl::filesystem::Folder` are the primary types used for existence checks and enumeration in `FileDialogViewModel` (`Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`). See knowledge base: `.github/KnowledgeBase/KB_VlppOS_FileSystemOperations.md`.
- The unit testing structure and macros used in `Test/GacUISrc/UnitTest` are described in `.github/KnowledgeBase/KB_Vlpp_UnitTesting.md` (e.g. `TEST_FILE`, `TEST_CATEGORY`, `TEST_CASE`).
- `IFileDialogViewModel` reflection wiring is in `Source/Reflection/TypeDescriptors/GuiReflectionBasic.cpp` and must be updated when adding interface members.

## Alternatives Considered

### Alternative A: Inject a `Func<>` callback instead of an interface

Pros:

- Lower surface area (no new interface type to reflect/maintain).

Cons:

- Harder to reflect and expose cleanly across the existing reflection pipeline.
- The request explicitly asks for an interface, and the project broadly prefers explicit interfaces (`vl::Object` / `vl::Interface` patterns).

Decision: prefer the requested `IFileSystemViewModelMessageBox` interface.

### Alternative B: Make `FileDialogViewModel` public in headers

Pros:

- Simplifies test construction.

Cons:

- Violates the “keep concrete type hidden” requirement and likely increases API surface and coupling.

Decision: keep the concrete type private and add a factory.

## Expected Impact / Follow-ups

- A small ripple of build fixes is expected wherever `FileDialogViewModel` was previously configured via individual members (primarily inside `FakeDialogServiceBase::ShowFileDialog`).
- Later tasks can implement `vl::filesystem::IFileSystemImpl` mocks and drive the file dialog view model without touching the real OS filesystem; this aligns with `vl::filesystem::InjectFileSystemImpl` described in `.github/KnowledgeBase/KB_VlppOS_FileSystemOperations.md`.

# !!!FINISHED!!!
