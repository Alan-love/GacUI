#include "../../../Source/GacUI.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::presentation;

extern WString GetTestResourcePath();
extern WString GetTestOutputPath();

void WriteErrors(GuiResourceError::List& errors, const WString& resourceName)
{
	auto outputPath = FilePath(GetTestOutputPath()) / (resourceName + L".txt");
	auto baselinePath = FilePath(GetTestResourcePath()) / (resourceName + L".txt");

	List<WString> output;
	GuiResourceError::SortAndLog(errors, output, GetTestResourcePath());
	File(outputPath).WriteAllLines(output, false, BomEncoder::Utf8);

	List<WString> baseline;
	File(baselinePath).ReadAllLinesByBom(baseline);
	if (baseline.Count() > 0 && baseline[baseline.Count() - 1] == L"")
	{
		baseline.RemoveAt(baseline.Count() - 1);
	}

	vint count = output.Count();
	if (count < baseline.Count())
	{
		count = baseline.Count();
	}

	for (vint i = 0; i < count; i++)
	{
		TEST_ASSERT(output.Count() > i);
		TEST_ASSERT(baseline.Count() > i);
		TEST_ASSERT(output[i] == baseline[i]);
	}
}

#define PRINT_ERROR\
	do\
	{\
		if (errors.Count() > 0)\
		{\
			WriteErrors(errors, resourceName);\
			return;\
		}\
	}while(0)\

Ptr<GuiResource> LoadResource(const WString& resourceName, bool requireErrors)
{
	Ptr<GuiResource> resource;
	TEST_CASE(L"Compare compiler output for: " + resourceName)
	{
		auto inputPath = FilePath(GetTestResourcePath()) / resourceName;
		GuiResourceError::List errors;

		resource = GuiResource::LoadFromXml(inputPath.GetFullPath(), errors);
		PRINT_ERROR;
		resource->Precompile(nullptr, errors);
		PRINT_ERROR;
		TEST_ASSERT(!requireErrors);

		auto outputPath = FilePath(GetTestOutputPath()) / (resourceName + L".bin");
		FileStream stream(outputPath.GetFullPath(), FileStream::WriteOnly);
		resource->SavePrecompiledBinary(stream);
	});
	return resource;
}

#undef PRINT_ERROR

TEST_FILE
{
	LoadResource(L"Resource.NotExists.xml", true);
	LoadResource(L"Resource.WrongSyntax.xml", true);
	LoadResource(L"Resource.WrongSyntax2.xml", true);
	LoadResource(L"Resource.WrongDoc.xml", true);
	LoadResource(L"Resource.WrongInstanceStyle.xml", true);
	LoadResource(L"Resource.WrongInstance.xml", true);
	LoadResource(L"Resource.FailedInstance.Ctor1.xml", true);
	LoadResource(L"Resource.FailedInstance.Ctor2.xml", true);
	LoadResource(L"Resource.FailedInstance.Ctor2_r.xml", true);
	LoadResource(L"Resource.FailedInstance.Ctor3.xml", true);
	LoadResource(L"Resource.FailedInstance.Ctor4.xml", true);
	LoadResource(L"Resource.FailedInstance.Ctor5.xml", true);
	LoadResource(L"Resource.FailedInstance.Control.xml", true);
	LoadResource(L"Resource.FailedInstance.Inheriting1.xml", true);
	LoadResource(L"Resource.FailedInstance.Inheriting2.xml", true);
	LoadResource(L"Resource.FailedScript.Workflow.xml", true);
	LoadResource(L"Resource.FailedScript.Properties.xml", true);
	LoadResource(L"Resource.FailedScript.Animations.xml", true);
	LoadResource(L"Resource.FailedScript.Animations2.xml", true);
	LoadResource(L"Resource.FailedScript.Strings.xml", true);
	LoadResource(L"Resource.FailedScript.Strings2.xml", true);
}