# Regular Simple Path Queries under Transitive Restricted Expressions

## Introduction
There are two fundamental problems in regular simple path queries (RSPQs). One is the reachability problem which asks whether there exists a simple path between the source and the target vertex matching the given regular expression, and the other is the enumeration problem which aims to find all the matched simple paths. As an important computing component of graph databases, RSPQs are supported in many graph database query languages such as PGQL and openCypher. However, answering RSPQs is known to be NP-hard, making it challenging to design scalable solutions to support a wide range of expressions. In this paper, we first introduce the class of transitive restricted expression, which covers more than 99\% of real-world queries. Then, we propose an efficient algorithm framework to support both reachability and enumeration problems under transitive restricted expression constraints. To boost the performance, we develop novel techniques for reachability detection, the search of candidate vertices, and the reduction of redundant path computation. Extensive experiments demonstrate that our exact method can achieve comparable efficiency to the state-of-the-art approximate approach, and outperforms the state-of-the-art exact methods by up to 2 orders of magnitude.

## Data Source:

Wikidata (WD) is available here: http://compact-leapfrog.tk/files/wikidata-enumerated.dat.gz

Youtube (YT) is available here: [Social Computing Data Repository at ASU - YouTube Dataset (syr.edu)](http://datasets.syr.edu/datasets/YouTube.html)

StackOverflow is availabel here: [SNAP: Network datasets: Stack Overflow temporal network (stanford.edu)](https://snap.stanford.edu/data/sx-stackoverflow.html)

StringsFC is available here: [Downloads - STRING functional protein association networks (string-db.org)](https://cn.string-db.org/cgi/download?sessionId=bWejWJrlm8uz&species_text=felis+catus)

StringsHS is available here: [Downloads - STRING functional protein association networks (string-db.org)](https://cn.string-db.org/cgi/download?sessionId=bWejWJrlm8uz&species_text=homo+sapiens)

NotreDame is available here: [SNAP: Network datasets: Notre Dame web graph (stanford.edu)](https://snap.stanford.edu/data/web-NotreDame.html)

Stanford is available here: [SNAP: Network datasets: Stanford web graph](https://snap.stanford.edu/data/web-Stanford.html)

Google is available here: [SNAP: Network datasets: Google web graph (stanford.edu)](https://snap.stanford.edu/data/web-Google.html)

Friendster is available here: [SNAP: Network datasets: Friendster social network (stanford.edu)](https://snap.stanford.edu/data/com-Friendster.html)

Rec-dating is available here: [dating | Recommendation Networks | Network Data Repository (networkrepository.com)](https://networkrepository.com/rec-dating.php)

Zhishihudong is available here: [zhishi-hudong-relatedpages | Miscellaneous Networks | Network Data Repository (networkrepository.com)](https://networkrepository.com/zhishi-hudong-relatedpages.php)

The graph file need to follow this rule:

1. The first line is two integers: <nodeNum, labelNum>
2. The next EdgeNum lines' format: <sourceID, targetID, labelID>
3. Vertex ID is [1, N]
4. Label ID is [0, labelNum)

Some example graphs (such as Advogato) are in the "Datasets" folder. 

## Code:

use the command “ulimit -s 102400” to increase the space of the stack.

### Reachability Query:

Our methods are shown in the Reachability Query. 

ARRIVAL and BBFS can be found at https://github.com/idea-iitd/ARRIVAL.

### Enumeration Query:

All the codes are shown in Enumeration Query.

### Run our data:

To compile: g++ -std=c++11 -O3 {name}.cpp

To run: ./a.out {path/to/dataset/edges.txt} {path/to/dataset/query.txt} {path/to/dataset/result.txt} {path/to/dataset/memory.txt}

You can change the main function to adjust to different input and output. You can use different code to answer different type of regular path queries (e.g., RTRE_Q14 for Query type 1 and type 4. We omit ETRE_Q5 since it is same as Baseline). We also show an example of our query file in folder "Example". 

The dataset folder should contain:

Edge file (edges.txt) (each line of the form u v l, where there is an edge from u to v with a label l)

Query file (the source vertex, the target vertex, the regular expression)

Result file (record the final result, such as reachability information and enumeration results).

Memory file (record the memory requirment)

