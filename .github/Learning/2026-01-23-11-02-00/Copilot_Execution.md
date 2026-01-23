# !!!EXECUTION!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Add `IFileSystemViewModelMessageBox` [DONE]

Update `Source/Utilities/FakeServices/GuiFakeDialogServiceBase.h`.

Insert the following code after `class IFileDialogFilter` and before `class IFileDialogViewModel`:

```cpp
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
```

## STEP 2: Add `FileSystemViewModelOptions` [DONE]

Update `Source/Utilities/FakeServices/GuiFakeDialogServiceBase.h`.

Insert the following code right after `IFileSystemViewModelMessageBox`:

```cpp
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
```

## STEP 3: Extend `IFileDialogViewModel` with confirmation hooks [DONE]

Update `Source/Utilities/FakeServices/GuiFakeDialogServiceBase.h`.

1) In `class IFileDialogViewModel`, add this type alias together with existing `using` declarations:

```cpp
			using ConfirmedSelection = collections::List<WString>;
```

2) In `class IFileDialogViewModel`, insert these members after `TryConfirm(...)` and before `InitLocalizedText(...)`:

```cpp
			virtual bool						IsConfirmed() = 0;
			virtual const ConfirmedSelection&	GetConfirmedSelection() = 0;
			virtual void						ResetConfirmation() = 0;
```

## STEP 4: Refactor `FileDialogViewModel` to use options + injected message box [DONE]

Update `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`.

### 4.1 Add default message box forwarder

Insert this code after `using namespace controls;` and before `class FileDialogFilter`:

```cpp
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
```

### 4.2 Replace configuration fields with `FileSystemViewModelOptions options`

In `class FileDialogViewModel`, replace:

```cpp
			WString						title;
			bool						enabledMultipleSelection = false;
			bool						fileMustExist = false;
			bool						folderMustExist = false;
			bool						promptCreateFile = false;
			bool						promptOverriteFile = false;
			WString						defaultExtension;
```

with:

```cpp
			FileSystemViewModelOptions	options;
```

Also change the type alias at the beginning of `FileDialogViewModel` from:

```cpp
			using ConfirmedSelection = collections::List<WString>;
```

to:

```cpp
			using ConfirmedSelection = IFileDialogViewModel::ConfirmedSelection;
```

### 4.3 Update getters + add confirmation hooks implementation

In `class FileDialogViewModel`, replace these method bodies:

```cpp
			WString GetTitle() override
			{
				return title;
			}

			bool GetEnabledMultipleSelection() override
			{
				return enabledMultipleSelection;
			}

			WString GetDefaultExtension() override
			{
				return defaultExtension;
			}
```

with:

```cpp
			WString GetTitle() override
			{
				return options.title;
			}

			bool GetEnabledMultipleSelection() override
			{
				return options.enabledMultipleSelection;
			}

			WString GetDefaultExtension() override
			{
				return options.defaultExtension;
			}

			bool IsConfirmed() override
			{
				return confirmed;
			}

			const ConfirmedSelection& GetConfirmedSelection() override
			{
				return confirmedSelection;
			}

			void ResetConfirmation() override
			{
				confirmed = false;
				confirmedSelection.Clear();
			}
```

### 4.4 Update `TryConfirm` to use `options.messageBox` and `options.*`

In `FileDialogViewModel::TryConfirm`, replace the whole function body with:

```cpp
			bool TryConfirm(controls::GuiWindow* owner, Selection selectedPaths) override
			{
				CHECK_ERROR(options.messageBox, L"vl::presentation::FileDialogViewModel::TryConfirm()#options.messageBox is not initialized.");

				auto wd = selectedFolder->folder.GetFilePath();
				List<filesystem::FilePath> paths;
				CopyFrom(
					paths,
					selectedPaths.Select([this, wd](auto path) { return wd / path; })
					);

				if (paths.Count() == 0)
				{
					options.messageBox->ShowMessageBox(
						owner->GetNativeWindow(),
						dialogErrorEmptySelection,
						owner->GetText(),
						INativeDialogService::DisplayOK,
						INativeDialogService::DefaultFirst,
						INativeDialogService::IconError
						);
					return false;
				}
				else if (paths.Count() == 1)
				{
					auto path = paths[0];
					if (filesystem::Folder(path).Exists())
					{
						SetSelectedFolderInternal(path, true);
						return false;
					}
				}
				else
				{
					if (!options.enabledMultipleSelection)
					{
						options.messageBox->ShowMessageBox(
							owner->GetNativeWindow(),
							dialogErrorMultipleSelectionNotEnabled,
							owner->GetText(),
							INativeDialogService::DisplayOK,
							INativeDialogService::DefaultFirst,
							INativeDialogService::IconError
							);
						return false;
					}
				}

				List<vint> files, folders, unexistings;
				for (auto [path, index] : indexed(paths))
				{
					if (filesystem::File(path).Exists())
					{
						files.Add(index);
					}
					else if (filesystem::Folder(path).Exists())
					{
						folders.Add(index);
					}
					else
					{
						unexistings.Add(index);
					}
				}

				if (folders.Count() > 0)
				{
					auto message = stream::GenerateToStream([&](stream::TextWriter& writer)
					{
						writer.WriteString(dialogErrorFileExpected);
						for (vint index : folders)
						{
							writer.WriteLine(WString::Empty);
							writer.WriteString(L"  ");
							writer.WriteString(wd.GetRelativePathFor(paths[index]));
						}
					});
					options.messageBox->ShowMessageBox(
						owner->GetNativeWindow(),
						message,
						owner->GetText(),
						INativeDialogService::DisplayOK,
						INativeDialogService::DefaultFirst,
						INativeDialogService::IconError
						);
					return false;
				}

				if (unexistings.Count() > 0)
				{
					if (options.fileMustExist)
					{
						auto message = stream::GenerateToStream([&](stream::TextWriter& writer)
						{
							writer.WriteString(dialogErrorFileNotExist);
							for (vint index : unexistings)
							{
								writer.WriteLine(WString::Empty);
								writer.WriteString(L"  ");
								writer.WriteString(wd.GetRelativePathFor(paths[index]));
							}
						});
						options.messageBox->ShowMessageBox(
							owner->GetNativeWindow(),
							message,
							owner->GetText(),
							INativeDialogService::DisplayOK,
							INativeDialogService::DefaultFirst,
							INativeDialogService::IconError
							);
						return false;
					}

					if (options.folderMustExist)
					{
						SortedList<filesystem::FilePath> folderOfUnexistings;
						for (vint index : unexistings)
						{
							auto path = paths[index].GetFolder();
							if (!folderOfUnexistings.Contains(path))
							{
								folderOfUnexistings.Add(path);
							}
						}

						// TODO: (enumerable) foreach:indexed(alterable(reversed))
						for (vint i = folderOfUnexistings.Count() - 1; i >= 0; i--)
						{
							if (filesystem::Folder(folderOfUnexistings[i]).Exists())
							{
								folderOfUnexistings.RemoveAt(i);
							}
						}

						if (folderOfUnexistings.Count() > 0)
						{
							auto message = stream::GenerateToStream([&](stream::TextWriter& writer)
							{
								writer.WriteString(dialogErrorFolderNotExist);
								for (auto path : folderOfUnexistings)
								{
									writer.WriteLine(WString::Empty);
									writer.WriteString(L"  ");
									writer.WriteString(wd.GetRelativePathFor(path));
								}
							});
							options.messageBox->ShowMessageBox(
								owner->GetNativeWindow(),
								message,
								owner->GetText(),
								INativeDialogService::DisplayOK,
								INativeDialogService::DefaultFirst,
								INativeDialogService::IconError
								);
							return false;
						}
					}

					WString questionMessage;
					List<vint>* questionFiles = nullptr;
					if (selectToSave && options.promptOverriteFile)
					{
						questionMessage = dialogAskOverrideFile;
						questionFiles = &files;
					}
					if (!selectToSave && options.promptCreateFile)
					{
						questionMessage = dialogAskCreateFile;
						questionFiles = &unexistings;
					}

					if (questionFiles && questionFiles->Count() > 0)
					{
						auto message = stream::GenerateToStream([&](stream::TextWriter& writer)
						{
							writer.WriteString(questionMessage);
							for (vint index : *questionFiles)
							{
								writer.WriteLine(WString::Empty);
								writer.WriteString(L"  ");
								writer.WriteString(wd.GetRelativePathFor(paths[index]));
							}
						});

						auto result = options.messageBox->ShowMessageBox(
							owner->GetNativeWindow(),
							message,
							owner->GetText(),
							INativeDialogService::DisplayOKCancel,
							INativeDialogService::DefaultThird,
							INativeDialogService::IconQuestion
							);

						if (result == INativeDialogService::SelectCancel)
						{
							return false;
						}
					}
				}

				CopyFrom(
					confirmedSelection,
					From(paths).Select([](auto path) { return path.GetFullPath(); })
					);

				Nullable<WString> extension;
				bool extensionFromFilter = false;
				if (selectedFilter)
				{
					extension = selectedFilter->GetDefaultExtension();
					extensionFromFilter = extension;
				}

				if (!extensionFromFilter && options.defaultExtension != WString::Empty)
				{
					extension = options.defaultExtension;
				}

				if (extension)
				{
					auto&& sExt = WString::Unmanaged(L".") + extension.Value();
					vint lExt = sExt.Length();

					// TODO: (enumerable) foreach
					for (vint i = 0; i < confirmedSelection.Count(); i++)
					{
						WString& selection = confirmedSelection[i];
						if (extensionFromFilter)
						{
							if (selection.Length() >= lExt && selection.Right(lExt) == sExt)
							{
								continue;
							}
						}
						else
						{
							auto selectedFileName = filesystem::FilePath(selection).GetName();
							if (INVLOC.FindFirst(selectedFileName, WString::Unmanaged(L"."), Locale::None).key != -1)
							{
								continue;
							}
						}
						selection += sExt;
					}
				}

				confirmed = true;
				return true;
			}
```

## STEP 5: Factor filter parsing into a shared helper and reuse it [DONE]

Update `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`.

1) Insert this helper after the closing `};` of `class FileDialogViewModel` and before the `/*********************************************************************** FakeDialogServiceBase` section:

```cpp
		namespace
		{
			void ParseFileDialogFilters(FileDialogViewModel& vm, vint& selectionFilterIndex, const WString& filter)
			{
				Regex regexFilterExt(L"/*.[^*?]+");
				Regex regexWildcard(L"[*?;]");
				vint filterStart = 0;
				while (true)
				{
					vint first = -1;
					vint second = -1;
					vint count = filter.Length();

					for (vint i = filterStart; i < count; i++)
					{
						if (filter[i] == L'|')
						{
							first = i;
							break;
						}
					}
					if (first == -1) break;

					for (vint i = first + 1; i < count; i++)
					{
						if (filter[i] == L'|')
						{
							second = i;
							break;
						}
					}

					auto filterItem = Ptr(new FileDialogFilter);
					filterItem->name = filter.Sub(filterStart, first - filterStart);
					filterItem->filter = filter.Sub(first + 1, (second == -1 ? count : second) - first - 1);

					if (auto match = regexFilterExt.MatchHead(filterItem->filter))
					{
						if (match->Result().Length() == filterItem->filter.Length())
						{
							filterItem->defaultExtension = filterItem->filter.Right(filterItem->filter.Length() - 2);
						}
					}

					auto regexFilter = stream::GenerateToStream([&](stream::TextWriter& writer)
					{
						writer.WriteString(L"^(");
						List<Ptr<RegexMatch>> matches;
						regexWildcard.Cut(filterItem->filter, false, matches);
						for (auto match : matches)
						{
							if (match->Success())
							{
								auto wildcard = match->Result().Value()[0];
								switch (wildcard)
								{
								case L'*':
									writer.WriteString(WString::Unmanaged(L"/.*"));
									break;
								case L'?':
									writer.WriteString(WString::Unmanaged(L"/."));
									break;
								case L';':
									writer.WriteString(WString::Unmanaged(L"|"));
									break;
								}
							}
							else
							{
								writer.WriteString(u32tow(regex_internal::EscapeTextForRegex(wtou32(match->Result().Value()))));
							}
						}
						writer.WriteString(L")$");
					});
					filterItem->regexFilter = Ptr(new Regex(regexFilter));

					vm.filters.Add(filterItem);

					if (second == -1) break;
					filterStart = second + 1;
				}

				if (vm.filters.Count() > 0)
				{
					if (selectionFilterIndex < 0 || vm.filters.Count() <= selectionFilterIndex)
					{
						selectionFilterIndex = 0;
					}
					vm.selectedFilter = vm.filters[selectionFilterIndex].Cast<FileDialogFilter>();
				}
			}
		}
```

2) In `FakeDialogServiceBase::ShowFileDialog`, delete the existing filter parsing block (from `Regex regexFilterExt...` to the `vm->selectedFilter = ...` block) and replace it with:

```cpp
			ParseFileDialogFilters(*vm.Obj(), selectionFilterIndex, filter);
```

## STEP 6: Wire `FileSystemViewModelOptions` in `FakeDialogServiceBase::ShowFileDialog` [DONE]

Update `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`.

In `FakeDialogServiceBase::ShowFileDialog`, replace:

```cpp
			vm->title = title;
			vm->enabledMultipleSelection = (options & INativeDialogService::FileDialogAllowMultipleSelection) != 0;
			vm->fileMustExist = (options & INativeDialogService::FileDialogFileMustExist) != 0;
			vm->folderMustExist = (options & INativeDialogService::FileDialogDirectoryMustExist) != 0;
			vm->promptCreateFile = (options & INativeDialogService::FileDialogPromptCreateFile) != 0;
			vm->promptOverriteFile = (options & INativeDialogService::FileDialogPromptOverwriteFile) != 0;
			vm->defaultExtension = defaultExtension;
```

with:

```cpp
			vm->options.title = title;
			vm->options.enabledMultipleSelection = (options & INativeDialogService::FileDialogAllowMultipleSelection) != 0;
			vm->options.fileMustExist = (options & INativeDialogService::FileDialogFileMustExist) != 0;
			vm->options.folderMustExist = (options & INativeDialogService::FileDialogDirectoryMustExist) != 0;
			vm->options.promptCreateFile = (options & INativeDialogService::FileDialogPromptCreateFile) != 0;
			vm->options.promptOverriteFile = (options & INativeDialogService::FileDialogPromptOverwriteFile) != 0;
			vm->options.defaultExtension = defaultExtension;
			vm->options.messageBox = Ptr(new DefaultFileSystemViewModelMessageBox);
```

## STEP 7: Add a cpp-only factory returning `Ptr<IFileDialogViewModel>` for unit tests [DONE]

Update `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`.

Append the following function after `FakeDialogServiceBase::ShowFileDialog` and before the closing namespace braces:

```cpp
		Ptr<IFileDialogViewModel> CreateFileDialogViewModelForTest(
			const FileSystemViewModelOptions& options,
			const WString& initialDirectory,
			const WString& filter,
			INativeDialogService::FileDialogTypes dialogType,
			vint selectionFilterIndex
		)
		{
			auto vm = Ptr(new FileDialogViewModel);
			vm->options = options;
			if (!vm->options.messageBox)
			{
				vm->options.messageBox = Ptr(new DefaultFileSystemViewModelMessageBox);
			}

			ParseFileDialogFilters(*vm.Obj(), selectionFilterIndex, filter);

			vm->initialDirectory = initialDirectory;
			vm->rootFolder = Ptr(new FileDialogFolder);
			vm->rootFolder->type = FileDialogFolderType::Root;

			switch (dialogType)
			{
			case INativeDialogService::FileDialogOpen:
			case INativeDialogService::FileDialogOpenPreview:
				vm->selectToSave = false;
				break;
			case INativeDialogService::FileDialogSave:
			case INativeDialogService::FileDialogSavePreview:
				vm->selectToSave = true;
				break;
			}

			return vm;
		}
```

## STEP 8: Update reflection for new members [DONE]

Update `Source/Reflection/TypeDescriptors/GuiReflectionBasic.cpp`.

In the `BEGIN_INTERFACE_MEMBER_NOPROXY(IFileDialogViewModel)` block, insert the following lines right after `CLASS_MEMBER_METHOD(TryConfirm, ...)`:

```cpp
				CLASS_MEMBER_METHOD(IsConfirmed, NO_PARAMETER)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(ConfirmedSelection)
				CLASS_MEMBER_METHOD(ResetConfirmation, NO_PARAMETER)
```

## STEP 9: Add empty test translation unit + project wiring [DONE]

1) Create an empty file:

- `Test/GacUISrc/UnitTest/TestFileDialogViewModel.cpp`

2) Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj`:

- In the `<ItemGroup>` that contains `<ClCompile Include="TestDocumentConfig.cpp" />`, add:

```xml
    <ClCompile Include="TestFileDialogViewModel.cpp" />
```

3) Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj.filters`:

- Near the existing entry for `TestDocumentConfig.cpp`, add:

```xml
    <ClCompile Include="TestFileDialogViewModel.cpp">
      <Filter>Source Files</Filter>
    </ClCompile>
```

## Validation

- Build affected projects (including `UnitTest`) to ensure the header change, implementation change, and reflection change compile together.

# FIXING ATTEMPTS

# !!!FINISHED!!!
