/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:Ast
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PRESENTATION_REMOTEPROTOCOL_AST_AST
#define VCZH_PRESENTATION_REMOTEPROTOCOL_AST_AST

#include <VlppGlrParser.h>

namespace vl::presentation::remoteprotocol
{
	class GuiRpArrayMapType;
	class GuiRpArrayType;
	class GuiRpAttribute;
	class GuiRpDeclaration;
	class GuiRpEnumDecl;
	class GuiRpEnumMember;
	class GuiRpEventDecl;
	class GuiRpEventRequest;
	class GuiRpMapType;
	class GuiRpMessageDecl;
	class GuiRpMessageRequest;
	class GuiRpMessageResponse;
	class GuiRpOptionalType;
	class GuiRpPrimitiveType;
	class GuiRpReferenceType;
	class GuiRpSchema;
	class GuiRpStructDecl;
	class GuiRpStructMember;
	class GuiRpType;
	class GuiRpUnionDecl;
	class GuiRpUnionMember;

	enum class GuiRpPrimitiveTypes
	{
		UNDEFINED_ENUM_ITEM_VALUE = -1,
		Boolean = 0,
		Integer = 1,
		Float = 2,
		Double = 3,
		String = 4,
		Char = 5,
		Key = 6,
		Color = 7,
		Binary = 8,
	};

	enum class GuiRpStructType
	{
		UNDEFINED_ENUM_ITEM_VALUE = -1,
		Struct = 0,
		Class = 1,
	};

	class GuiRpType abstract : public vl::glr::ParsingAstBase, vl::reflection::Description<GuiRpType>
	{
	public:
		class IVisitor : public virtual vl::reflection::IDescriptable, vl::reflection::Description<IVisitor>
		{
		public:
			virtual void Visit(GuiRpPrimitiveType* node) = 0;
			virtual void Visit(GuiRpReferenceType* node) = 0;
			virtual void Visit(GuiRpOptionalType* node) = 0;
			virtual void Visit(GuiRpArrayType* node) = 0;
			virtual void Visit(GuiRpArrayMapType* node) = 0;
			virtual void Visit(GuiRpMapType* node) = 0;
		};

		virtual void Accept(GuiRpType::IVisitor* visitor) = 0;

	};

	class GuiRpPrimitiveType : public GuiRpType, vl::reflection::Description<GuiRpPrimitiveType>
	{
	public:
		GuiRpPrimitiveTypes type = GuiRpPrimitiveTypes::UNDEFINED_ENUM_ITEM_VALUE;

		void Accept(GuiRpType::IVisitor* visitor) override;
	};

	class GuiRpReferenceType : public GuiRpType, vl::reflection::Description<GuiRpReferenceType>
	{
	public:
		vl::glr::ParsingToken name;

		void Accept(GuiRpType::IVisitor* visitor) override;
	};

	class GuiRpOptionalType : public GuiRpType, vl::reflection::Description<GuiRpOptionalType>
	{
	public:
		vl::Ptr<GuiRpType> element;

		void Accept(GuiRpType::IVisitor* visitor) override;
	};

	class GuiRpArrayType : public GuiRpType, vl::reflection::Description<GuiRpArrayType>
	{
	public:
		vl::Ptr<GuiRpType> element;

		void Accept(GuiRpType::IVisitor* visitor) override;
	};

	class GuiRpArrayMapType : public GuiRpType, vl::reflection::Description<GuiRpArrayMapType>
	{
	public:
		vl::glr::ParsingToken element;
		vl::glr::ParsingToken keyField;

		void Accept(GuiRpType::IVisitor* visitor) override;
	};

	class GuiRpMapType : public GuiRpType, vl::reflection::Description<GuiRpMapType>
	{
	public:
		vl::Ptr<GuiRpType> element;
		vl::Ptr<GuiRpType> keyType;

		void Accept(GuiRpType::IVisitor* visitor) override;
	};

	class GuiRpAttribute : public vl::glr::ParsingAstBase, vl::reflection::Description<GuiRpAttribute>
	{
	public:
		vl::glr::ParsingToken name;
		vl::glr::ParsingToken cppType;
	};

	class GuiRpDeclaration abstract : public vl::glr::ParsingAstBase, vl::reflection::Description<GuiRpDeclaration>
	{
	public:
		class IVisitor : public virtual vl::reflection::IDescriptable, vl::reflection::Description<IVisitor>
		{
		public:
			virtual void Visit(GuiRpEnumDecl* node) = 0;
			virtual void Visit(GuiRpUnionDecl* node) = 0;
			virtual void Visit(GuiRpStructDecl* node) = 0;
			virtual void Visit(GuiRpMessageDecl* node) = 0;
			virtual void Visit(GuiRpEventDecl* node) = 0;
		};

		virtual void Accept(GuiRpDeclaration::IVisitor* visitor) = 0;

		vl::collections::List<vl::Ptr<GuiRpAttribute>> attributes;
		vl::glr::ParsingToken name;
	};

	class GuiRpEnumMember : public vl::glr::ParsingAstBase, vl::reflection::Description<GuiRpEnumMember>
	{
	public:
		vl::glr::ParsingToken name;
	};

	class GuiRpEnumDecl : public GuiRpDeclaration, vl::reflection::Description<GuiRpEnumDecl>
	{
	public:
		vl::collections::List<vl::Ptr<GuiRpEnumMember>> members;

		void Accept(GuiRpDeclaration::IVisitor* visitor) override;
	};

	class GuiRpUnionMember : public vl::glr::ParsingAstBase, vl::reflection::Description<GuiRpUnionMember>
	{
	public:
		vl::glr::ParsingToken name;
	};

	class GuiRpUnionDecl : public GuiRpDeclaration, vl::reflection::Description<GuiRpUnionDecl>
	{
	public:
		vl::collections::List<vl::Ptr<GuiRpUnionMember>> members;

		void Accept(GuiRpDeclaration::IVisitor* visitor) override;
	};

	class GuiRpStructMember : public vl::glr::ParsingAstBase, vl::reflection::Description<GuiRpStructMember>
	{
	public:
		vl::glr::ParsingToken name;
		vl::Ptr<GuiRpType> type;
	};

	class GuiRpStructDecl : public GuiRpDeclaration, vl::reflection::Description<GuiRpStructDecl>
	{
	public:
		GuiRpStructType type = GuiRpStructType::UNDEFINED_ENUM_ITEM_VALUE;
		vl::collections::List<vl::Ptr<GuiRpStructMember>> members;

		void Accept(GuiRpDeclaration::IVisitor* visitor) override;
	};

	class GuiRpMessageRequest : public vl::glr::ParsingAstBase, vl::reflection::Description<GuiRpMessageRequest>
	{
	public:
		vl::Ptr<GuiRpType> type;
	};

	class GuiRpMessageResponse : public vl::glr::ParsingAstBase, vl::reflection::Description<GuiRpMessageResponse>
	{
	public:
		vl::Ptr<GuiRpType> type;
	};

	class GuiRpMessageDecl : public GuiRpDeclaration, vl::reflection::Description<GuiRpMessageDecl>
	{
	public:
		vl::Ptr<GuiRpMessageRequest> request;
		vl::Ptr<GuiRpMessageResponse> response;

		void Accept(GuiRpDeclaration::IVisitor* visitor) override;
	};

	class GuiRpEventRequest : public vl::glr::ParsingAstBase, vl::reflection::Description<GuiRpEventRequest>
	{
	public:
		vl::Ptr<GuiRpType> type;
	};

	class GuiRpEventDecl : public GuiRpDeclaration, vl::reflection::Description<GuiRpEventDecl>
	{
	public:
		vl::Ptr<GuiRpEventRequest> request;

		void Accept(GuiRpDeclaration::IVisitor* visitor) override;
	};

	class GuiRpSchema : public vl::glr::ParsingAstBase, vl::reflection::Description<GuiRpSchema>
	{
	public:
		vl::collections::List<vl::Ptr<GuiRpDeclaration>> declarations;
	};
}
namespace vl::reflection::description
{
#ifndef VCZH_DEBUG_NO_REFLECTION
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpType)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpType::IVisitor)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpPrimitiveTypes)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpPrimitiveType)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpReferenceType)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpOptionalType)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpArrayType)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpArrayMapType)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpMapType)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpAttribute)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpDeclaration)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpDeclaration::IVisitor)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpEnumMember)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpEnumDecl)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpUnionMember)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpUnionDecl)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpStructMember)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpStructType)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpStructDecl)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpMessageRequest)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpMessageResponse)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpMessageDecl)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpEventRequest)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpEventDecl)
	DECL_TYPE_INFO(vl::presentation::remoteprotocol::GuiRpSchema)

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

	BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(vl::presentation::remoteprotocol::GuiRpType::IVisitor)
		void Visit(vl::presentation::remoteprotocol::GuiRpPrimitiveType* node) override
		{
			INVOKE_INTERFACE_PROXY(Visit, node);
		}

		void Visit(vl::presentation::remoteprotocol::GuiRpReferenceType* node) override
		{
			INVOKE_INTERFACE_PROXY(Visit, node);
		}

		void Visit(vl::presentation::remoteprotocol::GuiRpOptionalType* node) override
		{
			INVOKE_INTERFACE_PROXY(Visit, node);
		}

		void Visit(vl::presentation::remoteprotocol::GuiRpArrayType* node) override
		{
			INVOKE_INTERFACE_PROXY(Visit, node);
		}

		void Visit(vl::presentation::remoteprotocol::GuiRpArrayMapType* node) override
		{
			INVOKE_INTERFACE_PROXY(Visit, node);
		}

		void Visit(vl::presentation::remoteprotocol::GuiRpMapType* node) override
		{
			INVOKE_INTERFACE_PROXY(Visit, node);
		}

	END_INTERFACE_PROXY(vl::presentation::remoteprotocol::GuiRpType::IVisitor)

	BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(vl::presentation::remoteprotocol::GuiRpDeclaration::IVisitor)
		void Visit(vl::presentation::remoteprotocol::GuiRpEnumDecl* node) override
		{
			INVOKE_INTERFACE_PROXY(Visit, node);
		}

		void Visit(vl::presentation::remoteprotocol::GuiRpUnionDecl* node) override
		{
			INVOKE_INTERFACE_PROXY(Visit, node);
		}

		void Visit(vl::presentation::remoteprotocol::GuiRpStructDecl* node) override
		{
			INVOKE_INTERFACE_PROXY(Visit, node);
		}

		void Visit(vl::presentation::remoteprotocol::GuiRpMessageDecl* node) override
		{
			INVOKE_INTERFACE_PROXY(Visit, node);
		}

		void Visit(vl::presentation::remoteprotocol::GuiRpEventDecl* node) override
		{
			INVOKE_INTERFACE_PROXY(Visit, node);
		}

	END_INTERFACE_PROXY(vl::presentation::remoteprotocol::GuiRpDeclaration::IVisitor)

#endif
#endif
	/// <summary>Load all reflectable AST types, only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is off.</summary>
	/// <returns>Returns true if this operation succeeded.</returns>
	extern bool GuiRemoteProtocolAstLoadTypes();
}
#endif