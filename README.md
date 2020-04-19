# 解释器的实现样例

实现步骤:

1. 词法分析:将源代码转换成 token
2. 语法分析:将 token 转换为 AST 抽象语法树
3. 语义分析:AST 处理
4. ......

## 概念

- [Recursive descent parser](https://en.wikipedia.org/wiki/Recursive_descent_parser)
- [Top-Down operator precedence parsing](https://eli.thegreenplace.net/2010/01/02/top-down-operator-precedence-parsing)

## Lisp 解释器实现

- [Lisp interpreter in 90 lines of C++](http://howtowriteaprogram.blogspot.com/2010/11/lisp-interpreter-in-90-lines-of-c.html)
- [Lisp interpreter in 90 lines of C++](https://gist.github.com/ofan/721464)
- [(How to Write a (Lisp) Interpreter (in Python))](https://norvig.com/lispy.html)
- [Implementing Scheme in C++ - Introduction](https://solarianprogrammer.com/2011/11/14/scheme-in-cpp/)

## Pratt Parser

- [Pratt Parsers: Expression Parsing Made Easy](https://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/)
- [Pratt Parser in Python](https://www.slideshare.net/percolate/pratt-parser-in-python)
  [源代码](https://github.com/percolate/pratt-parser)
- [让 Go 很容易手写 Parser](https://zhuanlan.zhihu.com/p/34161576)
- [Simple Top-Down Parsing in Python](http://effbot.org/zone/simple-top-down-parsing.htm)

## 表达式运算

- [Evaluating Expressions](https://mariusbancila.ro/blog/2009/02/03/evaluating-expressions-part-1/)

## 教程

- [Write your own compiler - Introduction](https://blog.klipse.tech/javascript/2017/02/08/tiny-compiler-intro.html)

- [Lexical Analysis](https://hackernoon.com/lexical-analysis-861b8bfe4cb0)
- [Implementing Lexers and Parsers](http://www.cse.chalmers.se/edu/year/2015/course/DAT150/lectures/proglang-04.html)

## 参考

- [从零开始用 Go 实现 Lexer & Parser](https://myslide.cn/slides/17269#)

- [Go 语言设计与实现](https://draveness.me/golang/docs/part1-prerequisite/ch02-compile/golang-compile-intro/)

- [A programming Language interpreter, in C++](https://www.reddit.com/r/cpp/comments/fvkb66/a_programming_language_interpreter_in_c/)

- [Let’s Build A Simple Interpreter.](https://ruslanspivak.com/lsbasi-part1/)

- [自己动手写编译器](https://pandolia.net/tinyc/index.html)

  

