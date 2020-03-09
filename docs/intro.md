About AParse
==================================

AParse is a parser generater, just like Antlr, Bison and Yacc etc.

[GitHub Page](https://github.com/mohitmv/aparse)

----------------------------------

## Abstract

We propose a new grammar, named `AParse`, which is strictly subset of Extended Backus–Naur form (E-BNF) of Context Free Grammar (CFG). AParse is powerful enough to express most of practically used programming languages like C++, Java, Python etc. We designed a mechanism to generate a parser from AParse grammar. Parser is capable of recognizing a string S and parse a valid string in O(|S|) time in worst case.

----------------------------------

## Why AParse

AParse is more powerful than other parser generators like Antlr, Bison, Yacc in following criterias:

1. AParse is guaranteed to be linear time parser on any possible AParse grammar.
2. AParse support sequential parsing, i.e. alphabets can be fed to parser one by one. There is no need of looking ahead in alphabet sequence. The parser can recognise the invalid alphabet in sequence and throw error.
3. AParse support `PossibleAlphabets()` API which returns all the acceptable alphabets at any point of time while feeding the sequential alphabets.
4. If there is no possiblity of ambiguity, AParse parser can construct the partial-syntax-tree as alphabets are being fed to it. This is highly useful feature for the parsing of programming languages, which are not purely context-free, and require partially parsed context to interpret a token (ex: `type` vs `variable` in C++).
----------------------------------

## AParse Grammar

[AParse Grammar](https://drive.google.com/open?id=1KFDTZ1qls-DymP5xzH-2KT_mU_GY9cBv)


----------------------------------

## How to use AParse C++

[AParse C++](aparse_cpp.md)

Look at `tests/advance_parser_integration_test.cpp` for a hello-world parser using AParse.

----------------------------------
