% aspdoc.cls
% Written by Alexis Engelke in 10/2018
% Copyright (c) TUM I10 2018. All rights reserved.

% Options:
%  - course: erap, asp, [rasp]
%      For course=rasp/asp/erap \ifasp is defined.
%  - tasksheet
%      Defines solution environment, defines \theSheetNumber
%  - project
%      Enhances content of header, defines \theTaskNumber and \theName

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{aspdoc}[2018/10/29 TUM I10 document class v0.3]

\LoadClass[11pt]{scrartcl}

\RequirePackage{kvoptions}
\RequirePackage[utf8]{inputenc}
\RequirePackage[T1]{fontenc}
\RequirePackage[ngerman]{babel} % TODO: Make this an option
\RequirePackage[babel,german=quotes]{csquotes}
\RequirePackage{amsmath}
\RequirePackage{amssymb}
\RequirePackage{graphicx}
\RequirePackage{color}
\RequirePackage{listings}
\RequirePackage{booktabs}
\RequirePackage{float}
\RequirePackage{microtype}
\RequirePackage{tikz}
\RequirePackage{tabularx}
\RequirePackage{hyperref}
\RequirePackage{xstring}
\RequirePackage[headsepline,footsepline,plainfootsepline,markcase=upper]{scrlayer-scrpage}

\usetikzlibrary{shapes,positioning}

\SetupKeyvalOptions{
  family=aspdoc,
  prefix=aspdoc@
}

% Fonts
\setkomafont{dictum}{\normalfont\normalcolor\rmfamily\small}
\renewcommand{\rmdefault}{ppl}
\newcommand{\tttilde}{\raise.17ex\hbox{$\scriptstyle\mathtt{\sim}$}}

\definecolor{dkgreen}{rgb}{0,0.6,0}
\definecolor{gray}{rgb}{0.5,0.5,0.5}
\definecolor{mauve}{rgb}{0.58,0,0.82}

\newcommand{\@maintitle}{\large\@title \\ \@date}
\newcommand{\@headertitle}{\@title}

\newif\if@solution

\DeclareVoidOption{tasksheet}{
    \IfSubStr{\jobname}{\detokenize{Lsg}}{
      \@solutiontrue
      \StrBetween*[2,3]{\jobname}{\detokenize{_}}{\detokenize{_}}[\sheetNumberRaw]
    }{
      \StrBehind*[2]{\jobname}{\detokenize{_}}[\sheetNumberRaw]
    }
    % Remove a single leading zero
    \IfBeginWith{\sheetNumberRaw}{0}{%
      \StrGobbleLeft{\sheetNumberRaw}{1}[\theSheetNumber]%
    }{%
      \global\let\theSheetNumber\sheetNumberRaw%
    }
    %
    \newcommand{\tutorSections}{\setcounter{section}{0}\def\thesection{T\arabic{section}}\renewcommand*{\theHsection}{chT.\the\value{section}}}
    \newcommand{\homeworkSections}{\setcounter{section}{0}\def\thesection{H\arabic{section}}\renewcommand*{\theHsection}{chH.\the\value{section}}}
    \newcommand{\extendedSections}{\setcounter{section}{0}\def\thesection{X\arabic{section}}\renewcommand*{\theHsection}{chX.\the\value{section}}}
    \newcommand{\homeworkAnnotations}{
    \vspace{1em}\noindent Es besteht \emph{keine} Verpflichtung, diese Hausaufgaben abzugeben. Sie dienen lediglich der Übung und werden in der folgenden Tutorstunde besprochen.
    }
    %
    \title{Arbeitsblatt \theSheetNumber}
    \renewcommand{\@headertitle}{Blatt \theSheetNumber}
}
\DeclareVoidOption{project}{
    \StrGobbleLeft{\jobname}{5}[\theTaskNumber]
    \renewcommand{\@maintitle}{\large\@title {} (A\theTaskNumber)\\ Projektaufgabe -- Aufgabenbereich \@subtitle}
    \renewcommand{\@headertitle}{Projektaufgabe A\theTaskNumber}
    \newcommand{\theName}{\@title}
}
\DeclareStringOption{course}
\ProcessKeyvalOptions*\relax

\IfEq{\aspdoc@course}{rasp}{\IfSubStr{\jobname}{\detokenize{ASP}}{\def\aspdoc@course{asp}}{\def\aspdoc@course{erap}}}{}
\IfEqCase{\aspdoc@course}{%
    {asp}{\newif\ifasp\asptrue%
        \newcommand{\@courselong}{\vspace{-.2em}Aspekte der systemnahen Programmierung\\bei der Spieleentwicklung}%
        \newcommand{\@courseshort}{Praktikum ASP}
        \newcommand{\raspi}{\textit{Raspberry Pi 3}\,}
        \newcommand{\raspis}{\textit{Raspberry Pi 3}\,}
        \newcommand{\board}{\raspi}
        \newcommand{\boards}{\raspis}}%
    {erap}{\newif\ifasp\aspfalse%
        \newcommand{\@courselong}{Praktikum Rechnerarchitektur}%
        \newcommand{\@courseshort}{Praktikum ERA}}%
    {erat}{\newif\ifasp\aspfalse%
        \newcommand{\@courselong}{Übungen zu Einführung in die Rechnerarchitektur}%
        \newcommand{\@courseshort}{Übung ERA}}%
}[
    \newcommand{\@courselong}{\textit{Lehrveranstaltung}}
    \newcommand{\@courseshort}{\textit{Veranstaltung LV}}
]

% Solution environment for task sheets
\newenvironment{solution}{%
  \if@solution\itshape\bfseries\else\setbox\z@\vbox\bgroup\fi%
}{%
  \if@solution\else\egroup\fi%
}

\pagestyle{scrheadings}
\clearscrheadfoot
\ihead{\rmfamily\normalcolor \@courseshort{} -- \@headertitle\headmark}
\if@solution\chead{\textbf{Lösung}}\fi
\ohead{\pagemark}


% Add json
% from https://tex.stackexchange.com/a/433961
\usepackage{xcolor}

\definecolor{eclipseStrings}{RGB}{42,0.0,255}
\definecolor{eclipseKeywords}{RGB}{127,0,85}
\colorlet{numb}{magenta!60!black}
\lstdefinelanguage{json}{
    basicstyle=\normalfont\ttfamily,
    commentstyle=\color{eclipseStrings}, % style of comment
    stringstyle=\color{eclipseKeywords}, % style of strings
    string=[s]{"}{"},
    comment=[l]{:\ "},
    morecomment=[l]{:"},
    literate=
        *{0}{{{\color{numb}0}}}{1}
         {1}{{{\color{numb}1}}}{1}
         {2}{{{\color{numb}2}}}{1}
         {3}{{{\color{numb}3}}}{1}
         {4}{{{\color{numb}4}}}{1}
         {5}{{{\color{numb}5}}}{1}
         {6}{{{\color{numb}6}}}{1}
         {7}{{{\color{numb}7}}}{1}
         {8}{{{\color{numb}8}}}{1}
         {9}{{{\color{numb}9}}}{1}
}

\lstset{ %
  language=C++,
  basicstyle=\upshape\footnotesize\ttfamily,
  numbers=left,                   % where to put the line-numbers
  numberstyle=\tiny\color{gray},  % the style that is used for the line-numbers
  numbersep=5pt,                  % how far the line-numbers are from the code
  showstringspaces=false,
  frame=single,                   % adds a frame around the code
  rulecolor=\color{black},
  tabsize=2,                      % sets default tabsize to 2 spaces
  captionpos=b,                   % sets the caption-position to bottom
  breaklines=true,                % sets automatic line breaking
  breakatwhitespace=false,        % sets if automatic breaks should only happen at whitespace
  title=\lstname,                 % show the filename of files included with \lstinputlisting;
  belowskip=-0.8 \baselineskip,   % avoid blank line after listing
                                  % also try caption instead of title
  keywordstyle=\color{blue},      % keyword style
  commentstyle=\color{dkgreen},   % comment style
  stringstyle=\color{mauve},      % string literal style
  escapeinside={\%*}{*)},         % if you want to add LaTeX within your code
  literate={ö}{{\"o}}1
           {ä}{{\"a}}1
           {ü}{{\"u}}1
           {~}{{\tttilde}}1
           {:)}{{\rotatebox{270}{\hspace{-.5em}\rmfamily :)}}}2
}
\renewcommand{\lstlistingname}{Quelltext}% Listing -> Quelltext

% Clear author macro so that we can check whether an author is set
\def\@author{}
\renewcommand{\maketitle}{
    \begin{center}
    \small\textsc{Lehrstuhl für Rechnerarchitektur und Parallele Systeme}\\
    {\Large\bfseries\@courselong}\\[.7em]
    \begin{tabular}{c}\@maintitle\end{tabular}
    \ifdefempty{\@author}{}{\\[1em]\begin{tabular}{c}\@author\end{tabular}}
    \end{center}
}

\endinput%
