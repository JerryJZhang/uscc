//
//  ParseExpr.cpp
//  uscc
//
//  Implements all of the recursive descent parsing
//  functions for the expression grammar rules.
//
//---------------------------------------------------------
//  Copyright (c) 2014, Sanjay Madhav
//  All rights reserved.
//
//  This file is distributed under the BSD license.
//  See LICENSE.TXT for details.
//---------------------------------------------------------

#include "Parse.h"
#include "Symbols.h"
#include <iostream>
#include <sstream>

using namespace uscc::parse;
using namespace uscc::scan;

using std::shared_ptr;
using std::make_shared;

shared_ptr<ASTExpr> Parser::parseExpr()
{
	shared_ptr<ASTExpr> retVal;
	
	// We should first get a AndTerm
	shared_ptr<ASTExpr> andTerm = parseAndTerm();
	
	// If we didn't get an andTerm, then this isn't an Expr
	if (andTerm)
	{
		retVal = andTerm;
		// Check if this is followed by an op (optional)
		shared_ptr<ASTLogicalOr> exprPrime = parseExprPrime(retVal);
		
		if (exprPrime)
		{
			// If we got a exprPrime, return this instead of just term
			retVal = exprPrime;
		}
	}
	
	return retVal;
}

shared_ptr<ASTLogicalOr> Parser::parseExprPrime(shared_ptr<ASTExpr> lhs)
{
	shared_ptr<ASTLogicalOr> retVal;
	
    // Must be ||
    int col = mColNumber;
	if (peekToken() == Token::Or)
	{
		// Make the binary cmp op
		Token::Tokens op = peekToken();
		retVal = make_shared<ASTLogicalOr>();
		consumeToken();
		
		// Set the lhs to our parameter
		retVal->setLHS(lhs);
		
		// We MUST get a AndTerm as the RHS of this operand
		shared_ptr<ASTExpr> rhs = parseAndTerm();
		if (!rhs)
		{
			throw OperandMissing(op);
		}
		
		retVal->setRHS(rhs);
		
        // PA2: Finalize op
        
        if (!retVal->finalizeOp()) {
            std::string err("Cannot perform op between type ");
            err += getTypeText(lhs->getType());
            err += " and ";
            err += getTypeText(rhs->getType());
            reportSemantError(err, col);
        }
		
		// See comment in parseTermPrime if you're confused by this
		shared_ptr<ASTLogicalOr> exprPrime = parseExprPrime(retVal);
		if (exprPrime)
		{
			retVal = exprPrime;
		}
	}
	
	return retVal;
}

// AndTerm -->
shared_ptr<ASTExpr> Parser::parseAndTerm()
{
	shared_ptr<ASTExpr> retVal;

	// PA1: This should not directly check factor
	// but instead implement the proper grammar rule
    
    // Syntax:
    // AndTerm --> RelTerm AndTerm'
    
    shared_ptr<ASTExpr> relExpr = parseRelExpr();
    if (relExpr) {
        shared_ptr<ASTLogicalAnd> binaryCmpOp = parseAndTermPrime(relExpr);
        if (binaryCmpOp) {
            retVal = binaryCmpOp;
        }
        else {
            retVal = relExpr;
        }
    }
	
	return retVal;
}

shared_ptr<ASTLogicalAnd> Parser::parseAndTermPrime(shared_ptr<ASTExpr> lhs)
{
	shared_ptr<ASTLogicalAnd> retVal;

	// PA1: Implement
    
    // Syntax:
    // AndTerm' --> && RelExpr AndTerm'
    //           |  Epsilon
    
    shared_ptr<ASTLogicalAnd> logicalAnd1;
    shared_ptr<ASTExpr> rhs;
    
    // And Token
    int col = mColNumber;
    if (peekToken() == Token::And) {
        
        // Construct AST for Binary Math Operator
        logicalAnd1 = make_shared<ASTLogicalAnd>();
        scan::Token::Tokens temp = peekToken();
        consumeToken();
        rhs = parseRelExpr();
        if (!rhs) {
            throw OperandMissing(temp);
        }
        logicalAnd1->setLHS(lhs);
        logicalAnd1->setRHS(rhs);
        
        // PA2 Semantic Check
        if (!logicalAnd1->finalizeOp()) {
            std::string err("Cannot perform op between type ");
            err += getTypeText(lhs->getType());
            err += " and ";
            err += getTypeText(rhs->getType());
            reportSemantError(err, col);
        }
        
        
        // Recurse and see if there is another level of AST
        shared_ptr<ASTLogicalAnd> logicalAnd2 = parseAndTermPrime(logicalAnd1);
        
        // If there is another level, return that one
        if (logicalAnd2) {
            retVal = logicalAnd2;
        }
        // Otherwise, return current level
        else {
            retVal = logicalAnd1;
        }
    }
    // Nothing (epsilon)
    else {
        retVal = nullptr;
    }
    
	
	return retVal;
}

// RelExpr -->
shared_ptr<ASTExpr> Parser::parseRelExpr()
{
	shared_ptr<ASTExpr> retVal;

	// PA1: Implement
    
    // Syntax:
    // RelExpr --> NumExpr RelExpr'
    
    shared_ptr<ASTExpr> numExpr = parseNumExpr();
    if (numExpr) {
        shared_ptr<ASTBinaryCmpOp> binaryCmpOp = parseRelExprPrime(numExpr);
        if (binaryCmpOp) {
            retVal = binaryCmpOp;
        }
        else {
            retVal = numExpr;
        }
    }
	
	return retVal;
}

shared_ptr<ASTBinaryCmpOp> Parser::parseRelExprPrime(shared_ptr<ASTExpr> lhs)
{
	shared_ptr<ASTBinaryCmpOp> retVal;
	
	// PA1: Implement
    
    // Syntax:
    // RelExpr' --> == NumExpr RelExpr'
    //           |  != NumExpr RelExpr'
    //           |  < NumExpr RelExpr'
    //           |  > NumExpr RelExpr'
    //           |  Epsilon
    
    shared_ptr<ASTBinaryCmpOp> binaryCmpOp1;
    shared_ptr<ASTExpr> rhs;
    
    // '==', '!=', '<', '>' Tokens
    int col = mColNumber;
    if (peekToken() == Token::EqualTo || peekToken() == Token::NotEqual
        || peekToken() == Token::LessThan || peekToken() == Token::GreaterThan) {
        
        // Construct AST for Binary Math Operator
        binaryCmpOp1 = make_shared<ASTBinaryCmpOp>(peekToken());
        scan::Token::Tokens temp = peekToken();
        consumeToken();
        rhs = parseNumExpr();
        if (!rhs) {
            throw OperandMissing(temp);
        }
        binaryCmpOp1->setLHS(lhs);
        binaryCmpOp1->setRHS(rhs);
        
        // PA2 Semantic Check
        if (!binaryCmpOp1->finalizeOp()) {
            std::string err("Cannot perform op between type ");
            err += getTypeText(lhs->getType());
            err += " and ";
            err += getTypeText(rhs->getType());
            reportSemantError(err, col);
        }
        
        // Recurse and see if there is another level of AST
        shared_ptr<ASTBinaryCmpOp> binaryCmpOp2 = parseRelExprPrime(binaryCmpOp1);
        
        // If there is another level, return that one
        if (binaryCmpOp2) {
            retVal = binaryCmpOp2;
        }
        // Otherwise, return current level
        else {
            retVal = binaryCmpOp1;
        }
    }
    // Nothing (epsilon)
    else {
        retVal = nullptr;
    }
    
	
	return retVal;
}

// NumExpr -->
shared_ptr<ASTExpr> Parser::parseNumExpr()
{
	shared_ptr<ASTExpr> retVal;
	
	// PA1: Implement
    
    // Syntax:
    // NumExpr --> Term NumExpr'
    
    shared_ptr<ASTExpr> term = parseTerm();
    if (term) {
        shared_ptr<ASTBinaryMathOp> binaryMathOp = parseNumExprPrime(term);
        if (binaryMathOp) {
            retVal = binaryMathOp;
        }
        else {
            retVal = term;
        }
    }
	
	return retVal;
}

shared_ptr<ASTBinaryMathOp> Parser::parseNumExprPrime(shared_ptr<ASTExpr> lhs)
{
	shared_ptr<ASTBinaryMathOp> retVal;

	// PA1: Implement
    
    // Syntax:
    // NumExpr' --> + Term NumExpr'
    //           |  - Term NumExpr'
    //           |  Epsilon
    
    shared_ptr<ASTBinaryMathOp> binaryMathOp1;
    shared_ptr<ASTExpr> rhs;
    
    // Plus or Minus Token
    int col = mColNumber;
    if (peekToken() == Token::Plus || peekToken() == Token::Minus) {
        
        // Construct AST for Binary Math Operator
        binaryMathOp1 = make_shared<ASTBinaryMathOp>(peekToken());
        scan::Token::Tokens temp = peekToken();
        consumeToken();
        rhs = parseTerm();
        if (!rhs) {
            throw OperandMissing(temp);
        }
        binaryMathOp1->setLHS(lhs);
        binaryMathOp1->setRHS(rhs);
        
        // PA2 Semantic Check
        if (!binaryMathOp1->finalizeOp()) {
            std::string err("Cannot perform op between type ");
            err += getTypeText(lhs->getType());
            err += " and ";
            err += getTypeText(rhs->getType());
            reportSemantError(err, col);
        }
        
        // Recurse and see if there is another level of AST
        shared_ptr<ASTBinaryMathOp> binaryMathOp2 = parseNumExprPrime(binaryMathOp1);
        
        // If there is another level, return that one
        if (binaryMathOp2) {
            retVal = binaryMathOp2;
        }
        // Otherwise, return current level
        else {
            retVal = binaryMathOp1;
        }
    }
    // Nothing (epsilon)
    else {
        retVal = nullptr;
    }
	
	return retVal;
}

// Term -->
shared_ptr<ASTExpr> Parser::parseTerm()
{
	shared_ptr<ASTExpr> retVal;

	// PA1: Implement
    
    // Syntax
    // Term --> Value Term'
    
    shared_ptr<ASTExpr> value = parseValue();
    if (value) {
        shared_ptr<ASTBinaryMathOp> binaryMathOp = parseTermPrime(value);
        if (binaryMathOp) {
            retVal = binaryMathOp;
        }
        else {
            retVal = value;
        }
    }
	
	return retVal;
}

shared_ptr<ASTBinaryMathOp> Parser::parseTermPrime(shared_ptr<ASTExpr> lhs)
{
	shared_ptr<ASTBinaryMathOp> retVal;

	// PA1: Implement
    
    // Syntax:
    // Term' --> * Value Term'
    //        |  / Value Term'
    //        |  % Value Term'
    //        |  Epsilon
    
    shared_ptr<ASTBinaryMathOp> binaryMathOp1;
    shared_ptr<ASTExpr> rhs;
    
    // Multiply. Division, or Modulus Token
    int col = mColNumber;
    if (peekToken() == Token::Mult || peekToken() == Token::Div || peekToken() == Token::Mod) {
        
        // Construct AST for Binary Math Operator
        binaryMathOp1 = make_shared<ASTBinaryMathOp>(peekToken());
        scan::Token::Tokens temp = peekToken();
        consumeToken();
        rhs = parseValue();
        if (!rhs) {
            throw OperandMissing(temp);
        }
        binaryMathOp1->setLHS(lhs);
        binaryMathOp1->setRHS(rhs);
        
        // PA2 Semantic Check
        if (!binaryMathOp1->finalizeOp()) {
            std::string err("Cannot perform op between type ");
            err += getTypeText(lhs->getType());
            err += " and ";
            err += getTypeText(rhs->getType());
            reportSemantError(err, col);
        }
        
        // Recurse and see if there is another level of AST
        shared_ptr<ASTBinaryMathOp> binaryMathOp2 = parseTermPrime(binaryMathOp1);
        
        // If there is another level, return that one
        if (binaryMathOp2) {
            retVal = binaryMathOp2;
        }
        // Otherwise, return current level
        else {
            retVal = binaryMathOp1;
        }
    }
    // Nothing (epsilon)
    else {
        retVal = nullptr;
    }
    
	
	return retVal;
}

// Value -->
shared_ptr<ASTExpr> Parser::parseValue()
{
	shared_ptr<ASTExpr> retVal;
	
	// PA1: Implement
    
    if (peekToken() == Token::Not) {
        consumeToken();
        shared_ptr<ASTExpr> factor = parseFactor();
        if (!factor) {
            throw ParseExceptMsg("! must be followed by an expression.");
        }
        retVal = make_shared<ASTNotExpr>(factor);
    }
    else {
        retVal = parseFactor();
    }

	
	return retVal;
}

// Factor -->
shared_ptr<ASTExpr> Parser::parseFactor()
{
	shared_ptr<ASTExpr> retVal;
	
	// Try parse identifier factors FIRST so
	// we make sure to consume the mUnusedIdents
	// before we try any other rules
	
	if ((retVal = parseIdentFactor()))
		;
    else if ((retVal = parseStringFactor()))
        ;
    else if ((retVal = parseConstantFactor()))
        ;
    else if ((retVal = parseParenFactor()))
        ;
    else if ((retVal = parseIncFactor()))
        ;
    else if ((retVal = parseDecFactor()))
        ;
    else if ((retVal = parseAddrOfArrayFactor()))
        ;
	// PA1: Add additional cases
	
	return retVal;
}

// ( Expr )
shared_ptr<ASTExpr> Parser::parseParenFactor()
{
	shared_ptr<ASTExpr> retVal;

	// PA1: Implement
    
    // Syntax:
    // Factor --> ( Expr )
    //         |  (and other options)
    
    if (peekToken() == Token::LParen) {
        consumeToken();
        retVal = parseExpr();
        if (!retVal) {
            throw ParseExceptMsg("Not a valid expression inside parenthesis");
        }
        matchToken(Token::RParen);
    }
	
	return retVal;
}

// constant
shared_ptr<ASTConstantExpr> Parser::parseConstantFactor()
{
	shared_ptr<ASTConstantExpr> retVal;

	// PA1: Implement
    
    // Syntax:
    // Factor --> constant
    //         |  (and other options)
    
    if (peekToken() == Token::Constant) {
        retVal = make_shared<ASTConstantExpr>(getTokenTxt());
        consumeToken();
    }
	
	return retVal;
}

// string
shared_ptr<ASTStringExpr> Parser::parseStringFactor()
{
	shared_ptr<ASTStringExpr> retVal;

	// PA1: Implement
    
    // Syntax:
    // Factor --> string
    //         |  (and other options)
    
    if (peekToken() == Token::String) {
        retVal = make_shared<ASTStringExpr>(getTokenTxt(), mStrings);
        consumeToken();
    }
	
	return retVal;
}

// id
// id [ Expr ]
// id ( FuncCallArgs )
shared_ptr<ASTExpr> Parser::parseIdentFactor()
{
	shared_ptr<ASTExpr> retVal;
	if (peekToken() == Token::Identifier ||
		mUnusedIdent != nullptr || mUnusedArray != nullptr)
	{
		if (mUnusedArray)
		{
			// "unused array" means that AssignStmt looked at this array
			// and decided it didn't want it, so it's already made an
			// array sub node
			retVal = make_shared<ASTArrayExpr>(mUnusedArray);
			mUnusedArray = nullptr;
		}
		else
		{
			Identifier* ident = nullptr;
			
			// If we have an "unused identifier," which means that
			// AssignStmt looked at this and decided it didn't want it,
			// that means we're already a token AFTER the identifier.
			if (mUnusedIdent)
			{
				ident = mUnusedIdent;
				mUnusedIdent = nullptr;
			}
			else
			{
				ident = getVariable(getTokenTxt());
				consumeToken();
			}
			
			// Now we need to look ahead and see if this is an array
			// or function call reference, since id is a common
			// left prefix.
			if (peekToken() == Token::LBracket)
			{
				// Check to make sure this is an array
				if (mCheckSemant && ident->getType() != Type::IntArray &&
					ident->getType() != Type::CharArray &&
					!ident->isDummy())
				{
					std::string err("'");
					err += ident->getName();
					err += "' is not an array";
					reportSemantError(err);
					consumeUntil(Token::RBracket);
					if (peekToken() == Token::EndOfFile)
					{
						throw EOFExcept();
					}
					
					matchToken(Token::RBracket);
					
					// Just return our error variable
					retVal = make_shared<ASTIdentExpr>(*mSymbols.getIdentifier("@@variable"));
				}
				else
				{
					consumeToken();
					try
					{
						shared_ptr<ASTExpr> expr = parseExpr();
						if (!expr)
						{
							throw ParseExceptMsg("Valid expression required inside [ ].");
						}
						
						shared_ptr<ASTArraySub> array = make_shared<ASTArraySub>(*ident, expr);
						retVal = make_shared<ASTArrayExpr>(array);
					}
					catch (ParseExcept& e)
					{
						// If this expr is bad, consume until RBracket
						reportError(e);
						consumeUntil(Token::RBracket);
						if (peekToken() == Token::EndOfFile)
						{
							throw EOFExcept();
						}
					}
					
					matchToken(Token::RBracket);
				}
			}
			else if (peekToken() == Token::LParen)
			{
				// Check to make sure this is a function
				if (mCheckSemant && ident->getType() != Type::Function &&
					!ident->isDummy())
				{
					std::string err("'");
					err += ident->getName();
					err += "' is not a function";
					reportSemantError(err);
					consumeUntil(Token::RParen);
					if (peekToken() == Token::EndOfFile)
					{
						throw EOFExcept();
					}
					
					matchToken(Token::RParen);
					
					// Just return our error variable
					retVal = make_shared<ASTIdentExpr>(*mSymbols.getIdentifier("@@variable"));
				}
				else
				{
					consumeToken();
					// A function call can have zero or more arguments
					shared_ptr<ASTFuncExpr> funcCall = make_shared<ASTFuncExpr>(*ident);
					retVal = funcCall;
					
					// Get the number of arguments for this function
					shared_ptr<ASTFunction> func = ident->getFunction();
					
					try
					{
						int currArg = 1;
						int col = mColNumber;
						shared_ptr<ASTExpr> arg = parseExpr();
						while (arg)
						{
							// Check for validity of this argument (for non-dummy functions)
							if (!ident->isDummy())
							{
								// Special case for "printf" since we don't make a node for it
								if (ident->getName() == "printf")
								{
									mNeedPrintf = true;
									if (currArg == 1 && arg->getType() != Type::CharArray)
									{
										reportSemantError("The first parameter to printf must be a char[]");
									}
								}
								else if (mCheckSemant)
								{
									if (currArg > func->getNumArgs())
									{
										std::string err("Function ");
										err += ident->getName();
										err += " takes only ";
										std::ostringstream ss;
										ss << func->getNumArgs();
										err += ss.str();
										err += " arguments";
										reportSemantError(err, col);
									}
									else if (!func->checkArgType(currArg, arg->getType()))
									{
										// If we have an int and the expected arg type is a char,
										// we can do a conversion
										if (arg->getType() == Type::Int &&
											func->getArgType(currArg) == Type::Char)
										{
											arg = intToChar(arg);
										}
										else
										{
											std::string err("Expected expression of type ");
											err += getTypeText(func->getArgType(currArg));
											reportSemantError(err, col);
										}
									}
								}
							}
							
							funcCall->addArg(arg);
							
							currArg++;
							
							if (peekAndConsume(Token::Comma))
							{
								col = mColNumber;
								arg = parseExpr();
								if (!arg)
								{
									throw
									ParseExceptMsg("Comma must be followed by expression in function call");
								}
							}
							else
							{
								break;
							}
						}
					}
					catch (ParseExcept& e)
					{
						reportError(e);
						consumeUntil(Token::RParen);
						if (peekToken() == Token::EndOfFile)
						{
							throw EOFExcept();
						}
					}
					
					// Now make sure we have the correct number of arguments
					if (!ident->isDummy())
					{
						// Special case for printf
						if (ident->getName() == "printf")
						{
							if (funcCall->getNumArgs() == 0)
							{
								reportSemantError("printf requires a minimum of one argument");
							}
						}
						else if (mCheckSemant && funcCall->getNumArgs() < func->getNumArgs())
						{
							std::string err("Function ");
							err += ident->getName();
							err += " requires ";
							std::ostringstream ss;
							ss << func->getNumArgs();
							err += ss.str();
							err += " arguments";
							reportSemantError(err);
						}
					}
					
					matchToken(Token::RParen);
				}
			}
			else
			{
				// Just a plain old ident
				retVal = make_shared<ASTIdentExpr>(*ident);
			}
		}
	}
    
    retVal = charToInt(retVal);
	
	return retVal;
}

// ++ id
shared_ptr<ASTExpr> Parser::parseIncFactor()
{
	shared_ptr<ASTExpr> retVal;
	
	// PA1: Implement
    
    if (peekToken() == Token::Inc) {
        consumeToken();
        retVal = make_shared<ASTIncExpr>(*getVariable(getTokenTxt()));
        consumeToken();
    }
    
    retVal = charToInt(retVal);
	
	return retVal;
}

// -- id
shared_ptr<ASTExpr> Parser::parseDecFactor()
{
	shared_ptr<ASTExpr> retVal;
	
	// PA1: Implement
    
    if (strcmp(getTokenTxt(), "--") == 0) {
        consumeToken();
        retVal = make_shared<ASTDecExpr>(*getVariable(getTokenTxt()));
        consumeToken();
    }
    
    retVal = charToInt(retVal);

	return retVal;
}

// & id [ Expr ]
shared_ptr<ASTExpr> Parser::parseAddrOfArrayFactor()
{
	shared_ptr<ASTExpr> retVal;
	
	// PA1: Implement
    
    // Syntax:
    // Factor --> & id [ Expr ]
    //         |  (and other options)
    
    shared_ptr<ASTArraySub> array;
    shared_ptr<ASTExpr> expr;
    Identifier* id;
    
    if (peekToken() == Token::Addr) {
        consumeToken();
        
        if (peekToken() == Token::Identifier) {
            id = getVariable(getTokenTxt());
            consumeToken();
            
            if (peekToken() == Token::LBracket) {
                
                consumeToken();
                
                expr = parseExpr();
                if (!expr) {
                    throw ParseExceptMsg("Missing required subscript expression.");
                }
                
                matchToken(Token::RBracket);
                
                array = make_shared<ASTArraySub>(*id, expr);
                retVal = make_shared<ASTAddrOfArray>(array);
            }
            
        }
        else {
            throw ParseExceptMsg("& must be followed by an identifier.");
        }
        
    }
	
	return retVal;
}
