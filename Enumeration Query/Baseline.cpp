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

bool dfs(int node,int state,int dst){
    if(TC.getTimerSecond() > 600 || resultNumber > 1000){
        return false;
    }
    // if(TC.getTimerSecond() > 60){
    //     return false;
    // }
    passNode.insert(node);
    if(!dfa.labelToState[state].empty()){
        for(auto it = graph[node].begin();it != graph[node].end();it++){
            resultsNode.push_back(node);
            resultsState.push_back(state);
            if(dfa.labelToState[state].count(it->label)){
                int nextnode = it->toNode;
                int nextstate = dfa.labelToState[state][it->label];
                resultsLabel.push_back(it->label);
                if(nextnode == dst && dfa.endStates.count(nextstate)){
                    //find a path from src to dst;
                    //cout << "find path!"<<endl;
                    resultNumber++;
                    // string path = "";
                    // for(int i = 0;i < resultsNode.size();i++){
                    //     path = path + to_string(resultsNode[i]) + "---<" + to_string(resultsLabel[i]) + ">---";
                    // }
                    // path = path + to_string(dst);
                    // result.push_back(path);    
                }
                if(nextnode != dst && !passNode.count(nextnode)){
                    //cout << "node : " << node  << "  next node: " << nextnode << endl; 
                    dfs(nextnode,nextstate,dst);
                }
                resultsLabel.pop_back();
            }
            resultsNode.pop_back();
            resultsState.pop_back();
        }
    }
    passNode.erase(node); 
    return true;
}

void init(){
    resultNumber = 0;
    resultsNode.clear();
    resultsLabel.clear();
    passNode.clear();
}

bool Baseline(int src,int dst,string regex){
    RegexToDFA d;
    d.update(regex);
    dfa = d.dfa;
    cout << "finish dfa!" << endl;
    dfs(src,dfa.startState,dst);
}

int main(int argc, char* argv[]){


    // string edgeFile = "/home/QiLiang/RSPQ/Datasets/SO/stackoverflow";
    // string queryFile = edgeFile + "_Type_true.query";
    // string resultFile = edgeFile + "_Type_Baseline_number.result";


    string edgeFile = argv[1];
    string queryFile = argv[2];
    string resultFile = argv[3];
    //string memoryfile = argv[4];

    ReadData(edgeFile);//read data;
    cout<<"node number: " << numNode << endl;
    cout<<"edge number: " << numEdge << endl;
    cout<<"label number: " << labelNum << endl;

    ofstream s;
    s.open(resultFile.c_str(),ios::out);

    ifstream query;
    query.open(queryFile.c_str());
    string line;
    vector<string> temp;
    int i = 0;//queryNumber;
    while(getline(query,line)){
        temp = Split(line,' ');
        long long src = stoll(temp[0]);
        long long dst = stoll(temp[1]);
        string regex = temp[2];
        cout << src << " " << dst << " " << regex << endl;
        i++;
        init();
        TC.update();
        Baseline(src,dst,regex);
        double time = TC.getTimerMilliSec();
        cout << i << " " << resultNumber << " " << time << endl;
        s << i << " " << resultNumber << " " << time << endl;
    }
    query.close();
    s.close();
    return 0;
}

