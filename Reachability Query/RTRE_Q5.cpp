#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "RegexToDFA.h"
#include "Struct_type.h"
#include "TimerClock.h"

using namespace std;

FinalDFA dfa;
TimerClock TC;
int numNode = 0;
int numEdge = 0;
int labelNum = 0;
int resultNumber = 0;
int max_depth = 0;
int search_count = 0;
vector<vector<edge>> graph;
vector<vector<edge>> Back_graph;
vector<int> resultsNode,resultsLabel,resultsState;
vector<string> result;
unordered_set<int> passNode;

vector<string> Split(const string& s, const char delim = '\t') {
	vector<string> elements;
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elements.push_back(item);
	}
	return elements;
}

void ReadData(string edgedata){
    ifstream infile(edgedata,ios::in);
    string line;
    getline(infile,line);
    istringstream ss(line);

    ss >> numNode;
    ss >> labelNum;
    graph.resize(numNode + 1);
    Back_graph.resize(numNode + 1);

    while(getline(infile, line)){
        istringstream ss(line);
        int src,dst,label;
        ss >> src;
        ss >> dst;
        ss >> label;
        if(src != dst){
            edge temp;
            temp.toNode = dst;
            temp.label = label;
            graph[src].push_back(temp); 
            edge temp1;
            temp1.toNode = src;
            temp1.label = label;
            Back_graph[dst].push_back(temp1);
        }
    }
}

void dfs(int node,int state,int dst){

    if(TC.getTimerSecond() > 30){
        return;
    }
    if(resultNumber > 0){
        return;
    }
    passNode.insert(node);
    if(!dfa.labelToState[state].empty()){
        for(auto it = graph[node].begin();it != graph[node].end();it++){
            search_count++;
            if(search_count > max_depth){
                max_depth = search_count;
            }
            if(dfa.labelToState[state].count(it->label)){
                int nextnode = it->toNode;
                int nextstate = dfa.labelToState[state][it->label];
                
                if(nextnode == dst && dfa.endStates.count(nextstate)){
                    resultNumber++; 
                }
                if(nextnode != dst && !passNode.count(nextnode)){
                    // cout << "node : " << node  << "  next node: " << nextnode << endl; 
                    dfs(nextnode,nextstate,dst);
                }
            }
            search_count--;
        }
    }
    passNode.erase(node);
}

void init(){
    resultNumber = 0;
    search_count = 0;
    max_depth = 0;
    resultsNode.clear();
    resultsLabel.clear();
    passNode.clear();
}

int Baseline(int src,int dst,string regex){
    RegexToDFA d;
    d.update(regex);
    dfa = d.dfa;
    // cout << "finish dfa!" << endl;
    dfs(src,dfa.startState,dst);
    return resultNumber;

}

void RunBBFS(string queryfile,string resultfile,string memoryfile){
    ifstream query;
    query.open(queryfile.c_str());
    string line;
    vector<string> temp;
    int i = 0;
    ofstream s;
    s.open(resultfile.c_str(),ios::out);
    s.setf(ios::fixed, ios::floatfield);  // 设定为 fixed 模式，以小数点表示浮点数
    s.precision(4);  // 设置精度 4

    ofstream s_m;
    s_m.open(memoryfile.c_str(),ios::out);
    s_m.setf(ios::fixed, ios::floatfield);  // 设定为 fixed 模式，以小数点表示浮点数
    s_m.precision(4);  // 设置精度 4


    while(getline(query,line)){
        temp = Split(line,' ');
        long long src = stoll(temp[0]);
        long long dst = stoll(temp[1]);
        string regex = temp[2];
        i++;
        TC.update();
        init();
        int result = Baseline(src,dst,regex);
        double time = TC.getTimerMicroSec();
        // result_record.push_back(result);
        // result_time.push_back(time);
        double memory = (max_depth * 4);
        s <<  i << " " << result << " " << time  << endl;
        s_m << i << " " << memory << endl;

    }
    query.close();
    s.close();
    s_m.close();
}

int main(int argc, char* argv[]){

    // string edgeFile = "/home/liangqi/Projects/QiLiang/RSPQ/Datasets/AD/advogato";
    // string queryFile = edgeFile + "_Type_true.query";
    // string resultFile = edgeFile + "_Type_Baseline_number.result";

    string edgeFile = argv[1];

    // string queryFile = edgeFile + "_enum.query";
    // string resultFile = edgeFile + "_enum_Base.result";

    string queryFile = argv[2];
    string resultFile = argv[3];
    string memoryfile = argv[4];

    ReadData(edgeFile);//read data;
    cout<<"node number: " << numNode << endl;
    cout<<"label number: " << labelNum << endl;

    // string queryFile = edgeFile + "_Reach_Q" + to_string(5) + "_bbfs.query";
    // string resultFile = edgeFile + "_Reach_Q" + to_string(5) + "_DFS.time.result";
    // string memoryFile = edgeFile + "_Reach_Q" + to_string(5) + "_DFS.memory.result";

    // string queryFile = edgeFile + "_bbfs_Q" + to_string(5) + "_true.query";
    // string resultFile = edgeFile + "_bbfs_Q" + to_string(5) + "_DFS.time.result";
    // string memoryFile = edgeFile + "_bbfs_Q" + to_string(5) + "_DFS.memory.result";

    // RunBBFS(queryFile,resultFile,memoryFile);

    // string queryFile = edgeFile + "_Reach_Q" + to_string(5) + "_bbfs.query";
    // string resultFile = edgeFile + "_Reach_Q" + to_string(5) + "_DFS.time.result";
    // string memoryFile = edgeFile + "_Reach_Q" + to_string(5) + "_DFS.memory.result";

    RunBBFS(queryFile,resultFile,memoryFile);


    return 0;
}

