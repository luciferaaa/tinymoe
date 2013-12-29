#ifndef VCZH_TINYMOEEXPRESSIONANALYZER
#define VCZH_TINYMOEEXPRESSIONANALYZER

#include "TinymoeLexicalAnalyzer.h"

namespace tinymoe
{
	/*************************************************************
	Symbol
	*************************************************************/

	enum class GrammarFragmentType
	{
		Name,					// for identifier list,				e.g. [repeat with] the current number [from] 1 [to] 100
		Type,					// for type name,					e.g. set names to new [hash set]
		Primitive,				// for primitive expression,		e.g. sum from 1 to [10]
		Expression,				// for all kinds of expressions,	e.g. repeat with the current number from [1] to [100]
		List,					// for tuple (marshalled as array),	e.g. set names to collection of [("a", "b", "c")]
		Assignable,				// for left value expression, create a new symbol in the containing block if the <assignable> does not exist
								//									e.g. [field unique identifier of person]
								//									e.g. [a variable]
		Argument,				// always create a new symbol in the block body
								//									e.g. repeat with [the current number] from 1 to sum from 1 to 10
	};

	class GrammarFragment
	{
	public:
		typedef shared_ptr<GrammarFragment>			Ptr;
		typedef vector<Ptr>							List;

		GrammarFragmentType			type;
		vector<string>				identifiers;

		GrammarFragment(GrammarFragmentType _type);
	};

	enum class GrammarSymbolTarget
	{
		Custom,					// user defined symbol

		Array,					// (type)		array
		String,					// (type)		string
		Integer,				// (type)		integer
		Float,					// (type)		float
		Symbol,					// (type)		symbol
		
		NewType,				// (primitive)	new <type>
		NewArray,				// (primitive)	new array of <expression> items
		GetArrayItem,			// (primitive)	item <expression> of array <primitive>
		Invoke,					// (primitive)	invoke <primitive>
		InvokeWith,				// (primitive)	invoke <expression> with (<list>)
		IsType,					// (primitive)	<primitive> is <type>
		IsNotType,				// (primitive)	<primitive> is not <type>
		GetField,				// (primitive)	field <argument> of <primitive>

		End,					// (sentence)	end
		Select,					// (sentence)	select <expression>
		Case,					// (sentence)	case <expression>
		TailCall,				// (sentence)	tail call <expression>
		RedirectTo,				// (sentence)	redirect to <expression>
		Assign,					// (sentence)	set <assignable> to <expression>
		SetArrayItem,			// (sentence)	set item <expression> of array <expression> to <expression>
		SetField,				// (sentence)	set field <argument> of <expression> to <expression>
	};

	enum class GrammarSymbolType
	{
		Type,
		Primitive,
		Sentence,
		Block,
	};

	class GrammarSymbol
	{
	public:
		typedef shared_ptr<GrammarSymbol>			Ptr;
		typedef vector<Ptr>							List;
		typedef multimap<string, Ptr>				MultiMap;

		GrammarFragment::List		fragments;		// grammar fragments for this symbol
		bool						statement;		// true means this symbol is a statement
													// a statement cannot be an expression
													// the top invoke expression's function of a statement should reference to a statement symbol
		string						uniqueId;		// a string that identifies the grammar structure
		GrammarSymbolTarget			target;
		GrammarSymbolType			type;

		GrammarSymbol(GrammarSymbolType _type, GrammarSymbolTarget _target = GrammarSymbolTarget::Custom);

		void						CalculateUniqueId();
	};

	/*************************************************************
	Expression
	*************************************************************/

	class Expression : public CodeFragment
	{
	public:
		typedef shared_ptr<Expression>				Ptr;
		typedef vector<Ptr>							List;
	};

	// for numbers and strings
	class LiteralExpression : public Expression
	{
	public:
		CodeToken					token;
	};

	// for new created symbols in <assignable> and <argument>
	class ArgumentExpression : public Expression
	{
	public:
		CodeToken::List				token;
	};

	// for symbol referencing
	class ReferenceExpression : public Expression
	{
	public:
		GrammarSymbol::Ptr			symbol;
	};

	// for function invoking
	class InvokeExpression : public Expression
	{
	public:
		Expression::Ptr				function;
		Expression::List			arguments;
	};

	// for <list>
	class ListExpression : public Expression
	{
	public:
		Expression::List			elements;
	};

	enum class UnaryOperator
	{
		Positive,
		Negative,
		Not,
	};

	// for unary operator invoking
	class UnaryExpression : public Expression
	{
	public:
		Expression::Ptr				first;
		Expression::Ptr				second;
		UnaryOperator				op;
	};

	enum class BinaryOperator
	{
		Concat,
		Add,
		Sub,
		Mul,
		Div,
		LT,
		GT,
		LE,
		GE,
		EQ,
		NE,
		And,
		Or,
	};

	// for binary operator invoking
	class BinaryExpression : public Expression
	{
	public:
		Expression::Ptr				first;
		Expression::Ptr				second;
		BinaryOperator				op;
	};

	/*************************************************************
	Symbol Stack
	*************************************************************/

	class GrammarStackItem
	{
	public:
		typedef shared_ptr<GrammarStackItem>		Ptr;
		typedef vector<Ptr>							List;
		
		GrammarSymbol::List			symbols;

		void						FillPredefinedSymbols();
	};

	class GrammarStack
	{
	public:
		typedef shared_ptr<GrammarStack>			Ptr;
		typedef CodeToken::List::iterator			Iterator;

		GrammarStackItem::List		stackItems;				// available symbols organized in a scope based structure
		GrammarSymbol::MultiMap		availableSymbols;		// available symbols grouped by the unique identifier
															// the last symbol overrides all other symbols in the same group

		void						Push(GrammarStackItem::Ptr stackItem);
		GrammarStackItem::Ptr		Pop();

		CodeError					SuccessError();
		CodeError					ParseToken(const string& token, Iterator input, Iterator end, vector<Iterator>& result);
		CodeError					PickError(CodeError::List& errors);

		CodeError					ParseGrammarFragment(GrammarFragment::Ptr fragment, Iterator input, Iterator end, vector<pair<Iterator, Expression::Ptr>>& result);
		CodeError					ParseGrammarSymbol(GrammarSymbol::Ptr symbol, Iterator input, Iterator end, vector<pair<Iterator, Expression::Ptr>>& result);
		CodeError					ParseType(Iterator input, Iterator end, vector<pair<Iterator, Expression::Ptr>>& result);
		CodeError					ParsePrimitive(Iterator input, Iterator end, vector<pair<Iterator, Expression::Ptr>>& result);
		CodeError					ParseExpression(Iterator input, Iterator end, vector<pair<Iterator, Expression::Ptr>>& result);
		CodeError					ParseList(Iterator input, Iterator end, vector<pair<Iterator, Expression::Ptr>>& result);
		CodeError					ParseAssignable(Iterator input, Iterator end, vector<pair<Iterator, Expression::Ptr>>& result);
		CodeError					ParseArgument(Iterator input, Iterator end, vector<pair<Iterator, Expression::Ptr>>& result);
	};
}

#endif