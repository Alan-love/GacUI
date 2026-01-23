# !!!PLANNING!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Add an injectable message box interface for confirmation

Change in Source/Utilities/FakeServices/GuiFakeDialogServiceBase.h:

- Add a new interface vl::presentation::IFileSystemViewModelMessageBox to represent the message box dependency used by vl::presentation::IFileDialogViewModel::TryConfirm.
- Its ShowMessageBox signature must match the existing usage in Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp (no modal parameter; same enums).

Proposed code:

	class IFileSystemViewModelMessageBox : public virtual IDescriptable
	{
	public:
		using ButtonsInput = INativeDialogService::MessageBoxButtonsInput;
		using ButtonsOutput = INativeDialogService::MessageBoxButtonsOutput;
		using DefaultButton = INativeDialogService::MessageBoxDefaultButton;
		using Icon = INativeDialogService::MessageBoxIcons;

		virtual ButtonsOutput	ShowMessageBox(
									INativeWindow* window,
									const WString& text,
									const WString& title,
									ButtonsInput buttons,
									DefaultButton defaultButton,
									Icon icon
								) = 0;
	};

Why this is necessary:

- TryConfirm currently calls GetCurrentController()->DialogService()->ShowMessageBox(...) directly, which makes the OK/Cancel branch non-deterministic in tests.
- Introducing a dedicated interface is the smallest seam that allows tests to force a result and assert that TryConfirm only asks once per call.

## STEP 2: Consolidate file dialog configuration into FileSystemViewModelOptions

Change in Source/Utilities/FakeServices/GuiFakeDialogServiceBase.h:

- Add a new struct vl::presentation::FileSystemViewModelOptions that holds the file dialog configuration currently stored as separate FileDialogViewModel fields, plus the message box dependency.
- This struct will be stored by FileDialogViewModel (in the .cpp) and used by getters and TryConfirm.

Proposed code:

	struct FileSystemViewModelOptions
	{
		WString								title;
		bool								enabledMultipleSelection = false;
		bool								fileMustExist = false;
		bool								folderMustExist = false;
		bool								promptCreateFile = false;
		bool								promptOverriteFile = false;
		WString								defaultExtension;
		Ptr<IFileSystemViewModelMessageBox>	messageBox;
	};

Why this is necessary:

- The factory required by this task must be able to configure the view model without reaching into many independent members.
- Centralizing options reduces churn: adding more flags later becomes a struct extension, not a constructor explosion or extra setters.

## STEP 3: Refactor FileDialogViewModel to store options and use the injected message box

Change in Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp:

- Replace the following FileDialogViewModel data members:
  - title
  - enabledMultipleSelection
  - fileMustExist
  - folderMustExist
  - promptCreateFile
  - promptOverriteFile
  - defaultExtension
  with a single member:
  - FileSystemViewModelOptions options;

- Update IFileDialogViewModel getter implementations:
  - GetTitle() returns options.title
  - GetEnabledMultipleSelection() returns options.enabledMultipleSelection
  - GetDefaultExtension() returns options.defaultExtension

- Update TryConfirm() to route every existing ShowMessageBox call through options.messageBox->ShowMessageBox(...).
  - For error message boxes, ignore the return value as before.
  - For the OK/Cancel question message box, use the returned ButtonsOutput to decide whether to cancel.
  - Preserve the current control flow property that at most one message box is shown per TryConfirm call.

Proposed transformation pattern:

	// before
	GetCurrentController()->DialogService()->ShowMessageBox(...);

	// after
	options.messageBox->ShowMessageBox(...);

Why this is necessary:

- This isolates UI prompting from validation logic, enabling deterministic unit tests for confirmation branches without driving any UI.
- Using a single options object avoids exposing the private FileDialogViewModel type in headers while still allowing full configuration.

## STEP 4: Wire a default production message box implementation from the fake dialog service setup

Change in Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp:

- Add a small internal class that implements IFileSystemViewModelMessageBox by forwarding to the current controller’s dialog service, preserving existing behavior.

Proposed code:

	namespace
	{
		class DefaultFileSystemViewModelMessageBox : public Object, public virtual IFileSystemViewModelMessageBox
		{
		public:
			ButtonsOutput ShowMessageBox(
				INativeWindow* window,
				const WString& text,
				const WString& title,
				ButtonsInput buttons,
				DefaultButton defaultButton,
				Icon icon
			) override
			{
				return GetCurrentController()->DialogService()->ShowMessageBox(window, text, title, buttons, defaultButton, icon);
			}
		};
	}

- In FakeDialogServiceBase::ShowFileDialog, populate vm->options once instead of setting individual members, and assign vm->options.messageBox:
  - vm->options.messageBox = Ptr(new DefaultFileSystemViewModelMessageBox);

Why this is necessary:

- Existing dialogs must continue to show the same message boxes via the same dialog service without requiring any new setup at call sites.
- Tests can override vm->options.messageBox with a mock via the test factory, while production keeps the forwarding implementation.

## STEP 5: Expose confirmation result via IFileDialogViewModel and implement the hooks

Change in Source/Utilities/FakeServices/GuiFakeDialogServiceBase.h:

- Add test-only hooks to vl::presentation::IFileDialogViewModel (no exposure of FileDialogViewModel concrete type):

Proposed code:

	class IFileDialogViewModel : public virtual IDescriptable
	{
		...
	public:
		virtual bool								GetConfirmed() = 0;
		virtual const collections::List<WString>&	GetConfirmedSelection() = 0;
		virtual void								ResetConfirmation() = 0;
		...
	};

Change in Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp:

- Implement these methods in FileDialogViewModel using existing private fields:
  - GetConfirmed() returns confirmed
  - GetConfirmedSelection() returns confirmedSelection
  - ResetConfirmation() sets confirmed = false and clears confirmedSelection

Why this is necessary:

- FakeDialogServiceBase::ShowFileDialog currently observes confirmation only after the modal dialog closes; unit tests need to observe the same results directly via the interface.
- ResetConfirmation keeps tests independent without needing to recreate the view model each time.

## STEP 6: Update reflection to include the new IFileDialogViewModel members

Change in Source/Reflection/TypeDescriptors/GuiReflectionBasic.cpp:

- In the BEGIN_INTERFACE_MEMBER_NOPROXY(IFileDialogViewModel) block, register the new members:
  - GetConfirmed as a method
  - GetConfirmedSelection as a read-only property ConfirmedSelection
  - ResetConfirmation as a method

Proposed code:

	BEGIN_INTERFACE_MEMBER_NOPROXY(IFileDialogViewModel)
		...
		CLASS_MEMBER_PROPERTY_READONLY_FAST(Confirmed)
		CLASS_MEMBER_PROPERTY_READONLY_FAST(ConfirmedSelection)
		CLASS_MEMBER_METHOD(ResetConfirmation, NO_PARAMETER)
		...
	END_INTERFACE_MEMBER(IFileDialogViewModel)

Why this is necessary:

- Source/Reflection/TypeDescriptors/GuiReflectionBasic.cpp is the reflection entry for these view model interfaces; it must remain in sync with interface changes.
- Keeping these members reflected avoids accidental breakages in reflection-based tooling and aligns with the request.

## STEP 7: Add a cpp-only factory returning Ptr<IFileDialogViewModel> for unit tests

Change in Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp:

- Add a free function defined in the .cpp (not declared in headers) that creates the private FileDialogViewModel and returns it as Ptr<IFileDialogViewModel>.
- The unit tests will declare it as extern and never include or reference FileDialogViewModel.

Proposed signature (stable and minimal for TryConfirm-focused tests):

	Ptr<IFileDialogViewModel> CreateFileDialogViewModelForTest(
		const FileSystemViewModelOptions& options,
		const WString& initialDirectory,
		const WString& defaultExtension,
		const WString& filter,
		INativeDialogService::FileDialogTypes dialogType
	);

Factory responsibilities:

- Allocate FileDialogViewModel and copy the provided options into vm->options.
  - If options.messageBox is null, set vm->options.messageBox = Ptr(new DefaultFileSystemViewModelMessageBox) to preserve safety.
- Initialize members that production code sets before the dialog UI interacts with the view model:
  - vm->initialDirectory
  - vm->rootFolder (type Root)
  - vm->selectToSave based on dialogType (Save/SavePreview true; Open/OpenPreview false)
  - Parse the filter string into vm->filters and set vm->selectedFilter using the same logic as FakeDialogServiceBase::ShowFileDialog.
- Leave localized text initialization to the test (via InitLocalizedText), or optionally provide a test helper that calls InitLocalizedText with known strings.

Why this is necessary:

- The task requires tests to construct and drive the view model via IFileDialogViewModel without exposing the concrete type in any header.
- A .cpp-only factory keeps encapsulation intact while allowing tests to configure options (including a mock message box).

## STEP 8: Add an empty TestFileDialogViewModel.cpp and include it in the unit test project

Changes:

- Create a new empty source file at:
  - Test/GacUISrc/UnitTest/TestFileDialogViewModel.cpp

- Add it to the unit test project file:
  - Test/GacUISrc/UnitTest/UnitTest.vcxproj
    - Add: <ClCompile Include="TestFileDialogViewModel.cpp" />

- Add it to the same solution explorer folder as Test/GacUISrc/UnitTest/TestDocumentConfig.cpp:
  - Test/GacUISrc/UnitTest/UnitTest.vcxproj.filters
    - Add:

	<ClCompile Include="TestFileDialogViewModel.cpp">
		<Filter>Source Files</Filter>
	</ClCompile>

Why this is necessary:

- This validates the plumbing early (project inclusion and linkage path for the factory function) before adding real test cases.

# TEST PLAN

## Build and regression

- Build the affected library and the UnitTest project to ensure:
  - Source/Utilities/FakeServices/GuiFakeDialogServiceBase.h interface changes compile everywhere.
  - Source/Reflection/TypeDescriptors/GuiReflectionBasic.cpp reflection registration compiles with the new members.

## New unit tests to add later in Test/GacUISrc/UnitTest/TestFileDialogViewModel.cpp

When the file is no longer empty, implement tests using the existing Vlpp unit test framework (.github/KnowledgeBase/KB_Vlpp_UnitTesting.md) and existing file system mocking (Test/GacUISrc/UnitTest/TestFileSystemMock.cpp).

Message box mocking strategy:

- Implement a small mock object that implements vl::presentation::IFileSystemViewModelMessageBox.
  - Record each call (text/title/buttons/defaultButton/icon) and return a preconfigured ButtonsOutput.
  - Assert the number of calls is 0 or 1 per TryConfirm invocation.

Suggested test cases (all operate on Ptr<vl::presentation::IFileDialogViewModel> from CreateFileDialogViewModelForTest):

- Confirmation hooks:
  - ResetConfirmation clears state and makes GetConfirmed() false.
  - After a successful TryConfirm, GetConfirmed() is true and ConfirmedSelection contains expected full paths.

- Validation without OK/Cancel prompt:
  - Empty selection triggers a single error message box and returns false.
  - Multiple selection when EnabledMultipleSelection is false triggers a single error message box and returns false.
  - Selecting a folder path (existing folder) changes selected folder and returns false without setting confirmation.

- Validation with filesystem constraints (use filesystem injection / FileSystemMock):
  - fileMustExist: selecting a non-existent file triggers an error message box and returns false.
  - folderMustExist: selecting a non-existent parent folder triggers an error message box and returns false.

- OK/Cancel question branch:
  - selectToSave + promptOverriteFile: selecting an existing file triggers an OK/Cancel prompt; returning SelectCancel cancels confirmation; returning SelectOK proceeds.
  - open-mode + promptCreateFile: selecting a non-existent file triggers an OK/Cancel prompt; returning SelectCancel cancels confirmation; returning SelectOK proceeds.

- Default extension behavior:
  - If selected filter provides a default extension, selections without that extension get it appended.
  - If filter does not provide one but options.defaultExtension is non-empty, selections without any extension get options.defaultExtension appended.

# !!!FINISHED!!!
