# Regular Simple Path Queries under Transitive Restricted Expressions

## Introduction
There are two fundamental problems in regular simple path queries (RSPQs). One is the reachability problem which asks whether there exists a simple path between the source and the target vertex matching the given regular expression, and the other is the enumeration problem which aims to find all the matched simple paths. As an important computing component of graph databases, RSPQs are supported in many graph database query languages such as PGQL and openCypher. However, answering RSPQs is known to be NP-hard, making it challenging to design scalable solutions to support a wide range of expressions. In this paper, we first introduce the class of \textit{transitive restricted expression}, which covers more than 99\% of real-world queries. Then, we propose an efficient algorithm framework to support both reachability and enumeration problems under transitive restricted expression constraints. To boost the performance, we develop novel techniques for reachability detection, the search of candidate vertices, and the reduction of redundant path computation. Extensive experiments demonstrate that our exact method can achieve comparable efficiency to the state-of-the-art approximate approach, and outperforms the state-of-the-art exact methods by up to 2 orders of magnitude.

## Data Source:

The link in the paper could find all the data. The data type should be "vertex1, vertex2, label".

## Code:

use the command “ulimit -s 102400” to increase the space of the stack.

### Reachability Query:

Our methods are shown in the Reachability Query. 

P2H+ could be found in [GitHub - unswpy/Answering-BillionScale-LabelConstrained-Reachability-Queries-within-Microsecond](https://github.com/unswpy/Answering-BillionScale-LabelConstrained-Reachability-Queries-within-Microsecond)

ARRIVAL and BBFS can be found at https://github.com/idea-iitd/ARRIVAL.

### Enumeration Query:

All the codes are shown in Enumeration Query.

To compile: g++ -std=c++11 -O3 {name}.cpp

To run: ./a.out {path/to/dataset/edges.txt} {path/to/dataset/query.txt} {path/to/dataset/result.txt} 

The dataset folder should contain:

edge file (edges.txt) (each line of the form u v l, where there is an edge from u to v with a label l)

query file (the source vertex, the target vertex, the regular expression) 

result file (record the final result, such as reachability information and enumeration results).


